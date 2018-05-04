#include "sit/SItPlayer.h"
#include <iostream>

namespace SVgm {

  
SItPlayer::SItPlayer(const SItFile& sit__)
  : sit(sit__),
    header(sit__.header),
    orderList(sit__.orderList),
    started(false),
    globalVolume(sit__.header.globalVolume),
    speed(sit__.header.initialSpeed),
    orderPos(-1),
    rowNum(-1),
    tickNum(-1),
    patternBreakTriggered(false),
    patternBreakTarget(-1),
    patternJumpTriggered(false),
    patternJumpTarget(-1),
    callbackObj(NULL),
    patternStartedCallback(NULL),
    patternFinishedCallback(NULL),
    playbackFinishedCallback(NULL),
    effectTriggeredCallback(NULL) {
  
  // initialize all (possible) channels
  for (int i = 0; i < maxNumChannels; i++) {
    channels[i] = SItPlayerChannel(*this);
    channels[i].channelNum = i;
  }
  
  // load default settings for channels
  for (int i = 0; i < maxNumChannels; i++) {
    channels[i].channelVolume = header.channelVolumeList[i];
    channels[i].panning = header.channelPanningList[i];
  }
  
  
}

void SItPlayer::setCallbacks(void* callbackObj__,
    void (*patternStartedCallback__)(void*, int, int),
    void (*patternFinishedCallback__)(void*, int, int),
    void (*playbackFinishedCallback__)(void*, int),
    void (*effectTriggeredCallback__)(void*, const SItPlayerChannel&,
                                  const SItEntry&, int channelNum)) {
  callbackObj = callbackObj__;
  patternStartedCallback = patternStartedCallback__;
  patternFinishedCallback = patternFinishedCallback__;
  playbackFinishedCallback = playbackFinishedCallback__;
  effectTriggeredCallback = effectTriggeredCallback__;
}

void SItPlayer::runTick() {
  if (!started) {
    goToOrder(0);
    tickNum = -1;
    started = true;
  }
  
  // this implementation is not correct -- B+C won't work
  // good enough for now
  // pattern break/jump check is done as soon as the row has finished (we
  // haven't incremented the tick number yet and it will roll over once
  // advancePattern() is called, so the check is -1)
  if ((tickNum >= (speed - 1))) {
    if (patternBreakTriggered) {
      advancePattern();
      patternBreakTriggered = false;
    }
    // as far as we're concerned, playback ends once we jump (loop)
    else if (patternJumpTriggered) {
      // orderPos is already updated to new target
      if (playbackFinishedCallback != NULL)
        (*playbackFinishedCallback)(callbackObj, patternJumpTarget);
//      patternJumpTriggered = false;
      return;
    }
    else {
      advanceTick();
    }
  }
  else {
    advanceTick();
  }
  
  // get the number of the next pattern
  int patternNum = orderList.orders[orderPos];
  
  // check for end marker (valid file should always have one)
//  if (orderPos >= orderList.orders.size()) {
  if (patternNum == songEndMarker) {
    if (playbackFinishedCallback != NULL)
      (*playbackFinishedCallback)(callbackObj, orderPos);
    return;
  }
  
  const SItPattern& pattern = activePattern();
  
  for (int i = 0; i < header.numChannels; i++) {
    channels[i].runTick(pattern.rows[rowNum].entries[i],
                        tickNum);
  }
}

int SItPlayer::numChannels() const {
  return header.numChannels;
}

const SItPlayerChannel& SItPlayer::channel(int channelNum) const {
  return channels[channelNum];
}

const SItPattern& SItPlayer::activePattern() {
  return sit.patterns[orderList.orders[orderPos]];
}

void SItPlayer::advanceTick() {
  // advance to next tick (row, pattern, etc. as necessary)
  ++tickNum;
  if (tickNum >= speed) {
    tickNum = 0;
    advanceRow();
  }
}

void SItPlayer::advanceRow() {
  ++rowNum;
  
  const SItPattern& pattern = activePattern();
  if (rowNum >= pattern.rows.size()) {
    rowNum = 0;
    advancePattern();
  }
}

void SItPlayer::advancePattern() {
  ++orderPos;
  // skip past any separators in the order list
  while (orderList.orders[orderPos] == separatorMarker) {
    // ... but for the sake of simplicity, inform the client that we "started"
    // the pattern
    if (patternStartedCallback != NULL)
      (*patternStartedCallback)(callbackObj, orderList.orders[orderPos],
                                orderPos);
    ++orderPos;
  }
  
  goToOrder(orderPos);
}

void SItPlayer::goToRow(int pos) {
  tickNum = 0;
  rowNum = pos;
}

void SItPlayer::goToOrder(int pos) {
  orderPos = pos;
  goToRow(0);

  if (patternStartedCallback != NULL)
    (*patternStartedCallback)(callbackObj, orderList.orders[orderPos],
                              orderPos);
}


}
