#!/usr/bin/env python3
"""Read ShadowMountPlus config/debug log over FTP and print compatibility lines."""

from __future__ import annotations

import argparse
import ftplib
from io import BytesIO

CONFIG = "/data/shadowmount/config.ini"
LOG = "/data/shadowmount/debug.log"

TOKENS = (
    "FAKELIB",
    "BACKPORT",
    "EMULATOR",
    "KSTUFF",
    "GAME",
    "CRASH",
    "AUTOTUNE",
)


def fetch_text(ftp: ftplib.FTP, path: str) -> str:
    buf = BytesIO()
    ftp.retrbinary("RETR " + path, buf.write)
    return buf.getvalue().decode("utf-8", errors="replace")


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("host")
    p.add_argument("--port", type=int, default=2121)
    p.add_argument("--title", help="Optional TITLE_ID filter, e.g. PPSA23226")
    p.add_argument("--tail", type=int, default=200)
    args = p.parse_args()

    ftp = ftplib.FTP()
    ftp.connect(args.host, args.port, timeout=10)
    ftp.login()

    print("=== managed config keys ===")
    try:
        cfg = fetch_text(ftp, CONFIG)
        for line in cfg.splitlines():
            upper = line.upper()
            if any(
                key in upper
                for key in (
                    "BACKPORT_FAKELIB",
                    "UPDATE_EMULATORS",
                    "GLOBAL_FAKELIB",
                    "KSTUFF_",
                    "EMULATORS_PATH",
                )
            ):
                print(line)
    except ftplib.error_perm as exc:
        print(f"config unavailable: {exc}")

    print("\n=== debug log ===")
    try:
        log = fetch_text(ftp, LOG)
        lines = log.splitlines()[-args.tail :]
        for line in lines:
            upper = line.upper()
            if args.title and args.title.upper() not in upper and not any(t in upper for t in TOKENS):
                continue
            if any(t in upper for t in TOKENS) or (args.title and args.title.upper() in upper):
                print(line)
    except ftplib.error_perm as exc:
        print(f"log unavailable: {exc}")

    ftp.quit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
