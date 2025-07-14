[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$Config,
    [Parameter(Mandatory = $false)]
    [bool]$Clean = $false,
    [Parameter(Mandatory = $false)]
    [bool]$Log = $false
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

if($Log -eq $false) {
    Write-Host "Disabling Chip8 VM logging"
    $C_FLAGS += "-DCHIP8_NOLOG"
}

$BuildDir = @("build", $Config) -join "-"
$BuildFlags = $C_FLAGS -join " "

Write-Host "Building to $BuildDir"
Write-Host "Build flags: $BuildFlags"

cmake -S . -B $BuildDir -G "Ninja" "-DCMAKE_BUILD_TYPE=$Config" "-DCMAKE_C_FLAGS=$BuildFlags"
cmake --build "$BuildDir" --config "$Config"
