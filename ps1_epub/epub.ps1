<#
.SYNOPSIS
  EPUB → preview HTML converter for Seer (keeps the same CLI switches).

.PARAMETER InputFile
  Path to the source EPUB (or any) file.  ⟵ accepts `-i`

.PARAMETER OutputFile
  Path to the target HTML file (including “.html”).  ⟵ accepts `-o`

.PARAMETER UseBackslash
  If present, leaves Windows backslashes (`\`) in the injected paths.  ⟵ accepts `--use_backslash`

.PARAMETER RemainingArgs
  Catches any extra/empty positional args so they don’t error out.
#>
[CmdletBinding()]
param(
  [Alias('i')][Parameter(Mandatory = $true)][string]$InputFile,
  [Alias('o')][Parameter(Mandatory = $true)][string]$OutputFile,
  [Parameter(ValueFromRemainingArguments = $true)][string[]]$RemainingArgs
)

# Force UTF‑8 for console & file I/O
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$PSDefaultParameterValues['*:Encoding'] = 'UTF8'

function Normalize-Path($p) {
  return ($p -replace '\\', '/').TrimEnd('/')
}

# Load template
$assets = Join-Path $PSScriptRoot 'assets'
$template = Join-Path $assets 'index.html'
if (-not (Test-Path $template)) {
  Throw "Template not found: $template"
}
$content = Get-Content -LiteralPath $template -Raw

# Build replacement paths
$zipJs = Normalize-Path (Join-Path $assets 'jszip-3.10.1.min.js')
$epubJs = Normalize-Path (Join-Path $assets 'epub-0.3.93.min.js')
$css = Normalize-Path (Join-Path $assets 'main.css')
$inputUri = Normalize-Path $InputFile

# **Literal** replacements—no escaping of dots or backslashes
$content = $content.Replace('PLACEHOLDER_ZIPJS', $zipJs)
$content = $content.Replace('PLACEHOLDER_EPUBJS', $epubJs)
$content = $content.Replace('PLACEHOLDER_CSS', $css)
$content = $content.Replace('PLACEHOLDER_INPUT', $inputUri)

# Ensure output dir exists
$outDir = Split-Path $OutputFile -Parent
if ($outDir -and -not (Test-Path $outDir)) {
  New-Item -ItemType Directory -Path $outDir | Out-Null
}

# Write result
Set-Content -LiteralPath $OutputFile -Value $content -Encoding UTF8
Write-Host "Preview HTML generated at: $OutputFile"
