# Static Analysis Results Parser
# Extracts high-priority security and safety issues from cppcheck.xml and clang_tidy.json

# Load XML for cppcheck results
[xml]$cppcheck = Get-Content "_reports/cppcheck.xml"

# Load JSON for clang-tidy results
$clang_tidy_content = Get-Content "_reports/clang_tidy.json" -Raw
$clang_tidy = $clang_tidy_content | ConvertFrom-Json

# Define high-priority categories
$high_priority_cppcheck = @(
    "bufferAccessOutOfBounds",
    "arrayIndexOutOfBounds", 
    "nullPointer",
    "uninitvar",
    "memleak",
    "resourceLeak",
    "useAfterFree",
    "doubleFree",
    "danglingPointer",
    "bufferOverflow",
    "invalidPointerCast",
    "invalidContainerLoop",
    "stlOutOfBounds",
    "negativeIndex",
    "possibleBufferAccessOutOfBounds",
    "uninitstring",
    "uninitdata",
    "uninitStructMember",
    "uninitMemberVar",
    "va_start_wrongParameter",
    "va_list_usedBeforeStarted",
    "va_end_missing",
    "invalidScanfArgType_int",
    "invalidPrintfArgType_int",
    "wrongPrintfScanfArgNum",
    "sprintfOverlappingData",
    "strncatUsage",
    "strncat",
    "strcpy",
    "gets",
    "sprintf",
    "vsprintf"
)

$high_priority_clang_tidy = @(
    "bugprone-*",
    "security-*",
    "clang-analyzer-security*",
    "modernize-avoid-c-arrays",
    "modernize-deprecated-headers",
    "bugprone-buffer-overflow",
    "bugprone-dangling-handle",
    "bugprone-infinite-loop",
    "bugprone-integer-division",
    "bugprone-narrowing-conversions",
    "bugprone-not-null-terminated-result",
    "bugprone-signed-char-misuse",
    "bugprone-sizeof-expression",
    "bugprone-string-constructor",
    "bugprone-use-after-move",
    "cert-*",
    "misc-include-cleaner",
    "readability-redundant-casting"
)

Write-Host "=== STATIC ANALYSIS SUMMARY ===" -ForegroundColor Green
Write-Host ""

# Parse cppcheck results
Write-Host "CPPCHECK HIGH-PRIORITY FINDINGS:" -ForegroundColor Yellow
Write-Host "================================" -ForegroundColor Yellow

$cppcheck_issues = @()
$cppcheck_count = 0

foreach ($error in $cppcheck.results.errors.error) {
    if ($error.id -in $high_priority_cppcheck -or $error.severity -eq "error") {
        $issue = [PSCustomObject]@{
            Type = "cppcheck"
            ID = $error.id
            Severity = $error.severity
            Message = $error.msg
            File = $error.location.file
            Line = $error.location.line
            Column = $error.location.column
        }
        $cppcheck_issues += $issue
        $cppcheck_count++
    }
}

# Display top cppcheck issues
$cppcheck_issues | Sort-Object Severity, ID | Select-Object -First 20 | ForEach-Object {
    Write-Host "[$($_.Severity.ToUpper())] $($_.ID): $($_.Message)" -ForegroundColor Red
    Write-Host "  File: $($_.File):$($_.Line):$($_.Column)" -ForegroundColor Gray
    Write-Host ""
}

Write-Host "CLANG-TIDY HIGH-PRIORITY FINDINGS:" -ForegroundColor Yellow
Write-Host "===================================" -ForegroundColor Yellow

# Parse clang-tidy results from the text output (since JSON might be malformed)
$clang_tidy_raw = Get-Content "_reports/clang_tidy.json" -Raw
$clang_tidy_lines = $clang_tidy_raw -split "`n"

$clang_tidy_issues = @()
$clang_tidy_count = 0

