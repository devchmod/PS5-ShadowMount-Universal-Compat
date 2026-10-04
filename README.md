# PS5-ShadowMount-Universal-Compat

Experimental compatibility helper for **ShadowMountPlus** on jailbroken PS5 consoles.

This project packages a conservative ShadowMountPlus configuration pattern that proved useful when a mounted PS5 title reached the launcher but failed during runtime because its fakelib/emulator layer and kstuff timing were not aligned.

> This project does **not** contain Sony files, keys, game files, decrypted modules, or copyrighted backports.

## What it does

The helper applies a conservative ShadowMountPlus profile that explicitly enables:

- `backport_fakelib=1`
- `update_emulators=1`
- `global_fakelib=1`
- `global_fakelib_priority=game`
- `kstuff_game_auto_toggle=1`
- `kstuff_crash_detection=1`
- LVD backends and the documented logical sector defaults
- debug logging
- read-only mounting by default

It also creates these folders when missing:

```text
/data/shadowmount/emus
/data/shadowmount/fakelib
/data/shadowmount/cache
```

The helper preserves custom per-title rules such as `kstuff_no_pause`, `kstuff_delay`, `scanpath`, fakelib exclusions and image overrides. It only replaces the single-value keys that it manages.

## Why this exists

ShadowMountPlus already contains the important compatibility mechanisms. This project does not reimplement them. It makes a known-good combination easy to apply, back up, restore and diagnose.

Upstream: https://github.com/drakmor/ShadowMountPlus

## Important limitation

This is **not** a promise that every game will boot. Some titles still need a title-specific backport, a compatible emulator file, or a per-title kstuff delay.

Do not run standalone **BackPork** together with ShadowMountPlus `backport_fakelib=1`; upstream documents that the two conflict.

## Initial field test

The first profile was derived from a setup where **PPSA23226 / 01.000.021** successfully launched after using the ShadowMountPlus fakelib/backport path, emulator-update cache, and normal kstuff auto-management.

A title-specific `kstuff_no_pause` rule should not be added blindly; let ShadowMountPlus crash detection/autotune determine the delay unless a game is known to require otherwise.

## Quick start

### Option A — helper payload

Requirements:

- PS5 Payload SDK
- ShadowMountPlus already installed
- a normal payload loader / PLDMGR

Build:

```bash
cd payload
make
```

Outputs:

```text
universal-compat.elf
universal-compat-restore.elf
```

Run `universal-compat.elf` once, then restart/reload ShadowMountPlus.

On first run, the current config is saved as:

```text
/data/shadowmount/config.ini.pre-universal
```

To roll back, run `universal-compat-restore.elf`.

### Option B — host-side FTP installer

For users who already run an FTP payload:

```bash
python3 tools/apply_profile.py 192.168.0.184
```

Default FTP port is `2121`. Use `--port` to change it.

The host tool downloads a local backup before uploading the merged config.

## Emulator/backport files

This repository intentionally does not ship emulator or backport binaries.

ShadowMountPlus uses:

```text
/data/shadowmount/emus
/data/shadowmount/fakelib
/data/shadowmount/cache/<TITLE_ID>/fakelib
```

Only use files you are legally allowed to use.

## Profile

The human-readable preset is in:

```text
profiles/universal.ini
```

The helper payload embeds the same values.

## Diagnostics

With ShadowMountPlus debug logging enabled:

```text
/data/shadowmount/debug.log
```

From a Mac/Linux host with FTP available:

```bash
python3 tools/diagnose.py 192.168.0.184 --title PPSA23226
```

Useful log markers include:

```text
FAKELIB
Backport
emulator
KSTUFF
GAME
crash
autotune
```

## Safety

- Config is backed up before the first payload-side modification.
- The helper does not delete games, saves or app.db entries.
- The profile defaults to read-only mounting.
- It does not change repeatable per-title rules automatically.
- Keep a copy of your own config and saves before experimenting.

## License

GPL-3.0. See `LICENSE`.

## Credits

- ShadowMountPlus / Drakmor
- RDX-Sci01 and the upstream ShadowMount lineage
- PS5 homebrew developers whose public work made these compatibility mechanisms possible
