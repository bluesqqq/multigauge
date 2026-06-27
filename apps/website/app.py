from flask import Flask
from flask_login import LoginManager
from models import db, bcrypt, User, Product
from datetime import datetime, timedelta

from routes import auth_bp, cart_bp, main_bp, admin_bp, users_bp, payment_bp, workshop_bp, products_bp, multigauge_web_bp

import stripe

app = Flask(__name__, static_folder="static", static_url_path='/static')

app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///site.db"
app.config["SECRET_KEY"] = "your_secret_key"  # Change this to something secure

stripe.api_key = 'oh-no-no-no'

'''###
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)
###'''

db.init_app(app)

# Set up Flask-Login
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = "auth.login"

@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))

app.register_blueprint(auth_bp)
app.register_blueprint(admin_bp)
app.register_blueprint(cart_bp)
app.register_blueprint(main_bp)
app.register_blueprint(multigauge_web_bp)
app.register_blueprint(payment_bp)
app.register_blueprint(products_bp)
app.register_blueprint(users_bp)
app.register_blueprint(workshop_bp)

# Run the application
if __name__ == "__main__":
    with app.app_context():
        db.create_all()

        if not User.query.filter_by(username="bluesq").first():
            admin_user = User(
                username = "bluesq",
                email = "test@gmail.com",
                password = bcrypt.generate_password_hash("@dm1nP@ssw0rd!-Mult1G@ug3-"),
                role = "admin"
            )
            db.session.add(admin_user)
            db.session.commit()

        # Check if the test product already exists
        if not Product.query.filter_by(slug="test-product").first():
            test_product = Product(
                name="Test Product",
                slug="test-product",
                stock=25,
                low_stock_threshold=50,
                description="This is a test product.",
                price=10,
                custom_template=None,  # or specify a template if needed
            )
            db.session.add(test_product)
            db.session.commit()

        # Check if the test product already exists
        if not Product.query.filter_by(slug="gauge").first():
            gauge_product = Product(
                name="Gauge",
                slug="gauge",
                stock=3,
                description="This is a test product.",
                price=70,
                discount_price=59.97,
                discount_expires_at=datetime.utcnow() + timedelta(days=30),
                custom_template=None,  # or specify a template if needed

                image_url="/static/images/core/gauge.jpg"
            )
            db.session.add(gauge_product)
            db.session.commit()

                    # Check if the test product already exists
        if not Product.query.filter_by(slug="wire").first():
            wire_product = Product(
                name="Wire",
                slug="wire",
                stock=105,
                description="This is a test product.",
                price=7,
                discount_price=5.97,
                custom_template=None,  # or specify a template if needed

                image_url="/static/images/wire.jpg"
            )
            db.session.add(wire_product)
            db.session.commit()

    app.run(debug=True, host='0.0.0.0', port=5000)
