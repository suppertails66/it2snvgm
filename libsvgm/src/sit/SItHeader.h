#ifndef SITHEADER_H
#define SITHEADER_H


#include "util/TStream.h"
#include "util/TByte.h"
#include <vector>

namespace SVgm {


class SItHeader {
public:
  friend class SItFile;

  const static int size = 0xC0;

  SItHeader();
  
  void read(BlackT::TStream& ifs);
  
  char header[4];
  char songName[26];
  int patternHighlight;
  int numOrders;
  int numInstruments;
  int numSamples;
  int numPatterns;
  int createdWithVersion;
  int compatibleWithVersion;
  int flags;
  int special;
  int globalVolume;
  int mixVolume;
  int initialSpeed;
  int initialTempo;
  int panningSeparation;
  int pitchWheelDepth;
  int messageLength;
  int messageOffset;
//  int reserved;
  BlackT::TByte channelVolumeList[64];
  BlackT::TByte channelPanningList[64];
  
  int numChannels;
protected:
  
  
};


}


#endif
