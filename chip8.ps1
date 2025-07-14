
[CmdletBinding()]
param ( 
    [Parameter(Mandatory = $true)]
    [string]$rom
)

$vm_path = Resolve-Path -Path Build-Release\chip8.exe
Push-Location "rom"
& $vm_path "$rom"
Pop-Location