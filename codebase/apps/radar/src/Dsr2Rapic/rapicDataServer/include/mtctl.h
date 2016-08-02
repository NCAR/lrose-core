#ifndef __MT_H
#define __MT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/invent.h>
#include <sys/stat.h>
#include <fcntl.h>

//typedef unsigned char bool; SD 21/12/99
typedef int bool;


class mt {
public:
	int	fd;									// tape device file descriptor
	char	mtname[128];			// mt device name
	int	error;				// operation error code
	FILE	*ErrFile;			// file handle to output error messages to
	time_t	elapsedtime;			// time to execute last rd/wr op
	mtop	op;				// general purpose variable
	mtget	status;				// derived from MTIOCGET ioctl call
	mtblkinfo blkinfo;			// derived from MTIOCGETBLKINFO ioctl call
	daddr_t wrt_blkno;			// block address of last written file
	int	maxblksz;			// max block size to use for rd/writes
	bool timeop;				// switch for mtiocptop timer
	mt_capablity capability;// derived from MTCAPABILITY ioctl call
	mt(char *name = 0);			// no param will default to /dev/nrtape
	~mt();					// closes fd	
	int	Open(char *name = 0);		// open given mt device
	void	Close();			// close current fd
	void	print_error(char *caller = 0, char *errstr = 0,FILE *errfile = 0);	// output error string to stderr (by default)
	// the following commands return error no, 0=OK
	int	mtiocptop(short op, ulong count, char *caller);	// used by followinf methods
	int	weof();				// write an end of file record
	int	fsf(int count);			// forward space file
	int	bsf(int count = 1);			// backward space file
	int	fsr(int count);			// forward space record
	int	bsr(int count);			// backward space record
	int	feom();				// space to end of recorded data
	int	rewind();			// rewind
	int	offline();			// rewind and put tape offline
	int	nop();				// no operation, set status only
	int	retension();			// retension operation
	int	reset();			// retension operation
	int	appendfile();			// space to end of last file BEFORE FM, next data written is appended to last file
	int	seeklastfile();			// space to end of last file BEFORE FM, next data written is appended to last file
	int	erase();			// erase tape from current pos to EOT
	int	unload();			// unload tape from drive
	int	reopen();			// close and open existing name device
	// only supported on SGI DAT. One partition max. Size=0 removes part. 
	int	setpart(int part);		// skip to given partition
	int	mkpart(int size);		// create partition of given size (megabytes)
	int	skipsetmark(int count);		// skip count setmarks,(may be -ve) DAT only
	int	wrtsetmarks(int count);		// write count setmarks (DAT only)
	int	audiomode(bool flag);	// set/clear audio mode
	int	seekblock(daddr_t blockno);	// seek to blk (not supported on all drvs) IF NOT SUPPORTED, WILL REWIND THEN fsr TO GIVEN BLOCK(RECORD)
	int	seekfile(int fileno);		// WILL REWIND THEN fsf TO GIVEN FILE
	int	getstatus();			// get mt status, modifies status
	int	getblkinfo();			// get block blkinfo
	ulong	getblksize();			// get block size
	ulong	currentblksize();		// return current block size
	daddr_t currentblkno();			// current blk no.
	int	setblksize(ulong blksz);	// set fixed block size
	int	getcapability();		// get dev capability, modifies capability
	void	printcapability(FILE *printfile = stdout);
	void	printstatus(FILE *printfile = stdout);
	void	setmaxblksz(int mxblksz) {
		maxblksz = mxblksz;
		}
	int	set_ansi(bool flag);		// set ansi mode if flag true
	bool can_seek();			// true if seek capability
	bool can_setmark();			// return if able to set mark
	bool can_bsf();			// true if bsf capability
	bool can_bsr();			// true if bsr capability
	bool can_part();			// true if partition capability
	bool can_locktape();			// true if lock tape media capability
	bool can_setsz();			// true if set block size capability
	bool can_var();			// true if variable block size capability
	bool can_compress();			// true if compression capability
	bool is_online();			// true if unit is online (tape present)
	bool is_dat();			// true if dat tape drive
	bool at_eod();			// true if tape is at end of data
	bool at_fmk();			// true if tape is at file mark
	bool at_eot();			// true if tape is at end of tape (media)
	bool at_bot();			// true if tape is at beginning of tape (media)
	int	WrtFile(char *SrcName,bool markeof = 1); // copy file SrcName to tape at current pos. If markeof, write an eof after file written
	int	AppendFile(char *SrcName, bool markeof = 1);// copy file SrcName to tape at EOM, ie after current last file
	int	AppendToLastFile(char *SrcName, bool markeof = 1);// copy file SrcName to tape at end of current last file
	int	OverWriteLastFile(char *SrcName, bool markeof = 1);// copy file SrcName over current last file
	int	RdFile(char *DestName, int size = -1);	// read size bytes from current pos on tape to file, if size = -1, read until file mark 
	int	WrtBuff(char *buf,int count,bool markeof = 1, int Padize = 0);// copy buffer to tape at current pos. If markeof, write an eof after buffer written
	int	AppendBuff(char *buf,int count,bool markeof = 1, int Padize = 0);// copy buffer to tape at EOM. If markeof, write an eof after buffer written
	int	AppendBuffToLastFile(char *buf,int count, bool markeof = 1, int Padize = 0);// copy file SrcName to tape at end of current last file
	int	RdBuff(char *buff, int count); // read count bytes from current pos on tape to buff, or until file mark
	};

#endif // __MT_H
