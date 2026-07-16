"""Shared validation for gauge package JSON payloads."""


def validate_package_payload(package_data):
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
