#ifndef SITENVELOPE_H
#define SITENVELOPE_H


#include "util/TStream.h"
#include "util/TByte.h"
#include <vector>

namespace SVgm {


struct SItNodePoint {
  int yValue;
  int tickNum;
};

class SItEnvelope {
public:
  const static int maxNumNodePoints = 25;
  const static int unsignedEnvelopeRange = 64;
  const static int signedEnvelopeRange = 32;
  
  SItEnvelope();
  
  void read(BlackT::TStream& ifs,
            bool signedYValues);
  
  bool enabled() const;
  bool sustainLoopOn() const;
  bool loopOn() const;

  BlackT::TByte flags;
  BlackT::TByte numNodePoints;
  BlackT::TByte loopStart;
  BlackT::TByte loopEnd;
  BlackT::TByte sustainLoopStart;
  BlackT::TByte sustainLoopEnd;
  SItNodePoint nodePoints[maxNumNodePoints];
protected:
};


}


#endif
