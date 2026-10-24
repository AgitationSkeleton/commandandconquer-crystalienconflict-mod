# Recompile both gameplay DLLs without overwriting the tested release inputs.
param([string]$MSBuild = '')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $MSBuild) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $MSBuild = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild Microsoft.VisualStudio.Component.VC.v141.x86.x64 -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    }
}
if (-not $MSBuild -or -not (Test-Path -LiteralPath $MSBuild)) { throw 'Pass -MSBuild with the full path to MSBuild.exe' }
$originalCL = $env:CL
try {
    foreach ($name in @('CrystAlienConflict', 'CrystAlienConflictPlus')) {
        $env:CL = $originalCL
        if ($name -eq 'CrystAlienConflictPlus') { $env:CL = "$originalCL /DCAC_SUPPLEMENTAL" }
        $binaryDir = Join-Path $repoRoot ".build\bin\$name\"
        $objectDir = Join-Path $repoRoot ".build\obj\$name\"
        New-Item -ItemType Directory -Force -Path $binaryDir,$objectDir | Out-Null
        & $MSBuild (Join-Path $repoRoot 'src\TiberianDawn\TiberianDawn.vcxproj') /t:Rebuild /p:Configuration=Release /p:Platform=Win32 "/p:OutDir=$binaryDir" "/p:IntDir=$objectDir" /m /v:minimal /nologo
        if ($LASTEXITCODE -ne 0) { throw "DLL compilation failed for $name" }
        if (-not (Test-Path -LiteralPath (Join-Path $binaryDir 'TiberianDawn.dll'))) { throw "DLL missing for $name" }
        Write-Output "Recompiled $name"
    }
}
finally { $env:CL = $originalCL }
Write-Output 'Run python build.py --rebuilt-dlls to package the recompiled binaries.'
