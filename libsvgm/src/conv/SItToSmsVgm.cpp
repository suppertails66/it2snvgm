#include "conv/SItToSmsVgm.h"
#include <iostream>
#include <cmath>

namespace SVgm {


const double SItToSmsVgm::noteFreqScaler = pow(2, (double)1 / (double)12);
const double SItToSmsVgm::baseNoteFreq = 16.35;   // note 0 = C0

// amount by which notes on the tone channel are shifted upward before
// being converted to pitch
//
// the lowest possible tone note is approximately A2; we map this to A0
// to allow use of more of the chip's range, resulting in a shift of 24 notes
const double SItToSmsVgm::toneNoteShift = 24;
// map periodic noise tones by the same logic (lowest note is 4 octaves lower
// than regular tone)
const double SItToSmsVgm::noisePeriodicNoteShift = 72;

const double SItToSmsVgm::noiseLowThreshold = 11;     // B0
const double SItToSmsVgm::noiseMidThreshold = 23;     // B1
const double SItToSmsVgm::noiseHighThreshold = 35;    // B2
const double SItToSmsVgm::noiseTone2Threshold = 47;   // B3

SmsChannelStates::SmsChannelStates() {
  tone0.tone = SSmsVgmFile::silenceTone;
  tone0.volume = SSmsVgmFile::minVolume;
  tone1.tone = SSmsVgmFile::silenceTone;
  tone1.volume = SSmsVgmFile::minVolume;
  tone2.tone = SSmsVgmFile::silenceTone;
  tone2.volume = SSmsVgmFile::minVolume;
  noise.noiseMode = SSmsVgmFile::noiseModeWhite;
  noise.noiseRate = SSmsVgmFile::noiseRateHigh;
  noise.volume = SSmsVgmFile::minVolume;
  
  tone0Set = false;
  tone1Set = false;
  tone2Set = false;
  noiseSet = false;
}

ItChannelState::ItChannelState()
  : noiseMode(SSmsVgmFile::noiseModeWhite) { }

SItToSmsVgm::SItToSmsVgm(const SItFile& sit__,
                         SSmsVgmFile& vgm__)
  : sit(sit__),
    vgm(vgm__),
    player(sit__),
    localLoopRecordPending(false),
    localLoopRecordOrderPos(-1),
    hasLoop(false),
    loopStartPos(-1),
    loopSampleStart(-1),
    done(false) {
  // initialize channel data
  // the sentinel values ensure that the initial channel state is output
  // as soon as activity occurs on a channel
  // we do this as opposed to e.g. initializing all channels to muted tone 0
  // to make sure tracks can purposely have no activity on a particular
  // channel
/*  channelStates.tone0.tone = -1;
  channelStates.tone0.volume = -1;
  channelStates.tone1.tone = -1;
  channelStates.tone1.volume = -1;
  channelStates.tone2.tone = -1;
  channelStates.tone2.volume = -1;
  channelStates.noise.rate = -1;
  channelStates.noise.volume = -1; */
  
  // screw it, this is probably better anyway
  vgm.setTone0(channelStates.tone0.tone);
  vgm.setTone1(channelStates.tone1.tone);
  vgm.setTone2(channelStates.tone2.tone);
  vgm.setNoise(channelStates.noise.noiseMode, channelStates.noise.noiseRate);
  vgm.setVolume(0, channelStates.tone0.volume);
  vgm.setVolume(1, channelStates.tone1.volume);
  vgm.setVolume(2, channelStates.tone2.volume);
  vgm.setVolume(3, channelStates.noise.volume);
  
  orderLoopPoints.resize(sit.orderList.orders.size());
  orderSampleSizes.resize(sit.orderList.orders.size());
}
  
void SItToSmsVgm::operator()() {
//  SItPlayer player(sit);
  player.setCallbacks((void*)this,
    &patternStartedCallback, &patternFinishedCallback,
    &playbackFinishedCallback, &effectTriggeredCallback);
  
  while (!done) {
    player.runTick();
    
    if (!done) {
      // the player runs a tick, notices it has started a new pattern, calls
      // the callback, and then runs the first tick of the next row.
      // we want to mark the start point before adding the commands for that
      // row
//      if (localLoopRecordPending) {
//        orderLoopPoints[localLoopRecordOrderPos] = vgm.dataSize();
//        orderSampleSizes[localLoopRecordOrderPos] = vgm.numSamples();
//        localLoopRecordPending = false;
//      }
      
      updateChannels();
    }
  }
  
  if (hasLoop) {
    // silence all channels before looping; otherwise, held values will stick
    // (and cannot be interrupted by note cut commands in the IT -- those don't
    // exist in the VGM!)
    SmsChannelStates defaultStates;
    vgm.setTone0(defaultStates.tone0.tone);
    vgm.setTone1(defaultStates.tone1.tone);
    vgm.setTone2(defaultStates.tone2.tone);
    vgm.setNoise(defaultStates.noise.noiseMode, defaultStates.noise.noiseRate);
    vgm.setVolume(0, defaultStates.tone0.volume);
    vgm.setVolume(1, defaultStates.tone1.volume);
    vgm.setVolume(2, defaultStates.tone2.volume);
    vgm.setVolume(3, defaultStates.noise.volume);
  }
  
  vgm.addEndOfData();
  
  if (hasLoop) {
    int loopLen = vgm.numSamples() - loopSampleStart;
    vgm.setLoop(loopStartPos, loopLen);
  }
}

void SItToSmsVgm::patternStartedCallback(
  void* obj, int patternNum, int orderPos) {
  ((SItToSmsVgm*)obj)->patternStarted(patternNum, orderPos);
}

void SItToSmsVgm::patternFinishedCallback(
  void* obj, int patternNum, int orderPos) {
  ((SItToSmsVgm*)obj)->patternFinished(patternNum, orderPos);
}

void SItToSmsVgm::playbackFinishedCallback(
  void* obj, int nextOrderPos) {
  ((SItToSmsVgm*)obj)->playbackFinished(nextOrderPos);
}

void SItToSmsVgm::effectTriggeredCallback(
  void* obj, const SItPlayerChannel& channel,
  const SItEntry& entry, int channelNum) {
  ((SItToSmsVgm*)obj)->effectTriggered(channel, entry, channelNum);
}

void SItToSmsVgm::patternStarted(int patternNum, int orderPos) {
//  localLoopRecordPending = true;
//  localLoopRecordOrderPos = orderPos;

  // note the position in the VGM of the start of each order, in case
  // we need to loop to it later
  orderLoopPoints[orderPos] = vgm.dataSize();
  orderSampleSizes[orderPos] = vgm.numSamples();
}

void SItToSmsVgm::patternFinished(int patternNum, int orderPos) {
  
}

void SItToSmsVgm::playbackFinished(int nextOrderPos) {
  done = true;
  
  // if the next order is a terminator pattern, there's no loop
  if (sit.orderList.orders[nextOrderPos] != SItPlayer::songEndMarker) {
    hasLoop = true;
    loopStartPos = orderLoopPoints[nextOrderPos];
    loopSampleStart = orderSampleSizes[nextOrderPos];
  }
}

void SItToSmsVgm::effectTriggered(const SItPlayerChannel& channel,
                     const SItEntry& entry, int channelNum) {
  if (entry.command == SItEntry::effectS) {
    BlackT::TByte param = entry.commandValue;
    if ((param & 0xF0) == 0x70) {
      if ((param & 0x0F) == 0x0D) {
        // white noise mode
        itChannelStates[channelNum].noiseMode = SSmsVgmFile::noiseModeWhite;
      }
      else if ((param & 0x0F) == 0x0E) {
        // periodic noise mode
        itChannelStates[channelNum].noiseMode = SSmsVgmFile::noiseModePeriodic;
      }
    }
  }
}
  
double SItToSmsVgm::noteToFrequency(double note) {
  return (baseNoteFreq * pow(noteFreqScaler, note));
}

// this table is taken from the SMS Power! SN76489 documentation, because I
// can't be bothered recalculating the values for my own representation of
// volume.
// the function below simply maps a 0->1.0 volume to the ranges given
// here in order to produce more more linear voluming
//
// (this is all supremely awful and would be much better served by just
// building a lookup table for the 64 possible IT volume levels)

//int volume_table[16]={
//  32767, 26028, 20675, 16422, 13045, 10362,  8231,  6568,
//  5193,  4125,  3277,  2603,  2067,  1642,  1304,     0
//};
// reversed
int volume_table[16]={
  0, 1304, 1642, 2067, 2603, 3277, 4125, 5193,
  6568, 8231, 10362, 13045, 16422, 20675, 26028, 32767
};
double SItToSmsVgm::getAdjustedVolume(double volume) {
/*  volume *= 32767;
  for (int i = 0; i < (sizeof(volume_table) / sizeof(int)) - 1; i++) {
    if (volume > volume_table[i + 1]) continue;
    
    // return whichever value the input volume is closer to as a fraction
    double first = volume - volume_table[i];
    double second = volume_table[i + 1];
    if ((volume - first) > (second - volume)) {
      return second / (double)32767;
    }
    else {
      return first / (double)32767;
    }
  }
  
  return 1.0; */
  
  // uuuugh what a mess
  // just use some hack for now and don't worry about it too much
  
  // linear scaling
/*  if      (volume >= ((double)15)/16)  return (double)15 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)14)/16)  return (double)14 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)13)/16)  return (double)13 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)12)/16)  return (double)12 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)11)/16)  return (double)11 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)10)/16)  return (double)10 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 9)/16)  return (double) 9 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 8)/16)  return (double) 8 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 7)/16)  return (double) 7 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 6)/16)  return (double) 6 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 5)/16)  return (double) 5 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 4)/16)  return (double) 4 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 3)/16)  return (double) 3 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 2)/16)  return (double) 2 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 1)/16)  return (double) 1 / SSmsVgmFile::maxVolume;
  else if (volume >  ((double) 0)/16)  return (double) 1 / SSmsVgmFile::maxVolume;
  else                                 return (double) 0 / SSmsVgmFile::maxVolume; */
  // expanded upper ranges
  if      (volume >= ((double)28)/32)  return (double)15 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)24)/32)  return (double)14 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)20)/32)  return (double)13 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)18)/32)  return (double)12 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)16)/32)  return (double)11 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)14)/32)  return (double)10 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)12)/32)  return (double) 9 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double)10)/32)  return (double) 8 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 8)/32)  return (double) 7 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 7)/32)  return (double) 6 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 6)/32)  return (double) 5 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 5)/32)  return (double) 4 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 4)/32)  return (double) 3 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 3)/32)  return (double) 2 / SSmsVgmFile::maxVolume;
  else if (volume >= ((double) 2)/32)  return (double) 1 / SSmsVgmFile::maxVolume;
  else if (volume >  ((double) 0)/32)  return (double) 1 / SSmsVgmFile::maxVolume;
  else                                 return (double) 0 / SSmsVgmFile::maxVolume;
  
}

