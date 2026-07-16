param(
    [string]$Repository = $env:GITHUB_REPOSITORY,
    [string]$OutputPath = "apps/website/static/firmware/multigauge/manifest.json"
)

$ErrorActionPreference = "Stop"

if (-not $Repository) {
    throw "Repository is required. Set GITHUB_REPOSITORY or pass -Repository owner/name."
}

$headers = @{
    "Accept" = "application/vnd.github+json"
    "User-Agent" = "multigauge-website"
}

if ($env:GITHUB_TOKEN) {
    $headers["Authorization"] = "Bearer $env:GITHUB_TOKEN"
}

$uri = "https://api.github.com/repos/$Repository/releases?per_page=100"
$releases = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get

$parsed = foreach ($release in $releases) {
    if ($release.tag_name -notmatch '^(?<product>[^@]+)@(?<version>.+)$') {
        continue
    }

    [ordered]@{
        product = $Matches.product
        display_name = $Matches.product.Replace("ports-", "").Replace("-", " ").ToUpperInvariant()
        version = $Matches.version
        tag_name = $release.tag_name
        name = $(if ($release.name) { $release.name } else { $release.tag_name })
        published_at = $release.published_at
        html_url = $release.html_url
        assets = @(
            foreach ($asset in $release.assets) {
                [ordered]@{
                    name = $asset.name
                    download_url = $asset.browser_download_url
                    size = $asset.size
                }
            }
        )
    }
}

$manifest = [ordered]@{
    product = "multigauge"
    generated_at = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    releases = @($parsed | Sort-Object published_at -Descending)
}

$outputDir = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
$manifest | ConvertTo-Json -Depth 10 | Set-Content -Path $OutputPath -Encoding utf8
