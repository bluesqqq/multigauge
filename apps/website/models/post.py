
from datetime import datetime
from models import db

class Post(db.Model):
    __tablename__ = 'posts'
    
    id          = db.Column(db.Integer, primary_key=True, autoincrement=True) # Post ID
    gauge       = db.Column(db.Text)                                          # JSON GaugeFace file
    title       = db.Column(db.String(255), nullable=False)                   # Post title
    description = db.Column(db.String(255), nullable=False)                   # Post description
    gauge_type  = db.Column(db.String(255), nullable=False)

    posted_at = db.Column(db.DateTime, default=datetime.utcnow)      # Date posted at
    posted_by = db.Column(db.Integer, nullable=False)                # The ID of the user who posted

    downloads = db.Column(db.Integer, default=0, nullable=False)  # Total number of downloads

    def posted_how_long_ago(self):
        now = datetime.utcnow()
        diff = now - self.posted_at

        if diff.total_seconds() < 60:
            return "now"
        elif diff.total_seconds() < 3600:
            minutes = int(diff.total_seconds() / 60)
            return f"{minutes} minute{'s' if minutes != 1 else ''} ago"
        elif diff.total_seconds() < 86400:
            hours = int(diff.total_seconds() / 3600)
            return f"{hours} hour{'s' if hours != 1 else ''} ago"
        elif diff.total_seconds() < 604800:
            days = int(diff.total_seconds() / 86400)
            return f"{days} day{'s' if days != 1 else ''} ago"
        else:
            return self.posted_at.strftime("%b %d, %Y")  # e.g., "Apr 07, 2025"

    def total_likes(self):
        return len(self.likes)
    
    def total_features(self):
        return len(self.features)

    def is_featured(self):
        return len(self.features) > 0
    
    def __repr__(self):
        return f'<Post {self.title}>'

class PostLike(db.Model):
    __tablename__ = 'post_likes'

    id         = db.Column(db.Integer, primary_key=True, autoincrement=True)      # Like ID
    post_id    = db.Column(db.Integer, db.ForeignKey('posts.id'), nullable=False) # The ID of the post being liked
    user_id    = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)  # The ID of the user who liked the post
    created_at = db.Column(db.DateTime, default=datetime.utcnow)                  # Date liked at

    post = db.relationship('Post', backref=db.backref('likes', lazy=True))
    user = db.relationship('User', backref=db.backref('likes', lazy=True))

    def __repr__(self):
        return f'<PostLike post_id={self.post_id}, user_id={self.user_id}>'

class PostComment(db.Model):
    __tablename__ = 'post_comments'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)           # Comment ID
    post_id = db.Column(db.Integer, db.ForeignKey('posts.id'), nullable=False) # The ID of the post being commented on
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)  # The ID of the user who commented
    content = db.Column(db.Text, nullable=False)                               # The content of the comment
    created_at = db.Column(db.DateTime, default=datetime.utcnow)               # Date commented at

    post = db.relationship('Post', backref=db.backref('comments', lazy=True))
    user = db.relationship('User', backref=db.backref('comments', lazy=True))

    def how_long_ago(self):
        now = datetime.utcnow()
        diff = now - self.created_at

        if diff.total_seconds() < 60:
            return "now"
        elif diff.total_seconds() < 3600:
            minutes = int(diff.total_seconds() / 60)
            return f"{minutes} minute{'s' if minutes != 1 else ''} ago"
        elif diff.total_seconds() < 86400:
            hours = int(diff.total_seconds() / 3600)
            return f"{hours} hour{'s' if hours != 1 else ''} ago"
        elif diff.total_seconds() < 604800:
            days = int(diff.total_seconds() / 86400)
            return f"{days} day{'s' if days != 1 else ''} ago"
        else:
            return self.posted_at.strftime("%b %d, %Y")  # e.g., "Apr 07, 2025"

    def __repr__(self):
        return f'<PostComment post_id={self.post_id}, user_id={self.user_id}>'

class PostFavorite(db.Model):
    __tablename__ = 'post_favorites'

    id         = db.Column(db.Integer, primary_key=True, autoincrement=True)      # Favorite ID
    post_id    = db.Column(db.Integer, db.ForeignKey('posts.id'), nullable=False) # The ID of the post being favorited
    user_id    = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)  # The ID of the user who favorited the post
    created_at = db.Column(db.DateTime, default=datetime.utcnow)                  # Date favorited at

    post = db.relationship('Post', backref=db.backref('favorites', lazy=True))
    user = db.relationship('User', backref=db.backref('favorites', lazy=True))

    def __repr__(self):
        return f'<PostFavorite post_id={self.post_id}, user_id={self.user_id}>'
    
class PostFeature(db.Model):
    __tablename__ = 'post_feature'

    id           = db.Column(db.Integer, primary_key=True, autoincrement=True)      # Feature ID
    post_id      = db.Column(db.Integer, db.ForeignKey('posts.id'), nullable=False) # The ID of the post being featured
    moderator_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)  # The ID of the user who featured the post
    created_at   = db.Column(db.DateTime, default=datetime.utcnow)                  # Date featured at

    post = db.relationship('Post', backref=db.backref('features', lazy=True))
    user = db.relationship('User', backref=db.backref('featured_posts', lazy=True))

    def __repr__(self):
        return f'<PostFeature post_id={self.post_id}, moderator_id={self.moderator_id}>'
    