void SItToSmsVgm::updateChannels() {
//  for (int i = 0; i < player.numChannels(); i++) {

  // Channels in the input are mapped as follows:
  //
  // Channels 1, 5, 9, ...  == tone channel 1
  // Channels 2, 6, 10, ... == tone channel 2
  // Channels 3, 7, 11, ... == tone channel 3
  // Channels 4, 8, 12, ... == noise channel
  //
  // Lower-numbered input channels have higher priority as long as they are
  // producing sound. For example, if channels 1 and 5 are both playing notes,
  // channel 1 will be output. However, if channel 1 is producing silence while
  // channel 5 is playing a note, the note on channel 5 will be output.
  
  // Buffer to hold output states
  SmsChannelStates newStates;

  updateToneChannels(newStates);
  updateNoiseChannels(newStates);
    
//    updateToneChannel(*target, targetIndex, rawOutputTone, rawOutputVolume);

  // output the finalized tone states
  updateToneChannel(channelStates.tone0, 0,
                    newStates.tone0.tone, newStates.tone0.volume);
  updateToneChannel(channelStates.tone1, 1,
                    newStates.tone1.tone, newStates.tone1.volume);
  updateToneChannel(channelStates.tone2, 2,
                    newStates.tone2.tone, newStates.tone2.volume);
  updateNoiseChannel(channelStates.noise, 3,
                    newStates.noise.noiseRate,
                    newStates.noise.noiseMode,
                    newStates.noise.volume);
  
  vgm.addWaitFrame();
}
  
