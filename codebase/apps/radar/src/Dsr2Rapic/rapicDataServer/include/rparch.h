r/*

	rparch.h	- header file for rparch class

*/

#ifndef __RPARCH_H
#define __RPARCH_H

#include <time.h>

//typedef unsigned char bool; SD 21/12/99
typedef int bool;

enum archstring_val 
	{AS_HDTITLE,AS_HDSZ,AS_DBENTSZ,AS_CREATEDT,AS_CREATEHST,AS_CREATEDEV,
	 AS_BLKSZ,AS_LASTUPD,AS_LASTDBBLK,AS_LASTDBSTAT,AS_APPENDBLK,
	 AS_DBCNT,AS_BLKCNT,AS_TOTSZ,AS_TAPESZ,
	 AS_ENTTITLE,AS_DBNM,AS_FRSTENT,AS_LASTENT,AS_DATAFL,AS_IDXFL,
	 AS_DATFL,AS_STATUS,AS_COMMENTS,AS_COUNT,AS_RDR,
	 AS_COMPLETE,AS_WORKING,AS_UNDEF};

extern char *archstrings[];

#define archhd_spares 15
#define archhd_size 2048  

class archheaderrec {
public:
	char titlestr[64];				// title should be: 3D-RAPIC ARCHIVE DIRECTORY - VERSION x.x
	char header_sizestr[64];	// size of this header in bytes: HEADER SIZE:2048
	char direntry_sizestr[64];	// size of directory entries in bytes: DB ENTRY SIZE:2048
	char create_datestr[64];	// date this archive created: CREATED DATE:dd/mm/yy hh:mn
	char create_hoststr[128]; // name of host archive created by: CREATED HOST:hostname
	char create_devicestr[128];// device archive created on: CREATED DEV:/dev/nrtape
	char block_sizestr[64]; 	// max block size to be used on this tape: BLOCK SIZE:16384
	char last_updatestr[64]; // date of last update to aarchive: LAST UPDATE:dd/mm/yy hh:mn
	char lastdb_blkstr[64];	// block position of last database on this archive: LASTDB BLK:19345
	char lastdb_statusstr[64]; // status of last db Working/Complete: LASTDB STATUS:Working/Complete
	char append_blkstr[64];	// blk pos of append posistion (same as eof): APPEND BLK:nnnnnnn
	char db_countstr[64];		// number of dbs on this archive: DB COUNT:10
	char total_blksstr[64];		// total no of blocks used: BLOCK COUNT:12345
	char total_sizestr[64];	// total size (in kbytes): TOTAL SIZE:123456
  char tape_sizestr[64];	// total tape size in kB: TAPE SIZE:1234/UNDEFINED
	char sparestr[archhd_spares][64];	// spare space, initialised to 0;
	archheaderrec();
	bool is_valid();			// returns true if title OK
	bool is_same(archheaderrec *header); // return true if other header matches this one's create date and host
	char *PrefixOK(char *Prefix,char *matchstr);	// check that prefix string is OK, return pointer to data or 0 if not OK
	int rd_headerrec(int fd);	// read headerrec from file
	int wrt_headerrec(int fd); // write headerrec to file
  void Init();
	};

class archheaderdata {
public:
	char title[48];				// 
	unsigned long header_size;	// size of header in bytes
	unsigned long direntry_size;// size of directory entries in bytes
	time_t create_date;					// date this archive created
	char create_host[128];			// name of host archive created by
	char create_device[128];		// device archive created on
	unsigned long block_size;		// max block size to be used on this tape
	time_t last_update;					// date of last update to aarchive
	unsigned long lastdb_blk;		// block position of last database on this archive
	bool lastdb_working; 		// status of last db, 1=working 0=complete
  unsigned long append_blk;		// block pos to append
	unsigned long db_count;			// number of dbs on this archive
	unsigned long total_blks;		// total no of blocks used
	unsigned long total_size;		// total size (in kbytes)
	unsigned long tape_size;		// size of tape in kB, 0=undefined
	archheaderdata();
	bool isvalid;						// true if valid
	bool is_valid() {
		return isvalid;
		}													// function to return isvalid value
	bool read_archheaderrec(archheaderrec *headerrec); // read data from archheaderrec
	bool write_archheaderrec(archheaderrec *headerrec); // write data to archheaderrec
	bool is_same(archheaderdata *headerdata);
	void Init();			// initialise class
	};


class archheader {
public:
	archheaderdata headerdata;
	bool ReadHeader(int fd);		// read header from given fd, return TRUE if read OK, ie header is valid
	int WriteHeader(int fd);			// write header to given fd
	bool is_valid();						// true if valid header
	void init();										// generate new header
	};

#define dirent_maxrdrs 71
class direntryrec {
public:
	char titlestr[32];		// directory entry title: 3D-RAPIC ARCHIVE ENTRY
	char namestr[64];			// database name string: DB NAME:00029309070742
	char firststr[32];				// first stn/date/time in db: FIRST:00002 12/11/93 12:30
	char laststr[32];				// last stn/date/time in db: LAST:0002 14/11/93 14:50
	char datafilestr[48];		// db data file size/start blk/no of blks: DATA FILE:50002345 20 213
	char idxfilestr[48];			// ISAM IDX file size/start blk/no of blks: IDX FILE:163840 233 10
	char datfilestr[48];			// ISAM DAT file size/start blk/no of blks: DAT FILE118784 243 7
	char statusstr[32];			// status of db (complete/working) only relevant for last db in archive: STATUS:COMPLETE
	char commentsstr[256];		// comment field: COMMENTS:Optional text data
	char countstr[20];			// number of radars in db: COUNT:12
	char rdrsstr[dirent_maxrdrs][20];		// array of radar present in db stnid, first&last date: RDR:0010931211931219
	bool is_valid();			// returns true if title OK
	int read_direntryrec(int fd);	// read headerrec from file
	int write_direntryrec(int fd);	// read headerrec from file
	};

struct stnfirstlast {
	int stnid;
	time_t first;
	time_t last;
	};

class direntrydata {
public:
	char title[32];
	char name[64];
	int firststn;
	time_t firstdate;
	int laststn;
	time_t lastdate;
	unsigned long datasize;	// size of data file;
	unsigned long datastart;// start block of data file
	unsigned long datablks;	// no of blocks in data file
	unsigned long idxsize;	// size of isam idx file;
	unsigned long idxstart;	// start block of isam idx file
	unsigned long idxblks;	// no of blocks in isam idx file
	unsigned long datsize;	// size of isam dat file;
	unsigned long datstart;	// start block of isam dat file
	unsigned long datblks;	// no of blocks in isam dat file
	int status;							// status of this db, 1=working 0=complete
	char commentsstr[256];	// comment field: Optional text data
	int	count;							// no. of radars in db
	stnfirstlast thisrdrentry;	// this radar entry
	int thispos;						// pos of this entry, start = 0;
	stnfirstlast *FirstRdr();		// return first entry in the radars list, also sets rdrentry, NULL = no entry  
	stnfirstlast *NextRdr();		// return next entry in the radars list, also sets rdrentry, NULL = no entry  
	void init();
	};

class direntry {
public:
	direntrydata DirEntry;
	bool ReadEntry(int fd);
	int WriteEntry(int fd);
	bool is_valid();
	void init();
	};

class directory {
public:
	int fd;				// fd of directory
	archheader header;
	direntry this_entry;
	bool Open(char *dirname);
	bool FirstEntry();
	bool NextEntry();
	bool GotoEntry(int entryno);
	};


#endif // __RPARCH_H
