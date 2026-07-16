import json
import os
import re
import urllib.error
import urllib.request
from datetime import datetime
from pathlib import Path
from typing import Any


_MANIFEST_PATH = Path(__file__).resolve().parents[1] / "static" / "firmware" / "multigauge" / "manifest.json"
_RELEASE_TAG_PATTERN = re.compile(r"^(?P<product>[^@]+)@(?P<version>.+)$")
_CACHE_TTL_SECONDS = 300
_CACHE = {"loaded_at": None, "payload": None}


def _load_json_file(path: Path) -> dict[str, Any] | None:
    if not path.is_file():
        return None

    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def _fetch_github_releases(repository: str) -> list[dict[str, Any]]:
    api_url = f"https://api.github.com/repos/{repository}/releases?per_page=100"
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "multigauge-website",
    }

    token = os.getenv("GITHUB_TOKEN") or os.getenv("GITHUB_API_TOKEN")
    if token:
        headers["Authorization"] = f"Bearer {token}"

    request = urllib.request.Request(api_url, headers=headers)
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            payload = json.loads(response.read().decode("utf-8"))
    except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError, json.JSONDecodeError):
        return []

    releases = []
    for release in payload:
        tag_name = release.get("tag_name", "")
        match = _RELEASE_TAG_PATTERN.match(tag_name)
        if not match:
            continue

        assets = [
            {
                "name": asset.get("name"),
                "download_url": asset.get("browser_download_url"),
                "size": asset.get("size"),
            }
            for asset in release.get("assets", [])
        ]

        releases.append(
            {
                "product": match.group("product"),
                "display_name": match.group("product").replace("ports-", "").replace("-", " ").upper(),
                "version": match.group("version"),
                "tag_name": tag_name,
                "name": release.get("name") or tag_name,
                "published_at": release.get("published_at"),
                "html_url": release.get("html_url"),
                "assets": assets,
            }
        )

    return releases


def _normalize_manifest(manifest: dict[str, Any] | None) -> dict[str, Any]:
    releases = []
    for release in (manifest or {}).get("releases") or []:
        normalized_release = dict(release)
        if not normalized_release.get("display_name"):
            product = normalized_release.get("product", "")
            normalized_release["display_name"] = product.replace("ports-", "").replace("-", " ").upper() or product.upper()
        releases.append(normalized_release)

    releases = sorted(
        releases,
        key=lambda release: release.get("published_at") or "",
        reverse=True,
    )

    return {
        "product": (manifest or {}).get("product", "multigauge"),
        "generated_at": (manifest or {}).get("generated_at"),
        "releases": releases,
    }


def _load_manifest_payload() -> dict[str, Any]:
    manifest = _load_json_file(_MANIFEST_PATH)
    if manifest:
        return _normalize_manifest(manifest)

    repository = os.getenv("MULTIGAUGE_RELEASE_REPOSITORY") or os.getenv("GITHUB_REPOSITORY")
    if not repository:
        return _normalize_manifest(None)

    return _normalize_manifest(
        {
            "product": "multigauge",
            "generated_at": datetime.utcnow().isoformat(timespec="seconds") + "Z",
            "releases": _fetch_github_releases(repository),
        }
    )


def get_firmware_manifest(force_refresh: bool = False) -> dict[str, Any]:
    current_time = datetime.utcnow().timestamp()
    loaded_at = _CACHE["loaded_at"]
    if not force_refresh and _CACHE["payload"] is not None and loaded_at is not None:
        if current_time - loaded_at < _CACHE_TTL_SECONDS:
            return _CACHE["payload"]

    payload = _load_manifest_payload()
    _CACHE["loaded_at"] = current_time
    _CACHE["payload"] = payload
    return payload


def get_latest_firmware_release() -> dict[str, Any] | None:
    releases = get_firmware_manifest().get("releases", [])
    return releases[0] if releases else None
