#include "svgm/SSmsVgmFile.h"
#include "sit/SItFile.h"
#include "conv/SItToSmsVgm.h"
#include "util/TBufStream.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace SVgm;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "IT to SN76489 (Sega Master System) VGM converter" << endl;
    cout << "Usage: " << argv[0] << " <infile.it> <outfile.vgm>" << endl;
    
    return 0;
  }
  
  SItFile sit;
  TBufStream ifs(1);
  ifs.open(argv[1]);
  sit.read(ifs);
  
  SSmsVgmFile vgm(SSmsVgmFile::regionNtsc);
  
  SItToSmsVgm(sit, vgm)();
  
  TBufStream ofs(0x400000);
  vgm.write(ofs);
  ofs.save(argv[2]);
  
  return 0;
}
