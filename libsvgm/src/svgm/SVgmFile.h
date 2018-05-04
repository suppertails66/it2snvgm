#ifndef SVGM_H
#define SVGM_H


#include "util/TStream.h"
#include "util/TBufStream.h"
#include "util/TByte.h"
#include <vector>

namespace SVgm {


class SVgmFile {
public:
  SVgmFile();
  
  int numSamples() const;
  int dataSize() const;
  
  /**
   * Sets the loop position.
   * It is the caller's responsibility to make sure the parameters are valid
   * for the contained data.
   * @param startPos Starting position of loop within data.
   * @param sampleLen Length of the loop in samples.
   */
  void setLoop(int startPos, int sampleLen);
  
  void addWaitVar(int length);
  void addWaitNtscFrame();
  void addWaitPalFrame();
  void addEndOfData();

  void write(BlackT::TStream& dst);
protected:
  
  void add_waitVar(int length);
  void add_waitNtscFrame();
  void add_waitPalFrame();
  void add_endOfData();
  
  BlackT::TBufStream header;
//  BlackT::TBufStream extraHeader;
  BlackT::TBufStream data;
  int numSamples_;
  
  //============================================
  // general constants
  //============================================
  // full header is 256 bytes, but only first 64 are required, and we don't
  // need any of the additional settings at the moment
  const static int headerSize = 64;
  // not needed or implemented
//  const static int extraHeaderSize = 16;
  // hardcoded target VGM version: 1.50
  const static int targetVgmVersion = 0x00000150;
  const static char* vgmIdent;
  const static int defaultNtscRate = 60;
  const static int defaultPalRate = 50;
  const static int ntscFrameSampleLength = 735;
  const static int palFrameSampleLength = 882;
  // TODO: add an auto-expanding derived class of TStream so output data
  // size is not limited by this constant
  const static int maxOutputDataSize = 0x400000;
  
  //============================================
  // header field offsets
  //============================================
  const static int headoff_ident = 0x00;
  const static int headoff_eofOffset = 0x04;
  const static int headoff_version = 0x08;
  const static int headoff_sn76489Clock = 0x0C;
  // ...
  const static int headoff_totalSamples = 0x18;
  const static int headoff_loopOffset = 0x1C;
  const static int headoff_loopNumSamples = 0x20;
  const static int headoff_rate = 0x24;
  const static int headoff_sn76489Feedback = 0x28;
  const static int headoff_sn76489ShiftRegisterWidth = 0x2A;
//  const static int headoff_sn76489ShiftRegisterWidth = 0x2B;
  // ...
  const static int headoff_vgmDataOffset = 0x34;
  
  //============================================
  // commands (non-system-specific)
  //============================================
  const static BlackT::TByte cmd_waitVar = 0x61;
  const static BlackT::TByte cmd_waitNtscFrame = 0x62;
  const static BlackT::TByte cmd_waitPalFrame = 0x63;
  const static BlackT::TByte cmd_endOfData = 0x66;
};


}


#endif
