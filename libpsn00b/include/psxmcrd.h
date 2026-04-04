/*
 * psxmcrd.h — PlayStation 1 memory card library
 *
 * Provides a higher-level file-oriented API over the BIOS memory card
 * routines, compatible with the original Psy-Q/LIBMCRD naming conventions.
 *
 * Typical write usage:
 *   McInit();
 *   int fd = McOpen(0, "MYFILE", 1);   // create/overwrite on slot 1
 *   McWrite(fd, &data, sizeof(data));   // len must be a multiple of 128
 *   McClose(fd);
 *
 * Typical read usage:
 *   int fd = McOpen(0, "MYFILE", 0);   // open existing for reading
 *   McRead(fd, &data, sizeof(data));
 *   McClose(fd);
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * Initialise the memory card subsystem.
     * Must be called once before any other Mc* function.
     * Starts the card controller and enables card interrupts.
     */
    void McInit(void);

    /*
     * Open a file on the memory card.
     *
     * port  Slot index: 0 = slot 1 (bu00:), 1 = slot 2 (bu01:).
     * name  File name, max 20 characters, no path separators.
     * mode  0 = open existing file for reading.
     *       1 = create or overwrite file for writing (allocates one 8 KB block).
     *
     * Returns a non-negative file descriptor on success, -1 on failure.
     */
    int McOpen(int port, const char *name, int mode);

    /*
     * Close a file descriptor returned by McOpen.
     * Returns 0 on success, -1 on failure.
     */
    int McClose(int fd);

    /*
     * Read len bytes from fd into buf.
     * Returns the number of bytes read, or -1 on failure.
     */
    int McRead(int fd, void *buf, int len);

    /*
     * Write len bytes from buf to fd.
     * len must be a multiple of 128 (one memory card sector).
     * Returns the number of bytes written, or -1 on failure.
     */
    int McWrite(int fd, const void *buf, int len);

    /*
     * Delete a file from the memory card.
     *
     * port  Slot index: 0 = slot 1, 1 = slot 2.
     * name  Name of the file to delete.
     *
     * Returns 0 on success, -1 on failure (e.g. file does not exist).
     */
    int McDelete(int port, const char *name);

#ifdef __cplusplus
}
#endif
