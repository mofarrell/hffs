# HFFS (Help find files script)

This utility program is designed as a HFS+ format disk recovery program.  Rather
than following the file system b-trees, it instead walks the disk looking for
anything that might be a file, directory, or extent record.  After compiling an
index of the contained information, it recreates all the files and directories
found into an output folder.

## Rough usage

Assuming a standard disk sector size of 512:
```
hffs <input-image>
```

This will look for the disk header and footer, and spit out some info including
the block size used for the HFS+ format.  We can then kick off the recovery
using the block size.
```
hffs --block-size <block-size> -o <output-directory> <input-image>
```

Additional options allow fine tuning of various other settings, and can
dramatically speed up the process.  Of particular interest is the `--stop-block`
option which will stop searching for file system information after a particular
block number.  Since HFS+ stores the file system catalog info in duplicate once
at the start of the disk, and once at the end of the disk, stopping the search
after the initial catalog entries have been found will often yield good results.

## Disclaimer

I worked on this until it fullfilled my needs and recovered data off of a
corrupted disk I had.  Your mileage may vary.  Of particular importance is to
only run this on an image of your corrupted disk, not the disk itself.  Use a
program to safely copy the contents off of the disk minimizing the chance for
additional damage.  Again HFFS makes no effort to minimize disk seeks, or reads.
Running it on a damaged disk may cause further harm.  Use a program like
ddrescue to create a copy of the data.
