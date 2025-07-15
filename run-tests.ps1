$vm_path = Resolve-Path -Path Build-Release\chip8tests.exe
$output = & $vm_path
$outputArray = $output -split "`n"

for ($i = 0; $i -lt $outputArray.Length; $i += 2) {
    $testName = $outputArray[$i]
    $dots = '.' * (40 - $testName.Length)
    Write-Host -NoNewline "$testName$dots"
    $line = $outputArray[$i + 1]
    if ($line -match "PASSED") {
        Write-Host $line -NoNewline -ForegroundColor Green
    } else {
        Write-Host $line -NoNewline -ForegroundColor Red
    }
    Write-Host
}

Write-Host