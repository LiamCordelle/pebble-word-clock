#!/usr/bin/env python3
"""Image checks after the final emulator harness (no emulator launched)."""
from pathlib import Path
import json
from PIL import Image,ImageChops
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'shots/emulator'
def crop(name,box):return Image.open(OUT/(name+'.png')).convert('RGB').crop(box)
def same(a,b):return ImageChops.difference(a,b).getbbox() is None
# The preamble is independent of the current minute, so timeout checks are stable.
assert same(crop('face',(12,21,92,35)),crop('timeout',(12,21,92,35))),'Detail did not time out'
assert Image.open(OUT/'settings-light.png').getpixel((0,100))[:3]==(255,255,255),'Light setting did not arrive'
assert same(crop('settings-light',(12,21,92,35)),crop('settings-persisted',(12,21,92,35))),'Preamble not persisted'
assert same(crop('settings-light',(20,210,180,225)),crop('settings-persisted',(20,210,180,225))),'Caption not persisted'
assert same(crop('weather-detail',(0,75,200,180)),crop('weather-cached-detail',(0,75,200,180))),'Weather cache not restored'
assert not same(crop('disconnected',(20,210,180,225)),crop('final-face',(20,210,180,225))),'Phone status did not change'
# Caption rendering is deterministic and independent of the local date/time.
for name in ['low-battery','charging']:
 expected=Image.open(ROOT/'shots'/(name+'.png')).convert('RGB').crop((20,210,180,225))
 assert same(crop(name,(20,210,180,225)),expected),name+' caption differs from render'

print('QEMU images passed: timeout, Clay theme/preamble/caption, restart persistence, cached forecast and phone status.')
print(json.loads((OUT/'result.json').read_text()))
