# Chapter 4.2: Reading a file from CD-ROM

This chapter details how to read a file from the CD-ROM, loading the
contents into system RAM using the CD-ROM library.

**PSn00bSDK Compatible:** Yes

## Tutorial Index

- [Initializing the CD-ROM](#initializing-the-cd-rom)
- [Locating a file in the CD-ROM](#locating-a-file-in-the-cd-rom)
- [Reading File Contents](#reading-file-contents)
- [Simple File Read Function](#simple-file-read-function)
- [Image Loader Example](#image-loader-example)
- [Building the ISO image](#building-the-iso-image)
- [Conclusion](#conclusion)

## Initializing the CD-ROM

Initializing the CD-ROM library is accomplished by simply calling
**CdInit()**. This function must be called after calling
**ResetGraph()** and before any CD library related function. If you
intend to use the SPU you must call **SpuInit()** immediately before
calling **CdInit()**. Once the CD-ROM library is initialized any other
CD-ROM related function can be used.

### SDK Difference

In PsyQ/Programmers\' Tool, **CdInit()** will throw a lot of CD retry
messages when no disc is inserted and may lock up. In PSn00bSDK the
function carries on like normal when no disc is inserted.

## Locating a file in the CD-ROM

The CD-ROM library does not have a concept of file handles. Instead,
files are read by locating the start position of a file through the
ISO9660 file system, seeking to it and issuing a read operation until a
set number of sectors have been read. This method of loading files from
disc should not be a problem under any circumstances as files stored in
a ISO9660 file system are never fragmented. In fact, this method of
reading files may have some advantages.

Locating a file on the disc is done through the **CdSearchFile()**
function, which parses through the disc\'s file system to locate the
file and returns a **CdlFILE** which describes information about the
file found including it\'s position in the disc.

    CdlFILE file;

    // Search for the file
    if( !CdSearchFile( &file;, "\\MYDIR\\MYFILE.DAT;1" ) )
    {
        // Return value is NULL, file is not found
        printf( "File not found.\n" );
        return 0;
    }

    // When file found
    printf( "File found!\n" );

The file name must be specified as a complete path from root and must
use backlash characters as directory separators (use \\\\ in C code).
The file name must also end with a file version number which is always
\';1\'. If the file is found, it will return the pointer to the
specified **CdlFILE** struct, otherwise it would simply return NULL.

The **CdlFILE** struct contains a copy of the file name, file size and
most importantly, the location.

## Reading File Contents

Now for the interesting stuff. Reading sectors is accomplished by using
**CdRead()**. But before you start a read operation, you must first set
the location of where to start reading from. This can be done using
**CdControl()** and the **CdlSetloc** command. Use this to set the
target location of the file.

    char *buffer;

    // Allocate a buffer for the file
    buffer = (char*)malloc( 2048*((file.size+2047)/2048) );

    // Set seek target (seek actually happens on CdRead())
    CdControl( CdSetloc, &file.loc;, 0 );

    // Read sectors
    CdRead( (file.size+2047)/2048, (unsigned int*)buffer, CdlModeSpeed );

    // Wait until read has completed
    CdReadSync( 0, 0 );

You may have noticed by now that read sizes in this chapter are
described in sector units rather than byte units. That\'s because the
CD-ROM subsystem can only read in sector units and the CD-ROM library is
optimized for reading in sector units rather than buffering sectors to
allow for byte by byte reading for performance reasons.

Data sectors are typically 2048 bytes in size so buffers where data read
from the CD would be loaded to must be multiples of 2048 bytes. The
arithmetic to snap byte sizes to the nearest sector unit is described in
the pseudo code described above.

Once a read operation has been issued, you must wait until reading has
completed using **CdReadSync()**. You can alternatively poll the status
asynchronously by setting the *mode* parameter to 1 to perform
animations while waiting for a long read to complete.

If you intend to read only a part of a file instead of a whole file in a
single read operation, be aware that you can\'t simply call **CdRead()**
again to read the following sectors. Instead, you must retrieve the
current location using **CdlGetloc**, set it as the seek target with
**CdlSetloc** then call **CdRead()**.

## Simple File Read Function

The examples described above can be consolidated into a single function,
which can be useful for quickly implementing a file read function for
testing purposes:

    u_long *load_file(const char* filename)
    {
        CdlFILE file;
        u_long  *buffer;
        
        
        printf( "Reading file %s... ", filename );

        // Search for the file
        if( !CdSearchFile( &file;, (char*)filename ) )
        {
            // Return value is NULL, file is not found
            printf( "Not found!\n" );
            return NULL;
        }
        
        // Allocate a buffer for the file
        buffer = (u_long*)malloc( 2048*((file.size+2047)/2048) );

        // Set seek target (seek actually happens on CdRead())
        CdControl( CdlSetloc, (u_char*)&file.pos;, 0 );

        // Read sectors
        CdRead( (file.size+2047)/2048, buffer, CdlModeSpeed );

        // Wait until read has completed
        CdReadSync( 0, 0 );
        
        printf( "Done.\n" );

        return buffer;
    }

If you want to do asynchronous loading to do background animations,
you\'ll have to use the CD functions more directly.

## Image Loader Example

For completedness, this example demonstrates loading a high resolution
24-bit TIM from CD and displaying it into the framebuffer in 24-bit
color mode. Take any image file you wish to use, scale it down to
640x480 and convert it into a 24-bit TIM with the load position set to
(0,0). Name that TIM file *myimage.tim* for this example.

**PsyQ/Programmers\' Tool Version**

    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/types.h>
    #include <libgte.h>
    #include <libgpu.h>
    #include <libetc.h>
    #include <libcd.h>

    // Display environment struct
    DISPENV disp;

    u_long *load_file(const char* filename)
    {
        CdlFILE file;
        u_long  *buffer;
        
        
        printf( "Reading file %s... ", filename );

        // Search for the file
        if( !CdSearchFile( &file;, (char*)filename ) )
        {
            // Return value is NULL, file is not found
            printf( "Not found!\n" );
            return NULL;
        }
        
        // Allocate a buffer for the file
        buffer = (u_long*)malloc( 2048*((file.size+2047)/2048) );

        // Set seek target (seek actually happens on CdRead())
        CdControl( CdlSetloc, (u_char*)&file.pos;, 0 );

        // Read sectors
        CdRead( (file.size+2047)/2048, buffer, CdlModeSpeed );

        // Wait until read has completed
        CdReadSync( 0, 0 );
        
        printf( "Done.\n" );

        return buffer;
    }

    void display_picture()
    {
        u_long      *image;
        TIM_IMAGE   tim;

        // Load the image file
        if( !(image = load_file( "\\MYIMAGE.TIM;1" )) )
        {
            printf( "Could not load image file.\n " );
            return;
        }

        // Read TIM header
        OpenTIM( image );
        ReadTIM( &tim; );

        // Load image to VRAM
        LoadImage( tim.prect, tim.paddr );
        DrawSync( 0 );

        // Enable video display
        VSync( 0 );
        SetDispMask( 1 );
    }

    void init()
    {
        // Reset GPU (also installs IRQ handlers which are mandatory)
        ResetGraph(0);

        // Init CD-ROM library
        CdInit();

        // Set DISPENV for 640x480 24-bit color mode
        SetDefDispEnv( &disp;, 0, 0, 640, 480 );
        disp.isrgb24 = 1;   // Enables 24-bit (cannot be used for GPU graphics)
        disp.isinter = 1;   // Enable interlace so hi-res will display properly

        // Set display environment
        PutDispEnv( &disp; );
    }

    int main(int argc, const char *argv[])
    {
        // Init stuff
        init();

        // Load and display picture
        display_picture();

        // Loop for safe idling
        while( 1 )
        {
            VSync( 0 );
        }

        return 0;
    }

**PSn00bSDK Version**

    #include <stdio.h>
    #include <malloc.h>
    #include <psxgpu.h>
    #include <psxcd.h>

    // Display environment struct
    DISPENV disp;


    unsigned int *load_file(const char* filename)
    {
        CdlFILE file;
        unsigned int *buffer;
        
        
        printf( "Reading file %s... ", filename );

        // Search for the file
        if( !CdSearchFile( &file;, (char*)filename ) )
        {
            // Return value is NULL, file is not found
            printf( "Not found!\n" );
            return NULL;
        }
        
        // Allocate a buffer for the file
        buffer = (unsigned int*)malloc( 2048*((file.size+2047)/2048) );

        // Set seek target (seek actually happens on CdRead())
        CdControl( CdlSetloc, (unsigned char*)&file.pos;, 0 );

        // Read sectors
        CdRead( (file.size+2047)/2048, buffer, CdlModeSpeed );

        // Wait until read has completed
        CdReadSync( 0, 0 );
        
        printf( "Done.\n" );

        return buffer;
    }

    void display_picture()
    {
        unsigned int    *image;
        TIM_IMAGE       tim;

        // Load the image file
        if( !(image = load_file( "\\MYIMAGE.TIM;1" )) )
        {
            printf( "Could not load image file.\n " );
            return;
        }

        // Read TIM header
        GetTimInfo( image, &tim; );

        // Load image to VRAM
        LoadImage( tim.prect, tim.paddr );
        DrawSync( 0 );
        
        // Enable video display
        VSync( 0 );
        SetDispMask( 1 );
    }

    void init()
    {
        // Reset GPU (also installs IRQ handlers which are mandatory)
        ResetGraph(0);

        // Init CD-ROM library
        CdInit( 0 );

        // Set DISPENV for 640x480 24-bit color mode
        SetDefDispEnv( &disp;, 0, 0, 640, 480 );
        disp.isrgb24 = 1;   // Enables 24-bit (cannot be used for GPU graphics)
        disp.isinter = 1;   // Enable interlace so hi-res will display properly

        // Set display environment
        PutDispEnv( &disp; );
    }

    int main(int argc, const char *argv[])
    {
        // Init stuff
        init();

        // Load and display picture
        display_picture();
        
        // Loop for safe idling
        while( 1 )
        {
            VSync( 0 );
        }

        return 0;
    }

Most notable differences between the PsyQ/Programmers\' Tool and
PSn00bSDK versions is the latter uses type **unsigned int** instead of
**u\_long** or **unsigned long** and for good reason. Modern compilers
such as GCC 7.4.0 tend to assume long to be a 64-bit integer and may
cause problems when using **unsigned long**. There\'s no **OpenTIM()**
and **ReadTIM()** equivalent. Instead, there\'s **GetTimInfo()** but
this may change in the future.

Make sure you\'ve compiled the example as readfile.exe for coherency
with the rest of this chapter.

## Building the ISO image

In the past, the only way to create proper ISO images for PS1 games is
to use a tool called BUILDCD included in leaked copies of the PsyQ SDK.
The biggest problem of using BUILDCD is it generates an image file of a
special format not supported by any burner program, so another tool has
to be used (stripiso) to convert the image file it generates into a
usable ISO image. Not only that, BUILDCD is a 16-bit DOS executable and
is difficult and very slow to use on a modern system.

Instead of using BUILDCD, use
[MKPSXISO](https://github.com/lameguy64/mkpsxiso)
instead. It offers about the same functionality as BUILDCD but better,
and is built for modern Windows and Linux platforms.

**MKPSXISO Project XML**

    <?xml version="1.0" encoding="UTF-8"?>

    <!-- Defines the ISO project -->
    <iso_project image_name="readfile.iso">

        <!-- Defines the data track for this tutorial -->
        <track type="data">

            <!-- Specifies identifier strings such as volume label -->
            <!-- System and application identifiers must be PLAYSTATION -->
            <identifiers
                system      ="PLAYSTATION"
                application ="PLAYSTATION"
                volume      ="PSXTUTORIAL"
                volume_set  ="PSXTUTORIAL"
                publisher   ="MEIDOTEK"
            />

            <!-- Defines the directory tree of the data track -->
            <directory_tree>

                <!-- Specify files in the directory tree -->
                <file name="system.cnf"      type="data" source="system.cnf"/>
                <file name="readfile.exe"    type="data" source="readfile.exe"/>
                <file name="myimage.tim"     type="data" source="myimage.tim"/>

                <!-- Place dummy sectors at the end -->
                <dummy sectors="1024"/>

            </directory_tree>

        </track>

    </iso_project>

While on the subject of ISO image creation, you\'ll also need to create
a special SYSTEM.CNF file in order to make your disc image bootable. A
SYSTEM.CNF file is simply a text file that specifies the file name of
the executable, stack address, number of task and event blocks. The
latter three wouldn\'t be discussed in this chapter and the typical
values are generally good enough.

**SYSTEM.CNF Contents**

    BOOT=cdrom:\readfile.exe;1
    TCB=4
    EVENT=10
    STACK=801FFFF0

Once your MKPSXISO project and SYSTEM.CNF files are set, execute
MKPSXISO like so to create the ISO image:

    mkpsxiso readfile.xml

Run the ISO image in an emulator and you should see your 24-bit TIM
image displayed. It may take awhile for the image to appear as the
24-bit TIM image is a pretty big file to load for the PS1.

## Conclusion

This chapter should cover about everything you need to know to load a
file from CD-ROM. The next chapter will cover common ways to optimize
CD-ROM accesses to speed up load times in your homebrew.

---

  [Previous](chapter_4_1.md)  |  [Back to Index](index.md)  |  [Next](chapter_4_3.md)

---