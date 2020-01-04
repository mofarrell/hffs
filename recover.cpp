// HFFS (Help find files script)
// Author: Michael O'Farrell

#include "recover.h"

#include "convert.h"
#include "hfs/hfs_format.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>

namespace {

void warning(const char* msg) {
  std::cerr << "Warning:" << msg << std::endl;
}

bool accessIsSafe(size_t length, size_t access) {
  if (access >= length) return false;
  return true;
}

template<typename Labda>
void logInfo(RGS& env, Labda l) {
  static auto lastInfoTime = std::chrono::system_clock::now();

  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = now - lastInfoTime;

  if (diff.count() > 5.0) {
    l();
    lastInfoTime = now;
  }
}

///////////////////////////////////////////////////////////////////////////////
// The index function are called on verified records.  We now add them to the
// index.  The input is still in HFSPlus byte order.

void index(RGS& env, HFSPlusCatalogKey* ck) {
  ConvertBigEndian(ck);
  
  char* record = (char*)ck + ck->keyLength + 2;
  uint16_t recordType = *(uint16_t*)(record);
  ConvertBigEndian(&recordType);

  char buf[kHFSPlusMaxFileNameChars + 1];
  DecodeU16(ck->nodeName.unicode, buf, ck->nodeName.length);

  switch (recordType) {
    case kHFSPlusFolderRecord: {
      FolderInfo fi;
      fi.name = std::string(buf);
      fi.parentID = ck->parentID;

      HFSPlusCatalogFolder* folder = (HFSPlusCatalogFolder*)record;
      ConvertBigEndian(folder);
      
      uint32_t folderID = folder->folderID;
      env.folders.emplace(std::make_pair(folderID, fi));
      break;
    }
    case kHFSPlusFileRecord: {
      FileInfo fi;
      fi.name = std::string(buf);
      fi.parentID = ck->parentID;

      HFSPlusCatalogFile* file = (HFSPlusCatalogFile*)record;
      ConvertBigEndian(file);

      fi.fileID = file->fileID;
      fi.logicalSize = file->dataFork.logicalSize;
      fi.totalBlocks = file->dataFork.totalBlocks;
      if (fi.logicalSize != 0 && fi.totalBlocks != 0 &&
          (fi.totalBlocks * env.options.blockSize < fi.logicalSize ||
           (fi.totalBlocks - 1) * env.options.blockSize >= fi.logicalSize)) {
        if (env.options.permissive) {
          warning("Block size appears wrong.");
        } else {
          std::cout << "File " << fi.name << " Size " << fi.logicalSize
                    << " Blocks " << fi.totalBlocks << std::endl;
          return;
          // throw std::runtime_error("Block size appears wrong.");
        }
      }
      if (fi.logicalSize == 0) {
        return;
      }
      fi.foundBlocks = 0;
      
      for (uint32_t i = 0;
             i < kHFSPlusExtentDensity && fi.foundBlocks < fi.totalBlocks;
           i++) {
        fi.extents.emplace_back(file->dataFork.extents[i]);
        fi.foundBlocks += file->dataFork.extents[i].blockCount;
      }
      env.files.emplace_back(fi);
      break;
    }
    default:
      throw std::logic_error("Shouldn't have non folder/file records here.");
  }
  
}

void index(RGS& env, HFSPlusExtentKey* ek) {
  ConvertBigEndian(ek);

  ExtentKey extentKey;
  extentKey.fileID = ek->fileID;
  extentKey.startBlock = ek->startBlock;

  HFSPlusExtentRecord* er = (HFSPlusExtentRecord*)(ek + 1);
  ConvertBigEndian(er);
  
  std::array<HFSPlusExtentDescriptor, kHFSPlusExtentDensity> eds;
  memcpy(&eds, er, sizeof(HFSPlusExtentRecord));
  env.extents.emplace(std::make_pair(extentKey.key, eds));
}

///////////////////////////////////////////////////////////////////////////////
// These process our two different types of nodes we care about.  Catalog nodes
// and extent nodes.
template<typename Lambda>
bool processNode(RGS& env, size_t nodeSize, char* buffer, Lambda lambda) {
  // Get the first records offset if it is a catalogNode.
  size_t reverseCursor = nodeSize - sizeof(uint16_t);
  uint16_t nodeEndOffset = *(uint16_t*)&buffer[reverseCursor];
  ConvertBigEndian(&nodeEndOffset);
  reverseCursor -= sizeof(uint16_t);
  
  if (env.options.permissive || nodeEndOffset == sizeof(BTNodeDescriptor)) {
    size_t cursor = sizeof(BTNodeDescriptor);
    uint16_t numRead = 0;
    while (cursor < nodeSize) {
      // Get the key length for the BTree key.
      BTreeKey* btkey = (BTreeKey*)&buffer[cursor];
      uint16_t length = btkey->length16;
      ConvertBigEndian(&length);

      // Get the records end offset.
      nodeEndOffset = *(uint16_t*)&buffer[reverseCursor];
      ConvertBigEndian(&nodeEndOffset);
      reverseCursor -= sizeof(uint16_t);

      if (!accessIsSafe(kMaxKeyLength, length)) break;
      char* record = &buffer[cursor + length + sizeof(uint16_t)];

      size_t cursorUpdate = lambda(btkey, record);
      if (cursorUpdate == -1) return false;
      if (cursorUpdate == 0) break;
      numRead++;

      if (nodeEndOffset != cursor + cursorUpdate) {
        if (!env.options.permissive) {
          break;
        } else {
          warning("Read record with incorrect offset label.");
        }
      }

      cursor += cursorUpdate;
    }

    if (numRead == 0) return false;
    if (numRead != ((BTNodeDescriptor*)buffer)->numRecords) {
      std::cerr << "Read some from block." << std::endl;
    }

    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// This is where we scan the image, locating and indexing the files, folders
// and extents as we scan.
void scan(RGS& env, std::ifstream& file) {
  char backbuffer[env.options.bufferSize * 2];
  char* buffer = backbuffer;
  if (!file.read(backbuffer, env.options.bufferSize * 2)) {
    std::runtime_error("File empty.");
  }
  size_t processedBTNodes = 0;
  size_t blockNumber = 0;

  // This will be used to advance our reading by the appropriate amount and no
  // more.
  uint64_t minNodeSize = std::min(env.options.catalogNodeSize,
                                  env.options.extentNodeSize);

  size_t printedFiles = 0;
  while (true) {
    uint64_t processedSize = 0;
    logInfo(env, [&]{
        // This will be used to advance our reading by the appropriate amount and no
        // more.
      std::cout << "Processed: " << blockNumber << " blocks "
        << blockNumber * env.options.blockSize << " bytes "
        << processedBTNodes << " BTNodes " 
        << env.files.size() << " files "
        << env.folders.size() << " folders "
        << env.extents.size() << " extents"
        << std::endl;

      if (env.files.size() > printedFiles) {
        std::cout << "Found additional " << env.files.size() - printedFiles
                  << " files" << std::endl;
        printedFiles = env.files.size();
      }
    });
    if (buffer - backbuffer >= env.options.bufferSize) {
      memcpy(&backbuffer, &backbuffer[env.options.bufferSize],
          env.options.bufferSize);
      if (!file.read(&backbuffer[env.options.bufferSize],
            env.options.bufferSize)) {
        break; // The file is empty.
      }
      buffer -= env.options.bufferSize;
      blockNumber += env.options.bufferSize / env.options.blockSize;
      if (env.options.stopBlock > 0 && blockNumber > env.options.stopBlock) {
        break;
      }
    }

    BTNodeDescriptor* btnode = (BTNodeDescriptor*)buffer;
    ConvertBigEndian(btnode);

    // We only care about leaf nodes that contain:
    //   - File records
    //   - Folder records
    //   - Extent records (exclusively)
    if (env.options.permissive || btnode->kind == kBTLeafNode) {
      processedBTNodes++;
      
      std::vector<HFSPlusCatalogKey*> foundCatalogEntries;
      std::vector<HFSPlusExtentKey*> foundExtentEntries;

      auto processCatalogNode = [&](BTreeKey* btkey, char* record) -> size_t {
        uint16_t length = btkey->length16;
        ConvertBigEndian(&length);

        // Find the name length if this is a catalogue key.
        uint16_t strLen = ((HFSPlusCatalogKey*)btkey)->nodeName.length;
        ConvertBigEndian(&strLen);

        // Find the record type if this is a catalog key.
        uint16_t recordType = *(uint16_t*)record;
        ConvertBigEndian(&recordType);

        if (  // Check the two lengths stored in catalog keys line up.
            length == strLen * sizeof(uint16_t)
            + kHFSPlusCatalogKeyMinimumLength
            && (  // Check the record type looks correct.
              recordType == kHFSPlusFolderRecord
              || recordType == kHFSPlusFileRecord
              || recordType == kHFSPlusFolderThreadRecord
              || recordType == kHFSPlusFileThreadRecord
              )
           ) {
          // It is highly likely that we have found a catalog record.
          // Handle and record the information it held.
          
          size_t cursorUpdate = length + sizeof(uint16_t);
          
          switch(recordType) {
            case kHFSPlusFolderRecord:
              cursorUpdate += sizeof(HFSPlusCatalogFolder);
              break;
            case kHFSPlusFileRecord:
              cursorUpdate += sizeof(HFSPlusCatalogFile);
              break;
            case kHFSPlusFolderThreadRecord:  // Falltrough
            case kHFSPlusFileThreadRecord: {
              cursorUpdate += sizeof(HFSPlusCatalogThread);
              cursorUpdate -= sizeof(HFSUniStr255);
              uint16_t threadNameLength = *(uint16_t*)(((char*)btkey) +
                                                       cursorUpdate);
              ConvertBigEndian(&threadNameLength);
              cursorUpdate += sizeof(uint16_t) * (threadNameLength + 1);
              break;
            }
            default:
              return 0;
          }

          // We have likely found a catalog record.  Record the location for
          // indexing.
          if (recordType == kHFSPlusFolderRecord ||
              recordType == kHFSPlusFileRecord) {
            foundCatalogEntries.emplace_back(
              (HFSPlusCatalogKey*)(btkey)
            );
          }
          return cursorUpdate;
        }
        return 0;
      };

      auto processExtentNode = [&](BTreeKey* btkey, char* record) -> size_t {
        uint16_t length = btkey->length16;
        ConvertBigEndian(&length);

        if (length == kHFSPlusExtentKeyMaximumLength // Only length.
            && ((HFSPlusExtentKey*)btkey)->forkType == 0  // data fork
            ) {
          // We have likely found an extent record.  Record the location to
          // index it.
          foundExtentEntries.emplace_back(
            (HFSPlusExtentKey*)(btkey)
          );

          return sizeof(HFSPlusExtentKey) +
            sizeof(HFSPlusExtentRecord);
        }
        return 0;
      };
      if (processNode(env, env.options.catalogNodeSize, buffer,
                      processCatalogNode)) {
        if (!foundExtentEntries.empty()) {
          throw std::runtime_error("Extent entries non empty.");
        }
        for (auto entry : foundCatalogEntries) {
          index(env, entry);
        }
        processedSize = env.options.catalogNodeSize;
      } else if (processNode(env, env.options.extentNodeSize, buffer,
                             processExtentNode)) {
        if (!foundCatalogEntries.empty()) {
          throw std::runtime_error("Catalog entries non empty.");
        }
        for (auto entry : foundExtentEntries) {
          index(env, entry);
        }
        processedSize = env.options.extentNodeSize;
      }
    }
    buffer += std::max(minNodeSize, processedSize);
  }
}

// Defragment the file looking up additional extents in the extent table.
void defragment(RGS& env, FileInfo& fi) {
  while (fi.foundBlocks < fi.totalBlocks) {
    ExtentKey extentKey;
    extentKey.fileID = fi.fileID;
    extentKey.startBlock = fi.foundBlocks;
    
    auto erit = env.extents.find(extentKey.key);
    if (erit == env.extents.end()) {
      warning("Couldn't find needed extent.");
      break;
    }

    for (auto ed : erit->second) {
      fi.extents.emplace_back(ed);
      fi.foundBlocks += ed.blockCount;
      if (fi.foundBlocks >= fi.totalBlocks) break;
    }
  }
}

std::string makeFolder(RGS& env, uint32_t parentID) {
  if (parentID < kHFSFirstUserCatalogNodeID) {
    return std::string(env.options.outdir);
  }
  auto fdit = env.folders.find(parentID);
  std::string path;
  if (fdit == env.folders.end()) {
    warning("Couldn't find folder in chain.");
    path = std::string(env.options.outdir) + "/lost";
  } else {
    path = makeFolder(env, fdit->second.parentID) + "/" + fdit->second.name;
  }
  if (mkdir(path.c_str(), 0777) < 0) {
    if (errno != EEXIST) {
      std::cerr << "Failed to create " << path << std::endl;
      warning("Couldn't create folder.");
    }
  }
  return path;
}
void save(RGS& env, std::ifstream& infile, const FileInfo& fi) {
  if (mkdir(env.options.outdir, 0777) < 0) {
    if (errno != EEXIST) {
      std::cerr << "Failed to create " << env.options.outdir << std::endl;
      warning("Couldn't create folder.");
    }
  }
  auto path = makeFolder(env, fi.parentID) + "/" + fi.name;

  std::ofstream outfile(path, std::ios::out|std::ios::binary);
  if (outfile.is_open()) {
    uint64_t sizeLeft = fi.logicalSize;
    for (const auto& extent : fi.extents) {
      uint64_t pos = extent.startBlock * env.options.blockSize;
      uint32_t blocks =  extent.blockCount;
      infile.seekg(pos);
      if (!infile) {
        infile.close();
        infile = std::ifstream(env.options.infile,
                               std::ios::in|std::ios::binary);
        infile.seekg(pos);
      }
      char buf[env.options.blockSize];
      while (blocks > 0) {
        uint64_t bytes = std::min(env.options.blockSize, sizeLeft);
        infile.read(buf, bytes);
        if (!infile) throw std::runtime_error("Failed to read.");
        outfile.write(buf, bytes);
        if (!outfile) throw std::runtime_error("Failed to write.");
        sizeLeft -= std::min(env.options.blockSize, sizeLeft);
        blocks--;
      }
    }

    outfile.close();
  } else {
    std::cerr << "Failed to write file " << path << std::endl;
    warning("Couldn't open output file.");
  }
}

}  // namespace
///////////////////////////////////////////////////////////////////////////////

void recover(RGS& env) {
  std::cout << std::endl << "Beginning recovery." << std::endl;

  std::ifstream file(env.options.infile, std::ios::in|std::ios::binary);
  if (file.is_open()) {
    scan(env, file);

    std::cout << std::endl << "Scanning done." << std::endl
              << "Found:" << std::endl
              << "  " << env.files.size() << " files" << std::endl
              << "  " << env.folders.size() << " folders" << std::endl
              << "  " << env.extents.size() << " fragment extents" << std::endl;

    size_t fileNumber = 0;
    for (auto& f : env.files) {
      logInfo(env, [&]{
        std::cout << "Defragmented: " << fileNumber << " files of "
          << env.files.size() << " files"
          << std::endl;
      });
      defragment(env, f);
      fileNumber++;
    }

    std::cout << "Defragmenting done." << std::endl;

    fileNumber = 0;
    for (auto const& f : env.files) {
      logInfo(env, [&]{
        std::cout << "Saving: " << fileNumber << " files of "
          << env.files.size() << " files"
          << std::endl;
      });
      save(env, file, f);
      fileNumber++;
    }

    std::cout << "Saving done." << std::endl;

    file.close();
  } else {
    throw std::runtime_error("Couldn't open image.");
  }
}

void verify(RGS& env) {
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

    std::cout << "Main header reporting:" << std::endl
      << "  fileCount: " << volHeader.fileCount << std::endl
      << "  folderCount: " << volHeader.folderCount << std::endl
      << "  blockSize: " << volHeader.blockSize << std::endl;
    std::cout << "Alternate header reporting:" << std::endl
      // These can mismatch with main header.  Hide them to avoid
      // confusion.
      // << "  fileCount: " << altHeader.fileCount << std::endl
      // << "  folderCount: " << altHeader.folderCount << std::endl
      << "  blockSize: " << altHeader.blockSize << std::endl;
  } else {
    throw std::runtime_error("Couldn't open image.");
  }
}

