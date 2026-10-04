# Technical notes

## Scope

This repository is a compatibility *profile and helper*, not a replacement for ShadowMountPlus.

The approach relies on features already present upstream:

1. per-game or global fakelib overlays;
2. emulator-file replacement into the game fakelib cache;
3. kstuff pause/resume around tracked game launches;
4. crash detection and automatic delay tuning;
5. LVD-backed image mounting.

## Managed keys

The helper intentionally manages only single-value keys:

```text
debug
mount_read_only
scan_depth
stability_wait_seconds
exfat_backend
ufs_backend
backport_fakelib
update_emulators
emulators_path
auto_update_ampr
global_fakelib
global_fakelib_path
global_fakelib_priority
kstuff_game_auto_toggle
kstuff_crash_detection
kstuff_pause_delay_image_seconds
kstuff_pause_delay_direct_seconds
lvd_exfat_sector_size
lvd_ufs_sector_size
lvd_pfs_sector_size
```

It does not remove repeatable rules such as:

```text
kstuff_no_pause
kstuff_delay
scanpath
fakelib_exclude
global_fakelib_exclude
image_ro
image_rw
image_sector
```

## Why 512 for lvd_exfat_sector_size?

ShadowMountPlus documents 512 as the logical LVD exFAT sector size. A 64 KB exFAT *cluster* is a different concept and should not be copied into this logical-sector setting.

## Why auto_update_ampr=0?

The profile is deterministic and offline by default. Upstream supports an AMPR emulator updater, but users should opt into network downloads themselves.

## Black Myth test case

The original field case used title `PPSA23226`, version `01.000.021`.

The successful pattern was:

- game image mounted normally;
- per-game fakelib selected;
- emulator update cache available;
- kstuff auto-management enabled;
- no forced `kstuff_no_pause=PPSA23226` rule;
- debug log confirming fakelib/emulator selection and game lifecycle.

This is one observed setup, not a universal compatibility claim.

## Conflict to avoid

ShadowMountPlus upstream documents that standalone BackPork conflicts with its own `backport_fakelib` watcher. Do not run both at the same time.
