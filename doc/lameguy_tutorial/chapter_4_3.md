# Chapter 4.3: CD-ROM Optimizations

This chapter details some of the usual tricks and techniques for
optimizing the CD-ROM image of your project, to help improve load times
and possibly helps reduce wear on the CD-ROM pickup in the console
itself.

## Data Ordering

Data in a CD-ROM is normally stored in a linear fashion and no file is
ever fragmented, somewhat similar to a record. A common method of
improving loading performance is by ordering files in a manner that
frequently used files are placed close together. This reduces the amount
of time it takes to seek between files.

Data ordering is typically controlled in BUILDCD and MKPSXISO by when in
the script the file is specified. As in, the first file specified will
be the closest to the very beginning of the disc wheras a file specified
last will be placed at the very end of the file system.

If your project loads a set of files sequentially, it is optimal to
order the files in the same manner. That way, the optical pickup does
not have to seek much to read the next file (ie. file1 -\> file2 -\>
file3).

Think of seeking as like having to lift the needle of the turntable to
select a different piece of music on the record to play.

## Single Reads are Faster

Much like a turntable, CD-ROM drives perform best when reading big
chunks of data linearly. This same principle also applies on the PS1.
Therefore, loading one big file in a single read operation is
significantly faster than loading several smaller files from CD-ROM.

This method of optimization can be accomplished by developing a very
simple file archive format just for consolidating several small files
such as texture images into one large file. This may be described in
future chapters.

## Place Streaming Data Last

It is ideal to place streaming data such as CD-XA and STR video files
after game data. This way, the seeking distance when loading game files
will be focussed in the innermost part of the disc. Having an XA stream
or STR video file in between such files will add a considerable amount
on seek distance as such files are usually quite large.

Naturally, CD Audio tracks cannot be placed within a data track and is
always placed the end of the file system, due to the way how CDs and
tracks work.

---

  [Previous](chapter_4_2.md)  |  [Back to Index](index.md)

---