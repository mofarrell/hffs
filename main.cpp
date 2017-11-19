// Author: Michael O'Farrell
// Utility for recovering files on an HFS+ filesystem


#include "hfs/hfs_format.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include <libkern/OSByteOrder.h>

#define SECTOR_SIZE 512

static void ConvertBigEndian(HFSPlusExtentRecord* er);

///////////////////////////////////////////////////////////////////////////////
// Credit for the section goes to folks over at google (lot of typing not
// enough time):
// http://goo.gl/VjNeAD

inline void ConvertBigEndian(uint16_t* x) {
  *x = OSSwapBigToHostInt16(*x);
}
inline void ConvertBigEndian(int16_t* x) {
  *x = OSSwapBigToHostInt16(*x);
}
inline void ConvertBigEndian(uint32_t* x) {
  *x = OSSwapBigToHostInt32(*x);
}
inline void ConvertBigEndian(uint64_t* x) {
  *x = OSSwapBigToHostInt64(*x);
}

static void ConvertBigEndian(HFSPlusForkData* fork) {
  ConvertBigEndian(&fork->logicalSize);
  ConvertBigEndian(&fork->clumpSize);
  ConvertBigEndian(&fork->totalBlocks);
  ConvertBigEndian(&fork->extents);
}
static void ConvertBigEndian(HFSPlusVolumeHeader* header) {
  ConvertBigEndian(&header->signature);
  ConvertBigEndian(&header->version);
  ConvertBigEndian(&header->attributes);
  ConvertBigEndian(&header->lastMountedVersion);
  ConvertBigEndian(&header->journalInfoBlock);
  ConvertBigEndian(&header->createDate);
  ConvertBigEndian(&header->modifyDate);
  ConvertBigEndian(&header->backupDate);
  ConvertBigEndian(&header->checkedDate);
  ConvertBigEndian(&header->fileCount);
  ConvertBigEndian(&header->folderCount);
  ConvertBigEndian(&header->blockSize);
  ConvertBigEndian(&header->totalBlocks);
  ConvertBigEndian(&header->freeBlocks);
  ConvertBigEndian(&header->nextAllocation);
  ConvertBigEndian(&header->rsrcClumpSize);
  ConvertBigEndian(&header->dataClumpSize);
  ConvertBigEndian(&header->nextCatalogID);
  ConvertBigEndian(&header->writeCount);
  ConvertBigEndian(&header->encodingsBitmap);
  ConvertBigEndian(&header->allocationFile);
  ConvertBigEndian(&header->extentsFile);
  ConvertBigEndian(&header->catalogFile);
  ConvertBigEndian(&header->attributesFile);
  ConvertBigEndian(&header->startupFile);
}
static void ConvertBigEndian(BTHeaderRec* header) {
  ConvertBigEndian(&header->treeDepth);
  ConvertBigEndian(&header->rootNode);
  ConvertBigEndian(&header->leafRecords);
  ConvertBigEndian(&header->firstLeafNode);
  ConvertBigEndian(&header->lastLeafNode);
  ConvertBigEndian(&header->nodeSize);
  ConvertBigEndian(&header->maxKeyLength);
  ConvertBigEndian(&header->totalNodes);
  ConvertBigEndian(&header->freeNodes);
  ConvertBigEndian(&header->reserved1);
  ConvertBigEndian(&header->clumpSize);
  ConvertBigEndian(&header->attributes);
}
static void ConvertBigEndian(BTNodeDescriptor* node) {
  ConvertBigEndian(&node->fLink);
  ConvertBigEndian(&node->bLink);
  ConvertBigEndian(&node->numRecords);
}
static void ConvertBigEndian(HFSPlusCatalogFolder* folder) {
  ConvertBigEndian(&folder->recordType);
  ConvertBigEndian(&folder->flags);
  ConvertBigEndian(&folder->valence);
  ConvertBigEndian(&folder->folderID);
  ConvertBigEndian(&folder->createDate);
  ConvertBigEndian(&folder->contentModDate);
  ConvertBigEndian(&folder->attributeModDate);
  ConvertBigEndian(&folder->accessDate);
  ConvertBigEndian(&folder->backupDate);
  ConvertBigEndian(&folder->bsdInfo.ownerID);
  ConvertBigEndian(&folder->bsdInfo.groupID);
  ConvertBigEndian(&folder->bsdInfo.fileMode);
  ConvertBigEndian(&folder->textEncoding);
  ConvertBigEndian(&folder->folderCount);
}
static void ConvertBigEndian(HFSPlusCatalogFile* file) {
  ConvertBigEndian(&file->recordType);
  ConvertBigEndian(&file->flags);
  ConvertBigEndian(&file->reserved1);
  ConvertBigEndian(&file->fileID);
  ConvertBigEndian(&file->createDate);
  ConvertBigEndian(&file->contentModDate);
  ConvertBigEndian(&file->attributeModDate);
  ConvertBigEndian(&file->accessDate);
  ConvertBigEndian(&file->backupDate);
  ConvertBigEndian(&file->bsdInfo.ownerID);
  ConvertBigEndian(&file->bsdInfo.groupID);
  ConvertBigEndian(&file->bsdInfo.fileMode);
  ConvertBigEndian(&file->userInfo.fdType);
  ConvertBigEndian(&file->userInfo.fdCreator);
  ConvertBigEndian(&file->userInfo.fdFlags);
  ConvertBigEndian(&file->textEncoding);
  ConvertBigEndian(&file->reserved2);
  ConvertBigEndian(&file->dataFork);
  ConvertBigEndian(&file->resourceFork);
}

