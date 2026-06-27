from flask import Blueprint, request, redirect, url_for, render_template, flash, session, current_app
from flask_login import user_logged_in, current_user

from models import db, Cart, CartItem, Product

import uuid

cart_bp = Blueprint('cart', __name__)

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

@cart_bp.route("/cart")
def cart():
    session_id = session.get('guest_cart_id')

    cart = Cart.query.filter_by(session_id = session_id).first()

    if not cart:
        cart = None

    return render_template('cart.html', cart=cart)

@cart_bp.route('/cart/update/<int:item_id>', methods=['POST'])
def update_cart_quantity(item_id):
    item = CartItem.query.get_or_404(item_id)
    action = request.form.get('action')

    if action == 'increment':
        item.increment()
    elif action == 'decrement':
        item.decrement()
        if item.quantity == 0:
            db.session.delete(item)
    elif action == 'remove':
        db.session.delete(item)
    
    db.session.commit()
    return redirect(url_for('cart.cart'))

@cart_bp.route('/add-to-cart/<int:product_id>', methods=['POST'])
def add_to_cart(product_id):
    product = Product.query.get_or_404(product_id)
    quantity = int(request.form.get('quantity', 1))

    cart = get_or_create_cart(current_user)
    existing_item = CartItem.query.filter_by(cart_id=cart.id, product_id=product.id).first()
    if existing_item:
        existing_item.increment()
    else:
        new_item = CartItem(cart_id=cart.id, product_id=product.id, quantity=quantity)
        db.session.add(new_item)

    db.session.commit()
    flash('Item added to cart!')
    return redirect(url_for('cart.cart'))

@user_logged_in.connect
def merge_guest_cart(app, user):
    sid = session.get('guest_cart_id')
    if sid:
        guest_cart = Cart.query.filter_by(session_id=sid).first()
        user_cart = Cart.query.filter_by(user_id=user.id).first()
        
        if guest_cart:
            if not user_cart:
                guest_cart.user_id = user.id
                guest_cart.session_id = None
            else:
                # Merge items
                for item in guest_cart.items:
                    existing = CartItem.query.filter_by(cart_id=user_cart.id, product_id=item.product_id).first()
                    if existing:
                        existing.quantity += item.quantity
                        db.session.delete(item)
                    else:
                        item.cart_id = user_cart.id
                db.session.delete(guest_cart)
        db.session.commit()
        session.pop('guest_cart_id', None)
