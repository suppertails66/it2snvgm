#include "sit/SItEnvelope.h"
#include <iostream>

namespace SVgm {


SItEnvelope::SItEnvelope() { }

void SItEnvelope::read(BlackT::TStream& ifs,
                       bool signedYValues) {
  flags = ifs.readu8();
  numNodePoints = ifs.readu8();
  loopStart = ifs.readu8();
  loopEnd = ifs.readu8();
  sustainLoopStart = ifs.readu8();
  sustainLoopEnd = ifs.readu8();
  
  for (int i = 0; i < maxNumNodePoints; i++) {
    int yValue = ifs.reads8();
//    if (signedYValues && (yValue > 32)) yValue -= 64;
    nodePoints[i].yValue = yValue;
    nodePoints[i].tickNum = ifs.readu16le();
  }
  
  // filler
  ifs.seekoff(1);
}

bool SItEnvelope::enabled() const {
  return ((flags & 0x01) != 0);
}

bool SItEnvelope::sustainLoopOn() const {
  return ((flags & 0x04) != 0);
}

bool SItEnvelope::loopOn() const {
  return ((flags & 0x02) != 0);
}


}
