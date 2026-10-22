#!/usr/bin/env python3
"""Run the shared regression suite and this consumer's configuration checks."""
from pathlib import Path
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[1]
subprocess.run([sys.executable, str(ROOT/'shared-kit/tests/run.py')], check=True)
subprocess.run(['node', 'tests/consumer.test.js'], cwd=ROOT, check=True)
OUT=ROOT/'build/tests';OUT.mkdir(parents=True,exist_ok=True)
subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-parameter',
  '-Ishared-kit/tests/health-stubs','-include','tests/settings_api.h',
  'tests/settings.test.c','src/c/face_settings.c','-o',str(OUT/'face-settings')],cwd=ROOT,check=True)
subprocess.run([str(OUT/'face-settings')],check=True)
