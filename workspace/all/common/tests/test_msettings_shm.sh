#!/bin/sh
# Verifies field-for-field struct offset and version parity between
# common/msettings_shm.h and both tg5040 (v13) and tg5050 (v4, HAS_FAN)
# libmsettings/msettings.c definitions.
set -eu
cd "$(dirname "$0")"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# tg5040: SettingsV12 -> SettingsV13 (no HAS_FAN)
{
	sed -n '/^typedef struct SettingsV12 {/,/^} SettingsV12;/p' ../../../tg5040/libmsettings/msettings.c
	echo "typedef SettingsV12 PrevSettings;"
	sed -n '/^typedef struct SettingsV13 {/,/^typedef SettingsV13 Settings;/p' ../../../tg5040/libmsettings/msettings.c
} >"$TMP/msettings_extracted.h"
cc -std=gnu99 -Wall -Werror -fsanitize=address -g -I"$TMP" test_msettings_shm.c -o "$TMP/test_tg5040"
"$TMP/test_tg5040"

# tg5050: SettingsV3 -> SettingsV4 (-DHAS_FAN)
{
	sed -n '/^typedef struct SettingsV3 {/,/^} SettingsV3;/p' ../../../tg5050/libmsettings/msettings.c
	echo "typedef SettingsV3 PrevSettings;"
	sed -n '/^typedef struct SettingsV4 {/,/^typedef SettingsV4 Settings;/p' ../../../tg5050/libmsettings/msettings.c
} >"$TMP/msettings_extracted.h"
cc -std=gnu99 -Wall -Werror -fsanitize=address -g -DHAS_FAN -I"$TMP" test_msettings_shm.c -o "$TMP/test_tg5050"
"$TMP/test_tg5050"

echo "test_msettings_shm: OK (tg5040 v13 + tg5050 v4)"
