from datetime import datetime

from models import db


class UserProfile(db.Model):
    __tablename__ = "user_profiles"

    user_id = db.Column(db.Integer, db.ForeignKey("users.id"), primary_key=True)
    display_name = db.Column(db.String(150), nullable=True)
    bio = db.Column(db.Text, nullable=True)
    avatar_url = db.Column(db.String(255), nullable=True)
    accent_color = db.Column(db.String(32), nullable=False, default="#ed1c24")

    email_notifications = db.Column(db.Boolean, nullable=False, default=True)
    order_updates = db.Column(db.Boolean, nullable=False, default=True)
    community_digest = db.Column(db.Boolean, nullable=False, default=False)
    product_announcements = db.Column(db.Boolean, nullable=False, default=True)

    email_verified = db.Column(db.Boolean, nullable=False, default=False)
    verified_at = db.Column(db.DateTime, nullable=True)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow, nullable=False)

    user = db.relationship("User", backref=db.backref("profile", uselist=False, cascade="all, delete-orphan"))

    __table_args__ = (
        db.Index("ix_user_profiles_email_verified", "email_verified"),
    )


def ensure_user_profile(user):
    profile = UserProfile.query.get(user.id)
    if profile is None:
        profile = UserProfile(
            user_id=user.id,
            display_name=user.username,
            accent_color="#ed1c24",
        )
        db.session.add(profile)
        db.session.commit()
    return profile
