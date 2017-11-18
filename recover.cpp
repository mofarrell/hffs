// HFFS (Help find files script)
// Author: Michael O'Farrell

#include "recover.h"

#include "convert.h"
#include "hfs/hfs_format.h"

#include <iostream>
#include <fstream>

namespace {

void warning(const char* msg) {
  std::cerr << "Warning:" << msg << std::endl;
}

}  // namespace
///////////////////////////////////////////////////////////////////////////////

void verify(RGS& env) {
  std::streampos size;
  char * memblock;

  std::ifstream file(env.options.infile, std::ios::in|std::ios::binary);
  HFSPlusVolumeHeader volHeader;
  HFSPlusVolumeHeader altHeader;
  if (file.is_open()) {
    file.seekg(2 * env.options.sectorSize);
    file.read((char*)&volHeader, sizeof(HFSPlusVolumeHeader));
    file.seekg(-2 * env.options.sectorSize, std::ios::end);
    file.read((char*)&altHeader, sizeof(HFSPlusVolumeHeader));
    file.close();

    ConvertBigEndian(&volHeader);
    ConvertBigEndian(&altHeader);
    if (volHeader.signature != kHFSPlusSigWord) {
      warning("Main volume header reporting incorrect signature.");
    }
    if (altHeader.signature != kHFSPlusSigWord) {
      warning("Alternate volume header reporting incorrect signature.");
    }
    if (!env.options.permissive && volHeader.signature != kHFSPlusSigWord &&
        altHeader.signature != kHFSPlusSigWord) {
      std::runtime_error("Incorrect signature for HFSPlus in both headers.");
    }
    
    if (volHeader.signature == kHFSPlusSigWord) {
      std::cout << "Main header reporting:" << std::endl
                << "  fileCount: " << volHeader.fileCount << std::endl
                << "  folderCount: " << volHeader.folderCount << std::endl
                << "  blockSize: " << volHeader.blockSize << std::endl;
    }
    if (altHeader.signature == kHFSPlusSigWord) {
      std::cout << "Alternate header reporting:" << std::endl
                // These can mismatch with main header.  Hide them to avoid
                // confusion.
                // << "  fileCount: " << altHeader.fileCount << std::endl
                // << "  folderCount: " << altHeader.folderCount << std::endl
                << "  blockSize: " << altHeader.blockSize << std::endl;
    }
  } else {
    throw std::runtime_error("Couldn't open image.");
  }
}

