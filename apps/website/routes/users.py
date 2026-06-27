from flask import Blueprint, request, render_template
from models import User, Post

users_bp = Blueprint('users', __name__)

# User Profile page route
@users_bp.route("/user/<int:user_id>")
def user(user_id):
    page = request.args.get('page', 1, type=int)

    # Fetch the user
    user = User.query.get(int(user_id))

    if user:
        # Fetch only the first 4 posts by the user
        user_posts = Post.query.filter_by(posted_by=user_id).order_by(Post.posted_at.desc()).limit(4).all()

        # Total number of posts (for display)
        post_count = Post.query.filter_by(posted_by=user_id).count()

        # Calculate total likes for those posts
        total_likes = sum(len(post.likes) for post in user_posts)

        return render_template(
            'user.html',
            user_posts=user_posts,
            user=user,
            total_likes=total_likes,
            post_count=post_count
        )
    else:
        return "User not found!", 404