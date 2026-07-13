from pathlib import Path
from uuid import uuid4

from flask import Blueprint, flash, redirect, render_template, request, url_for
from flask_login import current_user, login_required, logout_user
from sqlalchemy.orm import selectinload
from werkzeug.utils import secure_filename

from forms import AccountSettingsForm, ChangePasswordForm, DangerActionForm
from models import (
    Cart,
    Order,
    OrderItem,
    Post,
    PostComment,
    PostDownload,
    PostFavorite,
    PostFeature,
    PostLike,
    bcrypt,
    db,
)
from models.profile import ensure_user_profile

account_bp = Blueprint("account", __name__)
AVATAR_UPLOAD_DIR = Path("static") / "uploads" / "avatars"
AVATAR_URL_PREFIX = "/static/uploads/avatars"


def _recent_posts_for_user(user_id, limit=4):
    return (
        Post.query
        .filter_by(posted_by=user_id)
        .order_by(Post.posted_at.desc())
        .limit(limit)
        .all()
    )


def _recent_orders_for_user(user_id, limit=4):
    return (
        Order.query
        .options(selectinload(Order.items).selectinload(OrderItem.product))
        .filter_by(user_id=user_id)
        .order_by(Order.created_at.desc())
        .limit(limit)
        .all()
    )


def _delete_posts_for_user(user_id):
    post_ids = [row[0] for row in db.session.query(Post.id).filter_by(posted_by=user_id).all()]
    if not post_ids:
        return 0

    PostDownload.query.filter(PostDownload.post_id.in_(post_ids)).delete(synchronize_session=False)
    PostLike.query.filter(PostLike.post_id.in_(post_ids)).delete(synchronize_session=False)
    PostFavorite.query.filter(PostFavorite.post_id.in_(post_ids)).delete(synchronize_session=False)
    PostComment.query.filter(PostComment.post_id.in_(post_ids)).delete(synchronize_session=False)
    PostFeature.query.filter(PostFeature.post_id.in_(post_ids)).delete(synchronize_session=False)

    posts = Post.query.filter(Post.id.in_(post_ids)).all()
    for post in posts:
        db.session.delete(post)

    return len(posts)


def _delete_user_activity(user_id):
    PostLike.query.filter_by(user_id=user_id).delete(synchronize_session=False)
    PostFavorite.query.filter_by(user_id=user_id).delete(synchronize_session=False)
    PostComment.query.filter_by(user_id=user_id).delete(synchronize_session=False)
    PostDownload.query.filter_by(user_id=user_id).delete(synchronize_session=False)
    PostFeature.query.filter_by(moderator_id=user_id).delete(synchronize_session=False)


def _delete_user_cart(user_id):
    cart = Cart.query.filter_by(user_id=user_id).first()
    if cart:
        db.session.delete(cart)


def _save_avatar_upload(file_storage):
    if not file_storage or not file_storage.filename:
        return None

    original_name = secure_filename(file_storage.filename)
    suffix = Path(original_name).suffix.lower()
    if suffix not in {".jpg", ".jpeg", ".png", ".gif", ".webp"}:
        return None

    AVATAR_UPLOAD_DIR.mkdir(parents=True, exist_ok=True)
    file_name = f"{uuid4().hex}{suffix}"
    file_path = AVATAR_UPLOAD_DIR / file_name
    file_storage.save(file_path)
    return f"{AVATAR_URL_PREFIX}/{file_name}"


@account_bp.route("/account")
@login_required
def dashboard():
    profile = ensure_user_profile(current_user)
    recent_posts = _recent_posts_for_user(current_user.id)
    recent_orders = _recent_orders_for_user(current_user.id)

    post_count = Post.query.filter_by(posted_by=current_user.id).count()
    order_count = Order.query.filter_by(user_id=current_user.id).count()
    download_count = (
        PostDownload.query
        .join(Post, PostDownload.post_id == Post.id)
        .filter(Post.posted_by == current_user.id)
        .count()
    )

    return render_template(
        "account.html",
        profile=profile,
        recent_posts=recent_posts,
        recent_orders=recent_orders,
        post_count=post_count,
        order_count=order_count,
        download_count=download_count,
        uploaded_content_count=post_count,
        navbar_background=True,
    )


