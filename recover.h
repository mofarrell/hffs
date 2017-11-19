// HFFS (Help find files script)
// Author: Michael O'Farrell

#pragma once

#include "rgs.h"

// Attempt to recover files.  Using a two phase approach:
//   - Find blocks holding file, folder, and extent records.
//   - Recover each file by chaining the folders to determine its location, and
//     chaining the extents to find its location on the disk.
void recover(RGS& env);

// Verify the tags in the Volume blocks, and print out the block sizes.
// Arguments:
//   img: the filename to process.
//   permissive:  if we should ignore some errors.
void verify(RGS& env);

