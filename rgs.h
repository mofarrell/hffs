// HFFS (Help find files script)
// Author: Michael O'Farrell

#pragma once

#include<cstdint>

struct Options {
  char* infile;
  bool permissive;
  int64_t sectorSize;
  int64_t blockSize;
};

struct RGS {
  Options options;
};

