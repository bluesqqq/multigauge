import json
import hashlib
import re

from flask import Blueprint, request, render_template, redirect, url_for, jsonify, flash, current_app
from flask_login import login_required, current_user
from sqlalchemy.orm import selectinload

from models import db, Post, PostComment, PostFeature, PostDownload, Package
from routes.feed_utils import build_post_query, decorate_post_feed, normalize_sort_option

workshop_bp = Blueprint('workshop', __name__)

POSTS_PER_PAGE = 20


def _safe_filename(value, fallback="package"):
    slug = re.sub(r"[^A-Za-z0-9._-]+", "_", (value or "").strip()).strip("_")
    return slug or fallback


def _validate_package_payload(package_data):
    if not isinstance(package_data, dict):
        return "Package JSON must be an object."

    required_keys = {"name", "author", "description", "faces"}
    if set(package_data.keys()) != required_keys:
        return "Package JSON must contain only name, author, description, and faces."

    if not isinstance(package_data["name"], str) or not package_data["name"].strip():
        return "Package name must be a non-empty string."

    if not isinstance(package_data["author"], str) or not package_data["author"].strip():
        return "Package author must be a non-empty string."

    if not isinstance(package_data["description"], str):
        return "Package description must be a string."

    faces = package_data["faces"]
    if not isinstance(faces, list) or not faces:
        return "Package must contain at least one face."

    for face_entry in faces:
        if not isinstance(face_entry, dict):
            return "Face entries must be objects."
        if set(face_entry.keys()) != {"name", "face"}:
            return "Face entries must contain only name and face."
        if not isinstance(face_entry["name"], str) or not face_entry["name"].strip():
            return "Face name must be a non-empty string."
        if not isinstance(face_entry["face"], dict):
            return "Face payload must be an object."

    return None


@workshop_bp.route("/workshop/")
def workshop():
    requested_sort = normalize_sort_option(request.args.get('sort', 'recent'))
    featured = request.args.get('featured', 'false').lower() == 'true'
    featured_active = featured or requested_sort == 'featured'
    user_id = request.args.get('user', type=int)
    page = request.args.get('page', 1, type=int)

    query, sort_option = build_post_query(requested_sort, featured=featured, user_id=user_id)

    pagination = query.paginate(page=page, per_page=POSTS_PER_PAGE, error_out=False)
    posts = pagination.items
    decorate_post_feed(posts, include_user_state=False)

    active_filter = 'featured' if featured_active else sort_option
    feed_kwargs = {"user": user_id} if user_id is not None else {}
    filter_links = {
        "recent": url_for("workshop.workshop", sort="recent", **feed_kwargs),
        "top": url_for("workshop.workshop", sort="top", **feed_kwargs),
        "featured": url_for("workshop.workshop", featured="true", **feed_kwargs),
        "trending": url_for("workshop.workshop", sort="trending", **feed_kwargs),
    }

    page_kwargs = {"user": user_id} if user_id is not None else {}
    if active_filter == "featured":
        prev_page_url = (
            url_for("workshop.workshop", page=pagination.prev_num, featured="true", **page_kwargs)
            if pagination.has_prev
            else None
        )
        next_page_url = (
            url_for("workshop.workshop", page=pagination.next_num, featured="true", **page_kwargs)
            if pagination.has_next
            else None
        )
    else:
        prev_page_url = (
            url_for("workshop.workshop", page=pagination.prev_num, sort=sort_option, **page_kwargs)
            if pagination.has_prev
            else None
        )
        next_page_url = (
            url_for("workshop.workshop", page=pagination.next_num, sort=sort_option, **page_kwargs)
            if pagination.has_next
            else None
        )

    return render_template(
        'workshop.html',
        posts=posts,
        pagination=pagination,
        sort_option=sort_option,
        active_filter=active_filter,
        user_id=user_id,
        filter_links=filter_links,
        prev_page_url=prev_page_url,
        next_page_url=next_page_url,
        navbar_background=True,
    )


