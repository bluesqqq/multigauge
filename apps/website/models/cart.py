from datetime import datetime

from models import db

class Cart(db.Model):
    __tablename__ = 'carts'

    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable = True)
    session_id = db.Column(db.String(128), nullable=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow, nullable=False)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow, nullable=False)

    user = db.relationship('User', backref='cart')
    items = db.relationship('CartItem', backref='cart', cascade="all, delete-orphan")

    __table_args__ = (
        db.UniqueConstraint('user_id', name='uq_carts_user_id'),
        db.UniqueConstraint('session_id', name='uq_carts_session_id'),
        db.CheckConstraint('(user_id IS NOT NULL) OR (session_id IS NOT NULL)', name='ck_carts_has_identity'),
        db.Index('ix_carts_user_id', 'user_id'),
        db.Index('ix_carts_session_id', 'session_id'),
        db.Index('ix_carts_updated_at', 'updated_at'),
    )

    def get_subtotal(self):
        return sum(item.quantity * (item.product.current_price()) for item in self.items)
    
    def get_subtotal_cents(self):
        return int(self.get_subtotal() * 100)
    
    def clear_cart(self):
        self.items.clear()

    def stock_is_available(self):
        for item in self.items:
            if not item.stock_is_available():
                return False
        return True

class CartItem(db.Model):
    __tablename__ = 'cart_items'

    id = db.Column(db.Integer, primary_key=True)
    cart_id = db.Column(db.Integer, db.ForeignKey('carts.id'), nullable=False)
    product_id = db.Column(db.Integer, db.ForeignKey('products.id'), nullable=False)
    quantity = db.Column(db.Integer, default=1, nullable=False)
    unit_price_cents = db.Column(db.Integer, nullable=True)

    product = db.relationship('Product')

    __table_args__ = (
        db.UniqueConstraint('cart_id', 'product_id', name='uq_cart_items_cart_product'),
        db.CheckConstraint('quantity > 0', name='ck_cart_items_quantity_positive'),
        db.Index('ix_cart_items_cart_id', 'cart_id'),
        db.Index('ix_cart_items_product_id', 'product_id'),
    )

    def stock_is_available(self):
        return (self.product.stock >= self.quantity)
    
    def increment(self, amount = 1):
        if self.quantity + amount <= self.product.stock:
            self.quantity += amount

    def decrement(self, amount = 1):
        if self.quantity > amount:
            self.quantity -= amount
        else:
            self.quantity = 0
