from models import db
from datetime import datetime

class Address(db.Model):
    __tablename__ = "addresses"

    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=True)
    line1 = db.Column(db.String(255), nullable=False)
    line2 = db.Column(db.String(255))
    city = db.Column(db.String(100), nullable=False)
    state = db.Column(db.String(50))
    postal_code = db.Column(db.String(20))
    country = db.Column(db.String(50), nullable=False)

    __table_args__ = (
        db.Index("ix_addresses_user_id", "user_id"),
    )

    def to_string(self):
        parts = [self.line1]
        if self.line2:
            parts.append(self.line2)
        line = ', '.join(parts)

        city_state_zip = ', '.join(filter(None, [self.city, self.state, self.postal_code]))
        country = self.country

        return f"{line}, {city_state_zip}, {country}"


class Order(db.Model):
    __tablename__ = 'orders'

    id = db.Column(db.Integer, primary_key = True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'))
    customer_email = db.Column(db.String(255), nullable=True)
    customer_name = db.Column(db.String(255), nullable=True)

    stripe_session_id = db.Column(db.String(255), nullable=True)
    stripe_payment_intent = db.Column(db.String(255), nullable=True)

    total_amount = db.Column(db.Integer)
    currency = db.Column(db.String(10), default='usd')
    status = db.Column(db.String(50), default='pending')  # pending, completed, failed, refunded
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    delivery_address_id = db.Column(db.Integer, db.ForeignKey('addresses.id'))
    billing_address_id = db.Column(db.Integer, db.ForeignKey('addresses.id'))

    items = db.relationship('OrderItem', backref='order', lazy=True)
    user = db.relationship('User', backref='orders')
    delivery_address = db.relationship('Address', foreign_keys=[delivery_address_id])
    billing_address = db.relationship('Address', foreign_keys=[billing_address_id])

    __table_args__ = (
        db.Index("ix_orders_user_id", "user_id"),
        db.Index("ix_orders_customer_email", "customer_email"),
        db.Index("ix_orders_status", "status"),
        db.Index("ix_orders_created_at", "created_at"),
        db.Index("ix_orders_stripe_session_id", "stripe_session_id"),
        db.Index("ix_orders_stripe_payment_intent", "stripe_payment_intent"),
    )

    def total_price(self):
        if self.total_amount is not None:
            return self.total_amount / 100

        total = 0
        for item in self.items:
            total += item.total_price()

        return total
    
    def get_shipping_address(self):
        if self.delivery_address:
            return self.delivery_address.to_string()
        
        return "No shipping address provided."
    
    def get_billing_address(self):
        if self.billing_address:
            return self.billing_address.to_string()
        
        return "No billing address provided."


class OrderItem(db.Model):
    __tablename__ = 'order_items'

    id = db.Column(db.Integer, primary_key = True)
    order_id = db.Column(db.Integer, db.ForeignKey('orders.id'), nullable=False)
    product_id = db.Column(db.Integer, db.ForeignKey('products.id'), nullable=False)
    quantity = db.Column(db.Integer, default=1, nullable=False)
    unit_price_cents = db.Column(db.Integer, nullable=True)
    product_name_snapshot = db.Column(db.String(120), nullable=True)

    product = db.relationship('Product', backref='order_items')

    __table_args__ = (
        db.UniqueConstraint('order_id', 'product_id', name='uq_order_items_order_product'),
        db.Index("ix_order_items_order_id", "order_id"),
        db.Index("ix_order_items_product_id", "product_id"),
    )
    
    def total_price(self):
        if self.unit_price_cents is not None:
            return self.quantity * self.unit_price_cents / 100

        return self.quantity * self.product.current_price()

    def unit_price(self):
        if self.unit_price_cents is not None:
            return self.unit_price_cents / 100
        return self.product.current_price()
