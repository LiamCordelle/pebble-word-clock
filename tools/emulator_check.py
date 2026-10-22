#!/usr/bin/env python3
"""Final QEMU verification. Use pebble-tool's Python; preserves SDK profile.
Synthetic forecast and emulator sensors validate transport/rendering, not a phone
geolocation fix or the physical watch's sensor/battery behaviour.
"""
from pathlib import Path
import os,json,re,shutil,subprocess,sys,tempfile,threading,time
from uuid import UUID
from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.websocket import MessageTargetPhone
from libpebble2.communication.transports.websocket.protocol import AppConfigSetup,AppConfigResponse,WebSocketPhonesimAppConfig,WebSocketPhonesimConfigResponse
from libpebble2.services.appmessage import AppMessageService,Uint32,ByteArray
from libpebble2.protocol.apps import AppRunState,AppRunStateStart,AppRunStateStop,AppRunStateRequest
from pebble_tool.sdk.emulator import ManagedEmulatorTransport
ROOT=Path(__file__).resolve().parents[1];os.chdir(ROOT)
OUT=ROOT/'shots/emulator';OUT.mkdir(parents=True,exist_ok=True)
if '--child' not in sys.argv:
 from pebble_tool.sdk import get_sdk_persist_dir
 if subprocess.run(['pgrep','-f','(^|/)qemu-(system|pebble)( |$)'],capture_output=True).returncode==0:
  raise RuntimeError('Another emulator is active; leave it untouched')
 profile=Path(get_sdk_persist_dir('emery'));backup=Path(tempfile.mkdtemp(prefix='word-clock-emulator-'))/'saved'
 existed=profile.exists()
 if existed:shutil.move(str(profile),str(backup))
 try:
  result=subprocess.run([sys.executable,str(Path(__file__).resolve()),'--child'])
 finally:
  subprocess.run(['pebble','kill'],capture_output=True)
  if profile.exists():shutil.rmtree(profile)
  if existed:shutil.move(str(backup),str(profile))
  backup.parent.rmdir();print('SDK profile restored; emulator stopped.',flush=True)
 sys.exit(result.returncode)
def cli(*args):
 r=subprocess.run(['pebble',*args,'--emulator','emery'],text=True,capture_output=True,timeout=60,check=True)
 with (OUT/'commands.log').open('a') as f:f.write('pebble '+' '.join(args)+'\n'+r.stdout+r.stderr+'\n')
 if args[0] in ('emu-battery','emu-tap','emu-button'):time.sleep(.8)
 return r
def shot(name):cli('screenshot','--no-open',str(OUT/(name+'.png')))
pkg=json.loads((ROOT/'package.json').read_text());uuid=UUID(pkg['pebble']['uuid'])
keys=json.loads((ROOT/'build/js/message_keys.json').read_text())
settings={'TIME_MODE':0,'TEMP_UNIT':0,'STEP_GOAL':10000,'WORLD':0,'WEATHER_ENABLED':0,
 'DEMO_FALLBACK':0,'LARGE_LABELS':0,'PREAMBLE_LIT':0,'CAPTION_MODE':0,'LIGHT_THEME':0}
