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

struct EXEC {
	unsigned int pc0;
	unsigned int gp0;
	unsigned int t_addr;
	unsigned int t_size;
	unsigned int d_addr;
	unsigned int d_size;
	unsigned int b_addr;
	unsigned int b_size;
	unsigned int s_addr;
	unsigned int s_size;
	unsigned int sp,fp,rp,ret,base;
};

// Not recommended to use these functions to install IRQ handlers

typedef struct {
	unsigned int* next;
	unsigned int* func2;
	unsigned int* func1;
	unsigned int pad;
} INT_RP;

#ifdef __cplusplus
extern "C" {
#endif

void SysEnqIntRP(int pri, INT_RP *rp);
void SysDeqIntRP(int pri, INT_RP *rp);

// Event handler stuff

int OpenEvent(unsigned int cl, int spec, int mode, void (*func)());
int CloseEvent(int ev_desc);
int EnableEvent(int ev_desc);
int DisableEvent(int ev_desc);

// BIOS file functions

int open(const char *name, int mode);
int close(int fd);
int seek(int fd, unsigned int offset, int mode);
int read(int fd, char *buff, unsigned int len);
int write(int fd, const char *buff, unsigned int len);
int ioctl(int fd, int cmd, int arg);
struct DIRENTRY *firstfile(const char *wildcard, struct DIRENTRY *entry);
struct DIRENTRY *nextfile(struct DIRENTRY *entry);
int erase(const char *name);
int chdir(const char *path);

#define delete( p )	erase( p )
#define cd( p )		chdir( p )			// For compatibility

int AddDev(DCB *dcb);
int DelDev(const char *name);
void ListDev(void);

void EnterCriticalSection(void);
void ExitCriticalSection(void);

void _InitCd(void);
void _96_init(void);
void _96_remove(void);

// BIOS pad functions
void _InitPad(char *buff1, int len1, char *buff2, int len2);
void _StartPad(void);
void _StopPad(void);

void ChangeClearPAD(int mode);
void ChangeClearRCnt(int t, int m);

// Executable functions
int Exec(struct EXEC *exec, int argc, char *argv);

#ifdef __cplusplus
}
#endif

#endif
