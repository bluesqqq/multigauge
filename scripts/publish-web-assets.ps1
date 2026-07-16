param(
    [string]$SourceDir = (Join-Path $PSScriptRoot "..\ports\web\dist"),
    [string]$DestinationDir = (Join-Path $PSScriptRoot "..\apps\website\static\multigauge-web")
)

$ErrorActionPreference = "Stop"

$sourceRoot = (Resolve-Path $SourceDir).Path
$destinationRoot = [System.IO.Path]::GetFullPath($DestinationDir)

if (-not (Test-Path (Join-Path $sourceRoot "js\pageRenderer.js"))) {
    throw "Missing source pageRenderer.js in $sourceRoot"
}

if (-not (Test-Path (Join-Path $sourceRoot "wasm\multigauge.js"))) {
    throw "Missing source multigauge.js in $sourceRoot"
}

if (-not (Test-Path (Join-Path $sourceRoot "wasm\multigauge.wasm"))) {
    throw "Missing source multigauge.wasm in $sourceRoot"
}

New-Item -ItemType Directory -Force -Path $destinationRoot | Out-Null
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue (Join-Path $destinationRoot "js")
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue (Join-Path $destinationRoot "wasm")

New-Item -ItemType Directory -Force -Path (Join-Path $destinationRoot "js") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $destinationRoot "wasm") | Out-Null

Copy-Item -Force (Join-Path $sourceRoot "js\pageRenderer.js") (Join-Path $destinationRoot "js\pageRenderer.js")
Copy-Item -Force (Join-Path $sourceRoot "wasm\multigauge.js") (Join-Path $destinationRoot "wasm\multigauge.js")
Copy-Item -Force (Join-Path $sourceRoot "wasm\multigauge.wasm") (Join-Path $destinationRoot "wasm\multigauge.wasm")

$sourceMap = Join-Path $sourceRoot "wasm\multigauge.wasm.map"
if (Test-Path $sourceMap) {
    Copy-Item -Force $sourceMap (Join-Path $destinationRoot "wasm\multigauge.wasm.map")
}

Write-Host "Published web bundle to $destinationRoot"
