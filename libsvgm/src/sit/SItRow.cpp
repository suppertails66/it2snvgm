#include "sit/SItRow.h"
#include <iostream>

using namespace BlackT;

namespace SVgm {


SItRow::SItRow() {
  
}

void SItRow::read(BlackT::TStream& ifs,
                  int numChannels,
                  BlackT::TByte* maskVariableBuffer,
                  SItEntry* lastDataBuffer) {
  entries.resize(numChannels);
  
  while (true) {
    TByte channelVariable = ifs.readu8();
    if (channelVariable == 0) return;
    int channelNum = (channelVariable - 1) & 0x3F;
    
    TByte maskVariable;
    if (channelVariable & 0x80) {
      maskVariable = ifs.readu8();
      maskVariableBuffer[channelNum] = maskVariable;
    }
    else {
      maskVariable = maskVariableBuffer[channelNum];
    }
    
//    SItEntry* entryP = NULL;
//    if (lastRow != NULL) {
//      entryP = &(lastRow->entries[channelNum]);
//    }
    
    entries[channelNum].read(ifs, maskVariable, &(lastDataBuffer[channelNum]));
  }
}


}
