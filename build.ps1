[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",
    [Parameter(Mandatory = $false)]
    [bool]$Clean = $false,
    [Parameter(Mandatory = $false)]
    [ValidateSet("debug", "info", "warn", "error")]
    [string]$Log = "info",
    [Parameter(Mandatory = $false)]
    [bool]$Step = $false
)

if ($Clean) {
    Write-Host "Clean build"
    Remove-Item -Recurse -Force build-$Config
}

$C_FLAGS = @()

if($Config -eq "Debug") {
    Write-Host "Configuring for Debug build"
} else {
    Write-Host "Configuring for Release build"
    $C_FLAGS += "-DNDEBUG=1"
}

if($Step) {
    Write-Host "Step mode enabled"
    $C_FLAGS += "-DCHIP8_STEP=1"
} else {
    Write-Host "Step mode disabled"
}

switch ($Log) {
    "debug" { $C_FLAGS += "-DCHIP8_LOGLEVEL=0"; Write-Host "Log level: debug" }
    "info"  { $C_FLAGS += "-DCHIP8_LOGLEVEL=1"; Write-Host "Log level: info" }
    "warn"  { $C_FLAGS += "-DCHIP8_LOGLEVEL=2"; Write-Host "Log level: warn" }
    "error" { $C_FLAGS += "-DCHIP8_LOGLEVEL=3"; Write-Host "Log level: error" }
    default { Write-Host "Unknown log level: $Log" }
}

$BuildDir = @("build", $Config) -join "-"
$BuildFlags = $C_FLAGS -join " "

Write-Host "Building to $BuildDir"
Write-Host "Build flags: $BuildFlags"

cmake -S . -B $BuildDir -G "Ninja" "-DCMAKE_BUILD_TYPE=$Config" "-DCMAKE_C_FLAGS=$BuildFlags"
cmake --build "$BuildDir" --config "$Config"
