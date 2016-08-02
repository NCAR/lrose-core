/*
 * cdatasat_subset.h
 *
 * CDataSatSubset class
 *
 * There should be no satellite specific things in here
 *
 */

#ifndef __CDATA_SAT_SUBSET_H__
#define __CDATA_SAT_SUBSET_H__

// Local include files
#include "rdrutils.h"
#include "csatnav.h"
#include "csatcal.h"
#include "cdatasat.h"
// #include "csatnav_grid.h"
#include "textureProj.h"
#include "csatnav_svissr_sm.h"
#include "csatcal_svissr_dynamic.h"
#include "svissr_constants.h"

class CDataSatSvissr;

class SatSubsetParams : public CLinkedList<SatSubsetParams> {
public:    
	char	*label;
    int		ident;				// value of 0 means not valid ident, > 0 is registered through satdatamng
    gmschannel	channel;
    int		width, height,	    // dimensions of this array 
		x_ofs, y_ofs,	    // offsets from parent image origin
		x_reduc, y_reduc;   // data reduction values
    CBlend	*BlendFn;			// blend function to apply
    // refNavSatProjGrid is a pointer to a grid created and destroyed
    // by ogldisplsat
    projGrid	*refNavSatProjGrid;  // reference navigated sat proj grid
    CSatNavSvissrSm *refSvissrNavigation;	// pointer to nav
    bool	deleteRequested;    // if true, this subset is no longer required
	bool	autoLocalJPEGReqd;	// true if this should be written out to Local JPEG
	bool	autoWebJPEGReqd;	// true if this should be written out to Web JPEG
				    // sat data mng should remove it and all related subsets

    
    SatSubsetParams(SatSubsetParams *newsubset = NULL, bool assignIdent = false);
	virtual ~SatSubsetParams();
    char *Label();
    // setParams passed values of -1 indicate no change
    void    setParams(
		int Ident, 
		int Width, int Height,	    // dimensions of this array 
		int X_ofs, int Y_ofs, 
		int X_reduc, int Y_reduc, 
		gmschannel Channel, CBlend	*blendfn = 0, 
		projGrid *refnavsatprojgrid = 0, 
		CSatNavSvissrSm	*refsvissrnavigation = 0);
    void	setParams(SatSubsetParams *newsubset);
    bool	IsSame(SatSubsetParams *newsubset);
    void	setRefSatProjGrid(projGrid *refnavsatprojgrid, 
		    CSatNavSvissrSm *refsvissrnavigation);
    void	clearRefSatProjGrid();
    void	writeFile(int fildes);
    void	readFile(int fildes);
};

class CDataSatSubset : public CDataSat {
    friend	class	OGlDisplSat;	// Andrew, I'm in a hurry, should be fixed later
    friend	class	CDataSatSvissr;	// Andrew, I'm in a hurry, should be fixed later
    friend	class	SatDataMng;		// This should be OK.
private:
	// Meta data
	enum {NOT_ALLOCATED, DYNAMIC, MMAP} bufferStorage;

	// Data
	CSatNavSvissrSm			svissrnavigation;	// Appropriate subclass will be used
	CSatCalSvissrDynamic		svissrcalibration;	// Appropriate subclass for calibration
	unsigned char	*buf;		// Pixel buffer
	SatSubsetParams SubsetParams;	
	
/*
 * navigation and projected grids are typically calculated with
 * respect to the subset's coordinates,  ie not full svissr coords
 */
	projGrid	*navGrid;	// reference lat/long Grid for this subset 
	projGrid	*satProjGrid;	// satellite proj Grid for this subset 
	projGrid	*projectedGrid;	// proj grid for other than satellite projections
	projGrid	*currProjGrid;	// pointer to selected projection grid
	bool		reSampling;	// set true whilst resampling
	bool		needsReSampling;// set true to notify satdatamngr this needs resampling 
	bool		couldntResample;// set if svissr file been deleted and could resample, may continue to use this subset.
	SatSubsetParams *resampleSubsetParams;	
	float		resamplingProgress; // fraction of resample complete
	bool		navGridValid;	// if not valid nav and proj grids need to be recalculated
	bool		isNewSubset;	// set to true after subset has been created
	bool		needsRedraw;	// if true, window shoudl redraw this
	void		setNeedsRedraw(bool state = true);	// if true, window shoudl redraw this
	void		SetNeedsRender(bool state = true);
	// isNewSubset may be used in conjunction with DataValid to
	// identify new subsets that haven't been used yet
	// e.g. for auto jpeg generation
	rpProjection	currProj;	// current  projection
	void		init();
public:
	CDataSatSubset(CSatNavSvissrSm *Navigation, CSatCalSvissrDynamic *Calibration,
		unsigned char *Data, SatSubsetParams *subsetdef);
	CDataSatSubset(CSatNavSvissrSm *Navigation, CSatCalSvissrDynamic *Calibration,
		unsigned char *data, 
		int inputWidth, int inputHeight,
		int XOffset, int YOffset,
		int Reduction, gmschannel Channel, 
		CBlend	*blendfn = 0);
	~CDataSatSubset();
	char		*Label();
	// NewBuffer will delete the existing buf if exists & it is the wrong size
	// it will then allocate a new buf, and set the width and height variables
	void NewBuffer(int newWidth, int newHeight, gmschannel newCh);
	void		setParams(CSatNavSvissrSm *Navigation, CSatCalSvissrDynamic *Calibration,
			    int inputWidth, int inputHeight,
			    int XOffset, int YOffset,
			    int Reduction, gmschannel Channel, 
				CBlend	*blendfn = 0);
	void		setParams(CSatNavSvissrSm *Navigation, CSatCalSvissrDynamic *Calibration,
			    SatSubsetParams *subsetdef);
	    
