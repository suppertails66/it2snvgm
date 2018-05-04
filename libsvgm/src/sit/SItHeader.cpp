#include "sit/SItHeader.h"
#include <iostream>

using namespace BlackT;

namespace SVgm {


SItHeader::SItHeader()
  : numChannels(0) {
  
}

void SItHeader::read(BlackT::TStream& ifs) {
  numChannels = 0;

  ifs.read(header, sizeof(header));
  ifs.read(songName, sizeof(songName));
  patternHighlight = ifs.readu16le();
  numOrders = ifs.readu16le();
  numInstruments = ifs.readu16le();
  numSamples = ifs.readu16le();
  numPatterns = ifs.readu16le();
  createdWithVersion = ifs.readu16le();
  compatibleWithVersion = ifs.readu16le();
  flags = ifs.readu16le();
  special = ifs.readu16le();
  globalVolume = ifs.readu8();
  mixVolume = ifs.readu8();
  initialSpeed = ifs.readu8();
  initialTempo = ifs.readu8();
  panningSeparation = ifs.readu8();
  pitchWheelDepth = ifs.readu8();
  messageLength = ifs.readu16le();
  messageOffset = ifs.readu32le();
  ifs.readu32le();  // reserved
  for (int i = 0;
       i < sizeof(channelPanningList) / sizeof(BlackT::TByte);
       i++) {
    channelPanningList[i] = ifs.readu8();
    // we want numChannels to go up to the last non-disabled channel
    if ((unsigned char)channelPanningList[i] < 0x80) numChannels = i + 1;
  }
  for (int i = 0;
       i < sizeof(channelVolumeList) / sizeof(BlackT::TByte);
       i++) {
    channelVolumeList[i] = ifs.readu8();
  }
}


}
