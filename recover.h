// HFFS (Help find files script)
// Author: Michael O'Farrell

#pragma once

#include "rgs.h"

// Verify the tags in the Volume blocks, and print out the block sizes.
// Arguments:
//   img: the filename to process.
//   permissive:  if we should ignore some errors.
void verify(RGS& env);