	CDataSatSvissr	*ParentSvissr;
//	int AppendData(unsigned char *data);
	int writeFile(int fildes, long offset = 0);
	int readFile(int fildes, long offset = 0);
	void setSubsetParams(SatSubsetParams *newSetParams);
	SatSubsetParams* getSubsetParams();
	int getData(CBlend *blend,
		double lat1, double lon1, double lat2, double lon2,
		int reduction);
	int getData(int element1, int line1, int element2, int line2,
		int reduction);

	// return value at (svissr coord) elem/line
	int getVal(int elem, int line);
	// return value at (subset coord) elem/line
	int getValSubset(int elem, int line);

	int getDataSize(int *elements, int *lines);

	int getSvissrValue(unsigned char *val, int sv_elem, int sv_line);

	int getSvissrValTemp(int sv_elem, int sv_line, 
		unsigned char *val, double *temp);
	int getSvissrTemp(unsigned char val, double *temp, int sensor = 1);

	
	void subsetXYtoSvissrXY(int x, int y, 
			    int *sv_elem, int *sv_line);
	void subsetXYtoSvissrXY(int *sv_elem, int *sv_line);
	void svissrXYtoSubsetXY(int *x, int *y, 
			    int sv_elem, int sv_line);
	void svissrXYtoSubsetXY(int *x, int *y);
	int convertSubsetNavigation(double *lat, double *lon,
				int element, int line);
	int convertSvissrNavigation(double *lat, double *lon,
				int element, int line);
	int convertSubsetNavigation(int *element, int *line,
				double lat, double lon);
	int convertSvissrNavigation(int *element, int *line,
		double lat, double lon);
	bool		ProjectLLtoXY(rpProjection proj, 
				float Lat, float Long, float Height, 
			    float *projX, float *projY, float *projZ);
	bool		ProjectXYtoLL(rpProjection proj, 
				float *Lat, float *Long, float *Height, 
			    float projX, float projY, float projZ);
	// make a simple (unnavigated) satProjGrid for rawbuffer
	void		CreateSimpleSatProjGrid();
	// make a navigated satProjGrid for rawbuffer, using ref grid if provided
	// otherwise creating the ref grid
	void		CreateNavSatProjGrid(); 
	void		CreateProjGrid(rpProjection proj = projUndefined);
	void		CreateNavigationGrid(  // make navigationGrid for rawbuffer
					float minLat = -60.0, float maxLat = 60.0, 
					float minLong = 80.0, float maxLong = 200.0, 
					float latRes = 5.0, float longRes = 5.0);   // make latlongProjGrid for rawbuffer
	void		setRefSatProjGrid(projGrid *refnavsatprojgrid, 
			    CSatNavSvissrSm *refsvissrnavigation);
	/*
	 * convert subset relative grid to svissr relative grid
	 */
//	void projGridSubsetXYtoSvissrXY(projGrid *navGrid);

	LatLongtoMercator *mercatorConvert;	// mercator conversion class
	virtual int	buffSize();			// return size of buf
	bool		IsDataValid();	// if data valid and not resampling
	bool		IsResampling();	// if subset being resampled
	bool		IsNewSubset();	// if subset is "new"
	bool		NeedsResampling();	// if subset needs resampling
	float		ResamplingProgress();	// fraction of resample complete
	int			PercentResamplingProgress();	// fraction of resample complete
	bool		IsNavGridValid();
	void		SetResampling(bool state = TRUE);
	void		SetNeedsResampling(bool state = TRUE);
	void		SetNeedsResampling(SatSubsetParams *resampleSubsetParams);
	void		SetNavGridValid(bool state = TRUE);
	void		SetIsNewSubset(bool state = TRUE);
	void		SetProjGrid(rpProjection proj); 
	void		SetCouldntResample(bool state = TRUE); 
	bool		CouldntResample(); 
	virtual void	SetDataValid(bool state = TRUE);
	void		recalcAllGrids();
	projGrid*	getNavGrid();		// returns navGrid
	projGrid*	getSatProjGrid();	// return satprojection grid
	projGrid*	getProjGrid();		// return current projection grid
	projGrid*	getProjGrid(rpProjection proj);	// recalculate if necessary and return specified grid
	projGrid*	getCurrentGrid();	// return currentGrid
	char*		channelString();	
	void		getHistogram(int element1 = 0, int line1 = 0,
					int element2 = 0, int line2 = 0, 
					gmschannel channel = gms_ch_undef);
	virtual void	SetManNavOffsets(int elem_ofs, int line_ofs);
	virtual void	ClearManNavOffsets();
	virtual bool	DataChanged();
	virtual void	SetDataChanged(bool state = TRUE);
	// returns an array containing the values along the
	// line btwn the specified points (svissr line/elem points)
	virtual int	getSectionArray(int elem1, int line1, 
				int elem2, int line2, 
				unsigned char *outbuf, 
				int bufsize);
	virtual int	getSectionArray(double lat1, double lon1, 
				double lat2, double lon2, 
				unsigned char *outbuf, 
				int bufsize);
};

#endif

