from flask import Blueprint, render_template

from services.firmware_downloads import get_firmware_manifest, get_latest_firmware_release


downloads_bp = Blueprint("downloads", __name__)


@downloads_bp.route("/downloads")
def downloads():
    manifest = get_firmware_manifest()
    latest_release = get_latest_firmware_release()

    return render_template(
        "downloads.html",
        firmware_manifest=manifest,
        firmware_releases=manifest.get("releases", []),
        latest_release=latest_release,
        navbar_background=True,
    )
