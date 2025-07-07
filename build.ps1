[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$config,
    [Parameter(Mandatory = $false)]
    [bool]$clean = $false
)

if ($clean) {
    Remove-Item -Recurse -Force build-$config
}

if($config -eq "Debug") {
    Write-Host "Building in Debug mode..."
    cmake -S . -B build-Debug -G "Ninja" -DCMAKE_BUILD_TYPE="Debug"
} else {
    Write-Host "Building in Release mode..."
    cmake -S . -B build-Release -G "Ninja" -DCMAKE_BUILD_TYPE="Release"
}

cmake --build build-$config --config $config