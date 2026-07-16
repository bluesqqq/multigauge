import uuid

import stripe
from flask import Blueprint, flash, redirect, render_template, request, session as flask_session, url_for
from flask_login import current_user

from models import Address, Cart, CartItem, Order, OrderItem, db

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

    if 'guest_cart_id' not in flask_session:
        flask_session['guest_cart_id'] = str(uuid.uuid4())

    sid = flask_session['guest_cart_id']
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

    total_cents = cart.get_subtotal_cents()
    order = Order(
        user_id=current_user.id if current_user.is_authenticated else None,
        customer_email=current_user.email if current_user.is_authenticated else None,
        customer_name=current_user.username if current_user.is_authenticated else None,
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
            quantity=item.quantity,
            unit_price_cents=item.unit_price_cents or int(item.product.current_price() * 100),
            product_name_snapshot=item.product.name,
        ))

    line_items = []
    for item in cart.items:
        product = item.product
        unit_price = item.unit_price_cents or int(product.current_price() * 100)
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
        domain = YOUR_DOMAIN
        stripe_session = stripe.checkout.Session.create(
            payment_method_types=['card'],
            line_items=line_items,
            mode='payment',
            client_reference_id=str(order.id),
            success_url=domain + '/success?session_id={CHECKOUT_SESSION_ID}',
            cancel_url=domain + '/cancel',
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
        order.stripe_session_id = stripe_session.id
        db.session.commit()
        return redirect(stripe_session.url, code=303)
    except Exception as e:
        db.session.rollback()
        return str(e), 400


@payment_bp.route("/success")
def success():
    session_id = request.args.get('session_id')
    if not session_id:
        flash("Missing session ID.")
        return redirect(url_for('main.home'))

    stripe_session = stripe.checkout.Session.retrieve(session_id, expand=['payment_intent'])
    payment_intent = stripe_session.payment_intent

    order_id = stripe_session.client_reference_id
    order = Order.query.get(order_id)

    if not order:
        flash("Order not found.")
        return redirect(url_for('main.home'))

    order.stripe_session_id = stripe_session.id
    order.stripe_payment_intent = payment_intent.id
    order.status = 'paid'

    customer_details = getattr(stripe_session, "customer_details", None)
    if customer_details:
        order.customer_email = getattr(customer_details, "email", None) or order.customer_email
        order.customer_name = getattr(customer_details, "name", None) or order.customer_name

    shipping_details = getattr(stripe_session, "shipping_details", None)
    if shipping_details and getattr(shipping_details, "address", None):
        address = shipping_details.address
        delivery = Address(
            user_id=order.user_id,
            line1=address.line1,
            line2=address.line2,
            city=address.city,
            state=address.state,
            postal_code=address.postal_code,
            country=address.country,
        )
        db.session.add(delivery)
        db.session.flush()
        order.delivery_address_id = delivery.id

    billing_address = getattr(customer_details, "address", None) if customer_details else None
    if billing_address:
        billing = Address(
            user_id=order.user_id,
            line1=billing_address.line1,
            line2=billing_address.line2,
            city=billing_address.city,
            state=billing_address.state,
            postal_code=billing_address.postal_code,
            country=billing_address.country,
        )
        db.session.add(billing)
        db.session.flush()
        order.billing_address_id = billing.id

    cart = None
    if order.user_id:
        cart = Cart.query.filter_by(user_id=order.user_id).first()
    else:
        guest_sid = flask_session.get('guest_cart_id')
        if guest_sid:
            cart = Cart.query.filter_by(session_id=guest_sid).first()

    if cart:
        cart.clear_cart()

    db.session.commit()
    flask_session.pop('guest_cart_id', None)

    return render_template("success.html", order_id=order_id)
