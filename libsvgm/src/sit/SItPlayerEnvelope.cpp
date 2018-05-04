#include "sit/SItPlayerEnvelope.h"
#include <iostream>

namespace SVgm {


SItPlayerEnvelope::SItPlayerEnvelope() { }

SItPlayerEnvelope::SItPlayerEnvelope(const SItEnvelope& env__)
  : done(false),
    sustaining(true),
    env_(&env__),
    currentNode(0),
    tickPos(-1) { }

bool SItPlayerEnvelope::enabled() const {
  return env_->enabled();
}
  
void SItPlayerEnvelope::runTick() {
  if (!enabled() || done) return;

  ++tickPos;
  
  // if we just advanced past the current node, evaluate loop conditions
  // and so on
/*  if ((tickPos - 1) == env_->nodePoints[currentNode].tickNum) {
    // sustain loop overrides loop if both are active and note is sustaining
    if (env_->sustainLoopOn() && sustaining) {
      
    }
    else if (env_->loopOn()) {
      
    }
  } */
  
  if (sustaining
      && env_->sustainLoopOn()
      && (tickPos > env_->nodePoints[env_->sustainLoopEnd].tickNum)) {
    // in sustain loop
    tickPos = env_->nodePoints[env_->sustainLoopStart].tickNum;
    currentNode = env_->sustainLoopStart;
  }
  else if ((!sustaining
             || (!env_->sustainLoopOn()))
      && env_->loopOn()
      && (tickPos > env_->nodePoints[env_->loopEnd].tickNum)) {
    // in loop
    tickPos = env_->nodePoints[env_->loopStart].tickNum;
    currentNode = env_->loopStart;
  }
  else if ((currentNode < env_->numNodePoints - 1)
      && (tickPos >= env_->nodePoints[currentNode + 1].tickNum)) {
    // reached next node
    ++currentNode;
  }
  else if (currentNode == env_->numNodePoints - 1) {
    // reached end of envelope, and no loop
    tickPos = env_->nodePoints[currentNode].tickNum;
    done = true;
    return;
  }
}

double SItPlayerEnvelope::currentValue() const {
  // if at a node, return its value
  // (this also takes care of the node at the end of the envelope)
  if (tickPos == env_->nodePoints[currentNode].tickNum) {
    return env_->nodePoints[currentNode].yValue;
  }
  
  // otherwise, interpolate linearly between this node and the next one to get
  // the value at this point
  double currentNodeY = env_->nodePoints[currentNode].yValue;
  double currentNodeTick = env_->nodePoints[currentNode].tickNum;
  double nextNodeY = env_->nodePoints[currentNode + 1].yValue;
  double nextNodeTick = env_->nodePoints[currentNode + 1].tickNum;
  double x = tickPos - currentNodeTick;
  
  // slope should never be undefined; multiple nodes cannot exist at the same
  // point (legally)
  double slope = (nextNodeY - currentNodeY) / (nextNodeTick - currentNodeTick);
  double result = (slope * x) + currentNodeY;
  
  return result;
}


}
