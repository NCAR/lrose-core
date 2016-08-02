#ifndef __UIFTOVRC_H__
#define __UIFTOVRC_H__
/*

uiftovrc.h

Ensures straight C type un-mangled names are generated for
interface from UIMX C code to vrc C++ code.

*/

#ifdef __cplusplus
extern "C" {
#endif

#include "vrc.h"
#include <time.h>

  struct DBBrowseVars {
    int SelStn[2];
    time_t SelDate[2],SelTime[2];
    int		SelNo;
    short StnList[256];
    int   StnEntries;
  };

  struct ConnReqVars {
    int		stn;
    int		scan_type;
    int		angle;
    int		count;
    int		query_type;
    time_t query_time;
  };


#ifdef __cplusplus
  // if values are added here the relevant labels in rdrglb.C need to be 
  // updated to match 
  enum cursorvaltype {cv_undef, cv_refl, cv_vel, cv_spectw, 
		      cv_diffz, cv_ht, cv_vil, cv_accum, cv_sat_ir, 
		      cv_sat_vis, cv_sat_wv, cv_sat_blend,
                      cv_rf_rainrate, cv_rf_rainaccum };
#else
#define  cursorvaltype int
#endif

  struct CursorDataStruct {   /* units of km EXCEPT HEIGHT IN m */
    /* for dBZ, val1=dBZ, val2=R.... */
    float	value1, value2, lat, lng, ht, 
      rng, brg, kmn, kme;
    int         value1_idx;
    int         val1_defined, val2_defined, refval_defined; // false if no echo or no value
    float	reflat, reflong;/* ref pt. lat/long */
    float	refrng, refbrg, refkmn, refkme, 
                vel;	/* fromref time & vel */
    float       refvalue;       /* value at reference point */
    int	        dtime;		/* fromref delta time */
    time_t      time;
    int	        valmode;	/* 0-dbz, 1-vel, 2-vil */
    int	        distmode;	/* 0-fromradar, 1-fromref */
    char	ScanData[128];	/* scan details */
    cursorvaltype valtype;
    unsigned char mouse1Down, mouse2Down, mouse3Down;
    unsigned char ctlKey, altKey, shiftKey;
    int	RefPointDefined;   
  };

  struct RdrStCtlStruct {
    char    WidCreated;		/* set by widget when widget ready */
    char    StatusOnly;		/* true if RadarStatus widget only */
    /* TO WID */
    char    RdrNameStr[64];	/* TO WID */
    char    CommHndlStr[64];	/* Handler ID and Type of Service */
    char    ConnStatusStr[64];	/* TO WID */
    char    RdrStatusStr[64];	/* TO WID */
    char    TimeToNextStr[64];	/* TO WID */
    /* FROM WID */
    signed char    NewTxOnState;	/* -1 if no change, 0 if turn off, 1 if turn on */
    char    TxOnState;			/* current txon state */
    signed char    NewServoOnState;	/* -1 if no change, 0 if turn off, 1 if turn on */
    char    ServoOnState;		/* current servo on state */
    int	    NewRngRes;	    		/* -1 if no change, 500/1000/2000 for new rng res */
    int	    RngResState;			/* current rng res */
    signed char	    NewVolState;	/* -1 if no change, 0 if turn off, 1 if turn on */    	
    char    VolState;			/* current volumetric enable state */
    int	    DemandPPIElev;		/* tenths of degrees */
    signed char    PPIDemanded;		/* send command if 1 */
    int	    DemandRHIAzim;		/* tenths of degrees */
    signed char    RHIDemanded;		/* send command if 1 */
    int	    AzScanRate;		/* tenths of rpm */
    signed char    AzScanRateDemanded;		/* send command if 1 */
  };

  extern char SelSrcDBName[];
  extern char SelDestDBName[]; 
  extern char* RPScanTypeString[];
  extern char* RPScanTypeStringReadable[];
  extern int	DBSeqLoadSpc;
  extern int	DBSeqCopySpc;
  extern struct DBBrowseVars DBVars;
  extern struct ConnReqVars ReqVars;
  extern void *CommReqFormWid;
  /*extern char PrinterCMapFName[];*/
  extern void *Global_mainIface;
  extern char *cursorval1strings[];
  int StopSeq();
  int StartSeq();
  int	ToggleStartStopSeq();
  int LatestSeq();
  int LatestStatic();
  int LatestComplete();
  int LatestScanByScan();
  int OldestSeq();
  int PrevSeq();
  int NextSeq();
  int SeqDelete();
  int SetSeqPos(int pos);
  int UIFSeqPos(int pos, char *text);
  int UIFSeqSize(int sz);
  void UIFSeqMem(int mem);
  int UIFPoll();
  int SeqSetSpeed(int hundredthsperframe);
  int  SeqGetSpeed();
  int SeqGetSpeedStr(char *SpeedStr);
  void SeqSetMaxMem(int MemSize);
  int SeqGetMaxMem();
  int SeqGetMem();
  int SeqGetSize();
  void SeqSetDepth(int SeqDepth);
  int SeqGetDepth();
  void CallExitWarn();
  void SetCurrentSeqSize(int sz);
  void SetCurrentSeqMem(int sz);

  void ShowTime();
  void PutTime(char *TimeStr);
  void SeqUpdateData();
	
  void CallingWinSelectStn(void *CallingWin, int stn);
  void CallingWinRedraw(void *CallingWin);

  void *OpenMindBZ(void* CallingWin);
  void SetMindBZ(void *Wid, float val);
  void SetMindBZ_SetScaleLimits(void *Wid,  int minval, int maxval);
  void NewdBZ(void *CallingWin, float newdbz);
  void dBZWidClosed(void *CallingWin);

  void *OpenCAPPIHt(void* CallingWin);
  void SetCAPPIHt(void *Wid, float val);
  void SetCAPPIMode(void *CAPPIW, int flag); /* 0 - nearest, 1 - interp */
  void SetCAPPIUnits(void *CAPPIW, int kmflag); /* if !km, use kft */
  void NewCAPPIMode(void *CallingWin,  int flag);
  void NewCAPPIHt(void *CallingWin, float newht);
  void cappiWidClosed(void *CallingWin);
        

  void *OpenVILLimit(void *CallingWin);
  void SetVILLimit(void *Wid,  float VILLimit, float dBZHailLimit);
  void NewVILVal(void *CallingWin, float NewVILLimit, float NewdBZHailLimit);

  void CommMngOpenWid();	    /* tell commmng to open widget */
  void *OpenCommMngWid(void *CommMngPnt);	   /* from commmng to open wid */
  void CommMngWidCreated(void *CommMngPnt);  /* allows widget to notify comms manager when created */
  void CommMngWidClearList(void *Wid);
  void CommMngWidAddToList(void *Wid, char *newitem);
  void CommMngWidClosed(void *CommMngPnt);
  void CommMngOpenHandlerWid(void *CommMngPnt, int id, int StatusOnly);
  void CommMngSetLinkStatus(void *Wid,  char *LinkStatusStr);
  void UpdateCommMngWid(void *CommMngPnt);

  void CommMngSchedEditOpenWid(); /* tell commmng to open widget */
  void *OpenSchedEditWid(void *CommMngPnt); /* from commmng to open wid */
  void CommMngSchedEditWidClosed(void *CommMngPnt); /* tell commmng, wid closed */
		
  void CommMngReqEditOpenWid(); /* tell commmng to open widget */
  void *OpenReqEditWid(void *CommMngPnt); /* from commmng to open wid */
  void CommMngReqEditWidClosed(void *CommMngPnt); /* tell commmng, wid closed */
		
  void *OpenRdrStCtl(void *RPCommHndl, struct RdrStCtlStruct *WidStruct);
  void NewRdrStVals(void *Wid, struct RdrStCtlStruct   *StCtlVar);    /* update widget values */
  void NewRdrCtlVals(void *RPCommHndl); /* new command from widget */
  void RdrStCtlClosed(void *RPCommHndl); /* advise rpcommhndl of widget close */
  void RaiseWid(void *Wid); 
  void RaiseParentWid(void *Wid);
  void CloseWid(void *CloseW);

  void NewVal(void *CallingWin, float newval);
  void ValWidClosed(void *CallingWin);

  void UIFLoadMap(char *MpNm);

  void RestartCommMng();
  void SaveCommsConfig();
  /*	void CommConnect(char *connectstr); */
  int CommFirstConnStn();
  int CommNextConnStn();
  int CommPrevConnStn();
  int CommGetConnStnEntry(int entryno);
  char *StnName(int stn);
  int CommInitConnScanTypes(int stn);
  int CommNextConnScanType(int stn);
  int CommGetConnScanTypeItem(int stn,int item);
  void CommAddReq(int stn, int type, int angle);
  /*	void CommViewReq(); */
  void CommAddSched(int stn, int type, int angle,  int period, int offset);
  /*	void CommViewSched(); */
  int SchedEditUpdateList(void *SchedEditFormWid);
  int  FirstSchedString(char *str);
  int  NextSchedString(char *str);
  int DeleteSchedEntry(char *str);
  int ModifySchedEntry(char *str, int period, int offset);    /* units - seconds */
  int DeleteSchedEntryNum(int n);
  int ModifySchedEntryNum(int n, int period, int offset);	    /* units - seconds */
  int GetSchedEntryParamsNum(int n, int *period, int *offset);/* units - seconds */

  int ReqEditUpdateList(void *SchedEditFormWid);
  int  FirstReqString(char *str);
  int  NextReqString(char *str);
  int DeleteReqEntry(char *str);

  void CloseStnSelect(void *StnWid);
  void RaiseStnSelect(void *StnWid);
  void *OpenStnSelect(void *callingwin, char *newtitle, int *stn, int knownonly);
  void ListStns(void *Wid, int knownonly);
  void StnListClosed(void *callingwin);

  void NewCursorData(struct CursorDataStruct *cursordata);
  void ClearCursorData();
  void CursorDataUnmapRadarData();
  void CursorDataMapRadarData();
	
  void TimeTToStr(time_t tm, char *DateStr, char *TimeStr);
  time_t StrToTimeT(char *DateStr, char *TimeStr);
  time_t correctlocaltime(int localtime);

  int DBFirstStn();
  int DBNextStn();
  time_t DBFirstStnDate(int stn);
  time_t DBNextStnDate(int stn);
  time_t DBFirstStnDateTime(int stn,time_t tm, char *scantypestr);
  time_t DBNextStnDateTime(int stn,time_t tm, char *scantypestr);
  void DBViewImg(int stn,time_t tm);
  void DBLoadLatest();
  void DBLoadRealTime();
  void DBLoadTo(int stn,time_t tm);
  void DBLoadFrom(int stn,time_t tm);
  void DBLoadCentred(int stn,time_t tm);
  int DBOpenSrcDB(char *srcname);
  int DBSrcDBIsOpen();
  void DBCloseSrcDB();
  int DBOpenDestDB(char *destname);
  int DBCreateDestDB(char *srcname);
  int DBDestDBIsOpen();
  void DBCloseDestDB();
  void DBCreateMrgIsam();
  void DBRepairMrgIsam();
  void DBCopyRecs(int stn,time_t tm1,time_t tm2);
  void DBPutImgLbls();
  void DBNameSelected();

  char *XmGetTextPos(char *XmStr);

  void AddPPI(char *WName);
  void AddPPIRHI(char *WName);
  void AddPPIRHITriple(char *WName);
  void AddTops(char *WName);
  void AddCAPPI(char *WName);
  void AddVIL(char *WName);
  void AddCZBlk(char *WName);
  void OpenHeightPal();
  void OpenReflPal();
  void OpenVelPal();
  void OpenVILPal();
  void OpenImgData();

  void SaveDisplayState(char *name);
  void LoadDisplayState(char *name);

  void ReloadCMaps(char *FName);

  void AddPostAlertReq(char *title, 
		       char *alertstring, 
		       void *okCallback, 
		       void *clientdata, 
		       void **WidVar);
  void AddunPostAlertReq(void **WidVar);
  void *PostAlert(char *title, 
		  char *alertstring, 
		  void *okCallback, 
		  void *clientdata);
  void UnpostAlert(void *alertwid);

  /* IMPORTANT - struct OverlayProperties in displ.h MUST BE DECLARED
   * IDENTICALLY TO THE FIRST VARIABLES PART OF THIS
   */
  struct olayproperties {
    int OLayBG, RngRings, ShowText, Coast, ScanData, FontSize, LineThickness;
    int LayerFlag[16], ValueChanged;
  };
    
  void *OpenOlayPropWid(void *CallingOlayPropPnt, struct olayproperties *olayprops);
  void OlayPropWidClosed(void *CallingOlayPropPnt);
  void OlayPropNewVal(void *CallingOlayPropPnt);
  void UpdateZPal();

	
#ifdef __cplusplus
}
#endif

#endif /* __UIFTOVRC_H__ */
