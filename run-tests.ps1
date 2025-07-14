$vm_path = Resolve-Path -Path Build-Release\chip8tests.exe
$output = & $vm_path
Write-Host $output
Write-Host -ForegroundColor Green "PASSED"