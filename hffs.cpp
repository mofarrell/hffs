// HFFS (Help find files script)
// Author: Michael O'Farrell

#include <iostream>

// C includes
#include <getopt.h>
#include <stdlib.h>

// Prints the help message for launching the utility.
static void help(char* command) {
  std::cout << "Usage: "<< command <<
               " [-bs <block-size> -o <outfile>] <infile>" << std::endl;
  exit(EXIT_FAILURE);
}

// Command line option parsing.  Dispatches to implementation.
int main(int argc, char* argv[]) {
  int c;
  char* bs = nullptr;
  char* outdir = nullptr;
  char* infile = nullptr;
  bool permissive = false;

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
      {"bs",         required_argument, 0, 'b' },
      {"outdir",     required_argument, 0, 'o' },
      {"permissive", no_argument,       0, 'p' },
      {0,            0,                 0,  0 }
    };

    c = getopt_long(argc, argv, "b:o:ph", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
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
  if (outdir && bs) {
    // We have the arguments to hunt for files.

  } else {
    // Lets find the main block record and print info.

  }

  exit(EXIT_SUCCESS);
}

