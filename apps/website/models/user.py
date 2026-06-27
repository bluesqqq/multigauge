from models import db
from flask_login import UserMixin
from models.post import PostFavorite, PostLike

class User(db.Model, UserMixin):
    __tablename__ = 'users'

    id = db.Column(db.Integer, primary_key=True)                      # User ID
    username = db.Column(db.String(150), unique=True, nullable=False) # User's username
    email    = db.Column(db.String(150), unique=True, nullable=False) # User's email
    password = db.Column(db.String(256), nullable=False)              # User's password (hashed)
    role = db.Column(db.String(20), default="user")                  # User's role (admin, moderator, user)

    def is_moderator(self):
        """Check if the user has moderator privileges"""
        return self.role in ["moderator", "admin"]
    
    def is_admin(self):
        """Check if the user has admin privileges"""
        return self.role == "admin"



    def favorited_post(self, post):
        """Check if the user has favorited a post"""
        return PostFavorite.query.filter_by(user_id=self.id, post_id=post.id).count() > 0

    def get_favorite_posts(self):
        """Get all posts that the user has favorited"""
        return [favorite.post for favorite in self.favorites]
    
    def toggle_favorite_post(self, post):
        """Toggle a post's favorite from this user"""
        favorite = PostFavorite.query.filter_by(user_id=self.id, post_id=post.id).first()

        if favorite:
            db.session.delete(favorite)
            db.session.commit()
            return False
        else:
            # If the user is not favoriting the post, favorite it
            new_favorite = PostFavorite(user_id=self.id, post_id=post.id)
            db.session.add(new_favorite)
            db.session.commit()
            return True
        
    def liked_post(self, post):
        """Check if the user has liked a post"""
        return PostLike.query.filter_by(user_id=self.id, post_id=post.id).count() > 0

    def get_liked_posts(self):
        """Get all posts that the user has liked"""
        return [like.post for like in self.likes]
    
    def toggle_like_post(self, post):
        """Toggle a post's like from this user"""
        like = PostLike.query.filter_by(user_id=self.id, post_id=post.id).first()

        if like:
            db.session.delete(like)
            db.session.commit()
            return False
        else:
            # If the user is not favoriting the post, favorite it
            new_like = PostLike(user_id=self.id, post_id=post.id)
            db.session.add(new_like)
            db.session.commit()
            return True