env=os.environ.copy();env['PYTHONUNBUFFERED']='1'
with (OUT/'emulator.log').open('w') as output:
 logs=subprocess.Popen(['pebble','logs','--emulator','emery'],stdout=output,stderr=subprocess.STDOUT,env=env)
 connection=service=None
 try:
  time.sleep(18)
  cli('install',str(ROOT/'build/pebble-word-clock.pbw'))
  connection=PebbleConnection(ManagedEmulatorTransport('emery'));connection.connect();connection.run_async()
  connection.send_packet(AppRunState(data=AppRunStateStart(uuid=uuid)));time.sleep(2)
  current=connection.send_and_read(AppRunState(data=AppRunStateRequest()),AppRunState)
  assert current.data.uuid==uuid,'Wrong app active'
  print('Verified Word Clock UUID.',flush=True)
  service=AppMessageService(connection);service._current_txid=180
  def apply(phone=False,**changes):
   settings.update(changes)
   if phone:
    connection.transport.send_packet(WebSocketPhonesimAppConfig(config=AppConfigSetup()),target=MessageTargetPhone())
    connection.read_transport_message(MessageTargetPhone,WebSocketPhonesimConfigResponse,timeout=10)
    connection.transport.send_packet(WebSocketPhonesimAppConfig(config=AppConfigResponse(data=json.dumps(settings))),target=MessageTargetPhone())
   else:service.send_message(uuid,{keys[k]:Uint32(v) for k,v in settings.items()})
   time.sleep(.7)
  apply();cli('emu-steps','6842')
  for _ in range(3):cli('emu-heart-rate','64');time.sleep(1)
  cli('emu-battery','--percent','76')
  shot('face');cli('emu-tap');time.sleep(.4);shot('detail')
  time.sleep(8.2);shot('timeout')
  apply(phone=True,PREAMBLE_LIT=1,CAPTION_MODE=2,LIGHT_THEME=1,TIME_MODE=1,TEMP_UNIT=1)
  shot('settings-light');cli('emu-tap');shot('settings-detail');cli('emu-tap')
  connection.send_packet(AppRunState(data=AppRunStateStop(uuid=uuid)));time.sleep(1)
  connection.send_packet(AppRunState(data=AppRunStateStart(uuid=uuid)));time.sleep(1)
  shot('settings-persisted')
  # A current synthetic weather fixture, encoded by the production phone module.
  fixture_js="""const fs=require('fs'),f=require('./shared-kit/tests/fixture')();
const now=Math.floor(Date.now()/1000),base=f.daily.time[0];
const day=new Date();day.setHours(0,0,0,0);const shift=day.getTime()/1000-base;
f.current.time=now;f.utc_offset_seconds=-day.getTimezoneOffset()*60;
['time','sunrise','sunset'].forEach(k=>f.daily[k]=f.daily[k].map(v=>v+shift));
f.hourly.time=f.hourly.time.map(v=>v+shift);
fs.writeFileSync('build/tests/emulator-weather.bin',Buffer.from(require('./src/pkjs/kit/weather').encode(f,now)));"""
  (ROOT/'build/tests').mkdir(parents=True,exist_ok=True)
  subprocess.run(['node','-e',fixture_js],check=True)
  weather=(ROOT/'build/tests/emulator-weather.bin').read_bytes();received=threading.Event()
  def incoming(tid,app,data):
   if app==uuid and keys['REQUEST_WEATHER'] in data and not received.is_set():
    service.send_message(uuid,{keys['WEATHER_ID']:Uint32(data[keys['REQUEST_WEATHER']]),keys['WEATHER']:ByteArray(weather)})
    received.set()
  service.register_handler('appmessage',incoming)
  apply(PREAMBLE_LIT=0,CAPTION_MODE=0,LIGHT_THEME=0,TIME_MODE=0,TEMP_UNIT=0,WEATHER_ENABLED=1)
  assert received.wait(10),'No correlated weather request'
  time.sleep(.7);cli('emu-tap');shot('weather-detail');cli('emu-tap')
  apply(phone=True) # Save restored defaults on the phone before restart too.
  connection.send_packet(AppRunState(data=AppRunStateStop(uuid=uuid)));time.sleep(1)
  connection.send_packet(AppRunState(data=AppRunStateStart(uuid=uuid)));time.sleep(1)
  cli('emu-tap');shot('weather-cached-detail');cli('emu-tap')
  cli('emu-battery','--percent','15');shot('low-battery')
  cli('emu-battery','--percent','62','--charging');shot('charging')
  cli('emu-battery','--percent','76')
  cli('emu-bt-connection','--connected','no');time.sleep(27);shot('disconnected')
  cli('emu-bt-connection','--connected','yes');time.sleep(1);shot('final-face')
  cli('emu-tap');shot('final-detail');cli('emu-tap')
  connection.send_packet(AppRunState(data=AppRunStateStop(uuid=uuid)));time.sleep(1)
 finally:
  if service:service.shutdown()
  if connection:connection.transport.ws.close()
  logs.terminate()
  try:logs.wait(timeout=5)
  except subprocess.TimeoutExpired:logs.kill();logs.wait()
text=(OUT/'emulator.log').read_text()
assert not re.search(r'APP_FAULT|stack overflow|JS failed|app crash',text,re.I),text
assert 'Face settings applied: preamble=1 caption=2 light=1' in text
assert 'Weather snapshot accepted' in text
assert text.count('Watchface kit 2.1.0 started')>=3,'No fresh app starts'
assert text.count('Still allocated <0B>')>=3,'App did not exit cleanly'
frames=[int(n) for n in re.findall(r'Frame \w+ (\d+)ms',text)]
assert frames,'No frame timing logs'
(OUT/'result.json').write_text(json.dumps({'active_uuid':str(uuid),'frame_ms_min':min(frames),
 'frame_ms_max':max(frames),'fixture':'synthetic forecast via correlated AppMessage',
 'profile':'temporary; original restored by parent'},indent=2)+'\n')
print(f'QEMU passed: settings, forecast/cache, statuses and clean logs; draws {min(frames)}–{max(frames)}ms.',flush=True)
