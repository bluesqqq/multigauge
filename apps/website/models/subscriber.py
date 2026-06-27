from models import db

class EmailSubscribers(db.Model):
    __tablename__ = 'subscribers'

    id = db.Column(db.Integer, primary_key = True)
    email = db.Column(db.String(255), unique = True, nullable = False)