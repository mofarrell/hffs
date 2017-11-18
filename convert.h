// HFFS (Help find files script)
// Author: Michael O'Farrell

#pragma once

#include "hfs/hfs_format.h"

#include <libkern/OSByteOrder.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Waddress-of-packed-member"

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

#pragma clang diagnostic pop