///////////////////////////////////////////////////////////////////////////////
// These are more conversion routines I added.

inline void ConvertLittleEndian(uint32_t* x) {
  *x = OSSwapHostToBigInt32(*x);
}

inline void ConvertLittleEndian(uint16_t* x) {
  *x = OSSwapHostToBigInt16(*x);
}

inline void ConvertLittleEndian(uint16_t* x, size_t len) {
  while (len > 0) {
    ConvertLittleEndian(x);
    x++;
    len--;
  }
}

inline void ConvertBigEndian(uint16_t* x, size_t len) {
  while (len > 0) {
    ConvertBigEndian(x);
    x++;
    len--;
  }
}

static void ConvertBigEndian(HFSUniStr255* str) {
  ConvertBigEndian(&str->length);
  ConvertBigEndian(str->unicode, str->length);
}

static void ConvertBigEndian(HFSPlusCatalogKey* ck) {
  ConvertBigEndian(&ck->keyLength);
  ConvertBigEndian(&ck->parentID);
  ConvertBigEndian(&ck->nodeName);
}

static void ConvertBigEndian(HFSPlusExtentRecord* er) {
  HFSPlusExtentDescriptor* ed = (HFSPlusExtentDescriptor*)er;
  for (size_t i = 0; i < kHFSPlusExtentDensity; ++i) {
    ConvertBigEndian(&ed[i].startBlock);
    ConvertBigEndian(&ed[i].blockCount);
  }
}

static void ConvertBigEndian(HFSPlusExtentKey* ek) {
  ConvertBigEndian(&ek->keyLength);
  ConvertBigEndian(&ek->fileID);
  ConvertBigEndian(&ek->startBlock);
}

inline void EncodeU16(char* str, uint16_t* out, size_t len) {
  while (len > 0) {
    uint16_t c = (uint8_t)*str;
    *out = c;
    str++;
    out++;
    len--;
  }
}

