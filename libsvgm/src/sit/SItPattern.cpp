#include "sit/SItPattern.h"
#include <iostream>

namespace SVgm {


SItPattern::SItPattern() {
  
}

void SItPattern::read(BlackT::TStream& ifs,
                      int numChannels) {
//  int length = ifs.readu16le();
  ifs.readu16le();
  int numRows = ifs.readu16le();
  // reserved
  ifs.readu32le();

  BlackT::TByte maskVariableBuffer[99];
  SItEntry lastDataBuffer[99];
  rows.resize(numRows);
  for (int i = 0; i < numRows; i++) {
    rows[i].read(ifs, numChannels, maskVariableBuffer, lastDataBuffer);
  }
}


}
