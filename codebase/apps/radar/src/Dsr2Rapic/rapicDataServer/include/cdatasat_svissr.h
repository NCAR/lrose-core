/*
 * cdatasat_svissr.h
 *
 * CData
 * 	CDataSat
 * 		CDataSatSvissr - Handle SVISSR (GMS) data
 *
 * Thread safe
 */

#ifndef __CDATA_SAT_SVISSR_H__
#define __CDATA_SAT_SVISSR_H__

// Standard include files
#include <time.h>

// Local include files
#include "svissr_constants.h"
#include "csatnav_svissr_sm.h"
#include "csatcal_svissr_dynamic.h"
#include "cdatasat.h"
#include "cblend.h"
#include "cdatasat_subset.h"

#define SVISSR_PACKED_LINE_LENGTH 38734

#define CRC_LENGTH 2
#define FILLER_LENGTH 256

#define DOC_SECTOR_LENGTH 2293
#define VIS_SECTOR_LENGTH_WORDS 9166
#define ALL_VIS_SECTORS_LENGTH ((CRC_LENGTH + FILLER_LENGTH)*4 + 27498)

#define CRC_LENGTH 2
#define FILLER_LENGTH 256

// Does not include sector ID, CRC or filler
#define SVISSR_DOC_SECTOR_LENGTH 2291

#define VIS_SECTOR_WORDS ((54996 + 16 + 2048) / 6)

enum svissrFileType {gft_detect, gft_raw, gft_asda, gft_cdata};
extern char  *svissrFileTypeSuffix[];
extern char  *gmsChannelStr[];
extern int	gft_dataofs[4];	// offset to actual svissr data in file

class CDataSatSvissr:public CDataSat {
friend class SatDataMng;
private:
	// Private data
//	int appendFildes;	    // replace with mmapFiledes
	unsigned char	*mmapAddress;
	unsigned char	*mmapSvissrAddress;
	svissrFileType	fileType;// type of file to save svissr data as
	svissrFileType	getFileType(char *filename);	// return the filetype from the name
	int		mmap_dataofs;		// svissr data offset for current file
	int		mmapFiledes;
	size_t		mmapLength;
	virtual void	makeFileName();	// should have start time first
	char		mmapFileName[256];
	int		linesStored;		// Number of lines appended
	int		maxElements(gmschannel ch);	// based on svissrMaxElements, but scaled * 4 for Vis channel
	int		maxLine(gmschannel ch);
	spinlock	*subsetListLock;	
	int		debug;
	int		CRCFailCount;		// count of successive CRC fail lines
	int		badCRCLines;		// count of bad CRC lines before valid nav data found
	bool		badCRCData;			// unable to get enough valid lines in entire data
	bool		allowBadCRCData;	// may use bad CRC Data

	// Incoming data
	unsigned char	incomingLine[SVISSR_PACKED_LINE_LENGTH];
	int		bytesReceived;

	int		LinesAdded;	// to be used to indicate change in this
					// typically will be cleared by satdatamng

	CSatNavSvissrSm	svissrnavigation;	// Navigation object
	CSatCalSvissrDynamic	svissrcalibration;	// Calibration object

	// Decoded data
	int		setLineNumber;		// GMS set line number from header
	int		firstLineNumber;	// scan count of first line read
	int		firstLineOffset;	// Offset of first line read from setLineOffset
//	time_t		imageStartTime;		// use CData::FirstTm,LastTm
//	time_t		imageEndTime;

	// Decomutated data
	int		decommutatedBlockCount;
	int		decommutatedBlocks[SVISSR_DECOMMUTATION_GROUPS];

	unsigned char	orbitAndAttitudeBlock[SVISSR_OANDA_BLOCK_LENGTH*
		SVISSR_DECOMMUTATION_GROUPS];
	unsigned char	manamBlock[SVISSR_MANAM_BLOCK_LENGTH*
		SVISSR_DECOMMUTATION_GROUPS];

	// Private functions
	int		bcdToI(unsigned char bcdNumber);
	int		bcd2ToI(unsigned char *bcdNumber);
	unsigned char *	lineStart(gmschannel channel, int line);
	int		lineNumber(unsigned char *linestart);
	double		getR4(unsigned char *ptr, int m);
	double		getR2(unsigned char *ptr, int m);
	int		appendLine(unsigned char *data);

	// memory optimisation stuff?
	void mUnmap();
	int mMap(int fildes = -1, long offset = 0);
	
public:
	// Public functions
	CDataSatSvissr(void);
	~CDataSatSvissr(void);
	void Close();
	
