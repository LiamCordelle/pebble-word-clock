#!/usr/bin/env python3
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
image=Image.open(ROOT/'shots/demo.png').convert('RGB')
for size in [80,144]:
 canvas=Image.new('RGB',(size,size),'black')
 thumb=image.copy();thumb.thumbnail((size,size),Image.Resampling.NEAREST)
 canvas.paste(thumb,((size-thumb.width)//2,(size-thumb.height)//2))
 canvas.save(ROOT/f'icon_{size}x{size}.png')
image.save(ROOT/'screenshot_emery.png')
Image.open(ROOT/'shots/detail.png').save(ROOT/'screenshot_detail.png')
print('Store icons and Emery face/detail screenshots written.')
