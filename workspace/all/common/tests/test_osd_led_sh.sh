#!/bin/sh
# Host regression test for OSD toggle_led/{set,update}.sh and toggle_rumble/set.sh
# against sandboxed sysfs + /tmp fixtures.
set -eu
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

fail() {
	echo "FAIL: $1" >&2
	exit 1
}

for s in \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_led/set.sh" \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_led/update.sh" \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_rumble/set.sh" \
	"$ROOT/workspace/tg5040/install/boot.sh"; do
	sh -n "$s" || fail "sh -n failed on $s"
done

# Sandbox sysfs, /mnt/SDCARD, and /tmp for Brick Pro 5-zone LED toggle round-trip
SYS="$TMP/sys/class/led_anim"
SD="$TMP/sdcard/.userdata/shared"
TDIR="$TMP/tmp"
mkdir -p "$SYS" "$SD" "$TDIR"

printf '0\n' >"$SYS/max_scale"
printf '40\n' >"$SYS/max_scale_lr"
printf '0\n' >"$SYS/max_scale_f1f2"
printf '65\n' >"$SYS/max_scale_rear"
for z in f1 f2 m lr rear l r; do
	printf '4\n' >"$SYS/effect_$z"
	printf 'FFFFFF\n' >"$SYS/effect_rgb_hex_$z"
done

cat >"$SD/ledsettings_brickpro.txt" <<'EOF'
[FN 1 key]
filename=f1
brightness=0
effect=4
color1=0xFF00AA
[Topbar]
filename=m
brightness=0
effect=4
color1=0x00FF00
[Joysticks]
filename=lr
brightness=40
effect=2
color1=0x112233
[L/R triggers]
filename=rear
brightness=65
effect=4
color1=0x445566
EOF

sed -e "s#/tmp#$TDIR#g" \
	-e "s#/sys/class/led_anim#$SYS#g" \
	-e "s#/mnt/SDCARD/.userdata/shared#$SD#g" \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_led/set.sh" >"$TMP/set_led.sh"
sed -e "s#/tmp#$TDIR#g" \
	-e "s#/sys/class/led_anim#$SYS#g" \
	"$ROOT/skeleton/SYSTEM/osd/common/widgets/toggle_led/update.sh" >"$TMP/upd_led.sh"

# Top bar is 0 while sticks (40) + rear (65) are lit -> update.sh & set.sh report ON (1)
sh "$TMP/upd_led.sh"
[ "$(cat "$TDIR/trimui_osd/toggle_led/status")" = "1" ] || fail "update.sh missed non-main lit zone"

# Toggle OFF -> writes /tmp/leds_disabled and zeroes every brightness & effect node
sh "$TMP/set_led.sh" 1
[ -f "$TDIR/leds_disabled" ] || fail "toggle off did not create leds_disabled"
[ "$(cat "$TDIR/trimui_osd/toggle_led/status")" = "0" ] || fail "status not 0 after toggle off"
[ "$(cat "$SYS/max_scale_lr")" = "0" ] && [ "$(cat "$SYS/max_scale_rear")" = "0" ] || fail "zones not zeroed"
[ "$(cat "$SYS/effect_lr")" = "0" ] && [ "$(cat "$SYS/effect_rear")" = "0" ] || fail "effects not disabled"

# Toggle ON -> removes leds_disabled and restores per-zone brightness/color/effect (keeping top bar 0!)
sh "$TMP/set_led.sh" 1
[ ! -e "$TDIR/leds_disabled" ] || fail "toggle on kept leds_disabled"
[ "$(cat "$TDIR/trimui_osd/toggle_led/status")" = "1" ] || fail "status not 1 after toggle on"
[ "$(cat "$SYS/max_scale")" = "0" ] || fail "top bar (configured 0) was forced lit"
[ "$(cat "$SYS/effect_m")" = "0" ] || fail "zero-brightness top bar effect was not 0"
[ "$(cat "$SYS/max_scale_lr")" = "40" ] && [ "$(cat "$SYS/effect_lr")" = "2" ] || fail "lr zone not restored"
[ "$(cat "$SYS/effect_rgb_hex_lr")" = "112233" ] || fail "lr color not restored"
[ "$(cat "$SYS/max_scale_rear")" = "65" ] && [ "$(cat "$SYS/effect_rgb_hex_rear")" = "445566" ] || fail "rear zone not restored"

echo "test_osd_led_sh: OK"
