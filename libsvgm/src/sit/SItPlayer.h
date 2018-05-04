#ifndef SITPLAYER_H
#define SITPLAYER_H


#include "sit/SItFile.h"
#include "sit/SItPlayerChannel.h"
#include <cstdlib>
#include <vector>

namespace SVgm {


class SItPlayer {
public:
  const static int maxNumChannels = 64;
  const static int maxGlobalVolume = 128;
  
  const static int songEndMarker = 0xFF;
  const static int separatorMarker = 0xFE;

  friend class SItPlayerChannel;

  SItPlayer(const SItFile& sit__);
  
//  int getOrderPos() const;

  void setCallbacks(void* callbackObj__ = NULL,
    void (*patternStartedCallback__)(void*, int, int) = NULL,
    void (*patternFinishedCallback__)(void*, int, int) = NULL,
    void (*playbackFinishedCallback__)(void*, int) = NULL,
    void (*effectTriggeredCallback__)(void*, const SItPlayerChannel&,
                                  const SItEntry&, int channelNum) = NULL);

  void runTick();
  
  int numChannels() const;
  const SItPlayerChannel& channel(int channelNum) const;
  
protected:
  const SItPattern& activePattern();
  void advanceTick();
  void advanceRow();
  void advancePattern();
  void goToRow(int pos);
  void goToOrder(int pos);

  const SItFile& sit;
  const SItHeader& header;
  const SItOrderList& orderList;
  SItPlayerChannel channels[maxNumChannels];
  
  bool started;
  
  int globalVolume;
  int speed;
  
  int orderPos;
  int rowNum;
  int tickNum;
  
  bool patternBreakTriggered;
  int patternBreakTarget;
  bool patternJumpTriggered;
  int patternJumpTarget;
  
  void* callbackObj;
  void (*patternStartedCallback)(void* obj, int patternNum, int orderPos);
  void (*patternFinishedCallback)(void* obj, int patternNum, int orderPos);
  void (*playbackFinishedCallback)(void* obj, int nextOrderPos);
  void (*effectTriggeredCallback)(void* obj, const SItPlayerChannel& channel,
                                  const SItEntry& entry, int channelNum);
  
  
};


}


#endif