inline void DecodeU16(uint16_t* str, char* out, size_t len) {
  while (len > 0) {
    char c = (uint8_t)((*str) & 0xFF);
    *out = c;
    str++;
    out++;
    len--;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Utility functions.
void error(const char* msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(2);
}

void warning(const char* msg) {
  fprintf(stderr, "Warning: %s\n", msg);
}

void seek(FILE* fd, size_t pos) {
  if (fseek(fd, pos, SEEK_SET) < 0) {
    error("Could not seek through image.");
  }
}

size_t saferead(void* output, size_t bytes, size_t count, FILE* fd) {
  size_t readbytes = fread(output, bytes, count, fd);
  if (readbytes != bytes * count && ferror(fd)) {
    error("Error reading from image.");
  }
  return readbytes;
}

///////////////////////////////////////////////////////////////////////////////
// File system functions.

struct FileInfo {
  char name[256];
  HFSPlusCatalogFile fileRecord;
  ssize_t unlocatedBytes;
  std::vector<HFSPlusExtentDescriptor> extents;

  uint32_t numBlocks() {
    uint32_t bs = 0;
    for (auto e : extents) {
      bs += e.blockCount;
    }
    return bs;
  }
};

#define WINDOW_SIZE 4096
#define NOTIFY_BYTES (10 * 1024 * 1024)

#define BLOCK_SIZE 4096

template<typename Lambda>
void scan(FILE* in, void* needle, size_t patternLen, Lambda l) {
  char* pattern = (char*)needle;
  for (size_t i = 0; i < patternLen; i++) {
    printf("%02x", pattern[i]);
  }

  size_t nBytes = NOTIFY_BYTES;
  char buf[WINDOW_SIZE * 3];
  if (!saferead(buf, WINDOW_SIZE, 2, in)) error("So small");
  int block = 0;
  while (saferead(buf + WINDOW_SIZE * 2, WINDOW_SIZE, 1, in)) {
    for (size_t cursor = WINDOW_SIZE; cursor < WINDOW_SIZE * 2; cursor++) {
      ssize_t patternCursor = patternLen - 1;
      while (patternCursor >= 0 &&
             pattern[patternCursor] ==
             (buf + cursor)[patternCursor]) {
        patternCursor--;
      }
      if (patternCursor == (ssize_t)-1) {
        // We found a match for our pattern.
        printf("\n");
        l(buf, cursor + patternLen, block);
      }
    }
    memcpy(buf, buf + WINDOW_SIZE, WINDOW_SIZE);
    memcpy(buf + WINDOW_SIZE, buf + WINDOW_SIZE * 2, WINDOW_SIZE);
    nBytes -= WINDOW_SIZE;
    if (nBytes < WINDOW_SIZE) {
      printf(".");
      fflush(stdout);
      nBytes = NOTIFY_BYTES;
    }
    block++;
  }
}

void show(HFSPlusCatalogKey* ck) {
  char name[256];
  name[ck->nodeName.length] = '\0';
  DecodeU16(ck->nodeName.unicode, name, ck->nodeName.length); 
  printf("Name: %s\n", name);
}

void show(HFSPlusExtentKey* ek) {
  printf("  forkType: %d,  fileID: %d, startBlock: %d\n",
         ek->forkType, ek->fileID, ek->startBlock);
}

void show(HFSPlusExtentRecord* er) {
  HFSPlusExtentDescriptor* ed = (HFSPlusExtentDescriptor*)er;
  for (size_t i = 0; i < kHFSPlusExtentDensity; i++) {
    printf("  startBlk: %d,  blkCount: %d\n",
           ed[i].startBlock, ed[i].blockCount);
  }
}

void show(HFSPlusForkData* fd) {
  printf("Size: %d bytes\nBlocks: %d\n", fd->logicalSize, fd->totalBlocks);
  show(&(fd->extents));
}

bool getExtents(FileInfo& fi, HFSPlusExtentRecord* er) {
  HFSPlusExtentDescriptor* ed = (HFSPlusExtentDescriptor*)er;
  for (uint8_t iextents = 0;
       iextents < kHFSPlusExtentDensity && fi.unlocatedBytes > 0;
       iextents++) {
    fi.unlocatedBytes -= ed[iextents].blockCount * BLOCK_SIZE;
    fi.extents.push_back(ed[iextents]);
  }
  if (fi.unlocatedBytes > 0) return true;
  return false;
}

void match(FILE* in, char* buf, size_t position, int block) {
  HFSPlusCatalogFile* fileRecord = (HFSPlusCatalogFile*)(buf + position);
  ConvertBigEndian(fileRecord);
  if (fileRecord->recordType != kHFSPlusFileRecord) {
    warning("Found non file.");
    return;
  }
  
  FileInfo fi;
  fi.name[0] = '\0';
  // Look for filename.
  position -= 2;
  for (int i = 0; i < kHFSPlusCatalogKeyMaximumLength; i++) {
    HFSPlusCatalogKey* k = (HFSPlusCatalogKey*)(buf + position - i);
    uint16_t keyLength = k->keyLength;
    ConvertBigEndian(&keyLength);
    if (keyLength == i) {
      ConvertBigEndian(k);
      printf("Found file at block %d\n", block);
      show(k);
      fi.name[k->nodeName.length] = '\0';
      DecodeU16(k->nodeName.unicode, fi.name, k->nodeName.length); 
      
      i += sizeof(uint16_t);
      i += sizeof(uint16_t);
      uint16_t l16 = *(uint16_t*)(buf + position - i);
      ConvertBigEndian(&l16);
      i += sizeof(uint8_t);
      uint8_t l8 = *(uint8_t*)(buf + position - i);
      printf("L8: %d, L16: %d\n", l8, l16);
      break;
    }
  }
  
  printf("fileId %u, create %u, access %u, backup %u\n",
         fileRecord->fileID,
         fileRecord->createDate,
         fileRecord->accessDate,
         fileRecord->backupDate);
  show(&(fileRecord->dataFork));

  fi.fileRecord = *fileRecord;
  fi.unlocatedBytes = fileRecord->dataFork.logicalSize;

  HFSPlusExtentRecord er;
  memcpy(&er, &(fileRecord->dataFork.extents), sizeof(HFSPlusExtentRecord));
  while (getExtents(fi, &er)) {
    printf("Fragmented still need to locate %d bytes.\n", fi.unlocatedBytes);
    struct pattern {
      uint32_t fileID;
      uint32_t startBlock;
    };
    pattern p;
    p.fileID = fileRecord->fileID;
    ConvertLittleEndian(&p.fileID);
    p.startBlock = fi.numBlocks();
    ConvertLittleEndian(&p.startBlock);
    
    ssize_t seekPos = ftell(in);
    if (seekPos < 0) {
      error("Could not find seek position.");
    }
    seek(in, 0);
    scan(in, &p, sizeof(pattern), [&](char* b, size_t pos, int block){
      memcpy(&er, (HFSPlusExtentRecord*)(b + pos), sizeof(HFSPlusExtentRecord));
      HFSPlusExtentKey* ek =
        (HFSPlusExtentKey*)(b + pos - sizeof(HFSPlusExtentKey));
      printf("Found record:\n");
      ConvertBigEndian(ek);
      show(ek);
      ConvertBigEndian(&er);
      show(&er);
    });
    seek(in, seekPos);
  }
}

void save(FILE* in, char* ending) {
  size_t patternLen = strlen(ending);
  if (patternLen == 0) error("No pattern");
  uint16_t* pattern = (uint16_t*)calloc(sizeof(uint16_t), patternLen);
  EncodeU16(ending, pattern, patternLen);
  ConvertLittleEndian(pattern, patternLen);

  scan(in, pattern, patternLen * 2, [&](char* b, size_t pos, int block){
    match(in, b, pos, block);
  });

  free(pattern);
}

FILE* verify(char* filename) {
  FILE* fd = fopen(filename, "rb");

  if (!fd) error("Could not open input file.");

  // The volume header is in the third sector.
  seek(fd, SECTOR_SIZE * 2);

  HFSPlusVolumeHeader vheader;
  saferead(&vheader, sizeof(HFSPlusVolumeHeader), 1, fd);
  ConvertBigEndian(&vheader);
  printf("Signature %x\n", vheader.signature);
  printf("Block Size %d\n", vheader.blockSize);
  if (vheader.signature == kHFSPlusSigWord) {
    printf("Image signature passes.\n");
  } else {
    error("Image signature fails.");
  }
  return fd;
}

// Print the help for the utility.
void help() {
  fprintf(stderr, "Usage:  main [-p <pattern> -o <outfile>] <infile>\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  int opt;
  char *outfile = NULL;
  char *infile = NULL;
  char *pattern = NULL;

  while ((opt = getopt(argc, argv, "o:p:")) != -1) {
    switch (opt) {
      case 'o':
        // We found an outfile location.
        outfile = optarg;
        break;
      case 'p':
        // We found a pattern to find files with.
        pattern = optarg;
        break;
      default: /* '?' */
        help();
    }
  }

  infile = argv[optind];
  if (!infile) help();

  FILE* fd = verify(infile);

  if (pattern) {
    save(fd, pattern);
  }

  // Success.
  return 0;
}

