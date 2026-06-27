#!/usr/bin/env bash

set -euo pipefail

configuration="${1:-Debug}"

case "$configuration" in
  Debug|Release)
    ;;
  *)
    echo "Usage: ./build.sh [Debug|Release]" >&2
    exit 2
    ;;
esac

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

resolve_emsdk_root() {
  local candidate
  local candidates=()

  if [[ -n "${EMSDK:-}" ]]; then
    candidates+=("${EMSDK}")
  fi

  candidates+=(
    "${repo_root}/emsdk"
    "$(dirname "${repo_root}")/emsdk"
    "${repo_root}/../../emsdk"
    "${repo_root}/../../../emsdk"
  )

  for candidate in "${candidates[@]}"; do
    if [[ -f "${candidate}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" ]]; then
      (cd "${candidate}" && pwd)
      return 0
    fi
  done

  return 1
}

if [[ -z "${EMSDK:-}" ]]; then
  EMSDK="$(resolve_emsdk_root || true)"
fi

if [[ -z "${EMSDK:-}" ]]; then
  cat >&2 <<'EOF'
EMSDK was not found.

Install emsdk and either:
  1. export EMSDK=/path/to/emsdk
  2. run source /path/to/emsdk/emsdk_env.sh first, or
  3. place the emsdk folder next to this repo

Then rerun ./build.sh
EOF
  exit 1
fi

export EMSDK

if [[ -f "${EMSDK}/emsdk_env.sh" ]]; then
  # shellcheck disable=SC1090
  source "${EMSDK}/emsdk_env.sh" >/dev/null
else
  export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${PATH}"
fi

preset="wasm-debug"
if [[ "${configuration}" == "Release" ]]; then
  preset="wasm-release"
fi

cd "${repo_root}"
cmake --preset "${preset}"
cmake --build --preset "${preset}"
