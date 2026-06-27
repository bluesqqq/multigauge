"""
app.py - Entry point for multigauge website

Creates the Flask app, initializes extensions, configures authentication,
registers route blueprints, and starts the local development server when run
directly.

"""

import os

import stripe
from flask import Flask
from flask_login import LoginManager
from dotenv import load_dotenv

from models import db, User
from routes import (
    auth_bp,
    cart_bp,
    main_bp,
    admin_bp,
    users_bp,
    payment_bp,
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

login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = "auth.login"


@login_manager.user_loader
def load_user(user_id):
    """Load a user by ID for Flask-Login session management."""
    return db.session.get(User, int(user_id))


app.register_blueprint(auth_bp)
app.register_blueprint(admin_bp)
app.register_blueprint(cart_bp)
app.register_blueprint(main_bp)
app.register_blueprint(multigauge_web_bp)
app.register_blueprint(payment_bp)
app.register_blueprint(products_bp)
app.register_blueprint(users_bp)
app.register_blueprint(workshop_bp)

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)