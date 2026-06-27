from pathlib import Path

from flask import Blueprint, abort, send_from_directory


multigauge_web_bp = Blueprint("multigauge_web", __name__)

_REPO_ROOT = Path(__file__).resolve().parents[3]
_MULTIGAUGE_WEB_ROOT = _REPO_ROOT / "ports" / "web"
_MULTIGAUGE_WEB_DIST_ROOT = _MULTIGAUGE_WEB_ROOT / "web" / "dist"
_MULTIGAUGE_WEB_JS_ROOT = _MULTIGAUGE_WEB_DIST_ROOT / "js"
_MULTIGAUGE_WEB_WASM_ROOT = _MULTIGAUGE_WEB_DIST_ROOT / "wasm"


def _ensure_directory_exists(root: Path):
    if not root.is_dir():
        abort(404)


@multigauge_web_bp.route("/multigauge-web/js/<path:asset_path>")
def serve_multigauge_web_js(asset_path):
    _ensure_directory_exists(_MULTIGAUGE_WEB_JS_ROOT)
    return send_from_directory(_MULTIGAUGE_WEB_JS_ROOT, asset_path, conditional=True)


@multigauge_web_bp.route("/multigauge-web/wasm/<path:asset_path>")
def serve_multigauge_web_wasm(asset_path):
    _ensure_directory_exists(_MULTIGAUGE_WEB_WASM_ROOT)
    return send_from_directory(_MULTIGAUGE_WEB_WASM_ROOT, asset_path, conditional=True)


@multigauge_web_bp.route("/multigauge-web/web/dist/<path:asset_path>")
def serve_multigauge_web_dist(asset_path):
    _ensure_directory_exists(_MULTIGAUGE_WEB_DIST_ROOT)
    return send_from_directory(_MULTIGAUGE_WEB_DIST_ROOT, asset_path, conditional=True)
