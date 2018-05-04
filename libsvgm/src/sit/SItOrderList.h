#ifndef SITORDERLIST_H
#define SITORDERLIST_H


#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>

namespace SVgm {


class SItOrderList {
public:
  friend class SItFile;
  
  SItOrderList();
  
  void read(BlackT::TStream& ifs,
            int numOrders);
  
  std::vector<BlackT::TByte> orders;
protected:
};


}


#endif
