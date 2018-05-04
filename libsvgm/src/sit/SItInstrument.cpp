#include "sit/SItInstrument.h"
#include <iostream>

using namespace BlackT;

namespace SVgm {


SItInstrument::SItInstrument() {
  
}

void SItInstrument::read(BlackT::TStream& ifs) {
  int base = ifs.tell();
  
  ifs.seek(base + 0x14);
  fadeOut = ifs.readu16le();
  
  ifs.seek(base + 0x18);
  globalVolume = ifs.readu8();
  
  ifs.seek(base + 0x130);
  volumeEnvelope.read(ifs, false);
  panningEnvelope.read(ifs, true);
  pitchEnvelope.read(ifs, true);
}


}
