#ifndef SITINSTRUMENT_H
#define SITINSTRUMENT_H


#include "sit/SItEnvelope.h"
#include "util/TStream.h"
#include <vector>

namespace SVgm {


class SItInstrument {
public:
  friend class SItFile;
  
  const static int minGlobalVolume = 0;
  const static int maxGlobalVolume = 128;

  SItInstrument();
  
  void read(BlackT::TStream& ifs);
  
  int fadeOut;
  int globalVolume;
  SItEnvelope volumeEnvelope;
  SItEnvelope panningEnvelope;
  SItEnvelope pitchEnvelope;
protected:
};


}


#endif
