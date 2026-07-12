from flask import Blueprint, redirect, render_template, request, url_for

from routes.feed_utils import build_post_query, decorate_post_feed, normalize_sort_option

main_bp = Blueprint('main', __name__)

# Home route
@main_bp.route("/")
def home():
    requested_sort = normalize_sort_option(request.args.get('sort', 'recent'))
    featured = request.args.get('featured', 'false').lower() == 'true'
    featured_active = featured or requested_sort == 'featured'

    query, sort_option = build_post_query(requested_sort, featured=featured)
    community_posts = query.limit(4).all()
    decorate_post_feed(community_posts, include_user_state=False)

    active_filter = 'featured' if featured_active else sort_option
    view_all_url = (
        url_for("workshop.workshop", featured="true")
        if active_filter == "featured"
        else url_for("workshop.workshop", sort=sort_option)
    )

    return render_template(
        'index.html',
        community_posts=community_posts,
        active_filter=active_filter,
        view_all_url=view_all_url,
    )

# Editor route
@main_bp.route("/editor")
def editor():
    return render_template("editor.html")


@main_bp.route("/new-editor")
def editor_redirect():
    return redirect(url_for("main.editor"))

