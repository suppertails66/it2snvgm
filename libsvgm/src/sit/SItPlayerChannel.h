#ifndef SITPLAYERCHANNEL_H
#define SITPLAYERCHANNEL_H


#include "sit/SItEntry.h"
#include "sit/SItInstrument.h"
#include "sit/SItPlayerEnvelope.h"

namespace SVgm {


class SItPlayer;

class SItPlayerChannel {
public:
  friend class SItPlayer;

  const static int minNote = 0;     // C-0
  const static int maxNote = 119;   // B-9
  const static int noteFadeLow = maxNote + 1;
  const static int noteFadeHigh = 253;
  const static int noteCut = 254;
  const static int noteRelease = 255;
  const static double pitchEnvelopeFactor;
  
  const static int minChannelVolume = 0;
  const static int maxChannelVolume = 64;
  const static int minVolume = 0;
  const static int maxVolume = 64;
  const static int noInstrument = 0x100;

  SItPlayerChannel();
  SItPlayerChannel(SItPlayer& player__);
  
  bool enabled() const;
  bool isCut() const;
  
  void runTick(const SItEntry& entry, int tickNum);
  
  double currentOutputNote() const;
  double currentOutputVolumeMultiplier() const;
  
protected:
  
  //=========================================================
  // general members
  //=========================================================
  
  SItPlayer* player;
  
  int channelNum;
  int baseNote;
//  double note;
  
  int channelVolume;
  
  int volume;
  int panning;
  
  //=========================================================
  // instrument
  //=========================================================
  
  int instrument;
  bool instrumentExists;
  SItInstrument activeInstrument;
  SItPlayerEnvelope instrumentVolumeEnv;
  SItPlayerEnvelope instrumentPanningEnv;
  SItPlayerEnvelope instrumentPitchEnv;
  
  //=========================================================
  // volume slide (effect column)
  //=========================================================
  enum EffectVolSlideMode {
    effectVolSlideOff,
    effectVolSlideNormal,
    effectVolSlideFine
  };
  EffectVolSlideMode effectVolSlideMode;
  EffectVolSlideMode effectVolSlideLastMode;
  int effectVolSlideSpeed;
  int effectVolSlideDirection;
  bool effectVolIsContinue;
  
  //=========================================================
  // volume slide (volume column)
  //=========================================================
  EffectVolSlideMode volVolSlideMode;
  EffectVolSlideMode volVolSlideLastMode;
  int volVolSlideSpeed;
  int volVolSlideDirection;
  bool volVolIsContinue;
  
  //=========================================================
  // portamento
  //=========================================================
  
  const static double baseFineNoteSlidePerTick;
  const static double baseNoteSlidePerTick;
  const static int volPanPortamentoEquivalences[10];
  
  enum PortamentoMode {
    portamentoOff,
    portamentoSlide,
    portamentoTo
  };
  PortamentoMode portamentoMode;
  double portamentoLevel;   // amount of portamento from base note (+/-),
                            // in note units
  double portamentoTargetNote;  // for Gxx, the note we're aiming for
  double portamentoSpeed;   // the change in bend per tick
  bool portamentoIsFine;
  
  //=========================================================
  // vibrato
  //=========================================================
  
  const static int vibratoCoarseFactor = 4;
  const static int vibratoStepSize = 4;
  const static int vibratoTableSize = 256;
  const static double vibratoSpeedFactor;
  const static double vibratoDepthFactor;
  
  enum VibratoMode {
    vibratoOff,
    vibratoSine
  };
  VibratoMode vibratoMode;
  VibratoMode targetVibratoMode;
  double vibratoLevel;
//  double vibratoSpeed;
  int vibratoSpeed;
  double vibratoDepth;
//  double vibratoPos;
  int vibratoPos;         // position in vibrato waveform;
                          // (vibratoPos / 256) * (2 * pi) = position in
                          // sine (or other) wave
  
  //=========================================================
  // tremor
  //=========================================================
  
  enum TremorMode {
    tremorOff,
    tremorOn
  };
  TremorMode tremorMode;
  int tremorOnTime;
  int tremorOffTime;
  bool tremorOutputOn;
  int tremorCounter;
  
  //=========================================================
  // arpeggio
  //=========================================================
  
  enum ArpMode {
    arpOff,
    arpOn
  };
  ArpMode arpMode;
//  int arpNote1;
  int arpNote2;
  int arpNote3;
  int arpPlayingNote;
  
  //=========================================================
  // instanteous setting evaluation
  //=========================================================

  void evaluateTriggerTick(const SItEntry& entry, int tickNum);
  
  void evaluateNoteSet(const SItEntry& entry, int tickNum);
  void evaluateInstrumentSet(const SItEntry& entry, int tickNum);
  void evaluateVolumeSet(const SItEntry& entry, int tickNum);
  void evaluateEffectSet(const SItEntry& entry, int tickNum);
  
  void changeChannelVolume(int vol);
  void changeVolume(int vol);
  
  //=========================================================
  // ongoing effect evaluation
  //=========================================================
  
  void evaluateInstrumentUpdate(const SItEntry& entry, int tickNum);
  
  void evaluateEffectVolSlide(const SItEntry& entry, int tickNum,
                              EffectVolSlideMode& mode,
                              int& speed,
                              int& direction,
                              bool& isContinue,
                              bool targetVol = false);
  void evaluatePortamento(const SItEntry& entry, int tickNum);
  void evaluateVibrato(const SItEntry& entry, int tickNum);
  void evaluateTremor(const SItEntry& entry, int tickNum);
  void evaluateArp(const SItEntry& entry, int tickNum);
  
  void triggerNote(int note);
  void triggerInstrument(int instr);
  void cutNote();
  void disableEffects();
  
  void triggerEffectVolSlide(EffectVolSlideMode mode,
    int speed, int direction,
    EffectVolSlideMode& modeTarget,
    EffectVolSlideMode& lastModeTarget,
    int& speedTarget,
    int& directionTarget,
    bool& isContinueTarget,
    bool targetVol = false);
  void triggerPortamentoSlideGeneric(int speed, int direction,
                              double multiplier);
  void triggerPortamentoSlide(int speed, int direction);
  void triggerPortamentoSlideFine(int speed, int direction);
  void triggerPortamentoSlideExtraFine(int speed, int direction);
  void triggerPortamentoTo(int speed, bool hasNote = false, int note = 0);
  void triggerVibrato(int speed, int depth);
  void triggerTremor(int onTime, int offTime);
  void triggerArp(int note2, int note3);
  
};


}


#endif
