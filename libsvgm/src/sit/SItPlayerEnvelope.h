#ifndef SITPLAYERENVELOPE_H
#define SITPLAYERENVELOPE_H


#include "sit/SItEnvelope.h"
#include "util/TByte.h"

namespace SVgm {


class SItPlayerEnvelope {
public:
  SItPlayerEnvelope();
  SItPlayerEnvelope(const SItEnvelope& env__);
  
  bool enabled() const;
  
  bool done;
  bool sustaining;
  
  void runTick();
  double currentValue() const;
  
protected:
  
  const SItEnvelope* env_;
  int currentNode;
  int tickPos;
  
};


}


#endif
