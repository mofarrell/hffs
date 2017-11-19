// HFFS (Help find files script)
// Author: Michael O'Farrell

#include "recover.h"
#include "rgs.h"

#include <iostream>

// C includes
#include <getopt.h>
#include <stdlib.h>

namespace {

constexpr uint64_t kDefaultSectorSize = 512;

// Prints the help message for launching the utility.
void help(char* command) {
  std::cerr << "Usage: "<< command <<
               " [--sector-size <sector-size>=512] [--block-size <block-size>]"
               " [--scan-size <scan-size>=512]"
               " [-o <outfile>] <infile>" << std::endl;
  exit(EXIT_FAILURE);
}

} // namespace
///////////////////////////////////////////////////////////////////////////////

// Command line option parsing.  Dispatches to implementation.
int main(int argc, char* argv[]) {
  int c;
  char* bs = nullptr;
  char* ss = nullptr;
  char* scanSize = nullptr;
  char* outdir = nullptr;
  char* infile = nullptr;
  bool permissive = false;

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
      {"block-size",  required_argument, 0, 'b' },
      {"outdir",      required_argument, 0, 'o' },
      {"permissive",  no_argument,       0, 'p' },
      {"scan-size", required_argument, 0,  0 },
      {"sector-size", required_argument, 0, 's' },
      {0,             0,                 0,  0  }
    };

    c = getopt_long(argc, argv, "s:b:o:ph", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
        scanSize = optarg;
        break;
      case 'b':
        bs = optarg;
        break;
      case 'o':
        outdir = optarg;
        break;
      case 'p':
        permissive = true;
        break;
      case 's':
        ss = optarg;
        break;
      case 'h':
      default:
        help(argv[0]);
    }
  }

  if (optind != argc - 1) {
    help(argv[0]);
  }

  // The last argument is for the image file to be processed.
  infile = argv[argc - 1];
  RGS rgs{{
    infile,
    outdir,
    permissive,
    ss ? std::stoul(ss) : kDefaultSectorSize,
    bs ? std::stoul(bs) : 0,
    scanSize ? std::stoul(scanSize) : bs ? std::stoul(bs) : 0
  }};

  try {
    // Lets find the main block record and print info.
    verify(rgs);
    if (outdir && bs) {
      // We have the arguments to hunt for files.
      recover(rgs);
    }
  } catch (std::runtime_error err) {
    std::cerr << "Error: " << err.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

