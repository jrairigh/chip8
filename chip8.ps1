[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release"
)

Push-Location "build/${Config}/assets/rom"
& ..\..\chip8.exe
Pop-Location