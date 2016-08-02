/*

	arch.h

	Header file for archive functions
	
	Archives may be either a Tape or File Type
	Tape type are a series of Rapic DB tars, with a directory tar as
	the last tar file on the tape.
	File types are a directory which contains the archive db files
	(not tarred) and the archive directory RAPIC_ARCH.DIR

	Rapic archive tapes will be written as a series of tar files.
	
	The directory file is stored as the last tar file on the tape.
	Tapes will be a series of Rapic database tars with the last file on 
	tape being the directory file 
	RAPIC_ARCH.DIR
	NOTE: On CD Recordable (Write once) media there will be a 
	series of RAPIC_ARCH.DIR.00001 RAPIC_ARCH.DIR.0002 etc files
	The largest numbered file will be the authoritative directory.

	The directory file will contain an identifying string 
	(3D-RAPIC ARCHIVE DIRECTORY) followed by a header as specified below.
	
	To allow convenient modification of individual header entries (e.g. DB Count)
	each header entry will be a 64 char array padded with space chars, \n terminated.
	The comments field will be limited to 1024 bytes.
	The header must be present in the following order.
	Header fields will include:

	Directory title -   3D-RAPIC ARCHIVE DIRECTORY
	Volume Name	-   Created: 12/11/1993 12:30
			    On Host: hostname
	*Last data date	-   Last Updated: 13/11/1993 14:50 
	*Last dir name	-   Last Directory: RAPIC_ARCH.DIR.00002
	*DB Count	-   DB Count: 123
	Archive media sz-   Media Size: 2000000000
	Archive size	-   Archive Size: 1234566789
	Optional Text  	-   Comments: optional text data
	Padding		-   Pad out to 16 64 char lines of header

	Database entry records will have the following format:
	
	[DBENTRY]
	DBName: 00029309070742  12/11/1993 12:30 14/11/1993 14:50
	DBSize: 50002345
	BlockNo: 4567
	Radar: 0002 12/11/1993 12:30 14/11/1993 14:50
	.
	.
	.
	Radar: 0055 13/11/1993 10:30 14/11/1993 16:50
	TEXT: optional text follows, null terminated
	[ENDDBENTRY]



*/

#include "mtctl.h"
#include "rpdb.h"

/*
 * Support both tape type archives and file system based archives
 * 
 * Tape type contains a series of tarred db files with the directory
 * kept as the last tar on the tape. 
 * The directory is overwritten by new db tar files and the new directory
 * appended after the new db archive.
 * The directory file is updated after the dbtar file has been 
 * successfully written.
 * If the tape fills up while writing a db tar file the directory,  the 
 * previous directory is written back in place of the incomplete dn tar, 
 * and the user is prompted for a new tape.
 * 
 * File System type (e.g. removable file system type media) again 
 * maintains a directory corresponding to the dbs present (not tarred)
 * on that pathname.
 * If the directory is not found,  assume new media and start a new 
 * directory.
 * The EFileCDR is treated much like a EFile except that multiple
 * directories will be present, as the directory cannot be updated.
 * i.e. RAPIC_ARCH.DIR.00001 RAPIC_ARCH.DIR.00002
 */

enum EArchType = {ETape, EFile,EFileCDR};	

class archive {

public:
	archive(char *pathname = 0, ArchType archtype = ETape, double archsize = 0);
	~archive();
	bool	Open(char *pathname = 0, ArchType archtype = ETape, double archsize = 0);
	bool	ReOpen();
	void	Close();    
	bool		IsOpen;
    	mt		*MT;			// MT device for archive
	char		ArchiveDevice[256];	// name of file type archive path
	EArchType	ArchType;		// type of this archive
	bool		ArchiveValid;		// Valid archive flag
	char		VolName[256];		// Name of this tape volume
	double		ArchSize;		// Size of archive media
	double		ArchUsed;		// Archive space used
	int		DBCount;		// Number of Rapic Dbs on archive
	bool		LoadArchive();		// check tape layout, load directory
	void 		UnloadArchive();	// Unload tape
	// make new archive, checks for existing valid archive, only overwrite 
	// valid archive if flag set, make new directory
	bool		CreateArchive(bool OverWrite = FALSE);		
	// append Rapic DB to archive, and modify directory
	bool		AddDB(rp_isam *adddb);	
	
	// fetch Rapic DB from archive
	bool		GetDB(char *DBName,char *DestPath = 0);	
	bool		GetDB(int dbpos,char *DestPath = 0);	
	void		WriteDirToTape();	// write directory file to tape
	bool		ReadDirFromTape();	// read directory file from tape
	void		MakeNewDir();		// create new directory file
	void		PrintDir(int fd = stdout);	// print directory to fd
	}



