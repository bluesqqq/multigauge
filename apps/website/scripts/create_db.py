"""
create_db.py - Create database tables
"""

from app import app
from models import db

with app.app_context():
    db.create_all()