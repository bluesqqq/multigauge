from flask import Blueprint, render_template
from sqlalchemy.orm import selectinload

from models import User, Post, PostDownload
from models.profile import ensure_user_profile

users_bp = Blueprint('users', __name__)

# User Profile page route
@users_bp.route("/user/<int:user_id>")
def user(user_id):
    # Fetch the user
    user = User.query.get(int(user_id))

    if user:
        profile = ensure_user_profile(user)

        # Fetch only the first 4 posts by the user
        user_posts = (
            Post.query
            .options(
                selectinload(Post.user),
                selectinload(Post.package),
                selectinload(Post.likes),
                selectinload(Post.downloads),
            )
            .filter_by(posted_by=user_id)
            .order_by(Post.posted_at.desc())
            .limit(4)
            .all()
        )

        # Total number of posts (for display)
        post_count = Post.query.filter_by(posted_by=user_id).count()

        # Calculate total likes for those posts
        total_likes = sum(len(post.likes) for post in user_posts)

        total_downloads = (
            PostDownload.query
            .join(Post, PostDownload.post_id == Post.id)
            .filter(Post.posted_by == user_id)
            .count()
        )

        return render_template(
            'user.html',
            user_posts=user_posts,
            user=user,
            profile=profile,
            total_downloads=total_downloads,
            total_likes=total_likes,
            post_count=post_count
        )
    else:
        return "User not found!", 404
