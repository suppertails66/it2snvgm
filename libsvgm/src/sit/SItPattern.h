#ifndef SITPATTERN_H
#define SITPATTERN_H


#include "sit/SItRow.h"
#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>

namespace SVgm {


class SItPattern {
public:
  friend class SItFile;
  
  SItPattern();
  
  void read(BlackT::TStream& ifs,
            int numChannels);
  
  std::vector<SItRow> rows;
protected:
  
};


}


#endif