@workshop_bp.route("/workshop/upload", methods=['GET', 'POST'])
@login_required
def workshop_upload():
    if request.method == 'POST':
        package_file = request.files.get('package')
        title = request.form.get('title', '').strip()
        description = request.form.get('description', '').strip()
        posted_by = current_user.id

        if not package_file or not title or not description or not posted_by:
            flash("Please fill out the title, description, and package file.", "danger")
            return render_template('upload-gaugeface.html'), 400

        if not package_file.filename.lower().endswith(('.json', '.package')):
            flash("Please upload a valid package JSON file.", "danger")
            return render_template('upload-gaugeface.html'), 400

        try:
            file_content = package_file.read().decode('utf-8')
            package_data = json.loads(file_content)
        except (UnicodeDecodeError, json.JSONDecodeError):
            flash("Invalid package JSON.", "danger")
            return render_template('upload-gaugeface.html'), 400

        validation_error = _validate_package_payload(package_data)
        if validation_error:
            flash(validation_error, "danger")
            return render_template('upload-gaugeface.html'), 400

        package = Package(
            package_json=file_content,
            package_hash=hashlib.sha256(file_content.encode("utf-8")).hexdigest(),
        )
        new_post = Post(
            title=title,
            description=description,
            posted_by=posted_by,
            package=package,
        )

        db.session.add(new_post)
        db.session.commit()
        return redirect(url_for('workshop.view_post', post_id=new_post.id))

    return render_template('upload-gaugeface.html')


@workshop_bp.route("/workshop/<int:post_id>")
def view_post(post_id):
    post = Post.query.options(selectinload(Post.user), selectinload(Post.package)).filter_by(id=post_id).first()
    if not post:
        return "Post not found!", 404

    likes = len(post.likes)
    favorites = len(post.favorites)
    comments = PostComment.query.filter_by(post_id=post_id).order_by(PostComment.created_at.desc()).all()

    liked = False
    favorited = False
    if current_user.is_authenticated:
        liked = any(like.user_id == current_user.id for like in post.likes)
        favorited = current_user.favorited_post(post)

    return render_template(
        'post.html',
        post=post,
        posted_by_username=post.user.username if post.user else "Unknown",
        likes=likes,
        favorites=favorites,
        comments=comments,
        has_liked=liked,
        has_favorited=favorited,
    )


@workshop_bp.route("/workshop/<int:post_id>/download")
def download_package(post_id):
    post = Post.query.options(selectinload(Post.package)).filter_by(id=post_id).first()
    if not post or not post.package:
        return "Package not found!", 404

    download = PostDownload(
        post_id=post.id,
        user_id=current_user.id if current_user.is_authenticated else None,
    )
    db.session.add(download)
    db.session.commit()

    response = current_app.response_class(
        response=post.package.package_json,
        status=200,
        mimetype='application/json',
        headers={
            'Content-Disposition': f'attachment; filename=package_{_safe_filename(post.package.name or post.title)}.json'
        }
    )
    return response


@workshop_bp.route("/workshop/<int:post_id>/like", methods=['POST'])
@login_required
def toggle_like_post(post_id):
    post = Post.query.get(post_id)
    if not post:
        return jsonify({"error": "Post not found."}), 404

    liked = current_user.toggle_like_post(post)
    total_likes = len(post.likes)

    return jsonify({
        'total_likes': total_likes,
        'liked': liked
    })


@workshop_bp.route("/workshop/<int:post_id>/favorite", methods=['POST'])
@login_required
def toggle_favorite_post(post_id):
    post = Post.query.get(post_id)
    if not post:
        return jsonify({"error": "Post not found."}), 404

    favorited = current_user.toggle_favorite_post(post)
    total_favorites = len(post.favorites)

    return jsonify({
        'total_favorites': total_favorites,
        'favorited': favorited
    })


@workshop_bp.route("/workshop/<int:post_id>/feature", methods=["POST"])
@login_required
def toggle_feature_post(post_id):
    if not current_user.is_moderator():
        flash("You do not have permission to feature gauge faces.", "danger")
        return redirect(url_for('workshop.workshop'))

    existing_feature = PostFeature.query.filter_by(post_id=post_id, moderator_id=current_user.id).first()

    featured = False
    if existing_feature:
        db.session.delete(existing_feature)
    else:
        new_feature = PostFeature(post_id=post_id, moderator_id=current_user.id)
        db.session.add(new_feature)
        featured = True

    db.session.commit()

    return jsonify({'featured': featured})


@workshop_bp.route("/workshop/<int:post_id>/comment", methods=['POST'])
@login_required
def comment_on_gauge(post_id):
    post = Post.query.get(post_id)
    if not post:
        return "Post not found!", 404

    content = request.form.get('content')
    if not content:
        return "Comment content cannot be empty.", 400

    new_comment = PostComment(post_id=post_id, user_id=current_user.id, content=content)
    db.session.add(new_comment)
    db.session.commit()

    return redirect(url_for('workshop.view_post', post_id=post_id))
