#ifndef _CMDEFS_H
#define _CMDEFS_H

/* status values */

#define DB_STAT_STOP    0           /* target is stopped */
#define DB_STAT_BREAK   1           /* breakpoint (not by break opcodes ) */
#define DB_STAT_EXCEPT  2           /* unhandled exception */
#define DB_STAT_RUN     3           /* target is running */

/* general commands */

#define CMD_REBOOT      0xA0        /* soft reboot console */

/* debug and execution commands */

#define CMD_DB_GETSTAT  0xD0        /* get target status */
#define CMD_DB_GETINFO  0xD1        /* get debug monitor info */
#define CMD_DB_SETEXEC  0xD2        /* execution control */
#define CMD_DB_RUNTO    0xD3        /* run to address */
#define CMD_DB_SETBRK	0xD4		/* Set a program breakpoint */
#define CMD_DB_CLRBRK	0xD5		/* Clear all program breakpoints */
#define CMD_DB_GETREGS  0xD6        /* get registers */
#define CMD_DB_GETMEM   0xD7        /* get memory */
#define CMD_DB_WORD     0xD8		/* get a word value */
#define CMD_DB_GETBRK	0xD9		/* get current breakpoints */
#define CMD_DB_SETREG	0xDA		/* set processor registers */
#define CMD_DB_SETBDREG	0xDB		/* set data breakpoint registers */

/* exec control values for CMD_DB_SETEXEC */

#define CMD_EXEC_STOP   0
#define CMD_EXEC_STEP   1
#define CMD_EXEC_RESUME 2

#endif /* _CMDEFS_H */
