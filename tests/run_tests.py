#!/usr/bin/env python3
import os
import sys
import subprocess
import glob

# Configuration
TEST_DIR = os.path.dirname(os.path.abspath(__file__))
SQL_DIR = os.path.join(TEST_DIR, 'sql')
EXPECTED_DIR = os.path.join(TEST_DIR, 'expected')
RESULTS_DIR = os.path.join(TEST_DIR, 'results')

# Ensure results directory exists
os.makedirs(RESULTS_DIR, exist_ok=True)

def run_test(sql_file):
    filename = os.path.basename(sql_file)
    test_name = os.path.splitext(filename)[0]
    result_file = os.path.join(RESULTS_DIR, filename.replace('.sql', '.out'))
    expected_file = os.path.join(EXPECTED_DIR, filename.replace('.sql', '.out'))

    print(f"Running test: {test_name}...", end=' ', flush=True)

    psql_bin = 'psql'
    # Try to find psql in the known install dir
    potential_paths = [
        # Relative to tests/ directory
        os.path.abspath(os.path.join(TEST_DIR, '../../postgres/pg_carbon_installed/bin/psql')),
        '/Users/chenyunwen/proj/postgres/pg_carbon_installed/bin/psql'
    ]
    
    for path in potential_paths:
        if os.path.exists(path) and os.access(path, os.X_OK):
            psql_bin = path
            break

    # Basic psql command
    # -X: no .psqlrc
    # -a: echo all input from script
    # -q: quiet (no welcome messages)
    cmd = [psql_bin, '-d', 'postgres', '-X', '-a', '-q', '-f', sql_file]

    with open(result_file, 'w') as outfile:
        try:
            subprocess.run(cmd, stdout=outfile, stderr=subprocess.STDOUT, check=True)
        except subprocess.CalledProcessError as e:
            print(f"ERROR (psql failed with code {e.returncode})")
            return False

    # Sanitization: replace absolute paths with relative ones
    with open(result_file, 'r') as f:
        content = f.read()
    
    # Replace the full SQL_DIR path with just the filename
    # psql output format is usually: psql:/path/to/file.sql:line: ...
    # We want: psql:file.sql:line: ...
    # We handle both the SQL_DIR and potential other paths if needed, 
    # but primarily ensuring the sql file path is stable.
    content = content.replace(SQL_DIR + os.sep, '')
    
    with open(result_file, 'w') as f:
        f.write(content)

    # Check against expected
    if not os.path.exists(expected_file):
        print("WARNING: No expected file found. Result saved.")
        return False
    
    # Diff
    diff_cmd = ['diff', '-u', expected_file, result_file]
    p = subprocess.run(diff_cmd, capture_output=True, text=True)
    
    if p.returncode == 0:
        print("PASS")
        return True
    else:
        print("FAIL")
        print(p.stdout)
        return False

def main():
    sql_files = sorted(glob.glob(os.path.join(SQL_DIR, '*.sql')))
    if not sql_files:
        print("No SQL tests found!")
        sys.exit(1)

    passed = 0
    failed = 0

    for sql_file in sql_files:
        if run_test(sql_file):
            passed += 1
        else:
            failed += 1

    print("-" * 40)
    print(f"Summary: {passed} PASSED, {failed} FAILED")
    
    if failed > 0:
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    main()
