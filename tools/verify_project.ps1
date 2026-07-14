<#
.SYNOPSIS
    Architecture verification for SaikoSoundboard.
    Checks that project sources comply with invariant architecture rules.

.DESCRIPTION
    This script recursively inspects `src/` (excluding specific directories)
    and reports violations of architecture rules. It is intended to be run
    manually before merging or releasing - never during normal builds.

    Currently checks:
      - Logging Rules: Qt logging APIs must not appear outside src/logging/
      - Singleton Rules: Logger::instance() must not appear outside src/logging/
      - Include Rules:  logging/Logger.h must not be included outside src/logging/

    Future checks (e.g. Module Rules, Dependency Rules) should be added as
    new script blocks following the same pattern - see the Parser return
    type and the dispatch section at the bottom.

.PARAMETER SourceDir
    Path to the src/ directory. Defaults to "$PSScriptRoot\..\src" (resolved).

.EXAMPLE
    .\tools\verify_project.ps1
    # Runs all checks against the default src/ directory.

.EXAMPLE
    .\tools\verify_project.ps1 -SourceDir C:\MyProject\src
    # Runs against an explicit src/ path.

.OUTPUTS
    Prints grouped violation reports to stdout.
    Exit code 0 = PASS (no violations).
    Exit code 1 = FAIL (one or more violations found).
#>

param(
    [string]$SourceDir = (Resolve-Path "$PSScriptRoot\..\src")
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

$ErrorActionPreference = 'Continue'

# Colours - use simple VT escape codes (supported in Windows Terminal / VS Code).
$cHeader  = [char]27 + '[1;36m'   # bold cyan    - section header
$cPass    = [char]27 + '[1;32m'   # bold green   - pass message
$cFail    = [char]27 + '[1;31m'   # bold red     - fail message
$cFile    = [char]27 + '[33m'     # yellow       - file path
$cLine    = [char]27 + '[90m'     # bright black - line number
$cReset   = [char]27 + '[0m'

# Directories excluded from all scans (relative to $SourceDir).
$ExcludedDirs = @('logging')

# ---------------------------------------------------------------------------
# Parser - returns structured violation objects
# ---------------------------------------------------------------------------

function Find-Violations {
    param(
        [string]$Pattern,
        [string]$Label,
        [string[]]$FilePattern = @('*.cpp', '*.h', '*.hpp')
    )

    $results = Get-ChildItem -Path $SourceDir -Recurse -Include $FilePattern |
               Where-Object {
                   $relative = $_.FullName.Substring($SourceDir.Length).TrimStart('\')
                   $exclude = $false
                   foreach ($ed in $ExcludedDirs) {
                       if ($relative -like "$ed\*" -or $relative -eq $ed) {
                           $exclude = $true
                           break
                       }
                   }
                   -not $exclude
               } |
               Select-String -Pattern $Pattern |
               ForEach-Object {
                   [PSCustomObject]@{
                       File  = $_.Path
                       Line  = $_.LineNumber
                       Text  = $_.Line.Trim()
                       Label = $Label
                   }
               }

    return ,$results
}

# ---------------------------------------------------------------------------
# Check blocks - each returns a list of violations
# ---------------------------------------------------------------------------

function Invoke-LoggingRulesCheck {
    Write-Host ($cHeader + '[Logging Rules]' + $cReset)
    Write-Host '  Checking for Qt logging APIs outside src/logging/ ...'

    $patterns = @(
        @{ Pattern = 'qDebug\s*\(';   Label = 'qDebug() call' }
        @{ Pattern = 'qInfo\s*\(';    Label = 'qInfo() call' }
        @{ Pattern = 'qWarning\s*\('; Label = 'qWarning() call' }
        @{ Pattern = 'qCritical\s*\(';Label = 'qCritical() call' }
        @{ Pattern = 'qFatal\s*\(';   Label = 'qFatal() call' }
        @{ Pattern = 'qCDebug\s*\(';  Label = 'qCDebug() call' }
        @{ Pattern = 'qCInfo\s*\(';   Label = 'qCInfo() call' }
        @{ Pattern = 'qCWarning\s*\(';Label = 'qCWarning() call' }
        @{ Pattern = 'qCCritical\s*\(';Label = 'qCCritical() call' }
    )

    $all = @()
    foreach ($p in $patterns) {
        $results = Find-Violations -Pattern $p.Pattern -Label $p.Label
        $all += $results
    }

    return ,$all
}

function Invoke-SingletonRulesCheck {
    Write-Host ($cHeader + '[Singleton Rules]' + $cReset)
    Write-Host '  Checking for Logger::instance() outside src/logging/ ...'

    return Find-Violations -Pattern 'Logger::instance\s*\(' -Label 'Logger::instance() call'
}

function Invoke-IncludeRulesCheck {
    Write-Host ($cHeader + '[Include Rules]' + $cReset)
    Write-Host '  Checking for logging/Logger.h includes outside src/logging/ ...'

    return Find-Violations -Pattern '#include\s+["<]logging/Logger\.h[">]' -Label 'direct Logger.h include'
}

# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------

function Write-ViolationReport {
    param(
        [array]$Violations
    )

    if ($Violations.Count -eq 0) {
        Write-Host ('  ' + $cPass + 'PASS' + $cReset + '  - 0 violations')
        Write-Host ''
        return
    }

    Write-Host ('  ' + $cFail + 'FAIL' + $cReset + '  - ' + $Violations.Count + ' violation(s)')
    Write-Host ''

    $groups = $Violations | Group-Object -Property Label
    foreach ($g in $groups) {
        Write-Host ('    ' + $cFail + $g.Name + $cReset + ' (' + $g.Count + ' occurrence(s))')
        foreach ($v in $g.Group) {
            Write-Host ('      ' + $cFile + $v.File + $cReset + ':' + $cLine + $v.Line + $cReset)
            Write-Host ('        -> ' + $v.Text)
        }
        Write-Host ''
    }
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

Write-Host '============================================'
Write-Host ' SaikoSoundboard - Architecture Verification'
Write-Host '============================================'
Write-Host ''
Write-Host ('Source directory : ' + $SourceDir)
Write-Host ('Excluded dirs    : ' + ($ExcludedDirs -join ', '))
Write-Host ''

$globalViolations = @()

# Run all check blocks in order.
# To add a new check, write a new Invoke-* function and add it here.

$violations = Invoke-LoggingRulesCheck
Write-ViolationReport -Violations $violations
$globalViolations += $violations

$violations = Invoke-SingletonRulesCheck
Write-ViolationReport -Violations $violations
$globalViolations += $violations

$violations = Invoke-IncludeRulesCheck
Write-ViolationReport -Violations $violations
$globalViolations += $violations

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

Write-Host '============================================'
if ($globalViolations.Count -eq 0) {
    Write-Host ($cPass + ' RESULT: PASS' + $cReset)
    Write-Host '  All architecture rules are satisfied.'
    Write-Host '============================================'
    exit 0
} else {
    Write-Host ($cFail + ' RESULT: FAIL' + $cReset)
    Write-Host ('  ' + $globalViolations.Count + ' total violation(s) found.')
    Write-Host '  Fix the issues above before merging or releasing.'
    Write-Host '============================================'
    exit 1
}
