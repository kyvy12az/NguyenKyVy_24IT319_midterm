#!/bin/sh
set -eu

PROGRAM="${PROGRAM:-./myls}"
TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM

mkdir -p "$TEST_DIR/dir/sub"
printf 'alpha\n' > "$TEST_DIR/dir/a.txt"
printf 'beta beta\n' > "$TEST_DIR/dir/b.txt"
printf 'hidden\n' > "$TEST_DIR/dir/.hidden"
printf '#!/bin/sh\n' > "$TEST_DIR/dir/run.sh"
chmod +x "$TEST_DIR/dir/run.sh"
ln -s a.txt "$TEST_DIR/dir/link"
ln -s dir "$TEST_DIR/dir-link"
printf 'nested\n' > "$TEST_DIR/dir/sub/nested.txt"
dd if=/dev/zero of="$TEST_DIR/dir/big.bin" bs=1024 count=2 2>/dev/null
bad_name="bad$(printf '\001')name"
: > "$TEST_DIR/dir/$bad_name"

fail() { printf 'FAIL: %s\n' "$1" >&2; exit 1; }
pass() { printf 'PASS: %s\n' "$1"; }

output="$($PROGRAM "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx 'a.txt' || fail "default listing"
printf '%s\n' "$output" | grep -q '^\.hidden$' && fail "hidden file leaked"
pass "default listing hides dot files"

output="$($PROGRAM -A "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx '.hidden' || fail "-A"
printf '%s\n' "$output" | grep -qx '.' && fail "-A included dot"
pass "-A includes hidden entries except dot entries"

output="$($PROGRAM -a "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx '.' || fail "-a dot"
printf '%s\n' "$output" | grep -qx '..' || fail "-a dotdot"
pass "-a includes all entries"

output="$($PROGRAM -F "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx 'sub/' || fail "-F directory"
printf '%s\n' "$output" | grep -qx 'run.sh\*' || fail "-F executable"
printf '%s\n' "$output" | grep -qx 'link@' || fail "-F symlink"
pass "-F classifies entries"

output="$($PROGRAM -l "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -Eq '^l.* link -> a.txt$' || fail "long symlink"
pass "-l prints metadata and symlink target"

output="$($PROGRAM -R "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx 'nested.txt' || fail "recursive"
pass "-R descends into subdirectories"

output="$($PROGRAM -d "$TEST_DIR/dir")"
printf '%s\n' "$output" | grep -qx "$TEST_DIR/dir" || fail "-d"
pass "-d lists directory operand itself"

output="$($PROGRAM -ld "$TEST_DIR/dir/link")"
printf '%s\n' "$output" | grep -Eq '^l.* -> a.txt$' || fail "symlink operand"
pass "symlink operand keeps link metadata with -d"

output="$($PROGRAM "$TEST_DIR/dir-link")"
printf '%s\n' "$output" | grep -qx 'a.txt' || fail "symlink directory operand"
pass "directory symlink is followed without -d"

output="$($PROGRAM -ln "$TEST_DIR/dir/a.txt")"
printf '%s\n' "$output" | grep -Eq '^-.+ [0-9]+ [0-9]+ [0-9]+ +[0-9]+ ' || fail "-ln override"
pass "-n after -l selects numeric IDs"

output="$($PROGRAM -nl "$TEST_DIR/dir/a.txt")"
printf '%s\n' "$output" | grep -Eq "^-.+ [0-9]+ $(id -un) " || fail "-nl override"
pass "-l after -n selects names"

first="$($PROGRAM -S "$TEST_DIR/dir/a.txt" "$TEST_DIR/dir/big.bin" | sed -n '1p')"
[ "$first" = "$TEST_DIR/dir/big.bin" ] || fail "-S sorting"
pass "-S sorts larger files first"

first="$($PROGRAM -r "$TEST_DIR/dir/a.txt" "$TEST_DIR/dir/b.txt" | sed -n '1p')"
[ "$first" = "$TEST_DIR/dir/b.txt" ] || fail "-r sorting"
pass "-r reverses sorting"

output="$($PROGRAM -his "$TEST_DIR/dir/big.bin")"
printf '%s\n' "$output" | grep -Eq '^[[:space:]]*[0-9]+ +[0-9.]+K ' || fail "-his"
pass "-i, -s and -h print inode and human-readable blocks"

output="$($PROGRAM -q "$TEST_DIR/dir/$bad_name")"
printf '%s\n' "$output" | grep -Fq 'bad?name' || fail "-q"
pass "-q replaces non-printable filename bytes"

if $PROGRAM "$TEST_DIR/missing" >/dev/null 2>&1; then
    fail "missing operand status"
fi
pass "missing operand returns failure"

printf 'All tests passed.\n'
