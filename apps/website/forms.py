from flask_wtf import FlaskForm
from flask_wtf.file import FileAllowed, FileField
from wtforms import (
    BooleanField,
    PasswordField,
    SelectField,
    StringField,
    SubmitField,
    TextAreaField,
)
from wtforms.validators import DataRequired, Email, EqualTo, Length, ValidationError

from models import User

class RegisterForm(FlaskForm):
    username = StringField("Username", validators=[DataRequired(), Length(min=4, max=20)])
    email    = StringField("Email", validators=[DataRequired(), Email(), Length(max=150)])
    password = PasswordField("Password", validators=[DataRequired(), Length(min=6)])
    submit   = SubmitField("Sign Up")

    def validate_username(self, username):
        if User.query.filter_by(username = username.data).first():
            raise ValidationError("Username already taken.")
        
    def validate_email(self, email):
        if User.query.filter_by(email = email.data).first():
            raise ValidationError("Email already registered.")

class LoginForm(FlaskForm):
    username = StringField("Username or Email", validators=[DataRequired(), Length(max=150)])
    password = PasswordField("Password", validators=[DataRequired()])
    submit   = SubmitField("Login")


class AccountSettingsForm(FlaskForm):
    avatar_file = FileField(
        "Profile Picture (Avatar)",
        validators=[FileAllowed(["jpg", "jpeg", "png", "gif", "webp"], "Please upload an image file.")],
    )
    display_name = StringField("Display Name", validators=[Length(max=150)])
    username = StringField("Username", validators=[DataRequired(), Length(min=4, max=20)])
    bio = TextAreaField("Description (Bio)", validators=[Length(max=500)])
    accent_color = SelectField(
        "Accent color",
        choices=[
            ("#ed1c24", "Multigauge Red"),
            ("#111827", "Graphite"),
            ("#2563eb", "Blue"),
            ("#0f766e", "Teal"),
            ("#b45309", "Amber"),
        ],
        validators=[DataRequired()],
    )
    email_notifications = BooleanField("Email notification preferences")
    order_updates = BooleanField("Order updates")
    community_digest = BooleanField("Community digest")
    product_announcements = BooleanField("Product announcements")
    submit = SubmitField("Save changes")

    def __init__(self, original_username=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.original_username = original_username

    def validate_username(self, username):
        normalized = username.data.strip()
        if self.original_username and normalized == self.original_username:
            return
        if User.query.filter_by(username=normalized).first():
            raise ValidationError("Username already taken.")


class ChangePasswordForm(FlaskForm):
    current_password = PasswordField("Current password", validators=[DataRequired()])
    new_password = PasswordField("New password", validators=[DataRequired(), Length(min=6)])
    confirm_password = PasswordField(
        "Confirm new password",
        validators=[DataRequired(), EqualTo("new_password", message="Passwords must match.")],
    )
    submit = SubmitField("Change password")


class DangerActionForm(FlaskForm):
    submit = SubmitField("Confirm")
