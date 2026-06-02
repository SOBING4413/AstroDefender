#!/usr/bin/env python3
"""AstroDefender polyglot orchestrator.
ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0

This module is not a standalone example. It is the coordination point that verifies
all language modules are bound to the same contract before a polyglot toolchain is
allowed to package assets, gameplay scripts, overlays, telemetry, or ports.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "polyglot" / "contract" / "astro_polyglot_manifest.json"
CONTRACT_MARKER = "ASTRO_POLYGLOT_CONTRACT_VERSION:"


def load_manifest() -> dict:
    return json.loads(MANIFEST.read_text(encoding="utf-8"))


def verify_contract() -> list[str]:
    manifest = load_manifest()
    version = manifest["contract_version"]
    missing: list[str] = []
    for module in manifest["modules"]:
        path = ROOT / module["path"]
        if path.is_dir():
            continue
        text = path.read_text(encoding="utf-8")
        if f"{CONTRACT_MARKER} {version}" not in text:
            missing.append(f"{module['language']}:{module['role']} -> {module['path']}")
    return missing


def main() -> int:
    missing = verify_contract()
    if missing:
        print("Polyglot contract mismatch:")
        for item in missing:
            print(f"- {item}")
        return 1
    manifest = load_manifest()
    languages = ", ".join(module["language"] for module in manifest["modules"])
    print(f"AstroDefender polyglot contract {manifest['contract_version']} OK")
    print(f"Cooperating modules: {languages}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