	int deleteFile();
	int AppendData(unsigned char *data, int length);
	int	writeMetaData(int fildes);
	int writeFile(char *filename, long offset = 0, 
			svissrFileType ftype = gft_raw);
	int writeFile(int fildes, long offset = 0, 
			svissrFileType ftype = gft_raw);
	int	readMetaData(int fildes);
	int getNavCalData();
	int readLine(unsigned char *buffer);
	int readLine(unsigned char *buffer, bool ignoreCRC);
	int readFile(char *filename, long offset = 0);
	int readFile(int fildes, long offset = 0);
	int	getMetaDataSize();
	// return true if file exists, else has been deleted and is not available
	bool canRemap;
	bool FileIsAvailable();	

/*
 * getSubset passed newObj will create the newObj if none exists.
 * If newObj does exist, it will be checked to see if its buffer can
 * be reused. 
 * If so,  newObj will be "reused"
 * Otherwise,  newObj will be deleted and a new one created.
 */
	int getSubsetLine(unsigned char *bufPtr, int bufWidth, 
		int element1, int element2, int line, int reduction,
		gmschannel channel);
	int getSubsetBlendedLine(unsigned char *bufPtr, int bufWidth, 
		int element1, int element2, int line, int reduction,
		gmschannel channel, CBlend *blendfn);
	int getSubsetRGBLine(unsigned char *bufPtr, int bufWidth, 
		int element1, int element2, int line, int reduction,
		gmschannel channel, CBlend *blendfn);
	int getSubset(CDataSatSubset **newObj,
		double lat1, double lon1, double lat2, double lon2, 
		int reduction, gmschannel channel, 
		CBlend *blendfn = 0);
	int getSubset(CDataSatSubset **newObj, 
		int element1, int line1,
		int element2, int line2, int reduction, 
		gmschannel channel, CBlend *blendfn = 0);
	int getSubset(unsigned char *buf,  
		int bufWidth, int bufHeight, 
		int element1, int line1,
		int element2, int line2,
		int reduction,
		gmschannel channel, 
		CBlend *blendfn = 0, 
		unsigned char *interruptFlag = 0, 
		float *resamplingProgress = 0);
	int getSubset(CDataSatSubset **newObj, SatSubsetParams *subsetdef);
	int getSubset(unsigned char *buf, SatSubsetParams *subsetdef);
// This probably is not used
// pjp: agreed, it's too tricky to make it a 2D array!!!
// only prospect may be lat1,lon1,width,height... but still of dubious value
	int getSubset(unsigned char *buf,
		double lat1, double lon1,
		double lat2, double lon2, int reduction,
		gmschannel channel, 
		CBlend *blendfn = 0,  
		unsigned char *interruptFlag = 0, 
		float *resamplingProgress = 0);

/* pjp
 * this one is slanted at my current OGlSatDispl + SatDataMng
 * way of doing business. Refer satdatamng.h & ogldisplsat.h
 * Note: ogldisplsat still contains a lot of stuff that
 * belongs in subset
 * 
 * This creates a CDataSatSubset as specified and attaches it below
 * this CDataSatSvissr in the CData linked list
 * It could search for matching ident in linked list.
 * If found,  the new subset should replace the existing subset
 */
	int	createChildSubset(SatSubsetParams *subsetdef);
	CDataSatSubset* getCDataSubsetbyIdent(int ident, bool incusercount = 1);
	int	reSampleChildSubset(SatSubsetParams *subsetdef, int ForceResample = 0);
	int	delSubsetbyIdent(int ident);
	void delAllSubsets();
	int	getValidSubsetCount();	// return the number of valid subsets
/*
 *	Not sure about this. At the moment leave it to satdatamng to manage 
 *	resampling because holds the subset list
	int checkSubsets(int maxresamples = -1); // check all subsets for resampling
 */

	int getDataSize(gmschannel channel, int *elements, int *lines);
	// return value at (svissr coord) elem/line
	int getVal(gmschannel channel, int elem, int line);

	// Calibration functions
	int getTemp(int element, int line, gmschannel channel, double *temp);
	int getTemp(unsigned char value, gmschannel channel, double *temp);

	// Navigation functions
	int convertNavigation(double *lat, double *lon,
		int element, int line);
	int convertNavigation(int *element, int *line,
		double lat, double lon);

	virtual bool ProjectXYtoLL(rpProjection proj, 
				float *Lat, float *Long, float *Height, 
			    float projX, float projY, float projZ);
	virtual bool ProjectLLtoXY(rpProjection proj, 
				float Lat, float Long, float Height, 
			    float *projX, float *projY, float *projZ);

	unsigned char *getManam(void);
	int getStartTime(time_t *startTime);
	LatLongtoMercator *mercatorConvert;	// mercator conversion class
	virtual void SetManNavOffsetsAllSubsets(int elem_ofs, int line_ofs);
	virtual void SetManNavOffsets(int elem_ofs, int line_ofs);
	virtual void ClearManNavOffsetsAllSubsets();
	virtual void ClearManNavOffsets();
	virtual bool	DataChanged();
	virtual void	SetDataChanged(bool state = TRUE);
};

#endif

