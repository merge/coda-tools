# coda-tools
tools to play with i.MX VPU firmware binaries

## imx_vpu_fw_cleaner
This tool removes (overwrites with zero) parts of the binary firmware
file.

By default it overwrites as much as possible - anything that is not
needed for **decoding VP8**.

### why?
VP8 is a free video encoding / decoding standard. There are no license
consideration whatsoever connected to it. For other codecs one has to
become a licensee of various patent holders in order to be allowed to
play videos for customers.

It is great to have an alternative to patented technologies and it's
therefore often the preferred.

For video playback (and encoding) on i.MX platforms, binary firmware
images are needed, to be run on the included BIT digital signals
processor.

We cannot know **how** this processor decodes a video stream. It is
possible that this processor (using these binary files) includes
undesired information in the decoded data - think of invisible watermarks 
that identify the video in some way, if analysed.

While there is a certain amount of paranoia level required to be
convinced that this is an issue, it's always bad to only have compiled
binary files only.

### background

TODO coda-bits repo
