#include "sit/SItPlayerChannel.h"
#include "sit/SItPlayer.h"
#include "util/TConstants.h"
#include "util/MiscMath.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace BlackT;

namespace SVgm {


const double SItPlayerChannel::pitchEnvelopeFactor
  = (double)1 / (double)2;
  
const double SItPlayerChannel::baseFineNoteSlidePerTick
  = (double)1 / (double)64;
const double SItPlayerChannel::baseNoteSlidePerTick
  = baseFineNoteSlidePerTick * 4;
const int SItPlayerChannel::volPanPortamentoEquivalences[10]
  = { 0, 1, 4, 8, 16, 32, 64, 96, 128, 255 };
  
const double SItPlayerChannel::vibratoSpeedFactor
  = (double)1 * (double)4 / (double)256 * (TConstants::pi * (double)2);
const double SItPlayerChannel::vibratoDepthFactor
  = (double)1 / (double)63;
  
SItPlayerChannel::SItPlayerChannel()
  : player(NULL) { }
  
SItPlayerChannel::SItPlayerChannel(SItPlayer& player__)
  : player(&player__),
    channelNum(0),
    baseNote(noteCut),
    channelVolume(maxChannelVolume),
    volume(maxVolume),
    panning(32),
    instrument(-1),
    instrumentExists(false),
    effectVolSlideMode(effectVolSlideOff),
    effectVolSlideLastMode(effectVolSlideOff),
    effectVolSlideSpeed(0),
    effectVolSlideDirection(0),
    effectVolIsContinue(false),
    volVolSlideMode(effectVolSlideOff),
    volVolSlideLastMode(effectVolSlideOff),
    volVolSlideSpeed(0),
    volVolSlideDirection(0),
    volVolIsContinue(false),
    portamentoMode(portamentoOff),
    portamentoLevel(0),
    portamentoTargetNote(0),
    portamentoSpeed(0),
    portamentoIsFine(false),
    vibratoMode(vibratoOff),
    targetVibratoMode(vibratoSine),
    vibratoLevel(0),
    vibratoSpeed(0),
    vibratoDepth(0),
    vibratoPos(0),
    tremorMode(tremorOff),
    tremorOnTime(0),
    tremorOffTime(0),
    tremorOutputOn(false),
    tremorCounter(0),
    arpMode(arpOff),
//    arpNote1(0),
    arpNote2(0),
    arpNote3(0),
    arpPlayingNote(0)
     { }
  
bool SItPlayerChannel::enabled() const {
  return (panning < 0x80);
}

bool SItPlayerChannel::isCut() const {
  return (baseNote == noteCut);
}
  
void SItPlayerChannel::runTick(const SItEntry& entry, int tickNum) {
  // if this tick is the one that triggers the entry, evaluate for
  // note change and other related events
  if (tickNum == entry.getTriggerTick()) {
    evaluateTriggerTick(entry, tickNum);
  }
  
  evaluateInstrumentUpdate(entry, tickNum);
  
  evaluateEffectVolSlide(entry, tickNum,
    effectVolSlideMode, effectVolSlideSpeed,
    effectVolSlideDirection, effectVolIsContinue);
  evaluateEffectVolSlide(entry, tickNum,
    volVolSlideMode, volVolSlideSpeed,
    volVolSlideDirection, volVolIsContinue);
  evaluatePortamento(entry, tickNum);
  evaluateVibrato(entry, tickNum);
  evaluateTremor(entry, tickNum);
  evaluateArp(entry, tickNum);
  
  // check miscellaneous things that are simple enough not to deserve special
  // handlers
  
  if (entry.hasCommand && entry.command == SItEntry::effectS) {
    // note cut
    if ((entry.commandValue & 0xF0) == 0xC0) {
      if (tickNum >= (entry.commandValue & 0x0F)) {
        cutNote();
      }
    }
  }
  
}

double SItPlayerChannel::currentOutputNote() const {
  double outputNote = baseNote;
  
  outputNote += portamentoLevel;
  
  if (vibratoMode != vibratoOff)
    outputNote += vibratoLevel;
    
  if (arpMode != arpOff) {
    switch (arpPlayingNote) {
    case 0:
      
      break;
    case 1:
      outputNote += arpNote2;
      break;
    case 2:
      outputNote += arpNote3;
      break;
    default:
      
      break;
    }
  }
  
  if ((instrumentExists)
      && instrumentPitchEnv.enabled()) {
    outputNote += (instrumentPitchEnv.currentValue() * pitchEnvelopeFactor);
  }
  
  return outputNote;
}

double SItPlayerChannel::currentOutputVolumeMultiplier() const {
  if (isCut()) return 0;
  if ((tremorMode == tremorOn) && !tremorOutputOn) return 0;

  double result = ((double)volume / (double)maxVolume);
  
  if ((instrumentExists)
      && instrumentVolumeEnv.enabled()) {
      
    // global volume
    result *= ((double)activeInstrument.globalVolume
                / (double)SItInstrument::maxGlobalVolume);
    
    // volume envelope
    result *= (instrumentVolumeEnv.currentValue()
                / (double)SItEnvelope::unsignedEnvelopeRange);
    
    // TODO: note fade
    if (!instrumentVolumeEnv.enabled() || instrumentVolumeEnv.done) {
      
    }
  }

  result *= ((double)channelVolume / (double)maxChannelVolume);
  
  return result;
}

void SItPlayerChannel::evaluateTriggerTick(
    const SItEntry& entry, int tickNum) {
  // turn off active effects; if continued, they'll be turned on again during
  // the evaluation process
  disableEffects();
  
  evaluateNoteSet(entry, tickNum);
  evaluateInstrumentSet(entry, tickNum);
  evaluateVolumeSet(entry, tickNum);
  evaluateEffectSet(entry, tickNum);
}

void SItPlayerChannel::evaluateNoteSet(
    const SItEntry& entry, int tickNum) {
  if (!entry.hasNote) return;
  
  // no note change for tone portamento
  if ((entry.command != SItEntry::effectG)
      && !(entry.hasVolPan
            && entry.volPanEffect == SItEntry::volEffectTonePortamento)
      ) {
    triggerNote(entry.note);
    if (!entry.hasVolPan
        || (entry.volPanEffect != SItEntry::volEffectVolume)) {
      // TODO: get default volume
      // ...
      volume = maxVolume;
    }
  }
}

void SItPlayerChannel::evaluateInstrumentSet(
    const SItEntry& entry, int tickNum) {
  if (!entry.hasInstrument) {
//    triggerInstrument(noInstrument);
    return;
  }
  
  if (entry.hasInstrument
      // if used in conjunction with a note, we set (or re-set) the instrument
      && ((entry.hasNote)
          // we do the same if the new instrument differs from the current(?)
          || (entry.instrument != instrument))
//      && ((entry.instrument != instrument))
      // ...or, if instrument is same, if current instrument is silenced...
      ) {
    triggerInstrument(entry.instrument);
//    instrument = entry.instrument;
  }
}

void SItPlayerChannel::evaluateVolumeSet(
    const SItEntry& entry, int tickNum) {
//  if ((tickNum == 0)
//      // never delay?
//      ) {
  if (!entry.hasVolPan) return;
  
  switch (entry.volPanEffect) {
  case SItEntry::volEffectVolume:
    volume = entry.volPanValue;
    break;
  case SItEntry::volEffectPanning:
    panning = entry.volPanValue;
    break;
  case SItEntry::volEffectFineVolumeUp:
  case SItEntry::volEffectFineVolumeDown:
  case SItEntry::volEffectVolumeSlideUp:
  case SItEntry::volEffectVolumeSlideDown:
    
    // continue command is direction- and mode-agnostic
    if (entry.volPanValue == 0x00) {
      triggerEffectVolSlide(effectVolSlideOff, 0, 0,
        volVolSlideMode, volVolSlideLastMode, volVolSlideSpeed,
        volVolSlideDirection, volVolIsContinue);
    }
    else {
      switch (entry.volPanEffect) {
        case SItEntry::volEffectFineVolumeUp:
          triggerEffectVolSlide(effectVolSlideFine,
                                entry.volPanValue, 1,
            volVolSlideMode, volVolSlideLastMode, volVolSlideSpeed,
            volVolSlideDirection, volVolIsContinue);
          break;
        case SItEntry::volEffectFineVolumeDown:
          triggerEffectVolSlide(effectVolSlideFine,
                                entry.volPanValue, -1,
            volVolSlideMode, volVolSlideLastMode, volVolSlideSpeed,
            volVolSlideDirection, volVolIsContinue);
          break;
        case SItEntry::volEffectVolumeSlideUp:
          triggerEffectVolSlide(effectVolSlideNormal,
                                entry.volPanValue, 1,
            volVolSlideMode, volVolSlideLastMode, volVolSlideSpeed,
            volVolSlideDirection, volVolIsContinue);
          break;
        case SItEntry::volEffectVolumeSlideDown:
          triggerEffectVolSlide(effectVolSlideNormal,
                                entry.volPanValue, -1,
            volVolSlideMode, volVolSlideLastMode, volVolSlideSpeed,
            volVolSlideDirection, volVolIsContinue);
          break;
        default:
        
          break;
      }
      }
    break;
  case SItEntry::volEffectPitchSlideUp:
    triggerPortamentoSlide(entry.volPanValue * 4, 1);
    break;
  case SItEntry::volEffectPitchSlideDown:
    triggerPortamentoSlide(entry.volPanValue * 4, -1);
    break;
  case SItEntry::volEffectTonePortamento:
    triggerPortamentoTo(volPanPortamentoEquivalences[entry.volPanValue],
                        entry.hasNote, entry.note);
    break;
  case SItEntry::volEffectVibratoDepth:
    triggerVibrato(0, entry.volPanValue);
    break;
  default:
    
    break;
  }
  
//  }
}

void SItPlayerChannel::evaluateEffectSet(const SItEntry& entry, int tickNum) {
  if (!entry.hasCommand) return;
  
  switch (entry.command) {
  case SItEntry::effectA:
    {
    int amount = entry.commandValue;
    if (amount != 0) player->speed = amount;
    }
    break;
  case SItEntry::effectB:
    player->patternJumpTarget = entry.commandValue;
    player->patternJumpTriggered = true;
    break;
  case SItEntry::effectC:
    player->patternBreakTarget = entry.commandValue;
    player->patternBreakTriggered = true;
    break;
  case SItEntry::effectD:
    // continue command is direction- and mode-agnostic
    if (entry.commandValue == 0x00) {
      triggerEffectVolSlide(effectVolSlideOff, 0, 0,
        effectVolSlideMode, effectVolSlideLastMode, effectVolSlideSpeed,
        effectVolSlideDirection, effectVolIsContinue);
      break;
    }
  
    // down
    if (((entry.commandValue & 0xF0) == 0x00)) {
      triggerEffectVolSlide(effectVolSlideNormal,
                            entry.commandValue & 0x0F, -1,
        effectVolSlideMode, effectVolSlideLastMode, effectVolSlideSpeed,
        effectVolSlideDirection, effectVolIsContinue);
    }
    // up
    else if (((entry.commandValue & 0x0F) == 0x00)) {
      triggerEffectVolSlide(effectVolSlideNormal,
                            (entry.commandValue & 0xF0) >> 4, 1,
        effectVolSlideMode, effectVolSlideLastMode, effectVolSlideSpeed,
        effectVolSlideDirection, effectVolIsContinue);
    }
    // fine down
    else if (((entry.commandValue & 0xF0) == 0xF0)) {
      triggerEffectVolSlide(effectVolSlideFine,
                            entry.commandValue & 0x0F, -1,
        effectVolSlideMode, effectVolSlideLastMode, effectVolSlideSpeed,
        effectVolSlideDirection, effectVolIsContinue);
    }
    // fine up
    else if (((entry.commandValue & 0x0F) == 0x0F)) {
      triggerEffectVolSlide(effectVolSlideFine,
                            (entry.commandValue & 0xF0) >> 4, 1,
        effectVolSlideMode, effectVolSlideLastMode, effectVolSlideSpeed,
        effectVolSlideDirection, effectVolIsContinue);
    }
    break;
  case SItEntry::effectE:
  case SItEntry::effectF:
    {
/*    portamentoMode = portamentoSlide;
    
    // ignore continue commands...
    if (((entry.commandValue & 0xF0) == 0xF0)
        && ((entry.commandValue & 0x0F) != 0)) {
      portamentoSpeed
        = ((entry.commandValue & 0x0F) * baseNoteSlidePerTick);
    }
    else if (((entry.commandValue & 0xF0) == 0xE0)
        && ((entry.commandValue & 0x0F) != 0)) {
      portamentoSpeed
        = ((entry.commandValue & 0x0F) * baseFineNoteSlidePerTick);
    }
    else if (((entry.commandValue & 0xFF) != 0)) {
      portamentoSpeed
        = ((entry.commandValue & 0xFF) * baseNoteSlidePerTick);
    }
    
    // ...but make sure sign is correct for command type (e.g. if switching
    // from E00 to F00 or vice versa)
    if (entry.command == SItEntry::effectE) {
      if (portamentoSpeed > 0) portamentoSpeed = -portamentoSpeed;
    }
    
    else {
      if (portamentoSpeed < 0) portamentoSpeed = -portamentoSpeed;
    } */
    
    int direction;
    if (entry.command == SItEntry::effectE) direction = -1;
    else direction = 1;
    
    if (((entry.commandValue & 0xF0) == 0xF0)) {
      triggerPortamentoSlideFine(entry.commandValue & 0x0F, direction);
    }
    else if (((entry.commandValue & 0xF0) == 0xE0)) {
      triggerPortamentoSlideExtraFine(entry.commandValue & 0x0F, direction);
    }
    else {
      triggerPortamentoSlide(entry.commandValue, direction);
    }
    }
    break;
  case SItEntry::effectG:
    {
    triggerPortamentoTo(entry.commandValue,
                        entry.hasNote, entry.note);
    }
    break;
  case SItEntry::effectH:
    triggerVibrato((entry.commandValue & 0xF0) >> 4,
                   (entry.commandValue & 0x0F));
    break;
  case SItEntry::effectI:
    triggerTremor((entry.commandValue & 0xF0) >> 4,
                   (entry.commandValue & 0x0F));
    break;
  case SItEntry::effectJ:
    triggerArp((entry.commandValue & 0xF0) >> 4,
                   (entry.commandValue & 0x0F));
    break;
  case SItEntry::effectM:
    changeChannelVolume(entry.commandValue);
    break;
  // can't handle this here, since this function is only called on the first
  // tick
//  case SItEntry::effectS:
//    // note cut
//    if ((entry.commandValue & 0xF0) == 0xC0) {
//      if (tickNum >= (entry.commandValue & 0x0F)) {
//        cutNote();
//      }
//    }
//    break;
  default:
    
    break;
  }
  
  if (player->effectTriggeredCallback != NULL) {
    player->effectTriggeredCallback(player->callbackObj,
        *this, entry, channelNum);
  }
}

void SItPlayerChannel::changeChannelVolume(int vol) {
  channelVolume = vol;
}
  
void SItPlayerChannel::changeVolume(int vol) {
  MiscMath::clamp(vol, minVolume, maxVolume);
  volume = vol;
}
  
void SItPlayerChannel
    ::evaluateInstrumentUpdate(const SItEntry& entry, int tickNum) {
  if (instrumentExists) {
    instrumentVolumeEnv.runTick();
    instrumentPanningEnv.runTick();
    instrumentPitchEnv.runTick();
    
//    if (instrumentVolumeEnv.enabled()) {
//      // if envelope reached end and volume is zero, cut channel
//      if (instrumentVolumeEnv.done
//          && (instrumentVolumeEnv.currentValue() == 0)) {
//        cutNote();
//      }
//    }
//    else {
//      // TODO: note fade
//    }
  }
}

void SItPlayerChannel::evaluateEffectVolSlide(
    const SItEntry& entry, int tickNum,
    EffectVolSlideMode& mode,
    int& speed,
    int& direction,
    bool& isContinue,
    bool targetVol) {
  
  if (mode == effectVolSlideOff) return;
  
  // fine slide applies only on first tick of row
/*  if (((effectVolSlideMode == effectVolSlideFine)
      && (tickNum == entry.getTriggerTick()))) {
    changeVolume(volume + effectVolSlideSpeed);
    return;
  } */
  
  // if abs(speed) == 15, slide on all ticks (normal) or only first (fine)
  // if isContinue, slide on all ticks (normal) or only first (fine)
  // otherwise, slide on all ticks but first (normal) or only first (fine)
  if (((speed * direction) == 15)
      || (isContinue)) {
    if ((mode == effectVolSlideNormal)
         || ((mode == effectVolSlideFine)
             && (tickNum == entry.getTriggerTick()))) {
      changeVolume(volume + speed);
    }
  }
  else {
    if (((mode == effectVolSlideNormal)
            && (tickNum != entry.getTriggerTick()))
        || ((mode == effectVolSlideFine)
            && (tickNum == entry.getTriggerTick()))) {
      changeVolume(volume + speed);
    }
  }
}

void SItPlayerChannel::evaluatePortamento(const SItEntry& entry, int tickNum) {
  if (portamentoMode == portamentoOff) return;
  
  if ((!portamentoIsFine && (tickNum == entry.getTriggerTick()))
      || (portamentoIsFine && (tickNum != entry.getTriggerTick()))) return;
  
  portamentoLevel += portamentoSpeed;
  
  // stop portamento-to at the target note
  if (portamentoMode == portamentoTo) {
    double currentBendNote = baseNote + portamentoLevel;
    if ((portamentoSpeed > 0)
          && (currentBendNote >= portamentoTargetNote)) {
      portamentoLevel = portamentoTargetNote - baseNote;
    }
    else if ((portamentoSpeed <= 0)
          && (currentBendNote <= portamentoTargetNote)) {
      portamentoLevel = portamentoTargetNote - baseNote;
    }
  }
}

void SItPlayerChannel::evaluateVibrato(const SItEntry& entry, int tickNum) {
  if (vibratoMode == vibratoOff) return;
  
//  vibratoPos += vibratoSpeed;
//  vibratoLevel = std::sin(vibratoPos) * vibratoDepth;

  // advance table position
  vibratoPos = (vibratoPos + vibratoSpeed) % vibratoTableSize;
  
  if (vibratoMode == vibratoSine) {
    vibratoLevel
      = std::sin(((double)vibratoPos / (double)vibratoTableSize)
                  * (TConstants::pi * (double)2))
      * vibratoDepth;
  }
  
}

void SItPlayerChannel::evaluateTremor(const SItEntry& entry, int tickNum) {
  if (tremorMode == tremorOff) return;

  --tremorCounter;
  if (tremorCounter <= 0) {
    if (tremorOutputOn) tremorCounter = tremorOffTime;
    else tremorCounter = tremorOnTime;
    tremorOutputOn = !tremorOutputOn;
  }
  
}

void SItPlayerChannel::evaluateArp(const SItEntry& entry, int tickNum) {
  if (arpMode == arpOff) return;

  ++arpPlayingNote;
  arpPlayingNote %= 3;
  
  // for every tick an arp is active, we need to "retrigger" the note, which
  // resets its envelopes and sample playback but does _not_ affect the note
  // volume
  // TODO
  
  
}

void SItPlayerChannel::triggerNote(int note) {
  if (note == noteRelease) {
    if (instrumentExists) {
      instrumentVolumeEnv.sustaining = false;
      instrumentPanningEnv.sustaining = false;
      instrumentPitchEnv.sustaining = false;
    }
    
    return;
  }
  else if ((note >= noteFadeLow) && (note <= noteFadeHigh)) {
    // TODO
    return;
  }
  
  // "real" note (or cut)
  
  baseNote = note;
  
  if (note == noteCut) {
    disableEffects();
  }
  
  // reset:
  // portamento
  portamentoLevel = 0;
  // vibrato
  vibratoLevel = 0;
  // arp?
  arpPlayingNote = 0;
}

void SItPlayerChannel::triggerInstrument(int instr) {
  instrument = instr;
  if ((instr > 0)
      && ((instr - 1) < player->sit.instruments.size())) {
    instrumentExists = true;
    activeInstrument = player->sit.instruments[instr - 1];
    instrumentVolumeEnv
      = SItPlayerEnvelope(activeInstrument.volumeEnvelope);
    instrumentPanningEnv
      = SItPlayerEnvelope(activeInstrument.panningEnvelope);
    instrumentPitchEnv
      = SItPlayerEnvelope(activeInstrument.pitchEnvelope);
  }
  else {
    instrumentExists = false;
  }
}

void SItPlayerChannel::cutNote() {
  triggerNote(noteCut);
}

void SItPlayerChannel::disableEffects() {
  // save old volume slide mode so we can restore it for continue commands
  effectVolSlideLastMode = effectVolSlideMode;
  effectVolSlideMode = effectVolSlideOff;
  volVolSlideLastMode = volVolSlideMode;
  volVolSlideMode = effectVolSlideOff;
  
  portamentoMode = portamentoOff;
  vibratoMode = vibratoOff;
  tremorMode = tremorOff;
  arpMode = arpOff;
}
  
void SItPlayerChannel::triggerEffectVolSlide(EffectVolSlideMode mode,
    int speed, int direction,
    EffectVolSlideMode& modeTarget,
    EffectVolSlideMode& lastModeTarget,
    int& speedTarget,
    int& directionTarget,
    bool& isContinueTarget,
    bool targetVol) {
  if (speed != 0) {
    isContinueTarget = false;
    modeTarget = mode;
    speedTarget = speed * direction;
    directionTarget = direction;
  }
  // continue command
  else {
    isContinueTarget = true;
    modeTarget = lastModeTarget;
//    if ((direction > 0) && (effectVolSlideSpeed < 0))
//      effectVolSlideSpeed = -effectVolSlideSpeed;
//    else if ((direction < 0) && (effectVolSlideSpeed > 0))
//      effectVolSlideSpeed = -effectVolSlideSpeed;
  }
}

void SItPlayerChannel
  ::triggerPortamentoSlideGeneric(int speed, int direction,
                           double multiplier) {
  portamentoMode = portamentoSlide;
  
  // ignore continue commands...
  if (speed != 0) {
    portamentoSpeed
      = speed * multiplier * direction;
  }
  // ...but make sure sign is correct for command type (e.g. if switching
  // from E00 to F00 or vice versa)
  else {
    if (direction < 0) {
      if (portamentoSpeed > 0) portamentoSpeed = -portamentoSpeed;
    }
    else {
      if (portamentoSpeed < 0) portamentoSpeed = -portamentoSpeed;
    }
  }
  
}

void SItPlayerChannel
  ::triggerPortamentoSlide(int speed, int direction) {
  triggerPortamentoSlideGeneric(speed, direction, baseNoteSlidePerTick);
  if (speed != 0)
    portamentoIsFine = false;
}

void SItPlayerChannel
  ::triggerPortamentoSlideFine(int speed, int direction) {
  triggerPortamentoSlideGeneric(speed, direction, baseNoteSlidePerTick);
  if (speed != 0)
    portamentoIsFine = true;
}

void SItPlayerChannel
  ::triggerPortamentoSlideExtraFine(int speed, int direction) {
  triggerPortamentoSlideGeneric(speed, direction, baseFineNoteSlidePerTick);
  if (speed != 0)
    portamentoIsFine = true;
}

void SItPlayerChannel
  ::triggerPortamentoTo(int speed, bool hasNote, int note) {
/*    portamentoMode = portamentoTo;
    
    // update target note if one was given
    if (entry.hasNote) {
      portamentoTargetNote = entry.note;
    }
    
    // update speed if not a continue command
    if ((entry.commandValue & 0xFF) != 0) {
      portamentoSpeed = (entry.commandValue & 0x0F) * baseNoteSlidePerTick;
      
      // direction of bend is positive if note is higher than current note
      // (_including_ any previously applied bends*), and negative otherwise
      // * except in the compatibility mode we don't support
      double currentBendNote = baseNote + portamentoLevel;
      if (portamentoTargetNote < currentBendNote)
        portamentoSpeed = -portamentoSpeed;
      } */
  
  portamentoMode = portamentoTo;
  
  // update target note if one was given
  if (hasNote) {
    portamentoTargetNote = note;
  }
  
  // update speed if not a continue command
  if (speed != 0) {
    portamentoSpeed = speed * baseNoteSlidePerTick;
  }
  
  // direction of bend is positive if note is higher than current note
  // (_including_ any previously applied bends*), and negative otherwise
  // * except in the compatibility mode we don't support
  double currentBendNote = baseNote + portamentoLevel;
  if ((portamentoTargetNote < currentBendNote)
      && (portamentoSpeed > 0))
    portamentoSpeed = -portamentoSpeed;
}

void SItPlayerChannel::triggerVibrato(int speed, int depth) {
  vibratoMode = targetVibratoMode;
  
  // speed
  if (speed != 0) {
//      vibratoSpeed = (double)((entry.commandValue & 0xF0) >> 4)
//        * vibratoSpeedFactor;
    vibratoSpeed = speed
      * vibratoStepSize;
  }
  
  // depth
  if (depth != 0) {
    vibratoDepth = (double)depth
      * (double)vibratoCoarseFactor
      * vibratoDepthFactor;
  }
}

void SItPlayerChannel::triggerTremor(int onTime, int offTime) {
  if ((onTime == 0) && (offTime == 0)) {
    // continue
    tremorMode = tremorOn;
  }
  else {
    tremorMode = tremorOn;
    tremorOnTime = onTime;
    tremorOffTime = offTime;
    // initialize to +1 so the counter predecrement aligns correctly
    tremorCounter = onTime + 1;
    tremorOutputOn = true;
  }
}

void SItPlayerChannel::triggerArp(int note2, int note3) {
  if ((note2 == 0) && (note3 == 0)) {
    // continue
    arpMode = arpOn;
  }
  else {
    arpMode = arpOn;
    arpNote2 = note2;
    arpNote3 = note3;
    // start at -1 so the preincrement will increase this to 0 for the first
    // tick of playback
    arpPlayingNote = -1;
//    arpPlayingNote = 0;
  }
}


}