void SItToSmsVgm::updateToneChannels(SmsChannelStates& states) {
  for (int i = 0; i < sit.header.numChannels; i++) {
//  for (int i = 0; i < 4; i++) {
    // skip noise channels
    if (((i % 4) == 3)) continue;
    
    int targetIndex = (i % 4);
    ToneChannelState* target = NULL;
    bool* toneSetP;
    switch (targetIndex) {
    case 0:
      target = &states.tone0;
      toneSetP = &states.tone0Set;
      break;
    case 1:
      target = &states.tone1;
      toneSetP = &states.tone1Set;
      break;
    case 2:
      target = &states.tone2;
      toneSetP = &states.tone2Set;
      break;
    default:
      
      break;
    }
    bool& toneSet = *toneSetP;
    
    // stop if channel already finalized
    if (toneSet) continue;
    
    // stop if channel not available
    const SItPlayerChannel& channel = player.channel(i);
    if (!channel.enabled()) continue;
  
    // get note and apply shift
    double note = channel.currentOutputNote();
    note += toneNoteShift;
    
    // compute raw SMS values for note/volume
    double freq = noteToFrequency(note);
    int rawOutputTone = vgm.frequencyToTone(freq);
    double volume = channel.currentOutputVolumeMultiplier();
    volume = getAdjustedVolume(volume);
    int rawOutputVolume = volume * SSmsVgmFile::maxVolume;
    
    // set new values only if not producing silence
    if (rawOutputVolume != SSmsVgmFile::minVolume) {
      target->tone = rawOutputTone;
      target->volume = rawOutputVolume;
      
      // prevent this channel from being overriden by a higher one
      toneSet = true;
    }
    else if (channel.currentOutputNote() <= SItPlayerChannel::maxNote) {
      // as long as note is valid, update tone regardless of volume:
      // we need this for noise tone2 stuff
      target->tone = rawOutputTone;
    }
  }
}
  
