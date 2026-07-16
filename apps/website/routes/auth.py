from flask import Blueprint, render_template, redirect, url_for, flash, request
from flask_login import login_user, logout_user, login_required, current_user
from models import db, bcrypt, User
from forms import RegisterForm, LoginForm
from sqlalchemy import or_

auth_bp = Blueprint('auth', __name__)

@auth_bp.route("/register", methods=["GET", "POST"])
def register():
    if current_user.is_authenticated:
        return redirect(url_for("workshop.workshop"))

    form = RegisterForm()
    if form.validate_on_submit():
        hashed_password = bcrypt.generate_password_hash(form.password.data).decode("utf-8")
        user = User(
            username = form.username.data,
            email    = form.email.data,
            password = hashed_password
        )
        db.session.add(user)
        db.session.commit()
        flash("Account created! You can now log in.", "success")
        return redirect(url_for("auth.login"))

    return render_template("register.html", form=form)

@auth_bp.route("/login", methods=["GET", "POST"])
def login():
    if current_user.is_authenticated:
        return redirect(url_for("workshop.workshop"))

    form = LoginForm()

    if form.validate_on_submit():
        identifier = form.username.data
        user = User.query.filter(or_(User.username == identifier, User.email == identifier)).first()

        if user and bcrypt.check_password_hash(user.password, form.password.data):
            login_user(user)
            flash("Logged in successfully!", "success")
            return redirect(url_for("workshop.workshop"))
        else:
            flash("Login failed. Check your email and password.", "danger")

    else:
        print("form not validated.")
        print(form.errors)

    return render_template("login.html", form=form)

@auth_bp.route("/logout")
@login_required
def logout():
    logout_user()
    flash("You have been logged out.", "success")
    return redirect(url_for("auth.login"))