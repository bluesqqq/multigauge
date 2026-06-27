from flask import Blueprint, request, render_template, redirect, url_for, jsonify, flash, current_app
from flask_login import login_required, current_user
from models import db, Post, User, PostComment, PostFeature

workshop_bp = Blueprint('workshop', __name__)

POSTS_PER_PAGE = 12

# Workshop route
@workshop_bp.route("/workshop/")
def workshop():
    # Get the sort option from the query parameters (default is 'recent')
    sort_option = request.args.get('sort', 'recent')
    gauge_type_option = request.args.get('type', 'all')
    featured = request.args.get('featured', 'false') == 'true'
    user_id = request.args.get('user', type=int)

    page = request.args.get('page', 1, type=int)

    query = Post.query

    # FILTERING

    # Apply gauge type filter if provided
    if gauge_type_option != 'all':
        print(gauge_type_option)
        query = query.filter(Post.gauge_type == gauge_type_option)

    # Apply 'featured' filter
    if featured:
        query = query.filter(Post.features.any())

    if user_id is not None:
        query = query.filter(Post.posted_by == user_id)

    # SORTING

    # Sort by most downloaded: order by number of downloads in descending order
    if sort_option == 'most_downloaded':
        query = query.order_by(Post.downloads.desc())

    # Sort by top: order by number of likes in descending order
    elif sort_option == 'top':
        query = query.outerjoin(Post.likes).group_by(Post.id).order_by(db.func.count(Post.likes).desc())

    elif sort_option == 'trending':
        query = query.outerjoin(Post.likes).group_by(Post.id).order_by(
            db.func.count(Post.likes).desc(),
            Post.posted_at.desc()
        )

    # Sort by new: order by posted_at in descending order
    else:  # Default to 'new'
        query = query.order_by(Post.posted_at.desc())

    pagination = query.paginate(page=page, per_page=POSTS_PER_PAGE, error_out=False)
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
    
    return render_template('workshop.html', posts=posts, pagination=pagination, sort_option=sort_option)

# Workshop post upload route
@workshop_bp.route("/workshop/upload", methods=['GET', 'POST'])
@login_required
def workshop_upload():

    if request.method == 'POST':

        gauge = request.files['gauge']
        title = request.form.get('title')
        description = request.form.get('description')
        gauge_type = request.form.get('gauge-type')
        posted_by = current_user.id

        if gauge and title and description and gauge_type and posted_by:

            # Ensure the file is .gauge (JSON) and read its content
            if gauge.filename.endswith('.gauge'):
                file_content = gauge.read().decode('utf-8')  # Read and decode file content
                # Insert into the database using SQLAlchemy ORM
                new_post = Post(
                    gauge       = file_content,
                    title       = title,
                    description = description,
                    posted_by   = posted_by,
                    gauge_type  = gauge_type
                )

                db.session.add(new_post)
                db.session.commit()
                return redirect(url_for('workshop.view_post', post_id=new_post.id))
            
            else:
                return "Invalid file format. Only JSON is allowed.", 400
        
    return render_template('upload-gaugeface.html')

# Workshop single post view route
@workshop_bp.route("/workshop/<int:post_id>")
def view_post(post_id):

    # Fetch the post
    post = Post.query.get(post_id)

    # Fetch the user who posted it
    user = User.query.get(post.posted_by)

    if post:
        # Fetch the likes
        likes = len(post.likes)

        # Fetch the favorites
        favorites = len(post.favorites)

        # Fetch the comments
        comments = PostComment.query.filter_by(post_id=post_id).order_by(PostComment.created_at.desc()).all()

        # Check if current_user has liked or favorited this post
        liked = False
        favorited = False

        if current_user.is_authenticated:
            liked = any(like.user_id == current_user.id for like in post.likes)
            favorited = current_user.favorited_post(post)

        return render_template(
            'post.html',
            post=post,
            posted_by_username=user.username,
            likes=likes,
            favorites=favorites,
            comments=comments,
            has_liked=liked,
            has_favorited=favorited
        )
    
    else:
        return "Post not found!", 404

# Workshop post gauge download route
@workshop_bp.route("/workshop/<int:post_id>/download")
def download_gauge(post_id):

    post = Post.query.get(post_id)

    if post:
        post.downloads += 1  # Increment the download count
        db.session.commit()

        # Set up the file response to download the JSON data
        response = current_app.response_class(
            response=post.gauge,  # The raw JSON data
            status=200,
            mimetype='application/json',
            headers={
                'Content-Disposition': f'attachment; filename=gauge_{post.title}.gauge'
            }
        )
        return response
    
    else:
        return "Gauge not found!", 404

# Workshop post like route
@workshop_bp.route("/workshop/<int:post_id>/like", methods=['POST'])
@login_required
def toggle_like_post(post_id):
    # Fetch the post
    post = Post.query.get(post_id)

    # Use the `toggle_like_post` method from the User model to toggle the like
    liked = current_user.toggle_like_post(post)

    # Fetch the updated like count using SQLAlchemy
    total_likes = len(post.likes)
    
    return jsonify({
        'total_likes': total_likes,
        'liked': liked
    })

# Workshop post like route
@workshop_bp.route("/workshop/<int:post_id>/favorite", methods=['POST'])
@login_required
def toggle_favorite_post(post_id):
    # Fetch the post
    post = Post.query.get(post_id)

    # Use the `toggle_like_post` method from the User model to toggle the like
    favorited = current_user.toggle_favorite_post(post)

    # Fetch the updated like count using SQLAlchemy
    total_favorites = len(post.favorites)
    
    return jsonify({
        'total_favorites': total_favorites,
        'favorited': favorited
    })

# Workshop post feature route
@workshop_bp.route("/workshop/<int:post_id>/feature", methods=["POST"])
@login_required
def toggle_feature_post(post_id):
    # Check if the user is a moderator
    if not current_user.is_moderator():
        flash("You do not have permission to feature gauge faces.", "danger")
        return redirect(url_for('workshop'))
    
    print("USER IS MODERATOR")

    # Check if the moderator has already featured this post
    existing_feature = PostFeature.query.filter_by(post_id=post_id, moderator_id=current_user.id).first()

    featured = False
    if existing_feature:
        print("Post is already featured")
        # If already featured, unfeature it
        db.session.delete(existing_feature)

    else:
        print("Post is not featured")
        # If not featured, feature it
        new_feature = PostFeature(post_id=post_id, moderator_id=current_user.id)
        db.session.add(new_feature)
        featured = True
    
    db.session.commit()

    return jsonify({'featured' : featured})

# Workshop post comment route
@workshop_bp.route("/workshop/<int:post_id>/comment", methods=['POST'])
@login_required
def comment_on_gauge(post_id):
    user_id = current_user.id
    content = request.form.get('content')

    if not content:
        return "Comment content cannot be empty.", 400

    # Insert the comment into the database using SQLAlchemy
    new_comment = PostComment(post_id=post_id, user_id=user_id, content=content)
    db.session.add(new_comment)
    db.session.commit()
    
    return redirect(url_for('workshop.view_post', post_id=post_id))