# Parse clang-tidy output manually since it's text-based
foreach ($line in $clang_tidy_lines) {
    if ($line -match "warning:|error:") {
        # Extract file path, line, column, and message
        if ($line -match "([^:]+):(\d+):(\d+):\s+(warning|error):\s+(.+)\s+\[([^\]]+)\]") {
            $file = $matches[1]
            $line_num = $matches[2] 
            $col = $matches[3]
            $severity = $matches[4]
            $message = $matches[5]
            $check = $matches[6]
            
            # Check if this is a high-priority issue
            $is_high_priority = $false
            foreach ($pattern in $high_priority_clang_tidy) {
                if ($check -like $pattern) {
                    $is_high_priority = $true
                    break
                }
            }
            
            if ($is_high_priority) {
                $issue = [PSCustomObject]@{
                    Type = "clang-tidy"
                    Check = $check
                    Severity = $severity
                    Message = $message
                    File = $file
                    Line = $line_num
                    Column = $col
                }
                $clang_tidy_issues += $issue
                $clang_tidy_count++
            }
        }
    }
}

# Display top clang-tidy issues
$clang_tidy_issues | Sort-Object Severity, Check | Select-Object -First 20 | ForEach-Object {
    Write-Host "[$($_.Severity.ToUpper())] $($_.Check): $($_.Message)" -ForegroundColor Red
    Write-Host "  File: $($_.File):$($_.Line):$($_.Column)" -ForegroundColor Gray
    Write-Host ""
}

Write-Host "=== SUMMARY ===" -ForegroundColor Green
Write-Host "Total cppcheck high-priority issues: $cppcheck_count" -ForegroundColor White
Write-Host "Total clang-tidy high-priority issues: $clang_tidy_count" -ForegroundColor White
Write-Host ""

# Create a combined summary report
$summary_report = @"
STATIC ANALYSIS SUMMARY REPORT
==============================

Analysis Date: $(Get-Date)
Tools Used: 
- cppcheck 2.17.1 (--enable=all --std=c++17)
- clang-tidy 20.1.8 (modernize-*, bugprone-*, security-* checks)

Source Directory: src/ (excluding src/thirdparty/)

HIGH-PRIORITY CATEGORIES ANALYZED:
==================================
• Buffer overflow / bound issues
• Null dereference / uninitialized use  
• Memory / resource leaks
• Insecure API use (strcat, gets, etc.)

FINDINGS SUMMARY:
================
Total cppcheck high-priority issues: $cppcheck_count
Total clang-tidy high-priority issues: $clang_tidy_count

TOP 20 HIGHEST-CONFIDENCE FINDINGS:
==================================
"@

$all_issues = @()
$all_issues += $cppcheck_issues | Select-Object -First 10
$all_issues += $clang_tidy_issues | Select-Object -First 10

$counter = 1
foreach ($issue in $all_issues) {
    if ($issue.Type -eq "cppcheck") {
        $summary_report += "`n$counter. [CPPCHECK-$($issue.Severity.ToUpper())] $($issue.ID): $($issue.Message)"
        $summary_report += "`n   Location: $($issue.File):$($issue.Line):$($issue.Column)"
    } else {
        $summary_report += "`n$counter. [CLANG-TIDY-$($issue.Severity.ToUpper())] $($issue.Check): $($issue.Message)"
        $summary_report += "`n   Location: $($issue.File):$($issue.Line):$($issue.Column)"
    }
    $counter++
}

$summary_report += "`n`nRECOMMENDATIONS:"
$summary_report += "`n==============="
$summary_report += "`n1. Address buffer overflow and bounds checking issues immediately"
$summary_report += "`n2. Initialize all variables and check for null pointers"
$summary_report += "`n3. Replace deprecated C functions with safer alternatives"
$summary_report += "`n4. Use modern C++ constructs (std::array, smart pointers)"
$summary_report += "`n5. Review memory management and resource handling"

# Save summary report
$summary_report | Out-File "_reports/static_analysis_summary.txt" -Encoding UTF8

Write-Host $summary_report
Write-Host "`nFull report saved to: _reports/static_analysis_summary.txt" -ForegroundColor Green
