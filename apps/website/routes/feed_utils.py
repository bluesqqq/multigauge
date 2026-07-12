from flask_login import current_user
from sqlalchemy.orm import selectinload

from models import db, Post, PostLike


def normalize_sort_option(sort_option):
    normalized = (sort_option or "recent").strip().lower()
    if normalized == "new":
        return "recent"
    if normalized in {"recent", "top", "trending", "featured"}:
        return normalized
    return "recent"


def build_post_query(sort_option="recent", featured=False, user_id=None):
    sort_option = normalize_sort_option(sort_option)
    query = Post.query.options(
        selectinload(Post.user),
        selectinload(Post.package),
        selectinload(Post.likes),
        selectinload(Post.downloads),
    )

    if featured or sort_option == "featured":
        query = query.filter(Post.features.any())
        if sort_option == "featured":
            sort_option = "recent"

    if user_id is not None:
        query = query.filter(Post.posted_by == user_id)

    if sort_option in {"top", "trending"}:
        query = (
            query.outerjoin(Post.likes)
            .group_by(Post.id)
            .order_by(db.func.count(PostLike.id).desc(), Post.posted_at.desc())
        )
    else:
        query = query.order_by(Post.posted_at.desc())

    return query, sort_option


def decorate_post_feed(posts, include_user_state=True):
    for post in posts:
        username = post.user.username if post.user else "Unknown"
        post.user_username = f"@{username}"
        post.download_count = post.total_downloads()

        if include_user_state:
            post.liked = False
            post.favorited = False

            if current_user.is_authenticated:
                post.liked = current_user.liked_post(post)
                post.favorited = current_user.favorited_post(post)
