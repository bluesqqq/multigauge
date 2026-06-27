from models import db
from datetime import datetime

class Product(db.Model):
    __tablename__ = 'products'

    id = db.Column(db.Integer, primary_key = True)
    name = db.Column(db.String(120), nullable = False)
    description = db.Column(db.Text, nullable = True)

    price = db.Column(db.Numeric(10, 2), nullable = False)
    discount_price = db.Column(db.Numeric(10, 2), nullable=True)
    discount_expires_at = db.Column(db.DateTime, nullable=True)

    stock = db.Column(db.Integer, default = 0, nullable = False)
    low_stock_threshold = db.Column(db.Integer, default=10, nullable=False)

    slug            = db.Column(db.String(100), unique=True)
    custom_template = db.Column(db.String(100), nullable=True)

    image_url = db.Column(db.String(255), default="/static/images/placeholder.jpg", nullable = True)

    def is_discounted(self):
        return (self.discount_price is not None and (self.discount_expires_at is None or self.discount_expires_at > datetime.utcnow()))
    
    def current_price(self):
        if self.is_discounted():
            return self.discount_price
        
        return self.price

    def set_discount_price(self, amount):
        if amount >= self.price:
            raise ValueError("Discount price must be less than the original price.")
        self.discount_price = amount

    def set_discount_percent(self, percent):
        if not (0 <= percent <= 100):
            raise ValueError("Discount percent must be between 0 and 100.")
        discount = self.price * (percent / 100)
        self.discount_price = round(self.price - discount, 2)

    def get_discount_percent(self):
        if self.discount_price is None:
            return None
        return round(100 * (1 - float(self.discount_price) / float(self.price)))
    
    def is_low_stock(self):
        return self.stock <= self.low_stock_threshold