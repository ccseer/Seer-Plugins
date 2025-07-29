<#
.SYNOPSIS
  Generate a Jupyter‑Notebook preview HTML for Seer by injecting the proper asset paths and the .ipynb filename.

.PARAMETER InputFile
  Path to the source .ipynb file.  (uses `-i`)

.PARAMETER OutputFile
  Path to the target HTML file (including “.html”).  (uses `-o`)

Any extra/empty args (e.g. an unused `${use_backslash}` placeholder) are swallowed.
#>
[CmdletBinding()]
param(
    [Alias('i')][Parameter(Mandatory = $true)][string]$InputFile,
    [Alias('o')][Parameter(Mandatory = $true)][string]$OutputFile)

# — Force UTF‑8 for console & file I/O —
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$PSDefaultParameterValues['*:Encoding'] = 'UTF8'

# — Locate script root and assets folder —
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Definition
$assetsDir = Join-Path $scriptRoot 'assets'
$template = Join-Path $assetsDir 'index.html'
if (-not (Test-Path $template)) {
    Throw "Template not found: $template"
}

# — Helper to normalize or preserve backslashes —
function Normalize-Path($p) {
    return ($p -replace '\\', '/').TrimEnd('/')
}

# — Load template —
$content = Get-Content -LiteralPath $template -Raw

# — Compute the folder URI (forward‑slashes only) —
$assetsUri = ($assetsDir -replace '\\', '/')

# — Perform **literal** placeholder replacements —  
$content = $content.Replace(
    'PLACEHOLDER_FOLDER',
    $assetsUri
)
$notebookJson = Get-Content -LiteralPath $InputFile -Raw
$notebookJson = $notebookJson -replace "(\r\n|\n|\r)", ""

$content = $content.Replace('PLACEHOLDER_INPUT', $notebookJson)

# — Ensure output directory exists —
$outDir = Split-Path $OutputFile -Parent
if ($outDir -and -not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

# — Write the resulting HTML —
Set-Content -LiteralPath $OutputFile -Value $content -Encoding UTF8
Write-Host "Notebook preview generated at: $OutputFile"
