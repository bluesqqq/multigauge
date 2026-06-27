from datetime import datetime

from models import db

class EmailSubscribers(db.Model):
    __tablename__ = 'subscribers'

    id = db.Column(db.Integer, primary_key = True)
    email = db.Column(db.String(255), unique = True, nullable = False)
    status = db.Column(db.String(32), nullable=False, default='active')
    source = db.Column(db.String(64), nullable=True)
    confirmed_at = db.Column(db.DateTime, nullable=True)
    unsubscribed_at = db.Column(db.DateTime, nullable=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow, nullable=False)

    __table_args__ = (
        db.Index('ix_subscribers_status', 'status'),
        db.Index('ix_subscribers_created_at', 'created_at'),
    )
