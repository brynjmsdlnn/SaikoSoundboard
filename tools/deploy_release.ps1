# PowerShell Deployment Script for SaikoSoundboard Beta v0.1.1

$ErrorActionPreference = "Stop"

$ProjectRoot = Resolve-Path "$PSScriptRoot\.."
$QtPrefix = "C:\Qt\6.11.1\mingw_64"
$QtBin = "$QtPrefix\bin"
$WinDeployQt = "$QtBin\windeployqt.exe"
$BuildDir = "$ProjectRoot\build_release"
$DistDir = "$ProjectRoot\dist"
$StagingDir = "$DistDir\SaikoSoundboard-v0.1.1-beta"
$InstallerScript = "$ProjectRoot\installer\SaikoSoundboard.iss"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " SaikoSoundboard Release Deployment v0.1.1" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Terminate any running instances of SaikoSoundboard to prevent file locking
Get-Process SaikoSoundboard* -ErrorAction SilentlyContinue | Stop-Process -Force


$CMakeBin = "C:\Qt\Tools\CMake_64\bin"

# 1. Ensure PATH has CMake, Qt bin and MinGW bin
$env:PATH = "$CMakeBin;$QtBin;$QtPrefix\..\..\Tools\mingw1310_64\bin;" + $env:PATH

# 2. Configure CMake Release
Write-Host "`n[1/5] Configuring CMake (Release)..." -ForegroundColor Yellow
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

cmake -B $BuildDir -S $ProjectRoot -G "Ninja" `
    -DCMAKE_PREFIX_PATH="$QtPrefix" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_AUTOMOC_COMPILER_PREDEFINES=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
}

# 3. Build Executable
Write-Host "`n[2/5] Compiling SaikoSoundboard Release Target..." -ForegroundColor Yellow
cmake --build $BuildDir --config Release --target SaikoSoundboard

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
}

if (Test-Path $StagingDir) {
    Get-ChildItem -Path $StagingDir -Recurse -Force | ForEach-Object {
        if ($_.Attributes -match "ReadOnly") {
            $_.Attributes = "Normal"
        }
    }
    Remove-Item -Recurse -Force $StagingDir
}
New-Item -ItemType Directory -Path $StagingDir | Out-Null

$ExePath = "$BuildDir\SaikoSoundboard.exe"
if (-not (Test-Path $ExePath)) {
    Write-Error "Executable not found at $ExePath"
}

Copy-Item $ExePath -Destination $StagingDir
Copy-Item "$ProjectRoot\README.md" -Destination $StagingDir

# 5. Run windeployqt
Write-Host "`n[4/5] Running windeployqt to deploy runtime DLLs and QML modules..." -ForegroundColor Yellow
if (Test-Path $WinDeployQt) {
    & $WinDeployQt --qmldir "$ProjectRoot\src\qml" --compiler-runtime "$StagingDir\SaikoSoundboard.exe"
} else {
    Write-Warning "windeployqt.exe not found at $WinDeployQt. Relying on system PATH."
    windeployqt --qmldir "$ProjectRoot\src\qml" --compiler-runtime "$StagingDir\SaikoSoundboard.exe"
}

# 4.5 Optimize Deployed Folder Size (Clean unused QML styles and translations)
Write-Host "`nOptimizing deployment size..." -ForegroundColor Yellow

# Keep only English translations
$TranslationsDir = "$StagingDir\translations"
if (Test-Path $TranslationsDir) {
    Get-ChildItem -Path $TranslationsDir -File | ForEach-Object {
        if ($_.Name -ne "qt_en.qm") {
            Remove-Item $_.FullName -Force
        }
    }
}

# Remove unused QML style folders
$ControlsDir = "$StagingDir\qml\QtQuick\Controls"
if (Test-Path $ControlsDir) {
    $UnusedStyles = @("FluentWinUI3", "Fusion", "Imagine", "Material", "Universal", "Windows")
    foreach ($style in $UnusedStyles) {
        $path = Join-Path $ControlsDir $style
        if (Test-Path $path) {
            Remove-Item $path -Recurse -Force
        }
    }
}

# Remove unused style plugins and DLLs
$UnusedDlls = @(
    "Qt6QuickControls2FluentWinUI3StyleImpl.dll",
    "Qt6QuickControls2FusionStyleImpl.dll",
    "Qt6QuickControls2ImagineStyleImpl.dll",
    "Qt6QuickControls2MaterialStyleImpl.dll",
    "Qt6QuickControls2UniversalStyleImpl.dll",
    "Qt6QuickControls2WindowsStyleImpl.dll",
    "qtquickcontrols2fluentwinui3styleplugin.dll",
    "qtquickcontrols2fluentwinui3styleimplplugin.dll",
    "qtquickcontrols2fusionstyleplugin.dll",
    "qtquickcontrols2fusionstyleimplplugin.dll",
    "qtquickcontrols2imaginestyleplugin.dll",
    "qtquickcontrols2imaginestyleimplplugin.dll",
    "qtquickcontrols2materialstyleplugin.dll",
    "qtquickcontrols2materialstyleimplplugin.dll",
    "qtquickcontrols2universalstyleplugin.dll",
    "qtquickcontrols2universalstyleimplplugin.dll",
    "qtquickcontrols2windowsstyleplugin.dll",
    "qtquickcontrols2windowsstyleimplplugin.dll",
    "opengl32sw.dll"
)

foreach ($dll in $UnusedDlls) {
    $path = Get-ChildItem -Path $StagingDir -Filter $dll -Recurse -ErrorAction SilentlyContinue
    if ($path) {
        $path | Remove-Item -Force
    }
}

# 6. Create Zip Archive
Write-Host "`n[5/5] Creating portable ZIP distribution..." -ForegroundColor Yellow
$ZipPath = "$DistDir\SaikoSoundboard-v0.1.1-beta-portable.zip"
if (Test-Path $ZipPath) { Remove-Item -Force $ZipPath }
Compress-Archive -Path "$StagingDir\*" -DestinationPath $ZipPath

Write-Host "`nPortable release zip created: $ZipPath" -ForegroundColor Green

# 7. Check Inno Setup for EXE Installer
$Iscc = Get-Command "iscc.exe" -ErrorAction SilentlyContinue
if (-not $Iscc) {
    $CommonInnoPaths = @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles}\Inno Setup 6\ISCC.exe",
        "${env:LocalAppData}\Programs\Inno Setup 6\ISCC.exe"
    )
    foreach ($p in $CommonInnoPaths) {
        if (Test-Path $p) {
            $Iscc = $p
            break
        }
    }
}

if ($Iscc) {
    Write-Host "`nCompiling Windows Installer with Inno Setup..." -ForegroundColor Yellow
    & $Iscc $InstallerScript
    Write-Host "Windows Setup Installer created in $DistDir" -ForegroundColor Green
} else {
    Write-Host "`n[INFO] Inno Setup compiler (iscc.exe) not found on system." -ForegroundColor Cyan
    Write-Host "Portable zip package is ready in $DistDir" -ForegroundColor Cyan
}

Write-Host "`nRelease deployment complete!" -ForegroundColor Green
