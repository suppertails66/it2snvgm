#include "sit/SItOrderList.h"

namespace SVgm {


SItOrderList::SItOrderList() {
  
}

void SItOrderList::read(BlackT::TStream& ifs,
                    int numOrders) {
  orders.resize(numOrders);
  for (int i = 0; i < numOrders; i++) orders[i] = ifs.readu8();
}


}
