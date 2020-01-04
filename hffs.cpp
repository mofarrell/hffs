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
               " [--sector-size <sector-size>=512]"
               " [--block-size <block-size>]"
               " [--stop-block <block-number>]"
               " [--buffer-size <buffer-size>=<block-size>]"
               " [--catalog-node-size <node-size>=<block-size>]"
               " [--extent-node-size <node-size>=<block-size>]"
               " [-o <outfile>] <infile>" << std::endl;
  exit(EXIT_FAILURE);
}

} // namespace
///////////////////////////////////////////////////////////////////////////////

// Command line option parsing.  Dispatches to implementation.
int main(int argc, char* argv[]) {
  int c;
  char* bs = nullptr;
  char* stopBlock = nullptr;
  char* bufferSize = nullptr;
  char* ss = nullptr;
  char* catalogNodeSize = nullptr;
  char* extentNodeSize = nullptr;
  char* outdir = nullptr;
  char* infile = nullptr;
  bool permissive = false;

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
      {"block-size",  required_argument,         0, 'b' },
      {"buffer-size",  required_argument,        0,  0  },
      {"catalog-node-size", required_argument,   0,  1  },
      {"extent-node-size", required_argument,    0,  2  },
      {"outdir",      required_argument,         0, 'o' },
      {"permissive",  no_argument,               0, 'p' },
      {"sector-size", required_argument,         0, 's' },
      {"stop-block", required_argument,          0,  3  },
      {0,             0,                         0,  0  }
    };

    c = getopt_long(argc, argv, "s:b:o:ph", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
        bufferSize = optarg;
        break;
      case 1:
        catalogNodeSize = optarg;
        break;
      case 2:
        extentNodeSize = optarg;
        break;
      case 3:
        stopBlock = optarg;
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
  uint64_t blockSize = bs ? std::stoul(bs) : 0;
  RGS rgs{{
    infile,
    outdir,
    permissive,
    ss ? std::stoul(ss) : kDefaultSectorSize,
    blockSize,
    stopBlock ? std::stoul(stopBlock) : 0,
    bufferSize ? std::stoul(bufferSize) : blockSize,
    catalogNodeSize ? std::stoul(catalogNodeSize) : blockSize,
    extentNodeSize ? std::stoul(extentNodeSize) : blockSize,
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

