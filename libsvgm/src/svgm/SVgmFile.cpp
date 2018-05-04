#include "svgm/SVgmFile.h"
#include <iostream>
#include <cstring>

using namespace BlackT;

namespace SVgm {


const char* SVgmFile::vgmIdent("Vgm ");
  
SVgmFile::SVgmFile()
  : header(headerSize),
    data(maxOutputDataSize),
    numSamples_(0) {
  // zero header -- needed fields will be filled in later
  header.setEndPos(headerSize);
  std::memset((char*)header.data().data(), 0, header.size());
  
  // file ID
  header.seek(headoff_ident);
  header.write(vgmIdent, 4);
  // version
  header.seek(headoff_version);
  header.writeu32le(targetVgmVersion);
  
}

void SVgmFile::write(TStream& dst) {
  const static int outputSize = header.size() + data.size();
  
  // update header fields
  
  // eof
  header.seek(headoff_eofOffset);
  header.writeu32le(outputSize - headoff_eofOffset);
  // total number of samples
  header.seek(headoff_totalSamples);
  header.writeu32le(numSamples_);
  // VGM data offset
  header.seek(headoff_vgmDataOffset);
  header.writeu32le(header.size() - headoff_vgmDataOffset);

  // write header
  header.seek(0);
  dst.write((char*)header.data().data(), header.size());
  
  // write data
  data.seek(0);
  dst.write((char*)data.data().data(), data.size());
  
}
  
int SVgmFile::numSamples() const {
  return numSamples_;
}

int SVgmFile::dataSize() const {
  return data.size();
}

void SVgmFile::add_waitVar(int length) {
  data.writeu8(cmd_waitVar);
  data.writeu16le(length);
  numSamples_ += length;
}

void SVgmFile::add_waitNtscFrame() {
  data.writeu8(cmd_waitNtscFrame);
  numSamples_ += ntscFrameSampleLength;
}

void SVgmFile::add_waitPalFrame() {
  data.writeu8(cmd_waitPalFrame);
  numSamples_ += palFrameSampleLength;
}

void SVgmFile::add_endOfData() {
  data.writeu8(cmd_endOfData);
}
  
void SVgmFile::setLoop(int startPos, int sampleLen) {
  // write start pos
  header.seek(headoff_loopOffset);
  header.writeu32le(header.size() + startPos - headoff_loopOffset);
  
  // write length
  header.seek(headoff_loopNumSamples);
  header.writeu32le(sampleLen);
}

void SVgmFile::addWaitVar(int length) {
  add_waitVar(length);
}

void SVgmFile::addWaitNtscFrame() {
  add_waitNtscFrame();
}

void SVgmFile::addWaitPalFrame() {
  add_waitPalFrame();
}

void SVgmFile::addEndOfData() {
  add_endOfData();
}


}
