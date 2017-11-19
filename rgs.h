// HFFS (Help find files script)
// Author: Michael O'Farrell

#pragma once

#include "hfs/hfs_format.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Options {
  char* infile;
  char* outdir;
  bool permissive;
  uint64_t sectorSize;
  uint64_t blockSize;
  uint64_t scanSize;
};

struct FileInfo {
  std::string name;
  uint32_t parentID;
  uint32_t fileID;
  uint64_t logicalSize;
  uint32_t totalBlocks;
  uint32_t foundBlocks;
  std::vector<HFSPlusExtentDescriptor> extents;
};

struct FolderInfo {
  std::string name;
  uint32_t parentID;
};

union ExtentKey {
  struct {
    uint32_t fileID;
    uint32_t startBlock;
  };
  uint64_t key;
};

struct RGS {
  Options options;
  std::vector<FileInfo> files;
  std::unordered_map<uint32_t, FolderInfo> folders;
  std::unordered_map<
    uint64_t,
    std::array<HFSPlusExtentDescriptor, kHFSPlusExtentDensity>
  > extents;
};

