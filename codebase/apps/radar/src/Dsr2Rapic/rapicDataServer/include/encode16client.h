// scan client to manage encoding of raw data into 16 level ASCII data

#include "CThreshold.h"

class CEncode16Client : public scan_client {
public:
	CEncode16Client(/*CWnd *pOwner*/);
	virtual ~CEncode16Client();
protected:
//	rdr_scan_node* last;
//	rdr_scan_node* first;
	CSpinLock	*pLock;
	UCHAR			bThreadExit;			// use UCHAR to guarantee atomic control
	UCHAR			bThreadActive;		// use UCHAR to guarantee atomic control
//	CWnd*			m_pOwnerWnd;
	CThreshold* m_pdBzThreshold;
	CThreshold* m_pVelThreshold;
protected:
	BOOL FilterScan(rdr_scan *scan);
	void  ConvertScan(rdr_scan *pScan);
	void  EncodeScan(rdr_scan *pScan, rdr_scan *pNewScan, BOOL AzScan=TRUE);
	BOOL	CreateHeader(rdr_scan *pScan, rdr_scan *pNewScan);
	rdr_scan*  SpawnScan(rdr_scan *pScan);
	void	TruncateData(s_radl *pRadl);
public:
	rdr_scan* m_pCurrentScan;
	rdr_scan* m_pConvertedScan[2];  // double buffer resultant conversion.
	int			m_nCurrentConversion;
public:
	virtual int NewScanAvail(rdr_scan *scan);
	virtual bool Full();
	virtual void CheckNewScans();
	virtual UINT ThreadEntry(void *);
};

// stepping stone function used to
UINT LaunchEncode16Thread(void *Handler);																// launch the actual conversion thread


