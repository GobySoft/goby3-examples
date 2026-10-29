#!/bin/bash
# Launch one or more example .launch files, run a goby pub/sub test against them, then shut everything down.
#
# Usage: run_test.sh [-w seconds] <test.yml> <file.launch> [<file.launch> ...]
#
# Exit code is that of 'goby test' (0 pass, 1 expectation failed, 2 setup error).
# The example binaries and libgoby3_example_messages.so must be built (see ../README.md);
# GOBY3_EXAMPLES_BUILD overrides the default build directory.

set -u

startup_wait=0
while getopts "w:h" opt; do
    case $opt in
        w) startup_wait=$OPTARG ;;
        h|*) sed -n 2,9p "$0"; exit 2 ;;
    esac
done
shift $((OPTIND-1))
[ $# -lt 2 ] && { sed -n 2,9p "$0"; exit 2; }

test_file=$(readlink -f "$1"); shift
script_dir=$(cd "$(dirname "$0")" && pwd)
build_dir=${GOBY3_EXAMPLES_BUILD:-$script_dir/../build}

export PATH=$build_dir/bin:$PATH
export GOBY_TOOL_LOAD_SHARED_LIBRARY=$build_dir/lib/libgoby3_example_messages.so

pids=()
for launch in "$@"; do
    launch=$(readlink -f "$launch")
    (cd "$(dirname "$launch")" && exec setsid goby_launch -r "$launch") > /dev/null 2>&1 &
    pids+=($!)
done

cleanup() {
    for p in "${pids[@]}"; do kill -INT -- -"$p" 2>/dev/null; done
    for p in "${pids[@]}"; do wait "$p" 2>/dev/null; done
}
trap cleanup EXIT

sleep "$startup_wait"
goby test "$test_file" ${TEST_ARGS[@]+"${TEST_ARGS[@]}"}
