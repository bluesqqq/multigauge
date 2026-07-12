"""
import_packages.py - Import example gauge packages into the website database.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from app import app
from models import db, Package, Post, User

DEFAULT_PACKAGE_DIR = Path(__file__).resolve().parents[1] / "static" / "json" / "example-packages"


def _validate_package_payload(package_data):
    if not isinstance(package_data, dict):
        return "Package JSON must be an object."

    required_keys = {"name", "author", "description", "faces"}
    if set(package_data.keys()) != required_keys:
        return "Package JSON must contain only name, author, description, and faces."

    if not isinstance(package_data["name"], str) or not package_data["name"].strip():
        return "Package name must be a non-empty string."

    if not isinstance(package_data["author"], str) or not package_data["author"].strip():
        return "Package author must be a non-empty string."

    if not isinstance(package_data["description"], str):
        return "Package description must be a string."

    faces = package_data["faces"]
    if not isinstance(faces, list) or not faces:
        return "Package must contain at least one face."

    for face_entry in faces:
        if not isinstance(face_entry, dict):
            return "Face entries must be objects."
        if set(face_entry.keys()) != {"name", "face"}:
            return "Face entries must contain only name and face."
        if not isinstance(face_entry["name"], str) or not face_entry["name"].strip():
            return "Face name must be a non-empty string."
        if not isinstance(face_entry["face"], dict):
            return "Face payload must be an object."

    return None


def resolve_user(identifier):
    if identifier is None:
        user = User.query.filter_by(username="bluesq").first()
        if user:
            return user
        return User.query.order_by(User.id.asc()).first()

    if isinstance(identifier, int):
        return db.session.get(User, identifier)

    value = str(identifier).strip()
    if not value:
        return None

    if value.isdigit():
        return db.session.get(User, int(value))

    return User.query.filter_by(username=value).first()


def import_package_file(package_file, posted_by_user):
    file_content = package_file.read_text(encoding="utf-8")
    package_data = json.loads(file_content)

    validation_error = _validate_package_payload(package_data)
    if validation_error:
        return {"status": "invalid", "reason": validation_error}

    package_hash = hashlib.sha256(file_content.encode("utf-8")).hexdigest()
    existing_package = (
        Package.query.filter_by(package_hash=package_hash).first()
        or Package.query.filter_by(package_json=file_content).first()
    )

    if existing_package:
        existing_post = Post.query.filter_by(package_id=existing_package.id).first()
        if existing_post:
            return {
                "status": "skipped",
                "reason": "already imported",
                "post_id": existing_post.id,
                "package_id": existing_package.id,
            }

        post = Post(
            title=package_data["name"].strip(),
            description=package_data["description"].strip(),
            posted_by=posted_by_user.id,
            package=existing_package,
        )
        db.session.add(post)
        db.session.commit()
        return {
            "status": "created",
            "post_id": post.id,
            "package_id": existing_package.id,
        }

    package = Package(package_json=file_content, package_hash=package_hash)
    db.session.add(package)
    db.session.flush()

    post = Post(
        title=package_data["name"].strip(),
        description=package_data["description"].strip(),
        posted_by=posted_by_user.id,
        package=package,
    )

    db.session.add(post)
    db.session.commit()

    return {
        "status": "created",
        "post_id": post.id,
        "package_id": package.id,
    }


def main():
    parser = argparse.ArgumentParser(description="Import package JSON files into the website database.")
    parser.add_argument(
        "--directory",
        default=str(DEFAULT_PACKAGE_DIR),
        help=f"Directory containing package files (default: {DEFAULT_PACKAGE_DIR})",
    )
    parser.add_argument(
        "--user",
        default=None,
        help="Username or user ID to attach imported posts to (defaults to bluesq or the first user).",
    )
    args = parser.parse_args()

    package_dir = Path(args.directory).expanduser().resolve()
    if not package_dir.exists() or not package_dir.is_dir():
        raise SystemExit(f"Package directory not found: {package_dir}")

    with app.app_context():
        db.create_all()

        posted_by_user = resolve_user(args.user)
        if not posted_by_user:
            raise SystemExit("No user found to assign imported posts to. Pass --user explicitly.")

        package_files = sorted(
            [
                path
                for path in package_dir.iterdir()
                if path.is_file() and path.suffix.lower() in {".json", ".package"}
            ],
            key=lambda path: path.name.lower(),
        )

        if not package_files:
            raise SystemExit(f"No package files found in {package_dir}")

        created = 0
        skipped = 0
        invalid = 0

        for package_file in package_files:
            try:
                result = import_package_file(package_file, posted_by_user)
            except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
                invalid += 1
                print(f"[invalid] {package_file.name}: {error}")
                continue

            if result["status"] == "created":
                created += 1
                print(f"[created] {package_file.name} -> post {result['post_id']}")
            elif result["status"] == "skipped":
                skipped += 1
                print(f"[skipped] {package_file.name}: {result['reason']}")
            else:
                invalid += 1
                print(f"[invalid] {package_file.name}: {result['reason']}")

        print(f"Imported {created} package(s); skipped {skipped}; invalid {invalid}.")


if __name__ == "__main__":
    main()
