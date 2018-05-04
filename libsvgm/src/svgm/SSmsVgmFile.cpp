#include "svgm/SSmsVgmFile.h"
#include "util/MiscMath.h"
#include <iostream>
#include <cstring>
#include <cmath>

using namespace BlackT;

namespace SVgm {


const double SSmsVgmFile::sn76489_clockFactor = 2;
const double SSmsVgmFile::sn76489_clockDivider = 16;

SSmsVgmFile::SSmsVgmFile(SystemRegion systemRegion__)
  : SVgmFile(),
    systemRegion_(systemRegion__) {
  
  //==========================================
  // fill in system constants
  //==========================================
  
  // clock rate (region-dependent)
  header.seek(headoff_sn76489Clock);
  switch (systemRegion_) {
  case regionNtsc:
    header.writeu32le(sn78489_ntscClockRate);
    break;
  case regionPal:
    header.writeu32le(sn78489_palClockRate);
    break;
  default:
    break;
  }
  
  // noise feedback value
  header.seek(headoff_sn76489Feedback);
  header.writeu16le(sn76489_noiseFeedbackPattern);
  
  // noise shift register width
  header.seek(headoff_sn76489ShiftRegisterWidth);
  header.writeu16le(sn76489_shiftRegisterWidth);
  
  // playback scaling rate
  header.seek(headoff_rate);
  switch (systemRegion_) {
  case regionNtsc:
    header.writeu32le(defaultNtscRate);
    break;
  case regionPal:
    header.writeu32le(defaultPalRate);
    break;
  default:
    break;
  }
  
}
  
//void SSmsVgmFile::write(BlackT::TStream& dst) {
//  SVgmFile::write(dst);
//}

double SSmsVgmFile::toneToFrequency(int tone) const {
  MiscMath::clamp(tone, minTone, maxTone);
  if (tone == 0) return 0;
  
  return (clockRate()
            / ((double)tone * sn76489_clockFactor * sn76489_clockDivider));
}

int SSmsVgmFile::frequencyToTone(double freq) const {
  if ((freq == 0)) return 0;
  // round to the nearest frequency rather than just truncating to int
  int result = round(
      (clockRate() / (freq * sn76489_clockFactor * sn76489_clockDivider)));
  // clamp result into the valid tone range
  MiscMath::clamp(result, minTone, maxTone);
  return result;
}
  
int SSmsVgmFile::clockRate() const {
  int inputClock;
  switch (systemRegion_) {
  case regionNtsc:
    inputClock = sn78489_ntscClockRate;
    break;
  case regionPal:
  default:
    inputClock = sn78489_palClockRate;
    break;
  }
  return inputClock;
}
  
void SSmsVgmFile::setVolume(int chanNum, int volume) {
  TByte command = 0x90;
  command |= ((chanNum & 0x03) << 5);
  command |= (((maxVolume - volume) & 0x0F));
  add_psg(command);
//  TByte command1, command2 = 0;
}

void SSmsVgmFile::setTone(int chanNum, int tone) {
  TByte command1 = 0x80;
  TByte command2 = 0x00;
  
  command1 |= ((chanNum & 0x03) << 5);
  command1 |= ((tone & 0x0F));
  add_psg(command1);
  
  command2 |= ((tone & 0x3F0) >> 4);
  add_psg(command2);
}

void SSmsVgmFile::setTone0(int tone) {
  setTone(0, tone);
}

void SSmsVgmFile::setTone1(int tone) {
  setTone(1, tone);
}

void SSmsVgmFile::setTone2(int tone) {
  setTone(2, tone);
}


void SSmsVgmFile::setNoise(NoiseMode mode, NoiseRate rate) {
  TByte command = 0xE0;
  command |= ((int)mode << 2);
  command |= ((int)rate);
  add_psg(command);
}

void SSmsVgmFile::addWaitFrame() {
  if (systemRegion_ == regionNtsc) {
    addWaitNtscFrame();
  }
  else {
    addWaitPalFrame();
  }
}

void SSmsVgmFile::add_ggStereo(BlackT::TByte param) {
  data.writeu8(cmd_ggStereo);
  data.writeu8(param);
}

void SSmsVgmFile::add_psg(BlackT::TByte param) {
  data.writeu8(cmd_psg);
  data.writeu8(param);
}


}
