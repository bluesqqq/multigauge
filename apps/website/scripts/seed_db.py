"""
seed_db.py - Seeds the database with test data
"""

import os
from datetime import datetime, timedelta

from app import app
from models import bcrypt, db, Product, User

with app.app_context():
    if not User.query.filter_by(username="bluesq").first():
        admin_user = User(
            username="bluesq",
            email=os.environ["ADMIN_EMAIL"],
            password=bcrypt.generate_password_hash(os.environ["ADMIN_PASSWORD"]),
            role="admin",
        )
        db.session.add(admin_user)

    if not Product.query.filter_by(slug="test-product").first():
        db.session.add(
            Product(
                name="Test Product",
                slug="test-product",
                stock=25,
                low_stock_threshold=50,
                description="This is a test product.",
                price=10,
                custom_template=None,
            )
        )

    if not Product.query.filter_by(slug="gauge").first():
        db.session.add(
            Product(
                name="Gauge",
                slug="gauge",
                stock=3,
                description="This is a test product.",
                price=70,
                discount_price=59.97,
                discount_expires_at=datetime.utcnow() + timedelta(days=30),
                custom_template=None,
                image_url="/static/images/core/gauge.jpg",
            )
        )

    if not Product.query.filter_by(slug="wire").first():
        db.session.add(
            Product(
                name="Wire",
                slug="wire",
                stock=105,
                description="This is a test product.",
                price=7,
                discount_price=5.97,
                custom_template=None,
                image_url="/static/images/wire.jpg",
            )
        )

    db.session.commit()

print("Development seed data inserted.")