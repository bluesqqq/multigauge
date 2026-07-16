"""
app.py - Entry point for multigauge website

Creates the Flask app, initializes extensions, configures authentication,
registers route blueprints, and starts the local development server when run
directly.

"""

import os

import stripe
from flask import Flask, session
from flask_login import LoginManager, current_user
from dotenv import load_dotenv

from models import db, User, Cart
from routes import (
    auth_bp,
    cart_bp,
    downloads_bp,
    main_bp,
    admin_bp,
    account_bp,
    users_bp,
    payment_bp,
    subscribers_bp,
    workshop_bp,
    products_bp,
    multigauge_web_bp
)

load_dotenv()

app = Flask(__name__, static_folder="static", static_url_path='/static')

app.config["SQLALCHEMY_DATABASE_URI"] = os.getenv(
    "DATABASE_URL",
    "sqlite:///site.db",
)
app.config["SECRET_KEY"] = os.environ["SECRET_KEY"]

stripe.api_key = os.environ["STRIPE_KEY"]

db.init_app(app)

with app.app_context():
    db.create_all()

login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = "auth.login"


@login_manager.user_loader
def load_user(user_id):
    """Load a user by ID for Flask-Login session management."""
    return db.session.get(User, int(user_id))


@app.context_processor
def inject_cart_count():
    cart = None
    if current_user.is_authenticated:
        cart = Cart.query.filter_by(user_id=current_user.id).first()
    else:
        guest_cart_id = session.get("guest_cart_id")
        if guest_cart_id:
            cart = Cart.query.filter_by(session_id=guest_cart_id).first()

    cart_count = len(cart.items) if cart else 0
    return {"cart_count": cart_count}


app.register_blueprint(auth_bp)
app.register_blueprint(admin_bp)
app.register_blueprint(account_bp)
app.register_blueprint(cart_bp)
app.register_blueprint(downloads_bp)
app.register_blueprint(main_bp)
app.register_blueprint(multigauge_web_bp)
app.register_blueprint(payment_bp)
app.register_blueprint(products_bp)
app.register_blueprint(subscribers_bp)
app.register_blueprint(users_bp)
app.register_blueprint(workshop_bp)

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
