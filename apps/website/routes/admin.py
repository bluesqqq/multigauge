from flask import Blueprint, render_template, redirect, url_for, flash
from flask_login import login_required, current_user

from models import Order

admin_bp = Blueprint('admin', __name__)

@admin_bp.route("/admin")
@login_required
def admin_dashboard():
    # Check if the user is a moderator
    if not current_user.is_moderator():
        flash("You do not have permission to view this page.", "danger")
        return redirect(url_for('main.index'))
    
    return render_template("admin.html")

@admin_bp.route("/admin/orders")
@login_required
def orders():
    # Check if the user is a moderator
    if not current_user.is_moderator():
        flash("You do not have permission to view this page.", "danger")
        return redirect(url_for('main.index'))
    
    orders = Order.query.all()
    
    return render_template("orders.html", orders = orders)


@admin_bp.route("/admin/orders/<int:order_id>")
@login_required
def view_order(order_id):
    # Check if the user is a moderator
    if not current_user.is_moderator():
        flash("You do not have permission to view this page.", "danger")
        return redirect(url_for('main.index'))
    
    order = Order.query.get(order_id)
    
    return render_template("order.html", order = order)
