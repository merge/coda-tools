# coda-tools
tools to play with i.MX VPU firmware binaries

These tools are in a very early development phase and do ***not***
behave like they should. Please don't use them!

## coda-fw-cleaner
This tool removes (overwrites with zero) parts of the binary firmware
file.

By default it overwrites as much as possible - anything that is not
needed for **decoding VP8**.

### why VP8?
VP8 is an open video encoding / decoding standard used in
[webm](https://www.webmproject.org/) media container formats. There are
[no license consideration](https://en.wikipedia.org/wiki/VP8#Comparison_with_H.264)
or costs connected to it. For other codecs one has to
[become a licensee of patent holders](https://en.wikipedia.org/wiki/H.264/MPEG-4_AVC#Licensing)
and pay fees in order to be allowed to play videos for customers.

### why bother?
For video playback (and encoding) on i.MX platforms, binary firmware
images are needed, to be run on the included BIT digital signals
processor.

We cannot know **how** this processor decodes a video stream. It is
possible that this processor (using these binary files) includes
undesired information in the decoded data - think of invisible watermarks 
that identify the video in some way, if analysed.

As far as I know, even stripping down the firmware doesn't do anything
because binary blocks that aren't used by VP8 decoding really aren't
loaded into the BIT processor to execute. But that's only as far as I
know.

By stripping away large parts of the firmware, we reduce the probability
to use something we don't want to and just can be sure nothing from other
memory areas being used.

While there is a certain amount of paranoia level required to be
convinced that this changes things, it's always bad to only have compiled
binary files. And if we can't do without, they better be small.

If compressed, this saves about 70kB of space on disk.

### background
This work is based on the [coda-bits](https://github.com/pH5/coda-bits)
project. It's `coda-fwinfo` program is included here too.
