from pathlib import Path
import subprocess

Import("env")  # noqa: F821

port_dir = Path(env["PROJECT_DIR"]).resolve()
root = port_dir.parents[1]
script = port_dir / "cmake" / "BootstrapDependencies.cmake"

subprocess.check_call(
    ["cmake", "-P", str(script)],
    cwd=str(root),
)