void SItToSmsVgm::updateNoiseChannels(SmsChannelStates& states) {
  for (int i = 3; i < sit.header.numChannels; i += 4) {
    int targetIndex = (i % 4);
    NoiseChannelState* target = &states.noise;
    bool& noiseSet = states.noiseSet;
    
    // stop if channel already finalized
    if (noiseSet) continue;
    
    // stop if channel not available
    const SItPlayerChannel& channel = player.channel(i);
    if (!channel.enabled()) continue;
    
    // get the current noise mode for this IT channel
//    SSmsVgmFile::NoiseMode noiseMode
//      = SSmsVgmFile::noiseModeWhite;
    SSmsVgmFile::NoiseMode noiseMode = itChannelStates[i].noiseMode;
    // get current noise rate
    SSmsVgmFile::NoiseRate noiseRate = target->noiseRate;
  
    // get note
    double note = channel.currentOutputNote();
//    note += noiseNoteShift;
    
//    if (noiseMode == SSmsVgmFile::noiseModePeriodic) {
//      
//    }
//    else {
//    
//    }
    
    // determine whether to interpret note as low-, mid-, high-, or
    // tone2-rate noise
    if (note <= noiseLowThreshold)
      noiseRate = SSmsVgmFile::noiseRateLow;
    else if (note <= noiseMidThreshold)
      noiseRate = SSmsVgmFile::noiseRateMid;
    else if (note <= noiseHighThreshold)
      noiseRate = SSmsVgmFile::noiseRateHigh;
    else //if (note <= noiseToneThreshold)
      noiseRate = SSmsVgmFile::noiseRateTone2;
    
    double volume = channel.currentOutputVolumeMultiplier();
    volume = getAdjustedVolume(volume);
    int rawOutputVolume = volume * SSmsVgmFile::maxVolume;
    
    // set new values only if not producing silence
    if (rawOutputVolume != SSmsVgmFile::minVolume) {
      target->noiseRate = noiseRate;
      target->noiseMode = noiseMode;
      target->volume = rawOutputVolume;
      
      // prevent this channel from being overriden by a higher one
      noiseSet = true;
    }
    
    // compute raw SMS values for note/volume
/*    double freq = noteToFrequency(note);
    int rawOutputTone = vgm.frequencyToTone(freq);
    double volume = channel.currentOutputVolumeMultiplier();
//    volume = getAdjustedVolume(volume);
    int rawOutputVolume = volume * SSmsVgmFile::maxVolume;
    
    // set new values only if not producing silence
    if (rawOutputVolume != SSmsVgmFile::minVolume) {
      target->tone = rawOutputTone;
      target->volume = rawOutputVolume;
      
      // prevent this channel from being overriden by a higher channel
      toneSet = true;
    } */
  }
}

void SItToSmsVgm::updateToneChannel(ToneChannelState& state,
                       int index,
                       int newTone, int newVolume) {
  // write new values to VGM only if changed
  
  if (state.tone != newTone) {
    state.tone = newTone;
    vgm.setTone(index, state.tone);
  }
  
  if (state.volume != newVolume) {
    state.volume = newVolume;
    vgm.setVolume(index, state.volume);
  }
}

  
void SItToSmsVgm::updateNoiseChannel(NoiseChannelState& state,
                        int index,
                        SSmsVgmFile::NoiseRate newRate,
                        SSmsVgmFile::NoiseMode newMode,
                        int newVolume) {
  // write new values to VGM only if changed
  
  bool settingsChanged = false;
  
  if (state.noiseMode != newMode) {
    settingsChanged = true;
    state.noiseMode = newMode;
  }
  
  if (state.noiseRate != newRate) {
    settingsChanged = true;
    state.noiseRate = newRate;
  }
  
  if (settingsChanged)
    vgm.setNoise(state.noiseMode, state.noiseRate);
  
  if (state.volume != newVolume) {
    state.volume = newVolume;
    vgm.setVolume(index, state.volume);
  }
}


}
