#ifndef SSMSVGM_H
#define SSMSVGM_H


#include "svgm/SVgmFile.h"
#include "util/TBufStream.h"
#include <vector>
#include <cmath>

namespace SVgm {


class SSmsVgmFile : public SVgmFile {
public:

  const static int minVolume = 0x00;
  const static int maxVolume = 0x0F;
  const static int minTone = 0x000;
  const static int maxTone = 0x3FF;
  const static int silenceTone = 0x000;
  
  enum NoiseRate {
    noiseRateHigh = 0x00,
    noiseRateMid = 0x01,
    noiseRateLow = 0x02,
    noiseRateTone2 = 0x03
  };
  
  enum NoiseMode {
    noiseModePeriodic = 0x00,
    noiseModeWhite = 0x01
  };

  enum SystemRegion {
    regionNtsc,
    regionPal
  };

  SSmsVgmFile(SystemRegion systemRegion__);
  
//  void write(BlackT::TStream& dst);

  /**
   * Returns a Hz frequency corresponding to the given raw PSG tone.
   * System region is accounted for.
   */
  double toneToFrequency(int tone) const;

  /**
   * Returns a raw PSG tone value approximating the given Hz frequency.
   * System region is accounted for.
   */
  int frequencyToTone(double freq) const;
  
  void setVolume(int chanNum, int volume);
  void setTone(int chanNum, int tone);
  void setTone0(int tone);
  void setTone1(int tone);
  void setTone2(int tone);
  void setNoise(NoiseMode mode, NoiseRate rate);
  void addWaitFrame();
  
protected:

  SystemRegion systemRegion_;

  void add_ggStereo(BlackT::TByte param);
  void add_psg(BlackT::TByte param);
  
  int clockRate() const;
  
  //============================================
  // SN76489 constants
  //============================================
  const static int sn78489_ntscClockRate = 3579545;
  const static int sn78489_palClockRate = 3546893;
  const static int sn76489_noiseFeedbackPattern = 0x0009;
  const static int sn76489_shiftRegisterWidth = 0x10;
  const static double sn76489_clockFactor;
  const static double sn76489_clockDivider;
  
  //============================================
  // commands
  //============================================
  const static BlackT::TByte cmd_ggStereo = 0x4F;
  const static BlackT::TByte cmd_psg = 0x50;
  
};


}


#endif
