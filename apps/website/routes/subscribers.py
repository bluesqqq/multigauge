from datetime import datetime

from flask import Blueprint, flash, redirect, render_template, request, url_for

from models import EmailSubscribers, db

subscribers_bp = Blueprint('subscribers', __name__)


@subscribers_bp.route("/subscribe", methods=["POST"])
def subscribe():
    email = request.form.get("email", "").strip().lower()
    if not email:
        flash("Please enter an email address.", "danger")
        return redirect(request.referrer or url_for("main.home"))

    subscriber = EmailSubscribers.query.filter_by(email=email).first()
    if subscriber:
        subscriber.status = "active"
        subscriber.confirmed_at = subscriber.confirmed_at or datetime.utcnow()
        subscriber.unsubscribed_at = None
        subscriber.source = subscriber.source or "website"
    else:
        subscriber = EmailSubscribers(
            email=email,
            status="active",
            source="website",
            confirmed_at=datetime.utcnow(),
        )
        db.session.add(subscriber)

    db.session.commit()
    flash("You're on the mailing list!", "success")
    return redirect(request.referrer or url_for("main.home"))
