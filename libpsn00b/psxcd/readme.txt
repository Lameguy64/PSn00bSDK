PSX CD-ROM library, part of PSn00bSDK
2020 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

	Open source implementation of the long awaited CD-ROM library that
provides greater functionality than the BIOS CD-ROM subsystem. Supports
pretty much all features of the CD-ROM hardware such as CD data read,
CD Audio and XA audio playback. Data streaming should also be possible
with the use of CdlReadN/CdlReadS commands and CdReadyCallback().

	Library also includes an ISO9660 file system parser for locating
files within the CD-ROM file system. Unlike the file system parser in
the official libraries libpsxcd can parse directories containing any
number of files. Rock-ridge and Joliet extensions are not supported
however.

	Be aware that the CD-ROM library might have some loose ends as it
is still a work in progress, but should work flawlessly in most use
cases.


Library developer(s):

	Lameguy64


Library header(s):

	psxcd.h


Todo list:

	* Command query mechanism so that more than 2 CD-ROM commands can
	  easily be issued in callbacks. Official library probably does this.
	
	* Helper functions for handling disc changes (CdDiskReady and
	  CdGetDiskType) are not yet implemented.
	
	* CdReadBreak() not yet implemented.
	
	* Data streaming functions (prefixed with St*) not yet implemented.
	  Would require devising a PSn00bSDK equivalent of the STR file
	  format.