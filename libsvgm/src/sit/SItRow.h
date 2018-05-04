#ifndef SITROW_H
#define SITROW_H


#include "sit/SItEntry.h"
#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>
#include <cstdlib>

namespace SVgm {


class SItRow {
public:
//  friend class SItFile;
  
  SItRow();
  
  void read(BlackT::TStream& ifs,
            int numChannels,
            BlackT::TByte* maskVariableBuffer,
            SItEntry* lastDataBuffer);

  std::vector<SItEntry> entries;
protected:
  
};


}


#endif
