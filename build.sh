#!/usr/bin/env bash
#
# build.sh - build helper for my_ls.
#
# Usage:
#   ./build.sh            build the my_ls binary (same as `make`)
#   ./build.sh clean      remove build/ and the binary
#   ./build.sh rebuild    clean, then build from scratch
#   ./build.sh docs       generate HTML documentation with Doxygen
#   ./build.sh help       show this message
#
# Environment:
#   CC=<compiler>         override the compiler (default: gcc, as in Makefile)
#   JOBS=<n>              parallel jobs for make (default: number of CPUs)

set -euo pipefail

# Always run from the project root, wherever the script was invoked from.
cd "$(dirname "${BASH_SOURCE[0]}")"

usage() {
    sed -n '3,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

require() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "error: '$1' is required but not found in PATH" >&2
        [ -n "${2:-}" ] && echo "hint: $2" >&2
        exit 1
    fi
}

detect_jobs() {
    if [ -n "${JOBS:-}" ]; then
        echo "$JOBS"
    elif command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu
    else
        echo 1
    fi
}

cmd="${1:-build}"

case "$cmd" in
    build)
        require make
        require "${CC:-gcc}" "install gcc or clang, or set CC=<compiler>"
        make -j"$(detect_jobs)" ${CC:+CC="$CC"}
        echo "built ./my_ls"
        ;;
    clean)
        require make
        make clean
        ;;
    rebuild|re)
        require make
        require "${CC:-gcc}" "install gcc or clang, or set CC=<compiler>"
        make clean
        make -j"$(detect_jobs)" ${CC:+CC="$CC"}
        echo "built ./my_ls"
        ;;
    docs)
        require doxygen "brew install doxygen (macOS) or apt install doxygen (Debian/Ubuntu)"
        make docs
        echo "documentation generated in docs/html/index.html"
        ;;
    help|-h|--help)
        usage
        ;;
    *)
        echo "error: unknown command '$cmd'" >&2
        echo >&2
        usage >&2
        exit 2
        ;;
esac
