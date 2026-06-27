param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$Reconfigure
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Resolve-EmsdkRoot {
    $candidates = @()

    if ($env:EMSDK) {
        $candidates += $env:EMSDK
    }

    $candidates += @(
        (Join-Path $repoRoot "emsdk"),
        (Join-Path (Split-Path $repoRoot -Parent) "emsdk")
    )
    $candidates += @(
        (Join-Path $repoRoot "..\..\emsdk"),
        (Join-Path $repoRoot "..\..\..\emsdk")
    )

    foreach ($candidate in $candidates) {
        if (-not $candidate) {
            continue
        }

        $toolchain = Join-Path $candidate "upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake"
        if (Test-Path $toolchain) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw @"
EMSDK was not found.

Install emsdk and either:
  1. set the EMSDK environment variable, or
  2. place the emsdk folder next to this repo.

Then rerun .\build.ps1
"@
}

function Prepend-Path([string]$PathEntry) {
    if (-not $PathEntry -or -not (Test-Path $PathEntry)) {
        return
    }

    $currentEntries = $env:PATH -split ";"
    if ($currentEntries -contains $PathEntry) {
        return
    }

    $env:PATH = "$PathEntry;$env:PATH"
}

$emsdkRoot = Resolve-EmsdkRoot
$env:EMSDK = $emsdkRoot

$emsdkEnvScript = Join-Path $emsdkRoot "emsdk_env.ps1"
if (Test-Path $emsdkEnvScript) {
    . $emsdkEnvScript | Out-Null
} else {
    $emscriptenDir = Join-Path $emsdkRoot "upstream\emscripten"
    Prepend-Path $emsdkRoot
    Prepend-Path $emscriptenDir

    $nodeDir = Get-ChildItem (Join-Path $emsdkRoot "node") -Directory -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Select-Object -First 1
    if ($nodeDir) {
        $nodeBin = Join-Path $nodeDir.FullName "bin"
        Prepend-Path $nodeBin

        $nodeExe = Join-Path $nodeBin "node.exe"
        if (Test-Path $nodeExe) {
            $env:EMSDK_NODE = $nodeExe
        }
    }

    $pythonDir = Get-ChildItem (Join-Path $emsdkRoot "python") -Directory -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Select-Object -First 1
    if ($pythonDir) {
        Prepend-Path $pythonDir.FullName

        $pythonExe = Join-Path $pythonDir.FullName "python.exe"
        if (Test-Path $pythonExe) {
            $env:EMSDK_PYTHON = $pythonExe
        }
    }
}

$buildDir = Join-Path $repoRoot "build\wasm-debug"

function Stop-StaleBuildProcesses {
    $patterns = @(
        (Join-Path $repoRoot "*"),
        (Join-Path $buildDir "*")
    )

    function Test-BuildProcessMatches([string]$CommandLine) {
        foreach ($pattern in $patterns) {
            if ($pattern -and $CommandLine -like $pattern) {
                return $true
            }
        }

        return $false
    }

    $processes = Get-CimInstance Win32_Process -ErrorAction SilentlyContinue |
        Where-Object {
            ($_.Name -ieq "ninja.exe" -or $_.Name -ieq "cmake.exe") -and $_.CommandLine -and (Test-BuildProcessMatches $_.CommandLine)
        }

    foreach ($process in $processes) {
        try {
            Stop-Process -Id $process.ProcessId -Force -ErrorAction SilentlyContinue
        } catch {}
    }
}

Stop-StaleBuildProcesses

$preset = if ($Configuration -eq "Release") { "wasm-release" } else { "wasm-debug" }
$buildDir = Join-Path $repoRoot "build\$preset"
$cacheFile = Join-Path $buildDir "CMakeCache.txt"

Push-Location $repoRoot
try {
    if ($Reconfigure -or -not (Test-Path $cacheFile)) {
        cmake --preset $preset
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }

    cmake --build --preset $preset
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
