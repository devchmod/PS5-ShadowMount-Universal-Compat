#!/usr/bin/env python3
"""Merge the universal ShadowMountPlus profile over FTP.

Only managed single-value keys are replaced. Repeatable per-title rules are
preserved. A local backup is created before upload.
"""

from __future__ import annotations

import argparse
import ftplib
from io import BytesIO
from pathlib import Path
from datetime import datetime

REMOTE_DIR = "/data/shadowmount"
REMOTE_CONFIG = REMOTE_DIR + "/config.ini"

MANAGED = {
    "debug": "1",
    "mount_read_only": "1",
    "scan_depth": "1",
    "stability_wait_seconds": "10",
    "exfat_backend": "lvd",
    "ufs_backend": "lvd",
    "backport_fakelib": "1",
    "update_emulators": "1",
    "emulators_path": "/data/shadowmount/emus",
    "auto_update_ampr": "0",
    "global_fakelib": "1",
    "global_fakelib_path": "/data/shadowmount/fakelib",
    "global_fakelib_priority": "game",
    "kstuff_game_auto_toggle": "1",
    "kstuff_crash_detection": "1",
    "kstuff_pause_delay_image_seconds": "25",
    "kstuff_pause_delay_direct_seconds": "15",
    "lvd_exfat_sector_size": "512",
    "lvd_ufs_sector_size": "4096",
    "lvd_pfs_sector_size": "4096",
}


def key_of(line: str) -> str | None:
    s = line.lstrip()
    if not s or s.startswith(("#", ";")) or "=" not in s:
        return None
    return s.split("=", 1)[0].strip()


def merge_config(original: str) -> str:
    kept = []
    for line in original.splitlines():
        key = key_of(line)
        if key not in MANAGED:
            kept.append(line)

    kept.extend(
        [
            "",
            "# --- PS5-ShadowMount-Universal-Compat managed block ---",
            *[f"{k}={v}" for k, v in MANAGED.items()],
            "# Preserves per-title kstuff_no_pause/kstuff_delay, scanpath and exclusions.",
            "# Do not run standalone BackPork together with backport_fakelib=1.",
            "# --- end managed block ---",
            "",
        ]
    )
    return "\n".join(kept)


def ensure_dir(ftp: ftplib.FTP, path: str) -> None:
    current = ""
    for part in path.strip("/").split("/"):
        current += "/" + part
        try:
            ftp.mkd(current)
        except ftplib.error_perm as exc:
            if not str(exc).startswith("550"):
                raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("host", help="PS5 IP address")
    parser.add_argument("--port", type=int, default=2121)
    parser.add_argument("--user", default="anonymous")
    parser.add_argument("--password", default="")
    args = parser.parse_args()

    ftp = ftplib.FTP()
    ftp.connect(args.host, args.port, timeout=10)
    ftp.login(args.user, args.password)

    ensure_dir(ftp, REMOTE_DIR)
    for directory in ("emus", "fakelib", "cache"):
        ensure_dir(ftp, f"{REMOTE_DIR}/{directory}")

    buf = BytesIO()
    try:
        ftp.retrbinary("RETR " + REMOTE_CONFIG, buf.write)
        original = buf.getvalue().decode("utf-8", errors="replace")
    except ftplib.error_perm:
        original = ""

    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    backup = Path(f"config.ini.backup-{stamp}")
    backup.write_text(original, encoding="utf-8")
    print(f"Local backup: {backup}")

    merged = merge_config(original).encode("utf-8")
    ftp.storbinary("STOR " + REMOTE_CONFIG, BytesIO(merged))
    ftp.quit()

    print("Profile applied.")
    print("Restart/reload ShadowMountPlus before testing a game.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
