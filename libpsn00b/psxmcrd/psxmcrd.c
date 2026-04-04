/*
 * psxmcrd.c — PlayStation 1 memory card library implementation
 *
 * Wraps the BIOS file I/O (bu00:/bu01: device paths) and card-init routines
 * exposed by PSn00bSDK's psxapi.h into a higher-level API matching the
 * original Psy-Q LIBMCRD naming conventions.
 *
 * Memory card file system notes:
 *   - Each memory card block is 8 KB (64 × 128-byte sectors).
 *   - FNBLOCKS(n) in the open flags reserves n blocks on creation.
 *   - Write lengths must be multiples of 128 (one sector).
 *   - BIOS open mode 8 triggers file deletion as per the nocash PSX
 *     specification §A(00h) FileOpen(filename, accessmode).
 */

#include "psxmcrd.h"
#include <psxapi.h>
#include <sys/fcntl.h>

/* Maximum filename length on a PS1 memory card directory entry. */
#define MC_NAME_MAX 20

/*
 * BIOS FileOpen access-mode bit for deletion.
 * Ref: nocash PSX spec §BIOS A(00h): bit3 = delete (no data transfer needed).
 */
#define BIOS_OPEN_DELETE 8

/* ─── Internal helpers ──────────────────────────────────────────────────────── */

/*
 * Build a full device path "bu0X:NAME" into out.
 * out must be at least 27 bytes ("bu0X:" is 5, name up to 20, plus NUL).
 */
static void build_path(char *out, int port, const char *name)
{
    int i;
    out[0] = 'b';
    out[1] = 'u';
    out[2] = '0';
    out[3] = '0' + (char)(port & 1); /* slot 0 -> '0', slot 1 -> '1' */
    out[4] = ':';
    for (i = 0; name[i] && i < MC_NAME_MAX; i++)
        out[5 + i] = name[i];
    out[5 + i] = '\0';
}

/* ─── Public API ────────────────────────────────────────────────────────────── */

void McInit(void)
{
    InitCARD(1); /* 1 = share IRQ with pad driver */
    StartCARD();
    /*
     * _bu_init() probes the card directory and blocks until the controller
     * has finished its initialisation sequence.  On real hardware this is
     * mandatory — without it the first open() fires before the card is
     * ready and returns -1.  Emulators rarely enforce the timing, which is
     * why omitting this call only manifests as a failure on hardware.
     */
    _bu_init();
}

int McOpen(int port, const char *name, int mode)
{
    char path[27];
    build_path(path, port, name);

    if (mode == 0)
        return open(path, FREAD);

    /*
     * mode 1: overwrite existing file, or create if absent.
     * Try a plain write-open first; if the file does not yet exist the BIOS
     * returns -1 and we retry with FCREATE|FNBLOCKS(1) to allocate one 8 KB
     * block.  This avoids FCREATE failing on an already-existing file.
     */
    {
        int fd = open(path, FWRITE);
        if (fd >= 0)
            return fd;
        return open(path, FWRITE | FCREATE | FNBLOCKS(1));
    }
}

int McClose(int fd)
{
    return close(fd);
}

int McRead(int fd, void *buf, int len)
{
    return read(fd, buf, len);
}

int McWrite(int fd, const void *buf, int len)
{
    return write(fd, (void *)buf, len);
}

int McDelete(int port, const char *name)
{
    char path[27];
    int fd;
    build_path(path, port, name);

    /*
     * The PS1 BIOS deletes a file by opening it with access-mode 8.
     * No read/write is needed — the file is removed on close.
     * Ref: nocash PSX specification §BIOS A(00h) FileOpen.
     */
    fd = open(path, BIOS_OPEN_DELETE);
    if (fd < 0)
        return -1;
    close(fd);
    return 0;
}
