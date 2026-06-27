from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import DataRequired, Length, Email, EqualTo, ValidationError
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
