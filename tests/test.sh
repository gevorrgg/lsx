#!/usr/bin/env bash
#
# Compares the output of lsx with the system ls for a set of flag
# combinations. Both commands run inside a pseudo-terminal (via `script`)
# so that column layouts match what a user would see, and tabs are expanded
# before comparison. Run from the tests/ directory after copying the
# lsx binary here (or symlinking it): ./test.sh
#
# Exit status: 0 if every case matches, 1 otherwise, 2 if lsx is missing.

set -euo pipefail

MINI_LS="./lsx"

if [[ ! -x "$MINI_LS" ]]; then
    echo "Error: '$MINI_LS' not found or is not executable"
    exit 2
fi

TMP_DIR="out"
mkdir "$TMP_DIR"
trap 'rm -rf "$TMP_DIR"' EXIT

MINI_OUT="$TMP_DIR/mini.out"
LS_OUT="$TMP_DIR/ls.out"

# run_in_pty OUTPUT_FILE COMMAND...
# Runs COMMAND in a pty and stores its output (minus script's own header
# and footer lines) in OUTPUT_FILE.
run_in_pty() {
    local output_file=$1
    shift

	script -q -c "$*" /dev/null \
	| sed '1d;$d' \
	> "$output_file"

	#sed -i 's/\t/        /g' $output_file
}

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

passed=0
failed=0
# test_case NAME ARGS...
# Runs "lsx ARGS" and "ls ARGS", prints a gtest-style status line and a
# unified diff on mismatch.
test_case() {
    local test_name=$1
    shift

    echo -e "${GREEN}[ RUN      ] $test_name"

    run_in_pty "$MINI_OUT" "$MINI_LS $*"
    run_in_pty "$LS_OUT" "ls $*"

    expand -t 8 "$MINI_OUT" > "$MINI_OUT.tmp"
    mv "$MINI_OUT.tmp" "$MINI_OUT"

    expand -t 8 "$LS_OUT" > "$LS_OUT.tmp"
    mv "$LS_OUT.tmp" "$LS_OUT"    
	
    if cmp -s "$MINI_OUT" "$LS_OUT"; then
        echo "[       OK ] $test_name"
        passed=$((passed + 1))
		echo -e "${GREEN}[       OK ] $test_name"
    else
        echo -e "${RED}[  FAILED  ] $test_name"
        failed=$((failed + 1))

        echo
        diff -u "$LS_OUT" "$MINI_OUT" || true
        echo
    fi
}

echo "[==========] Running tests."

# Basic
test_case "LsxTest.Default" .
test_case "LsxTest.Root" /

# Format options
test_case "LsxTest.LongFormat" -l /
test_case "LsxTest.OnePerLineFormat" -1 /
test_case "LsxTest.CommaFormat" -m /
test_case "LsxTest.HorizontalFormat" -x /
test_case "LsxTest.ColumnsFormat" -C /

# Hidden files
test_case "LsxTest.ShowAll" -a /
test_case "LsxTest.ShowAlmostAll" -A /

# Recursion
test_case "LsxTest.Recursive" -R .

# Directory handling
test_case "LsxTest.DirectoryItself" -d /
test_case "LsxTest.DirectoryItselfLong" -ld /

# Sorting
test_case "LsxTest.SortBySize" -S /
test_case "LsxTest.SortByTime" -t /
test_case "LsxTest.SortReversed" -r /
test_case "LsxTest.NoSort" -U /
test_case "LsxTest.UnsortedShowAll" -f /

# Metadata display
test_case "LsxTest.ShowInode" -i /
test_case "LsxTest.NumericIds" -n /
test_case "LsxTest.NoOwner" -g /
test_case "LsxTest.NoGroup" -o /
test_case "LsxTest.ShowBlockSizes" -s /

# File type indicators
test_case "LsxTest.ClassifyAll" -F /
test_case "LsxTest.ClassifyDirectories" -p /

# Time selection
test_case "LsxTest.AccessTime" -u -l /
test_case "LsxTest.ChangeTime" -c -l /

# Symbolic links
test_case "LsxTest.FollowSymlinks" -L /

# Combined options
test_case "LsxTest.LongRecursiveAll" -laR ./test_dir
test_case "LsxTest.InodeLongAll" -ila /
test_case "LsxTest.SortTimeReverse" -tr ./test_dir
test_case "LsxTest.SortSizeReverse" -Sr ./test_dir
test_case "LsxTest.LongWithIndicators" -lF ./test_dir
test_case "LsxTest.BlocksHumanAll" -sah ./test_dir

# Multiple arguments
test_case "LsxTest.MultipleFiles" /etc/passwd /etc/hosts
test_case "LsxTest.MultipleDirs" /tmp /var
test_case "LsxTest.MixedFilesAndDirs" /etc/passwd /tmp

# Non-existing files
test_case "LsxTest.NonExistent" does_not_exist
test_case "LsxTest.MixedExistingAndMissing" /tmp does_not_exist

# Symlinks
test_case "LsxTest.SymlinkLong" -l test_dir/s1
test_case "LsxTest.SymlinkFollow" -lL test_dir/s2
test_case "LsxTest.BrokenSymlink" -l test_dir/s1

# Empty dirs
mkdir -p empty_dir
test_case "LsxTest.EmptyDirectory" empty_dir
test_case "LsxTest.EmptyDirectoryLong" -l empty_dir
rm -rf empty_dir

# Directory with no access
test_case "LsxTest.PermissionDeniedRecursive" -R test_dir/d

# Conflicting flags
test_case "LsxTest.LongAndNumeric" -ln /
test_case "LsxTest.AllAndDirectory" -ad test_dir
test_case "LsxTest.SortTimeSize" -tS test_dir

# Special files
test_case "LsxTest.DevNull" -l /dev/null
test_case "LsxTest.DevZero" -l /dev/zero

echo "[==========] Test run complete."

if [[ $failed -eq 0 ]]; then
    echo -e "${GREEN}[  PASSED  ] $passed test(s).${NC}"
else
    echo -e "${GREEN}[  PASSED  ] $passed test(s).${NC}"
    echo -e "${RED}[  FAILED  ] $failed test(s).${NC}"
    exit 1
fi
