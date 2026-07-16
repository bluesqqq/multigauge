#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="${1:-"${script_dir}/../ports/web/dist"}"
dest_dir="${2:-"${script_dir}/../apps/website/static/multigauge-web"}"

if [[ ! -f "${source_dir}/js/pageRenderer.js" ]]; then
  echo "Missing source pageRenderer.js in ${source_dir}" >&2
  exit 1
fi

if [[ ! -f "${source_dir}/wasm/multigauge.js" ]]; then
  echo "Missing source multigauge.js in ${source_dir}" >&2
  exit 1
fi

if [[ ! -f "${source_dir}/wasm/multigauge.wasm" ]]; then
  echo "Missing source multigauge.wasm in ${source_dir}" >&2
  exit 1
fi

rm -rf "${dest_dir}/js" "${dest_dir}/wasm"
mkdir -p "${dest_dir}/js" "${dest_dir}/wasm"

cp -f "${source_dir}/js/pageRenderer.js" "${dest_dir}/js/pageRenderer.js"
cp -f "${source_dir}/wasm/multigauge.js" "${dest_dir}/wasm/multigauge.js"
cp -f "${source_dir}/wasm/multigauge.wasm" "${dest_dir}/wasm/multigauge.wasm"

if [[ -f "${source_dir}/wasm/multigauge.wasm.map" ]]; then
  cp -f "${source_dir}/wasm/multigauge.wasm.map" "${dest_dir}/wasm/multigauge.wasm.map"
fi

echo "Published web bundle to ${dest_dir}"
