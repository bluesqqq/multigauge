from flask import Blueprint, redirect, render_template, url_for
from flask_login import current_user
from models import db, Post, User

main_bp = Blueprint('main', __name__)

# Home route
@main_bp.route("/")
def home():
    query = Post.query.order_by(Post.posted_at.desc())

    pagination = query.paginate(page=1, per_page=4, error_out=False)
    posts = pagination.items

    # For each post, add the like count
    for post in posts:
        user = User.query.get(int(post.posted_by))
        post.user_username = user.username
        post.liked = False
        post.favorited = False
        if current_user.is_authenticated:
            post.liked = current_user.liked_post(post)
            post.favorited = current_user.favorited_post(post)

    return render_template('index.html', posts=posts)

# Editor route
@main_bp.route("/editor")
def editor():
    return render_template("editor.html")


@main_bp.route("/new-editor")
def editor_redirect():
    return redirect(url_for("main.editor"))

