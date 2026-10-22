$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectFile = Join-Path $projectRoot 'Project\Keil\GD32E230K8U6_UWB.uvprojx'
$buildLog = Join-Path $projectRoot 'Project\Keil\build.log'
$uv4 = 'C:\Keil_v5\UV4\UV4.exe'

if (-not (Test-Path -LiteralPath $uv4)) {
    throw "Keil uVision was not found at $uv4"
}

$process = Start-Process -FilePath $uv4 `
    -ArgumentList @('-j0', '-r', "`"$projectFile`"", '-o', "`"$buildLog`"") `
    -Wait -PassThru -WindowStyle Hidden

Get-Content -LiteralPath $buildLog
exit $process.ExitCode
