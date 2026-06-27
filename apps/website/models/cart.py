from models import db

class Cart(db.Model):
    __tablename__ = 'carts'

    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable = True)
    session_id = db.Column(db.String(128), nullable=True)

    user = db.relationship('User', backref='cart')
    items = db.relationship('CartItem', backref='cart', cascade="all, delete-orphan")

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

    product = db.relationship('Product')

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