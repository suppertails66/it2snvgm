#include "sit/SItEntry.h"
#include <iostream>

namespace SVgm {


SItEntry::SItEntry()
  : hasNote(false),
    hasInstrument(false),
    hasVolPan(false),
    volPanEffect(volEffectNone),
    hasCommand(false),
    command(effectNone) {
  
}

void SItEntry::read(BlackT::TStream& ifs,
                    int maskVariable,
                    SItEntry* lastEntry) {
  if (maskVariable & 0x10) {
    hasNote = lastEntry->hasNote;
    note = lastEntry->note;
  }
  
  if (maskVariable & 0x20) {
    hasInstrument = lastEntry->hasInstrument;
    instrument = lastEntry->instrument;
  }
  
  if (maskVariable & 0x40) {
    hasVolPan = lastEntry->hasVolPan;
    volPanEffect = lastEntry->volPanEffect;
    volPanValue = lastEntry->volPanValue;
  }
  
  if (maskVariable & 0x80) {
    hasCommand = lastEntry->hasCommand;
    command = lastEntry->command;
    commandValue = lastEntry->commandValue;
  }
  
  if (maskVariable & 0x01) {
    hasNote = true;
    note = ifs.readu8();
    
    lastEntry->hasNote = hasNote;
    lastEntry->note = note;
  }
  
  if (maskVariable & 0x02) {
    hasInstrument = true;
    instrument = ifs.readu8();
    
    lastEntry->hasInstrument = hasInstrument;
    lastEntry->instrument = instrument;
  }
  
  if (maskVariable & 0x04) {
    hasVolPan = true;
    int volPan = (unsigned char)ifs.readu8();
    
    if (volPan <= 64) {
      volPanEffect = volEffectVolume;
      volPanValue = volPan;
    }
    else if ((volPan >= 128) && (volPan <= 192)) {
      volPanEffect = volEffectPanning;
      volPanValue = volPan - 128;
    }
    else if ((volPan >= 65) && (volPan <= 74)) {
      volPanEffect = volEffectFineVolumeUp;
      volPanValue = volPan - 65;
    }
    else if ((volPan >= 75) && (volPan <= 84)) {
      volPanEffect = volEffectFineVolumeDown;
      volPanValue = volPan - 75;
    }
    else if ((volPan >= 85) && (volPan <= 94)) {
      volPanEffect = volEffectVolumeSlideUp;
      volPanValue = volPan - 85;
    }
    else if ((volPan >= 95) && (volPan <= 104)) {
      volPanEffect = volEffectVolumeSlideDown;
      volPanValue = volPan - 95;
    }
    else if ((volPan >= 105) && (volPan <= 114)) {
      volPanEffect = volEffectPitchSlideDown;
      volPanValue = volPan - 105;
    }
    else if ((volPan >= 115) && (volPan <= 124)) {
      volPanEffect = volEffectPitchSlideUp;
      volPanValue = volPan - 115;
    }
    else if ((volPan >= 193) && (volPan <= 202)) {
      volPanEffect = volEffectTonePortamento;
      volPanValue = volPan - 193;
    }
    else if ((volPan >= 203) && (volPan <= 212)) {
      volPanEffect = volEffectVibratoDepth;
      volPanValue = volPan - 203;
    }
    else {
      volPanEffect = volEffectNone;
      volPanValue = 0;
    }
    
    lastEntry->hasVolPan = hasVolPan;
    lastEntry->volPanEffect = volPanEffect;
    lastEntry->volPanValue = volPanValue;
  }
  
  if (maskVariable & 0x08) {
    hasCommand = true;
//    command = ifs.readu8();
    command = static_cast<EffectId>(ifs.readu8());
    commandValue = ifs.readu8();
    
    lastEntry->hasCommand = hasCommand;
    lastEntry->command = command;
    lastEntry->commandValue = commandValue;
  }
}

bool SItEntry::isEmpty() const {
  return !(hasNote || hasInstrument || hasVolPan || hasCommand);
}

int SItEntry::getTriggerTick() const {
  // if there is an SDx delay effect, the trigger tick is moved back
  if (hasCommand
      && (command == effectS)
      && ((commandValue & 0xF0) == 0xD0)) {
    return (commandValue & 0x0F);
  }
  else {
    return 0;
  }
}


}
