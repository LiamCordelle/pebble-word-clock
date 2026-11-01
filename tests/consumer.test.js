'use strict';
var assert=require('assert');
var options=require('../src/pkjs/app-config');
var schema=require('../src/pkjs/kit/config')(options);
var settings=require('../src/pkjs/kit/settings')(options);
var pkg=require('../package.json');
assert.strictEqual(schema[0].defaultValue,'Word Clock');
assert.strictEqual(options.storageKey,'word-clock-kit-settings-v1');
assert.strictEqual(schema[1].items.some(function(v){return v.messageKey==='LARGE_LABELS';}),false);
assert.deepStrictEqual(pkg.pebble.messageKeys,['READY','REQUEST_WEATHER','WEATHER_ID','WEATHER',
  'WEATHER_ERROR','TIME_MODE','TEMP_UNIT','STEP_GOAL','WORLD','WEATHER_ENABLED','DEMO_FALLBACK',
  'LARGE_LABELS','PREAMBLE_LIT','CAPTION_MODE','LIGHT_THEME']);
var memory={},store={getItem:function(k){return memory[k]||null;},setItem:function(k,v){memory[k]=v;}};
settings.save(store,{PREAMBLE_LIT:1,CAPTION_MODE:2,LIGHT_THEME:true,STEP_GOAL:18000});
var saved=settings.load(store);
assert.strictEqual(saved.PREAMBLE_LIT,1);assert.strictEqual(saved.CAPTION_MODE,2);
assert.strictEqual(saved.LIGHT_THEME,1);assert.strictEqual(saved.STEP_GOAL,18000);
assert.strictEqual(settings.normalize({CAPTION_MODE:99}).CAPTION_MODE,2);
assert.strictEqual(options.fullTimezones,true);
var displayItems=require('../src/pkjs/kit/config')(options)[1].items;
assert.strictEqual(displayItems.find(function(item){return item.messageKey==='WORLD';}).options.length,597);
assert(displayItems.some(function(item){return item.id==='kit-timezone-search';}));
[0,3,6,268,289,596].forEach(function(zone){
  settings.save(store,Object.assign(settings.load(store),{WORLD:zone}));
  assert.strictEqual(settings.load(store).WORLD,zone);
});
console.log('Word Clock configuration: key numbering, defaults, normalization and persistence passed.');
