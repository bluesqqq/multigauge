from flask import Blueprint, session, flash, redirect, url_for, request, render_template

from flask_login import current_user
from models import db, Order, OrderItem, Address, Cart

import uuid
import stripe

payment_bp = Blueprint('payment', __name__)

YOUR_DOMAIN = 'http://localhost:5000'  # change to your domain when live

def get_or_create_cart(user):
    if user.is_authenticated:
        cart = Cart.query.filter_by(user_id=user.id).first()
        if not cart:
            cart = Cart(user_id=user.id)
            db.session.add(cart)
            db.session.commit()
        return cart
    else: # Guest user
        if 'guest_cart_id' not in session:
            session['guest_cart_id'] = str(uuid.uuid4())
        sid = session['guest_cart_id']
        cart = Cart.query.filter_by(session_id=sid).first()
        if not cart:
            cart = Cart(session_id=sid)
            db.session.add(cart)
            db.session.commit()
        return cart

@payment_bp.route('/cancel')
def cancel():
    return 'Payment canceled.'

@payment_bp.route('/create-checkout-session', methods=['POST'])
def create_checkout_session():
    cart = get_or_create_cart(current_user)

    if not cart or not cart.items:
        flash("Your cart is empty!")
        return redirect(url_for('cart.cart'))
    
    # Create pending order from cart
    total_cents = cart.get_subtotal_cents()
    order = Order(
        user_id=current_user.id if current_user.is_authenticated else None,
        total_amount=total_cents,
        status='pending',
        currency='usd',
    )
    db.session.add(order)
    db.session.flush()

    for item in cart.items:
        db.session.add(OrderItem(
            order_id=order.id,
            product_id=item.product.id,
            quantity=item.quantity
        ))

    db.session.commit()

    # Create line items for Stripe
    line_items = []
    for item in cart.items:
        product = item.product
        unit_price = int(product.current_price() * 100)

        line_items.append({
            'price_data': {
                'currency': 'usd',
                'product_data': {
                    'name': product.name,
                },
                'unit_amount': unit_price,
            },
            'quantity': item.quantity,
        })

    try:
        DOMAIN = 'http://127.0.0.1:5000'

        session = stripe.checkout.Session.create(
            payment_method_types=['card'],
            line_items=line_items,
            mode='payment',
            client_reference_id=str(order.id),
            success_url=DOMAIN + '/success?session_id={CHECKOUT_SESSION_ID}',
            cancel_url=DOMAIN + '/cancel',

            shipping_address_collection={
                'allowed_countries': ['US', 'CA']
            },
            shipping_options=[
                {
                    "shipping_rate_data": {
                        "type": "fixed_amount",
                        "fixed_amount": {
                            "amount": 500,
                            "currency": "usd",
                        },
                        "display_name": "Standard shipping",
                        "delivery_estimate": {
                            "minimum": {"unit": "business_day", "value": 5},
                            "maximum": {"unit": "business_day", "value": 7},
                        },
                    }
                }
            ],
        )
        return redirect(session.url, code=303)
    except Exception as e:
        return str(e), 400
    
@payment_bp.route("/success")
def success():
    session_id = request.args.get('session_id')
    print(f"Session ID: {session_id}")

    if not session_id:
        flash("Missing session ID.")
        return redirect(url_for('main.index'))

    session = stripe.checkout.Session.retrieve(session_id, expand=['payment_intent'])
    
    payment_intent = session.payment_intent
    print("PAYMENT INTENT:", payment_intent)

    order_id = session.client_reference_id
    order = Order.query.get(order_id)

    if not order:
        flash("Order not found.")
        return redirect(url_for('main.index'))
    
    # Update Stripe info
    order.stripe_session_id = session.id
    order.stripe_payment_intent = payment_intent.id
    order.status = 'paid'

    # Store shipping address
    shipping = session.get('shipping')
    if shipping and shipping.get('address'):
        delivery = Address(
            user_id=current_user.id,
            line1=shipping["address"].get("line1"),
            line2=shipping["address"].get("line2"),
            city=shipping["address"].get("city"),
            state=shipping["address"].get("state"),
            postal_code=shipping["address"].get("postal_code"),
            country=shipping["address"].get("country"),
        )
        db.session.add(delivery)
        db.session.flush()

        order.delivery_address_id = delivery.id

    # Store billing address if available
    billing = session.get('customer_details', {}).get('address')
    if billing:
        billing_addr = Address(
            user_id=current_user.id,
            line1=billing.get('line1'),
            line2=billing.get('line2'),
            city=billing.get('city'),
            state=billing.get('state'),
            postal_code=billing.get('postal_code'),
            country=billing.get('country')
        )
        db.session.add(billing_addr)
        db.session.flush()
        order.billing_address_id = billing_addr.id

    db.session.commit()

    return render_template("success.html", order_id=order_id)