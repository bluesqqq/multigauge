from flask_sqlalchemy import SQLAlchemy
from flask_bcrypt import Bcrypt

db = SQLAlchemy()
bcrypt = Bcrypt()

from .post import Post, PostLike, PostComment, PostFavorite, PostFeature
from .user import User
from .cart import Cart, CartItem
from .subscriber import EmailSubscribers
from .product import Product
from .order import Order, OrderItem, Address