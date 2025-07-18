#!/usr/bin/env python3
import glob
import re
import os
from collections import defaultdict

def parse_logs():
    log_files = glob.glob('reports/build/*.log')
    
    # Regex patterns for warnings and errors
    warning_patterns = [
        r'(.*):([\d]+):([\d]+): warning: (.*)',
        r'(.*):([\d]+): warning: (.*)',
        r'warning: (.*)',
        r'Warning: (.*)'
    ]
    
    error_patterns = [
        r'(.*):([\d]+):([\d]+): error: (.*)',
        r'(.*):([\d]+): error: (.*)',
        r'error: (.*)',
        r'Error: (.*)'
    ]
    
    file_counts = defaultdict(lambda: {'errors': 0, 'warnings': 0})
    pattern_counts = defaultdict(int)
    total_warnings = 0
    total_errors = 0
    
    for log_file in log_files:
        config_name = os.path.basename(log_file).replace('.log', '')
        print(f"\nAnalyzing {config_name}:")
        print("-" * 50)
        
        try:
            with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # Count warnings
                for pattern in warning_patterns:
                    matches = re.findall(pattern, content, re.MULTILINE)
                    for match in matches:
                        if isinstance(match, tuple) and len(match) >= 3:
                            file_path = match[0]
                            warning_msg = match[-1]
                        else:
                            file_path = "unknown"
                            warning_msg = str(match)
                        
                        if file_path != "unknown":
                            file_counts[file_path]['warnings'] += 1
                        
                        # Extract warning type for pattern counting
                        warning_type = warning_msg.split()[0] if warning_msg else "unknown"
                        pattern_counts[f"WARNING: {warning_type}"] += 1
                        total_warnings += 1
                
                # Count errors
                for pattern in error_patterns:
                    matches = re.findall(pattern, content, re.MULTILINE)
                    for match in matches:
                        if isinstance(match, tuple) and len(match) >= 3:
                            file_path = match[0]
                            error_msg = match[-1]
                        else:
                            file_path = "unknown"
                            error_msg = str(match)
                        
                        if file_path != "unknown":
                            file_counts[file_path]['errors'] += 1
                        
                        # Extract error type for pattern counting
                        error_type = error_msg.split()[0] if error_msg else "unknown"
                        pattern_counts[f"ERROR: {error_type}"] += 1
                        total_errors += 1
                
                config_warnings = content.count('warning:')
                config_errors = content.count('error:')
                print(f"  Warnings: {config_warnings}")
                print(f"  Errors: {config_errors}")
                
        except Exception as e:
            print(f"  Error reading {log_file}: {e}")
    
    # Summary report
    print("\n" + "="*60)
    print("SUMMARY REPORT")
    print("="*60)
    print(f"Total Warnings: {total_warnings}")
    print(f"Total Errors: {total_errors}")
    
    # Top files with most warnings/errors
    print("\nTop 10 files with most warnings/errors:")
    print("-" * 40)
    sorted_files = sorted(file_counts.items(), 
                         key=lambda x: x[1]['warnings'] + x[1]['errors'], 
                         reverse=True)[:10]
    
    for file_path, counts in sorted_files:
        print(f"{file_path}: {counts['warnings']} warnings, {counts['errors']} errors")
    
    # Top recurring patterns
    print("\nTop 10 recurring warning/error patterns:")
    print("-" * 40)
    sorted_patterns = sorted(pattern_counts.items(), key=lambda x: x[1], reverse=True)[:10]
    
    for pattern, count in sorted_patterns:
        print(f"{pattern}: {count} occurrences")
    
    # Write detailed report
    with open('reports/build/analysis_report.txt', 'w') as f:
        f.write("FluffOS Build Analysis Report\n")
        f.write("="*50 + "\n\n")
        f.write(f"Total Warnings: {total_warnings}\n")
        f.write(f"Total Errors: {total_errors}\n\n")
        
        f.write("Files with warnings/errors:\n")
        f.write("-" * 30 + "\n")
        for file_path, counts in sorted_files:
            f.write(f"{file_path}: {counts['warnings']} warnings, {counts['errors']} errors\n")
        
        f.write("\nRecurring patterns:\n")
        f.write("-" * 20 + "\n")
        for pattern, count in sorted_patterns:
            f.write(f"{pattern}: {count} occurrences\n")
    
    print(f"\nDetailed report saved to: reports/build/analysis_report.txt")

if __name__ == "__main__":
    parse_logs()
