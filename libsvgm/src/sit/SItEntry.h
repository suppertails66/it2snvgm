#ifndef SITENTRY_H
#define SITENTRY_H


#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>
#include <cstdlib>

namespace SVgm {


class SItEntry {
public:
//  friend class SItFile;
  
  // arbitrarily assigned identifiers
  enum VolEffectId {
    volEffectNone,
    volEffectVolume,
    volEffectPanning,
    volEffectFineVolumeUp,
    volEffectFineVolumeDown,
    volEffectVolumeSlideUp,
    volEffectVolumeSlideDown,
    volEffectPitchSlideDown,
    volEffectPitchSlideUp,
    volEffectTonePortamento,
    volEffectVibratoDepth
  };
  
  // these correspond to the actual values
  enum EffectId {
    effectNone =  0,
    effectA    =  1,    // etc.
    effectB,
    effectC,
    effectD,
    effectE,
    effectF,
    effectG,
    effectH,
    effectI,
    effectJ,
    effectK,
    effectL,
    effectM,
    effectN,
    effectO,
    effectP,
    effectQ,
    effectR,
    effectS,
    effectT,
    effectU,
    effectV,
    effectW,
    effectX,
    effectY,
    effectZ,
    effectBackslash,
    effectColon,
    effectPound
  };
  
  SItEntry();
  
  void read(BlackT::TStream& ifs,
            int maskVariable,
            SItEntry* lastEntry);

  bool hasNote;
  BlackT::TByte note;
  
  bool hasInstrument;
  BlackT::TByte instrument;
  
  bool hasVolPan;
//  BlackT::TByte volPan;
  VolEffectId volPanEffect;
  BlackT::TByte volPanValue;
  
  bool hasCommand;
  EffectId command;
  BlackT::TByte commandValue;
  
  bool isEmpty() const;
  int getTriggerTick() const;
  
protected:
  
};


}


#endif