@account_bp.route("/account/settings", methods=["GET", "POST"])
@login_required
def settings():
    profile = ensure_user_profile(current_user)
    profile_form = AccountSettingsForm(original_username=current_user.username)
    password_form = ChangePasswordForm()
    danger_form = DangerActionForm()

    if request.method == "POST":
        action = request.form.get("action")
        if action in {"delete_content", "delete_account"} and not danger_form.validate_on_submit():
            flash("Unable to verify that request.", "danger")
            return redirect(url_for("account.settings"))

        if action == "profile" and profile_form.validate_on_submit():
            current_user.username = profile_form.username.data.strip()
            profile.display_name = profile_form.display_name.data.strip() or None
            profile.bio = profile_form.bio.data.strip() or None
            avatar_upload = request.files.get("avatar_file")
            if avatar_upload and avatar_upload.filename:
                avatar_url = _save_avatar_upload(avatar_upload)
                if not avatar_url:
                    flash("Please upload a valid image file for your profile picture.", "danger")
                    return redirect(url_for("account.settings"))
                profile.avatar_url = avatar_url
            profile.accent_color = profile_form.accent_color.data
            profile.email_notifications = bool(profile_form.email_notifications.data)
            profile.order_updates = bool(profile_form.order_updates.data)
            profile.community_digest = bool(profile_form.community_digest.data)
            profile.product_announcements = bool(profile_form.product_announcements.data)
            db.session.commit()
            flash("Profile updated.", "success")
            return redirect(url_for("account.settings"))

        if action == "password" and password_form.validate_on_submit():
            if not bcrypt.check_password_hash(current_user.password, password_form.current_password.data):
                flash("Current password is incorrect.", "danger")
                return redirect(url_for("account.settings"))

            current_user.password = bcrypt.generate_password_hash(password_form.new_password.data).decode("utf-8")
            db.session.commit()
            flash("Password updated.", "success")
            return redirect(url_for("account.settings"))

        if action == "delete_content":
            deleted_posts = _delete_posts_for_user(current_user.id)
            db.session.commit()
            flash(f"Deleted {deleted_posts} uploaded post{'s' if deleted_posts != 1 else ''}.", "success")
            return redirect(url_for("account.settings"))

        if action == "delete_account":
            Order.query.filter_by(user_id=current_user.id).update({Order.user_id: None}, synchronize_session=False)
            _delete_user_activity(current_user.id)
            _delete_user_cart(current_user.id)
            _delete_posts_for_user(current_user.id)
            if current_user.profile:
                db.session.delete(current_user.profile)
            db.session.delete(current_user)
            db.session.commit()
            logout_user()
            flash("Your account has been deleted.", "success")
            return redirect(url_for("auth.login"))

    if request.method == "GET":
        profile_form.display_name.data = profile.display_name or ""
        profile_form.username.data = current_user.username
        profile_form.bio.data = profile.bio or ""
        profile_form.accent_color.data = profile.accent_color or "#ed1c24"
        profile_form.email_notifications.data = profile.email_notifications
        profile_form.order_updates.data = profile.order_updates
        profile_form.community_digest.data = profile.community_digest
        profile_form.product_announcements.data = profile.product_announcements

    return render_template(
        "account_settings.html",
        profile=profile,
        profile_form=profile_form,
        password_form=password_form,
        danger_form=danger_form,
        uploaded_content_count=Post.query.filter_by(posted_by=current_user.id).count(),
        navbar_background=True,
    )


@account_bp.route("/account/orders")
@login_required
def orders():
    orders = (
        Order.query
        .options(selectinload(Order.items).selectinload(OrderItem.product))
        .filter_by(user_id=current_user.id)
        .order_by(Order.created_at.desc())
        .all()
    )

    return render_template(
        "account_orders.html",
        orders=orders,
        navbar_background=True,
    )
