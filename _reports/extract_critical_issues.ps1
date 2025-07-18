# Extract Critical Issues from Static Analysis Results

Write-Host "=== CRITICAL SECURITY ISSUES ANALYSIS ===" -ForegroundColor Red
Write-Host ""

# Function to log actions
function Log-Action {
    param([string]$Action)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content "_reports/bugfix_log.md" "`n### [$timestamp] $Action"
    Write-Host "[$timestamp] $Action" -ForegroundColor Yellow
}

Log-Action "Starting critical issue extraction"

# Extract critical cppcheck errors
Write-Host "CRITICAL CPPCHECK FINDINGS:" -ForegroundColor Red
Write-Host "==========================" -ForegroundColor Red

$critical_cppcheck = Get-Content "_reports/cppcheck.xml" | Select-String -Pattern "severity.*error.*msg"
$buffer_issues = $critical_cppcheck | Select-String -Pattern "Array.*accessed.*index|buffer|bounds|negative"
$pointer_issues = $critical_cppcheck | Select-String -Pattern "pointer|null|uninit|lifetime"

Write-Host "`nBUFFER/ARRAY ISSUES:" -ForegroundColor Magenta
$buffer_issues | ForEach-Object { 
    Write-Host $_.Line -ForegroundColor White
}

Write-Host "`nPOINTER/MEMORY ISSUES:" -ForegroundColor Magenta  
$pointer_issues | ForEach-Object {
    Write-Host $_.Line -ForegroundColor White
}

# Extract high-priority clang-tidy warnings
Write-Host "`nCRITICAL CLANG-TIDY FINDINGS:" -ForegroundColor Red
Write-Host "============================" -ForegroundColor Red

$clang_output = Get-Content "_reports/clang_tidy.json" | Out-String
$bugprone_issues = $clang_output -split "`n" | Select-String -Pattern "bugprone-.*warning"
$security_issues = $clang_output -split "`n" | Select-String -Pattern "security-.*warning"
$deprecated_issues = $clang_output -split "`n" | Select-String -Pattern "modernize-deprecated-headers|modernize-avoid-c-arrays"

Write-Host "`nBUGPRONE ISSUES:" -ForegroundColor Magenta
$bugprone_issues | Select-Object -First 10 | ForEach-Object {
    Write-Host $_.Line -ForegroundColor White
}

Write-Host "`nSECURITY ISSUES:" -ForegroundColor Magenta
$security_issues | Select-Object -First 10 | ForEach-Object {
    Write-Host $_.Line -ForegroundColor White
}

Write-Host "`nDEPRECATED/UNSAFE CONSTRUCTS:" -ForegroundColor Magenta
$deprecated_issues | Select-Object -First 10 | ForEach-Object {
    Write-Host $_.Line -ForegroundColor White
}

Log-Action "Critical issue extraction completed"

# Create action plan
$action_plan = @"

## PRIORITY FIXES IDENTIFIED

### High Priority (Security Critical)
1. **Array bounds violations** - Multiple negative index accesses
2. **Pointer lifetime issues** - Using pointers to out-of-scope variables
3. **Deprecated C headers** - Replace with modern C++ equivalents
4. **Buffer operations** - Review buffer access patterns

### Medium Priority (Code Quality)
1. **Include cleanup** - Remove unused headers
2. **Modern C++ constructs** - Replace C-style arrays with std::array
3. **Function signatures** - Use trailing return types

### Action Plan
1. Fix array bounds checking in critical sections
2. Replace deprecated headers (time.h -> ctime, etc.)
3. Add bounds checking to array accesses
4. Review pointer usage patterns
5. Modernize C-style constructs

"@

Add-Content "_reports/bugfix_log.md" $action_plan
Write-Host $action_plan -ForegroundColor Green
