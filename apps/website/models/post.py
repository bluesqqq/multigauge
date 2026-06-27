import json
from datetime import datetime

from models import db


class Post(db.Model):
    __tablename__ = "posts"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    title = db.Column(db.String(255), nullable=False)
    description = db.Column(db.Text, nullable=False)
    posted_at = db.Column(db.DateTime, default=datetime.utcnow, nullable=False)
    posted_by = db.Column(db.Integer, db.ForeignKey("users.id"), nullable=False)
    package_id = db.Column(db.Integer, db.ForeignKey("packages.id"), nullable=False, unique=True)

    user = db.relationship("User", backref=db.backref("posts", lazy=True))
    package = db.relationship(
        "Package",
        backref=db.backref("post", uselist=False),
        cascade="all, delete-orphan",
        single_parent=True,
        uselist=False,
    )

    __table_args__ = (
        db.Index("ix_posts_posted_by", "posted_by"),
        db.Index("ix_posts_posted_at", "posted_at"),
        db.Index("ix_posts_package_id", "package_id"),
    )

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
            return self.posted_at.strftime("%b %d, %Y")

    def total_likes(self):
        return len(self.likes)

    def total_features(self):
        return len(self.features)

    def is_featured(self):
        return len(self.features) > 0

    def __repr__(self):
        return f"<Post id={self.id}, package_id={self.package_id}>"


class PostLike(db.Model):
    __tablename__ = "post_likes"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    post_id = db.Column(db.Integer, db.ForeignKey("posts.id"), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey("users.id"), nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    post = db.relationship("Post", backref=db.backref("likes", lazy=True))
    user = db.relationship("User", backref=db.backref("likes", lazy=True))

    __table_args__ = (
        db.UniqueConstraint("post_id", "user_id", name="uq_post_likes_post_user"),
        db.Index("ix_post_likes_post_id", "post_id"),
        db.Index("ix_post_likes_user_id", "user_id"),
    )

    def __repr__(self):
        return f"<PostLike post_id={self.post_id}, user_id={self.user_id}>"


class PostComment(db.Model):
    __tablename__ = "post_comments"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    post_id = db.Column(db.Integer, db.ForeignKey("posts.id"), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey("users.id"), nullable=False)
    content = db.Column(db.Text, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    post = db.relationship("Post", backref=db.backref("comments", lazy=True))
    user = db.relationship("User", backref=db.backref("comments", lazy=True))

    __table_args__ = (
        db.Index("ix_post_comments_post_id", "post_id"),
        db.Index("ix_post_comments_user_id", "user_id"),
        db.Index("ix_post_comments_created_at", "created_at"),
    )

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
            return self.created_at.strftime("%b %d, %Y")

    def __repr__(self):
        return f"<PostComment post_id={self.post_id}, user_id={self.user_id}>"


class PostFavorite(db.Model):
    __tablename__ = "post_favorites"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    post_id = db.Column(db.Integer, db.ForeignKey("posts.id"), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey("users.id"), nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    post = db.relationship("Post", backref=db.backref("favorites", lazy=True))
    user = db.relationship("User", backref=db.backref("favorites", lazy=True))

    __table_args__ = (
        db.UniqueConstraint("post_id", "user_id", name="uq_post_favorites_post_user"),
        db.Index("ix_post_favorites_post_id", "post_id"),
        db.Index("ix_post_favorites_user_id", "user_id"),
    )

    def __repr__(self):
        return f"<PostFavorite post_id={self.post_id}, user_id={self.user_id}>"


class PostFeature(db.Model):
    __tablename__ = "post_feature"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    post_id = db.Column(db.Integer, db.ForeignKey("posts.id"), nullable=False)
    moderator_id = db.Column(db.Integer, db.ForeignKey("users.id"), nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    post = db.relationship("Post", backref=db.backref("features", lazy=True))
    user = db.relationship("User", backref=db.backref("featured_posts", lazy=True))

    __table_args__ = (
        db.UniqueConstraint("post_id", "moderator_id", name="uq_post_feature_post_moderator"),
        db.Index("ix_post_feature_post_id", "post_id"),
        db.Index("ix_post_feature_moderator_id", "moderator_id"),
    )

    def __repr__(self):
        return f"<PostFeature post_id={self.post_id}, moderator_id={self.moderator_id}>"


class Package(db.Model):
    __tablename__ = "packages"

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    package_json = db.Column(db.Text, nullable=False)
    package_format = db.Column(db.String(64), nullable=False, default="package+json")
    package_version = db.Column(db.Integer, nullable=False, default=1)
    package_hash = db.Column(db.String(128), nullable=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow, nullable=False)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow, nullable=False)

    __table_args__ = (
        db.Index("ix_packages_package_format", "package_format"),
        db.Index("ix_packages_package_version", "package_version"),
    )

    def data(self):
        try:
            parsed = json.loads(self.package_json)
            return parsed if isinstance(parsed, dict) else {}
        except (TypeError, json.JSONDecodeError):
            return {}

    @property
    def name(self):
        return self.data().get("name", "")

    @property
    def author(self):
        return self.data().get("author", "")

    @property
    def description(self):
        return self.data().get("description", "")

    @property
    def faces(self):
        faces = self.data().get("faces", [])
        return faces if isinstance(faces, list) else []

    @property
    def face_count(self):
        return len(self.faces)

    def first_face(self):
        faces = self.faces
        if not faces:
            return {}
        first = faces[0]
        if not isinstance(first, dict):
            return {}
        face = first.get("face", {})
        return face if isinstance(face, dict) else {}

    def first_face_json(self):
        return json.dumps(self.first_face())
