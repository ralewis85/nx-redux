#!/bin/sh
# Host regression test for tg5040 boot/suspend/power scripts:
#   1. .shadercache/.nx_version stamp preserves cache across matching boots and wipes on version change.
#   2. bin/suspend + bt_init.sh coordinate /tmp/nx_suspend_active and /tmp/nx_bt_wake_active so
#      neither prematurely unlinks /tmp/system_suspend while the other is mid-flight.
#   3. OSD toggle_power/set.sh resumes SIGSTOPped daemons (SIGCONT) and routes fallback to poweroff_next.
#   4. POSIX sh syntax passes on modified tg5040/OSD shell scripts.
set -eu
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

fail() {
	echo "FAIL: $1" >&2
	exit 1
}

for s in \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_power/set.sh" \
	"$ROOT/skeleton/SYSTEM/tg5040/bin/suspend" \
	"$ROOT/skeleton/SYSTEM/tg5040/etc/bluetooth/bt_init.sh" \
	"$ROOT/skeleton/SYSTEM/tg5040/paks/MinUI.pak/launch.sh"; do
	sh -n "$s" || fail "sh -n failed on $s"
done

SDCARD_PATH="$TMP/sdcard"
SYSTEM_PATH="$TMP/system"
mkdir -p "$SDCARD_PATH/.shadercache" "$SYSTEM_PATH"
printf 'v2.5.1-a\n' >"$SYSTEM_PATH/version.txt"
printf 'cached_blob' >"$SDCARD_PATH/.shadercache/shader0.bin"

STAMP_SNIPPET="$(sed -n '/^SHADER_VER_STAMP=/,/^fi$/p' "$ROOT/skeleton/SYSTEM/tg5040/paks/MinUI.pak/launch.sh")"
[ -n "$STAMP_SNIPPET" ] || fail "SHADER_VER_STAMP block missing from launch.sh"

SDCARD_PATH="$SDCARD_PATH" SYSTEM_PATH="$SYSTEM_PATH" sh -c "$STAMP_SNIPPET"
[ ! -e "$SDCARD_PATH/.shadercache/shader0.bin" ] || fail "unstamped cache was not cleared"
cmp -s "$SYSTEM_PATH/version.txt" "$SDCARD_PATH/.shadercache/.nx_version" || fail "version stamp not written"

printf 'compiled_pvr_shader' >"$SDCARD_PATH/.shadercache/shader1.bin"
SDCARD_PATH="$SDCARD_PATH" SYSTEM_PATH="$SYSTEM_PATH" sh -c "$STAMP_SNIPPET"
[ -f "$SDCARD_PATH/.shadercache/shader1.bin" ] || fail "same-version boot wiped .shadercache"

printf 'v2.5.2-b\n' >"$SYSTEM_PATH/version.txt"
SDCARD_PATH="$SDCARD_PATH" SYSTEM_PATH="$SYSTEM_PATH" sh -c "$STAMP_SNIPPET"
[ ! -e "$SDCARD_PATH/.shadercache/shader1.bin" ] || fail "version change did not invalidate .shadercache"
cmp -s "$SYSTEM_PATH/version.txt" "$SDCARD_PATH/.shadercache/.nx_version" || fail "updated stamp mismatch"

TDIR="$TMP/tmp"
mkdir -p "$TDIR"
BT_CLEAN="$(sed -n '/rm -f \/tmp\/nx_bt_wake_active/,/^[[:space:]]*fi$/p' "$ROOT/skeleton/SYSTEM/tg5040/etc/bluetooth/bt_init.sh" | sed "s#/tmp#$TDIR#g")"
SUSP_CLEAN="$(sed -n '/rm -f \/tmp\/nx_suspend_active/,/^fi$/p' "$ROOT/skeleton/SYSTEM/tg5040/bin/suspend" | sed "s#/tmp#$TDIR#g")"
[ -n "$BT_CLEAN" ] && [ -n "$SUSP_CLEAN" ] || fail "coordination lines missing"

touch "$TDIR/nx_bt_wake_active" "$TDIR/nx_suspend_active" "$TDIR/system_suspend"
sh -c "$BT_CLEAN"
[ -e "$TDIR/system_suspend" ] || fail "bt_init removed system_suspend during active suspend"

sh -c "$SUSP_CLEAN"
[ ! -e "$TDIR/system_suspend" ] || fail "suspend left stale system_suspend after bt_init finished"

touch "$TDIR/nx_bt_wake_active" "$TDIR/nx_suspend_active" "$TDIR/system_suspend"
sh -c "$SUSP_CLEAN"
[ -e "$TDIR/system_suspend" ] || fail "suspend removed system_suspend during active bt wake"
sh -c "$BT_CLEAN"
[ ! -e "$TDIR/system_suspend" ] || fail "bt_init left stale system_suspend after both finished"

grep -q 'killall -CONT keymon.elf audiomon.elf' "$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_power/set.sh" \
	|| fail "toggle_power/set.sh missing SIGCONT for stopped daemons"
grep -q 'poweroff_next' "$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_power/set.sh" \
	|| fail "toggle_power/set.sh missing poweroff_next fallback"

echo "test_tg5040_power_boot: OK"
