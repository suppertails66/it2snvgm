#include "sit/SItFile.h"
#include <iostream>

namespace SVgm {


SItFile::SItFile() {
  
}

void SItFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
  orderList.read(ifs, header.numOrders);
  
    
  instruments.resize(header.numInstruments);
  for (int i = 0; i < header.numInstruments; i++) {
    int instrumentOffset = ifs.readu32le();
    int oldaddr = ifs.tell();
    ifs.seek(instrumentOffset);
    instruments[i].read(ifs);
    ifs.seek(oldaddr);
  }
  
  for (int i = 0; i < header.numSamples; i++) {
    int sampleOffset = ifs.readu32le();
  }
  
  patterns.resize(header.numPatterns);
//  std::cout << header.numChannels << std::endl;
  for (int i = 0; i < header.numPatterns; i++) {
    int patternOffset = ifs.readu32le();
    
    // offset of zero = implicit 64-row empty pattern
    if (patternOffset == 0) {
      patterns[i].rows.resize(64);
      continue;
    }
    
    int oldaddr = ifs.tell();
    ifs.seek(patternOffset);
    patterns[i].read(ifs, 64);
    ifs.seek(oldaddr);
  }
  
  // we don't handle the old instrument format
  if (header.compatibleWithVersion >= 0x200) {
    
  }
}


}
