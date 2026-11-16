#!/usr/bin/env bash
#
# run_tests.sh - build my_ls and run the test suite (tests/test.sh).
#
# The suite compares my_ls with GNU ls and relies on the Linux version of
# `script`, so it only gives meaningful results on Linux. On other systems
# (macOS) the tests are run inside an Ubuntu container via Docker.
#
# Usage:
#   ./run_tests.sh            Linux: run locally; otherwise: run in Docker
#   ./run_tests.sh --local    force a local run
#   ./run_tests.sh --docker   force a run in Docker
#   ./run_tests.sh --help     show this message
#
# Environment:
#   TEST_TIMEOUT=<seconds>    abort the suite after this long (default: 300)
#   DOCKER_IMAGE=<image>      image for Docker runs (default: ubuntu:24.04)
#
# Exit code: 0 if every test passed, non-zero otherwise.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_TIMEOUT="${TEST_TIMEOUT:-300}"
DOCKER_IMAGE="${DOCKER_IMAGE:-ubuntu:24.04}"

usage() {
    sed -n '3,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

die() {
    echo "error: $1" >&2
    [ -n "${2:-}" ] && echo "hint: $2" >&2
    exit 1
}

# Build, copy the binary next to test.sh and run the suite from tests/.
# Used directly on Linux and, inside the container, for Docker runs.
run_local() {
    [ "$(uname -s)" = "Linux" ] || echo "warning: not on Linux, results will not match GNU ls" >&2

    command -v make >/dev/null 2>&1 || die "'make' not found"

    echo "==> building"
    make -C "$ROOT" >/dev/null

    cp "$ROOT/my_ls" "$ROOT/tests/my_ls"

    # test.sh creates tests/out and fails if it already exists, which
    # happens after an interrupted run.
    rm -rf -- "$ROOT/tests/out"

    echo "==> running tests (timeout ${TEST_TIMEOUT}s)"
    cd "$ROOT/tests"

    if command -v timeout >/dev/null 2>&1; then
        timeout "$TEST_TIMEOUT" ./test.sh
    else
        ./test.sh
    fi
}

# Run the suite in a fresh Ubuntu container. The project is mounted
# read-only and copied inside, so nothing on the host is modified.
# Tests run as an unprivileged user: as root the permission-denied test
# would not behave like it does for a real user.
run_docker() {
    command -v docker >/dev/null 2>&1 || die "'docker' not found" "install Docker Desktop or run on Linux with --local"
    docker info >/dev/null 2>&1 || die "Docker daemon is not running" "start Docker Desktop (macOS: open -a Docker) and retry"

    echo "==> running tests in $DOCKER_IMAGE"

    docker run --rm \
        -v "$ROOT":/src:ro \
        -e TEST_TIMEOUT="$TEST_TIMEOUT" \
        "$DOCKER_IMAGE" \
        bash -c '
            set -e
            echo "==> installing build tools"
            apt-get update -qq >/dev/null
            apt-get install -y -qq build-essential bsdutils >/dev/null 2>&1
            useradd -m tester
            cp -r /src /home/tester/my_ls
            chown -R tester:tester /home/tester/my_ls
            # Drop host build artifacts (possibly built for another OS).
            su tester -s /bin/bash -c "make -C ~/my_ls clean >/dev/null"
            su tester -s /bin/bash -c "TEST_TIMEOUT=$TEST_TIMEOUT ~/my_ls/run_tests.sh --local"
        '
}

mode="auto"

case "${1:-}" in
    "")             ;;
    --local)        mode="local" ;;
    --docker)       mode="docker" ;;
    -h|--help|help) usage; exit 0 ;;
    *)              echo "error: unknown option '$1'" >&2; usage >&2; exit 2 ;;
esac

if [ "$mode" = "auto" ]; then
    if [ "$(uname -s)" = "Linux" ]; then
        mode="local"
    else
        mode="docker"
    fi
fi

if [ "$mode" = "local" ]; then
    run_local
else
    run_docker
fi
