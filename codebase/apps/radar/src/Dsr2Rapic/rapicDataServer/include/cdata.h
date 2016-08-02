#ifndef __CDATA_H__
#define __CDATA_H__

// definition for generic data class

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "linkedlisttemplate.h"
#include "varlinkedlisttemplate.h"

#include "spinlock.h"
#include "expbuff.h"
#include <time.h>
#include "rdr.h"

enum eCDataType { eBaseCData, eRadarData, 
    eDataSat, eDataSatSvissr, eDataSatSubset };  

class CData : public CLinkedList<CData> {
//friend class CDataRdr;
private:
  exp_buff*		pDataBuff;	    // data buffer
	int		UserCount;						// count of how many users want this data
	int		lockwaitmax;
	bool		bDecdUserCountToZero;
protected:
	CData*		pThisData;
	CData*		pRootData;
	char		Creater[128];	    // string that can be filled in to identify creater
	char		Deleter[128];	    // string that can be filled in to identify destroyer
	char		ClassName[32];	    // string that can be filled in to identify derived class name
	int		nThisSetNo;
	bool		bDataComplete;      // true if all data present
	bool		bDataFinished;	    // true if no further additions
	bool		bDataValid;
	bool		bSubSetFinished;    // true if no further additions for this subset of data
	bool		bDebugLog;	    // if true create debug level log
	bool		bShownToClientsNew; // true if it has been passed through scan manager
	bool		bShownToClientsFin; // true if it has been passed through scan manager
	bool		bReleaseMe;	    // if true, all users should release this
	bool		bDataChanged;	    // true if this cdata or linked list child changed
	bool		bNeedsRender;	    // true if this cdata should be re-rendered, e.g. to image file
	time_t		releaseReqTime;	    // time releaseme flag was set
	char		label[256];	    // string with data type and short info e.g. datetime
public:
	time_t		FirstTm;
	time_t		LastTm;
	int		DataIndex;
	exp_buff_rd	dbuff_rd;	    // data buffer reader
	int		data_size;
	spinlock	*pCDataLock;	    // spinlock
	eCDataType	DataSetType;	    // nominal data type identifier
	// constructors
public:
	CData(int blocksize);
	CData();
	virtual	~CData();		    // must be virtual to correctly destroy child classes
	virtual void    Close();	    // clear buffers and initialise
	// methods
	virtual void	Rewind();
	virtual void	Flush();
	virtual void	Seek(int newpos);
	virtual void	CreateLock(int timeout = -1);
	virtual bool	Lock();
	virtual void	Unlock();
	virtual void	DelLock();
	virtual void	SetReadOnly();
	virtual void	CreateBuffer(int blocksize);
	virtual int	AppendData(unsigned char* buffer, int length);
//	virtual int	AppendLine(unsigned char* buffer, int length);
        // Save whole state to file
	virtual int	writeFile(int fildes, long offset);
	virtual int	writeFile(char *filename, long offset);
	virtual int	writeJpeg(char *filename, unsigned char *buffer, 
				  int width, int height);
	virtual int	writeGif(char *filename, unsigned char *buffer, 
			int width, int height);
        // Restore whole state from file
	virtual int	readFile(int fildes, long offset);
	virtual int	readFile(char *filename, long offset);
	virtual int	GetDataSize();
	virtual void	OpenReader(exp_buff_rd* pdbuff_rd = NULL);
	virtual	void	ResetReader(exp_buff_rd* pdbuff_rd = NULL, int pos = 0); // was reset_radl
	bool		ShouldDelete(char *deleter = 0);
	void		IncUserCount(char *user = 0);
	int		GetUserCount();
	bool		HasOSLock();
	time_t		FirstTime();
	time_t		LastTime();
	virtual	bool	IsDataComplete();	// true if all data sets present
	virtual	bool	IsDataFinished();	// true if no further additions
	virtual bool	IsDataValid();
	virtual bool	IsShownToClientsNew();
	virtual bool	IsShownToClientsFin();
	virtual bool	IsSubSetFinished();
	virtual bool	IsReleaseMe();
	virtual void	SetDataFinished(bool state = TRUE);
	virtual void	SetDataValid(bool state = TRUE);
	virtual void	SetDataComplete(bool state = TRUE);
	virtual void	SetSubSetFinished(bool state = TRUE);
	virtual void	SetShownToClientsNew(bool state = TRUE);
	virtual void	SetShownToClientsFin(bool state = TRUE);
	virtual void	SetReleaseMe(bool state = TRUE);
	virtual void	SetNeedsRender(bool state = true);
	// contained data linked list traversing routines 
	virtual CData*	ResetDataSet();
	virtual	CData*	ThisDataSet();
	virtual CData*	NextDataSet();
	virtual CData*	PrevDataSet();
	virtual CData*	GotoDataSet(int n);
	virtual bool	DataChanged();
	virtual void	SetDataChanged(bool state = TRUE);

	void SetDeleter(char *deleter = 0);
	void SetCreater(char *creater = 0);
	virtual char *	Label();		// return label string
};

#endif
