#!/usr/bin/env python3
"""CI wrapper for the AstroDefender polyglot contract check."""
from pathlib import Path
import runpy

ROOT = Path(__file__).resolve().parents[1]
module_path = ROOT / "polyglot" / "python-orchestrator" / "astro_polyglot_orchestrator.py"
namespace = runpy.run_path(str(module_path))
raise SystemExit(namespace["main"]())
