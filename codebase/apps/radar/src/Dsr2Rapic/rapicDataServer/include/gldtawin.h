#ifndef __CMAP_H
#define __CMAP_H

#include "displ.h"
#include "siteinfo.h"
#include "palette.h"

/*

	GlDataWins.h
	
*/

extern long CMapWin;
extern long CGraphWin;
extern long HtMapWin;

void UpDateCGraph();

void ShowCGraph();

void CloseCGraph();


void UpDateHtMap();

void ShowHtMap();

void SetHtMapInd(float IndVal);

void CloseHtMap();

class GlImgData {
public:
	long   ImgDataW;
	long   ImgDataH;
	long    ImgDataX;
	long    ImgDataY;
	long   ImgDataMinX;
	long   ImgDataMinY;
	long  ImgDataWin;
	float 	ClkTbl[60][2];          // X/Y Values for each minute (-100 to 100)
	GlImgData(WinXYWH *init = 0);
	virtual ~GlImgData();
	void UpDate(rdr_img* this_seq = 0);
	virtual void popwin();
	void DrawClk(int hr, int min);
	void GetPos();
	void GetState(WinXYWH *init);		// load init with current window state
	};

#endif	/* __CMAP_H */
