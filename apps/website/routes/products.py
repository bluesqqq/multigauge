from flask import Blueprint, render_template

from models import Product

products_bp = Blueprint('products', __name__)

@products_bp.route("/products")
def products():
    all_products = Product.query.all()
    return render_template('products.html', products=all_products)

@products_bp.route("/products/<slug>")
def view_product(slug):
    product = Product.query.filter_by(slug=slug).first_or_404()

    template = product.custom_template or 'product.html'
    return render_template(template, product=product)