#ifndef SITTOSMSVGM_H
#define SITTOSMSVGM_H


#include "util/TStream.h"
#include "sit/SItFile.h"
#include "svgm/SSmsVgmFile.h"
#include "sit/SItPlayer.h"
#include <vector>

namespace SVgm {


struct ToneChannelState {
  int tone;
  int volume;
};

struct NoiseChannelState {
  SSmsVgmFile::NoiseMode noiseMode;
  SSmsVgmFile::NoiseRate noiseRate;
  int volume;
};

struct SmsChannelStates {
  SmsChannelStates();

  ToneChannelState tone0;
  ToneChannelState tone1;
  ToneChannelState tone2;
  NoiseChannelState noise;
  
  // for determining state during output computation
  bool tone0Set;
  bool tone1Set;
  bool tone2Set;
  bool noiseSet;
};

struct ItChannelState {
  ItChannelState();

  SSmsVgmFile::NoiseMode noiseMode;
};

class SItToSmsVgm {
public:
  SItToSmsVgm(const SItFile& sit__,
              SSmsVgmFile& vgm__);
  
  void operator()();
protected:
  const static double noteFreqScaler;
  const static double baseNoteFreq;
  const static double toneNoteShift;
  const static double noisePeriodicNoteShift;
  const static double noiseLowThreshold;
  const static double noiseMidThreshold;
  const static double noiseHighThreshold;
  const static double noiseTone2Threshold;
  
  static void patternStartedCallback(
    void* obj, int patternNum, int orderPos);
  static void patternFinishedCallback(
    void* obj, int patternNum, int orderPos);
  static void playbackFinishedCallback(
    void* obj, int nextOrderPos);
  static void effectTriggeredCallback(
    void* obj, const SItPlayerChannel& channel,
    const SItEntry& entry, int channelNum);
  
  void patternStarted(int patternNum, int orderPos);
  void patternFinished(int patternNum, int orderPos);
  void playbackFinished(int nextOrderPos);
  void effectTriggered(const SItPlayerChannel& channel,
                       const SItEntry& entry, int channelNum);
  
  static double noteToFrequency(double note);
  static double getAdjustedVolume(double volume);
  
  void updateChannels();
  
  void updateToneChannels(SmsChannelStates& states);
  void updateNoiseChannels(SmsChannelStates& states);
  
  void updateToneChannel(ToneChannelState& state,
                         int index,
                         int newTone, int newVolume);
  
  void updateNoiseChannel(NoiseChannelState& state,
                          int index,
                          SSmsVgmFile::NoiseRate newRate,
                          SSmsVgmFile::NoiseMode newMode,
                          int newVolume);
  
  const SItFile& sit;
  SSmsVgmFile& vgm;
  SItPlayer player;
  
  SmsChannelStates channelStates;
  ItChannelState itChannelStates[SItPlayer::maxNumChannels];
  
  std::vector<int> orderLoopPoints;
  std::vector<int> orderSampleSizes;
  bool localLoopRecordPending;
  int localLoopRecordOrderPos;
  bool hasLoop;
  int loopStartPos;
  int loopSampleStart;
  
  bool done;
};


}


#endif
