#ifndef __PSXAPI__
#define __PSXAPI__

typedef struct {			// Device control block
	char	*name;
	int		flags;
	int		ssize;
	char	*desc;
	void	*f_init;
	void	*f_open;
	void	*f_inout;
	void	*f_close;
	void	*f_ioctl;
	void	*f_read;
	void	*f_write;
	void	*f_erase;
	void	*f_undelete;
	void	*f_firstfile;
	void	*f_nextfile;
	void	*f_format;
	void	*f_chdir;
	void	*f_rename;
	void	*f_remove;
	void	*f_testdevice;
} DCB;

typedef struct {			// File control block
	int				status;
	unsigned int	diskid;
	void			*trns_addr;
	unsigned int	trns_len;
	unsigned int	filepos;
	unsigned int	flags;
	unsigned int	lasterr;
	DCB				*dcb;
	unsigned int	filesize;
	unsigned int	lba;
	unsigned int	fcbnum;
} FCB;

struct DIRENTRY {			// Directory entry
	char			name[20];
	int				attr;
	int				size;
	struct DIRENTRY	*next;
	int				head;
	char			system[4];
};

// Not recommended to use these functions to install IRQ handlers

typedef struct {
	unsigned int* next;
	unsigned int* func2;
	unsigned int* func1;
	unsigned int pad;
} INT_RP;

extern void SysEnqIntRP(int pri, INT_RP *rp);
extern void SysDeqIntRP(int pri, INT_RP *rp);

// Use event handlers instead

extern int OpenEvent(unsigned int class, int spec, int mode, void (*func)());
extern int CloseEvent(int ev_desc);
extern int EnableEvent(int ev_desc);
extern int DisableEvent(int ev_desc);

// BIOS file functions

extern int open(const char *name, int mode);
extern int close(int fd);
extern int seek(int fd, unsigned int offset, int mode);
extern int read(int fd, char *buff, unsigned int len);
extern int write(int fd, const char *buff, unsigned int len);
extern int ioctl(int fd, int cmd, int arg);
extern struct DIRENTRY *firstfile(const char *wildcard, struct DIRENTRY *entry);
extern struct DIRENTRY *nextfile(struct DIRENTRY *entry);
extern int erase(const char *name);
extern int chdir(const char *path);

#define delete( p )	erase( p )
#define cd( p )		chdir( p )			// For compatibility

int AddDev(DCB *dcb);
int DelDev(const char *name);
extern void ListDev();

extern void EnterCriticalSection();
extern void ExitCriticalSection();

extern void _96_init();
extern void _96_remove();

extern void ChangeClearPAD(int mode);

#endif
