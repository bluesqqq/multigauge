from pathlib import Path
import subprocess

Import("env")  # noqa: F821

root = Path(env["PROJECT_DIR"]).resolve().parents[1]
script = root / "cmake" / "MultigaugeDependencies.cmake"

subprocess.check_call(
    ["cmake", "-P", str(script)],
    cwd=str(root),
)
