'use strict';
module.exports = {
  heading: 'Word Clock', storageKey: 'word-clock-kit-settings-v1', largeLabels: false,
  faceSettings: [
    {type:'toggle',messageKey:'PREAMBLE_LIT',label:'Fully lit IT IS',defaultValue:false},
    {type:'select',messageKey:'CAPTION_MODE',label:'Bottom caption',defaultValue:'0',min:0,max:2,
      options:[{label:'Steps',value:'0'},{label:'Date',value:'1'},{label:'Battery',value:'2'}]},
    {type:'toggle',messageKey:'LIGHT_THEME',label:'Light face',defaultValue:false}
  ]
};
