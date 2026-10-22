#!/usr/bin/env python3
"""Deterministic coverage and prototype comparison. Never launches QEMU."""
import concurrent.futures,json,os,subprocess
from pathlib import Path
from PIL import Image,ImageChops,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
LAB=Path(os.environ.get('WATCHFACE_LAB',ROOT.parent/'watchface-lab'))
SHOTS=ROOT/'shots';SHOTS.mkdir(exist_ok=True)
scenes=[('no-sun',0,True,{'NO_SUN':1})]
for name,scenario in [('demo',0),('stress',1),('empty',2)]:
 for detail in [False,True]:scenes.append((('detail' if name=='demo' else 'detail-'+name) if detail else name,scenario,detail,{}))
for name,values in [('12h',{'12H':1}),('midnight',{'HOUR':0,'MINUTE':2}),
 ('after-sunset',{'HOUR':23,'MINUTE':59}),('no-phone',{'PHONE':0}),
 ('no-health',{'NO_HEALTH':1}),('no-weather',{'WEATHER_OK':0}),
 ('low-battery',{'BATTERY':15}),('critical-battery',{'BATTERY':5,'PHONE':0}),
 ('charging',{'BATTERY':62,'CHARGING':1}),
 ('charging-offline',{'BATTERY':62,'CHARGING':1,'PHONE':0}),
 ('light-empty',{'LIGHT_THEME':1,'NO_HEALTH':1,'WEATHER_OK':0,'PHONE':0}),('fahrenheit',{'FAHRENHEIT':1,'TEMP':-18}),
 ('classic',{'PREAMBLE_LIT':1}),('date-caption',{'CAPTION_MODE':1}),
 ('battery-caption',{'CAPTION_MODE':2}),('light',{'LIGHT_THEME':1}),
 ('light-stress',{'LIGHT_THEME':1,'12H':1,'HOUR':23,'MINUTE':59,'TEMP':-18,'FAHRENHEIT':1,'UV':120,'STEPS':99999,'HR':220})]:
 for detail in [False,True]:scenes.append((name+('-detail' if detail else ''),0,detail,values))
for name,code in [('sun',0),('partly',2),('cloud',3),('rain',63),('snow',73),('storm',95),('fog',45)]:
 scenes.append(('weather-'+name,0,True,{'WEATHER_CODE':code}))
def render(scene):
 name,scenario,detail,values=scene;env=os.environ.copy()
 for k,v in values.items():env['WORDCLOCK_'+k]=str(v)
 args=['python3',str(LAB/'tools/render.py'),str(ROOT),str(SHOTS/(name+'.png')),'--scenario',str(scenario)]
 if detail:args.append('--detail')
 result=subprocess.run(args,env=env,capture_output=True,text=True)
 if result.returncode:raise RuntimeError(result.stdout+result.stderr)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:list(pool.map(render,scenes))
for name in ['demo','detail','detail-stress','detail-empty','light','light-detail','light-stress-detail','12h-detail']:
 subprocess.run(['python3',str(LAB/'tools/glance.py'),str(SHOTS/(name+'.png'))],check=True,capture_output=True)
for name in ['demo','stress','empty']:
 ref=Image.open(LAB/'designs/07-wordclock/shots'/f'{name}.png').convert('RGB')
 actual=Image.open(SHOTS/(name+'.png')).convert('RGB')
 # Production charging/disconnected captions intentionally change; grid/date/dots stay exact.
 if name!='demo':
  for im in [ref,actual]:ImageDraw.Draw(im).rectangle((20,209,180,227),fill='black')
 assert ImageChops.difference(ref,actual).getbbox() is None,name+' prototype art changed'
cols=6;rows=(len(scenes)+cols-1)//cols
sheet=Image.new('RGB',(cols*216,rows*254),'#dedede');draw=ImageDraw.Draw(sheet)
for i,(name,*_) in enumerate(scenes):
 x=i%cols*216+8;y=i//cols*254
 draw.text((x,y+4),name,fill='black');sheet.paste(Image.open(SHOTS/(name+'.png')).convert('RGB'),(x,y+22))
sheet.save(SHOTS/'coverage.png')
(SHOTS/'coverage.json').write_text(json.dumps({'scenes':[s[0] for s in scenes],'prototype':'demo exact; stress/empty grid/header/dots exact; production status captions differ'},indent=2)+'\n')
print(f'{len(scenes)} scenes rendered; prototype pixels match; glance outputs written.')
