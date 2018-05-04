#ifndef SITFILE_H
#define SITFILE_H


#include "sit/SItHeader.h"
#include "sit/SItOrderList.h"
#include "sit/SItInstrument.h"
#include "sit/SItPattern.h"
#include "util/TStream.h"
#include <vector>

namespace SVgm {


class SItFile {
public:
  SItFile();
  
  void read(BlackT::TStream& ifs);
  
  SItHeader header;
  SItOrderList orderList;
  std::vector<SItInstrument> instruments;
  std::vector<SItPattern> patterns;
protected:
  
  
};


}


#endif
