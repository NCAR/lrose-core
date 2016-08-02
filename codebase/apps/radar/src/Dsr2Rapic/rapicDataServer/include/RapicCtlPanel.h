
/*******************************************************************************
       RapicCtlPanel.h
       This header file is included by RapicCtlPanel.c

*******************************************************************************/

#ifndef	_RAPICCTLPANEL_INCLUDED
#define	_RAPICCTLPANEL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/RepType.h>
#include <Xm/TextF.h>
#include <Xm/Scale.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/CascadeBG.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>

/*******************************************************************************
       The definition of the context structure:
       If you create multiple copies of your interface, the context
       structure ensures that your callbacks use the variables for the
       correct copy.

       For each swidget in the interface, each argument to the Interface
       function, and each variable in the Interface Specific section of the
       Declarations Editor, there is an entry in the context structure.
       and a #define.  The #define makes the variable name refer to the
       corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	Widget	UxRapicCtlPanel;
	Widget	UxMainMenu;
	Widget	UxSequence;
	Widget	UxSeq_SpDpth;
	Widget	UxSeqAddWin;
	Widget	UxSeqAddWin_Label;
	Widget	UxSeqAddPPI;
	Widget	UxSeqAddPPIRHI;
	Widget	UxSeqAddWinTriple;
	Widget	UxSeqAddWinTops;
	Widget	UxSeqAddWinCAPPI;
	Widget	UxSeqAddWinVIL;
	Widget	UxSeqAddWinConstZ;
	Widget	UxSeqAddSep;
	Widget	UxSeqAddImgData;
	Widget	UxSeqAddReflPal;
	Widget	UxSeqAddVelPal;
	Widget	UxSeqAddHtPal;
	Widget	UxSeqAddVILPal;
	Widget	UxSeqAddWinTitanSep;
	Widget	UxSeqAddWinTitanStormDetails;
	Widget	UxSeqAddWinTitanTimeHtProf;
	Widget	UxSeq_AddWin;
	Widget	UxSeqLatestMode;
	Widget	UxSeqLatestMode_b1;
	Widget	UxSeqLatestMode_b2;
	Widget	UxSeqLatestMode_b3;
	Widget	UxSeq_LatestMd;
	Widget	UxSequence_b14;
	Widget	UxSeq_LoadLayoutDflt;
	Widget	UxSeq_LoadLayout;
	Widget	UxSeq_SaveLayout;
	Widget	UxSeq_LoadCMap;
	Widget	UxSeq_LoadPrintCMap;
	Widget	UxUTCTime;
	Widget	UxLclTime;
	Widget	UxSeq_ZR;
	Widget	UxSeq_Sep3;
	Widget	UxSeq_TitanParams;
	Widget	UxSeq_Sep4;
	Widget	UxSeq_DelImg;
	Widget	Uxmenu1_top_b1;
	Widget	UxDatabase;
	Widget	UxMainMenu_p3_b1;
	Widget	UxDBRealTime;
	Widget	UxMainMenu_top_b2;
	Widget	UxComms;
	Widget	UxRadarStatus;
	Widget	UxCommsMenuAddReq;
	Widget	UxEditSched;
	Widget	UxEditReq;
	Widget	UxSaveComm;
	Widget	UxRestartComm;
	Widget	UxMainMenu_top_b3;
	Widget	UxExit;
	Widget	UxMainMenu_p2_b1;
	Widget	UxExit_b2;
	Widget	UxMainMenu_top_b1;
	Widget	UxdrawingArea1;
	Widget	UxstopButton;
	Widget	UxlatestButton;
	Widget	UxstepbackButton;
	Widget	UxSeqPosScale;
	Widget	UxloopButton;
	Widget	UxfirstButton;
	Widget	UxstepfwdButton;
	Widget	Uxlabel7;
	Widget	UxTitleLabel;
	Widget	UxImageTimetext;
	Widget	Uxlabel21;
	Widget	UxCtlPanelDateTime1;
	Widget	Uxlabel1;
	Widget	Uxlabel31;
	Widget	UxScanDetailstext;
	Widget	Uxseparator8;
	Widget	Uxlabel32;
	Widget	UxdBZLabel;
	Widget	UxdBZText;
	Widget	UxRLabel;
	Widget	UxRText;
	Widget	UxLatLongLabel;
	Widget	UxLatLongText;
	Widget	UxhtText;
	Widget	UxHtLabel;
	Widget	UxrngbrngLabel;
	Widget	UxDistNELabel;
	Widget	UxDistNEText;
	Widget	UxUnitMode;
	Widget	UxUnitsMetric;
	Widget	UxUnitskmkft;
	Widget	UxUnitsImperial;
	Widget	UxUnitsNautical;
	Widget	UxUnitSep;
	Widget	UxDistFromRef1;
	Widget	UxDistFromRadar1;
	Widget	UxoptionMenu1;
	Widget	UxRngBrngText;
	Widget	UxdeltatimeLabel;
	Widget	UxVelLabel;
	Widget	UxvelText;
	Widget	UxdtimeText;
	Widget	UxDragToggleButton;
	Widget	UxtoggleButton3;
	Widget	UxDistNELabel1;
	int	UxUnits;
	int	UxUseRef;
	int	UxSliderDrag;
	int	UxAlwaysOnTop;
	char	UxTitleFontStr[128];
	char	UxTitleFontStr2[128];
} _UxCRapicCtlPanel;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRapicCtlPanel       *UxRapicCtlPanelContext;
#define RapicCtlPanel           UxRapicCtlPanelContext->UxRapicCtlPanel
#define MainMenu                UxRapicCtlPanelContext->UxMainMenu
#define Sequence                UxRapicCtlPanelContext->UxSequence
#define Seq_SpDpth              UxRapicCtlPanelContext->UxSeq_SpDpth
#define SeqAddWin               UxRapicCtlPanelContext->UxSeqAddWin
#define SeqAddWin_Label         UxRapicCtlPanelContext->UxSeqAddWin_Label
#define SeqAddPPI               UxRapicCtlPanelContext->UxSeqAddPPI
#define SeqAddPPIRHI            UxRapicCtlPanelContext->UxSeqAddPPIRHI
#define SeqAddWinTriple         UxRapicCtlPanelContext->UxSeqAddWinTriple
#define SeqAddWinTops           UxRapicCtlPanelContext->UxSeqAddWinTops
#define SeqAddWinCAPPI          UxRapicCtlPanelContext->UxSeqAddWinCAPPI
#define SeqAddWinVIL            UxRapicCtlPanelContext->UxSeqAddWinVIL
#define SeqAddWinConstZ         UxRapicCtlPanelContext->UxSeqAddWinConstZ
#define SeqAddSep               UxRapicCtlPanelContext->UxSeqAddSep
#define SeqAddImgData           UxRapicCtlPanelContext->UxSeqAddImgData
#define SeqAddReflPal           UxRapicCtlPanelContext->UxSeqAddReflPal
#define SeqAddVelPal            UxRapicCtlPanelContext->UxSeqAddVelPal
#define SeqAddHtPal             UxRapicCtlPanelContext->UxSeqAddHtPal
#define SeqAddVILPal            UxRapicCtlPanelContext->UxSeqAddVILPal
#define SeqAddWinTitanSep       UxRapicCtlPanelContext->UxSeqAddWinTitanSep
#define SeqAddWinTitanStormDetails UxRapicCtlPanelContext->UxSeqAddWinTitanStormDetails
#define SeqAddWinTitanTimeHtProf UxRapicCtlPanelContext->UxSeqAddWinTitanTimeHtProf
#define Seq_AddWin              UxRapicCtlPanelContext->UxSeq_AddWin
#define SeqLatestMode           UxRapicCtlPanelContext->UxSeqLatestMode
#define SeqLatestMode_b1        UxRapicCtlPanelContext->UxSeqLatestMode_b1
#define SeqLatestMode_b2        UxRapicCtlPanelContext->UxSeqLatestMode_b2
#define SeqLatestMode_b3        UxRapicCtlPanelContext->UxSeqLatestMode_b3
#define Seq_LatestMd            UxRapicCtlPanelContext->UxSeq_LatestMd
#define Sequence_b14            UxRapicCtlPanelContext->UxSequence_b14
#define Seq_LoadLayoutDflt      UxRapicCtlPanelContext->UxSeq_LoadLayoutDflt
#define Seq_LoadLayout          UxRapicCtlPanelContext->UxSeq_LoadLayout
#define Seq_SaveLayout          UxRapicCtlPanelContext->UxSeq_SaveLayout
#define Seq_LoadCMap            UxRapicCtlPanelContext->UxSeq_LoadCMap
#define Seq_LoadPrintCMap       UxRapicCtlPanelContext->UxSeq_LoadPrintCMap
#define UTCTime                 UxRapicCtlPanelContext->UxUTCTime
#define LclTime                 UxRapicCtlPanelContext->UxLclTime
#define Seq_ZR                  UxRapicCtlPanelContext->UxSeq_ZR
#define Seq_Sep3                UxRapicCtlPanelContext->UxSeq_Sep3
#define Seq_TitanParams         UxRapicCtlPanelContext->UxSeq_TitanParams
#define Seq_Sep4                UxRapicCtlPanelContext->UxSeq_Sep4
#define Seq_DelImg              UxRapicCtlPanelContext->UxSeq_DelImg
#define menu1_top_b1            UxRapicCtlPanelContext->Uxmenu1_top_b1
#define Database                UxRapicCtlPanelContext->UxDatabase
#define MainMenu_p3_b1          UxRapicCtlPanelContext->UxMainMenu_p3_b1
#define DBRealTime              UxRapicCtlPanelContext->UxDBRealTime
#define MainMenu_top_b2         UxRapicCtlPanelContext->UxMainMenu_top_b2
#define Comms                   UxRapicCtlPanelContext->UxComms
#define RadarStatus             UxRapicCtlPanelContext->UxRadarStatus
#define CommsMenuAddReq         UxRapicCtlPanelContext->UxCommsMenuAddReq
#define EditSched               UxRapicCtlPanelContext->UxEditSched
#define EditReq                 UxRapicCtlPanelContext->UxEditReq
#define SaveComm                UxRapicCtlPanelContext->UxSaveComm
#define RestartComm             UxRapicCtlPanelContext->UxRestartComm
#define MainMenu_top_b3         UxRapicCtlPanelContext->UxMainMenu_top_b3
#define Exit                    UxRapicCtlPanelContext->UxExit
#define MainMenu_p2_b1          UxRapicCtlPanelContext->UxMainMenu_p2_b1
#define Exit_b2                 UxRapicCtlPanelContext->UxExit_b2
#define MainMenu_top_b1         UxRapicCtlPanelContext->UxMainMenu_top_b1
#define drawingArea1            UxRapicCtlPanelContext->UxdrawingArea1
#define stopButton              UxRapicCtlPanelContext->UxstopButton
#define latestButton            UxRapicCtlPanelContext->UxlatestButton
#define stepbackButton          UxRapicCtlPanelContext->UxstepbackButton
#define SeqPosScale             UxRapicCtlPanelContext->UxSeqPosScale
#define loopButton              UxRapicCtlPanelContext->UxloopButton
#define firstButton             UxRapicCtlPanelContext->UxfirstButton
#define stepfwdButton           UxRapicCtlPanelContext->UxstepfwdButton
#define label7                  UxRapicCtlPanelContext->Uxlabel7
#define TitleLabel              UxRapicCtlPanelContext->UxTitleLabel
#define ImageTimetext           UxRapicCtlPanelContext->UxImageTimetext
#define label21                 UxRapicCtlPanelContext->Uxlabel21
#define CtlPanelDateTime1       UxRapicCtlPanelContext->UxCtlPanelDateTime1
#define label1                  UxRapicCtlPanelContext->Uxlabel1
#define label31                 UxRapicCtlPanelContext->Uxlabel31
#define ScanDetailstext         UxRapicCtlPanelContext->UxScanDetailstext
#define separator8              UxRapicCtlPanelContext->Uxseparator8
#define label32                 UxRapicCtlPanelContext->Uxlabel32
#define dBZLabel                UxRapicCtlPanelContext->UxdBZLabel
#define dBZText                 UxRapicCtlPanelContext->UxdBZText
#define RLabel                  UxRapicCtlPanelContext->UxRLabel
#define RText                   UxRapicCtlPanelContext->UxRText
#define LatLongLabel            UxRapicCtlPanelContext->UxLatLongLabel
#define LatLongText             UxRapicCtlPanelContext->UxLatLongText
#define htText                  UxRapicCtlPanelContext->UxhtText
#define HtLabel                 UxRapicCtlPanelContext->UxHtLabel
#define rngbrngLabel            UxRapicCtlPanelContext->UxrngbrngLabel
#define DistNELabel             UxRapicCtlPanelContext->UxDistNELabel
#define DistNEText              UxRapicCtlPanelContext->UxDistNEText
#define UnitMode                UxRapicCtlPanelContext->UxUnitMode
#define UnitsMetric             UxRapicCtlPanelContext->UxUnitsMetric
#define Unitskmkft              UxRapicCtlPanelContext->UxUnitskmkft
#define UnitsImperial           UxRapicCtlPanelContext->UxUnitsImperial
#define UnitsNautical           UxRapicCtlPanelContext->UxUnitsNautical
#define UnitSep                 UxRapicCtlPanelContext->UxUnitSep
#define DistFromRef1            UxRapicCtlPanelContext->UxDistFromRef1
#define DistFromRadar1          UxRapicCtlPanelContext->UxDistFromRadar1
#define optionMenu1             UxRapicCtlPanelContext->UxoptionMenu1
#define RngBrngText             UxRapicCtlPanelContext->UxRngBrngText
#define deltatimeLabel          UxRapicCtlPanelContext->UxdeltatimeLabel
#define VelLabel                UxRapicCtlPanelContext->UxVelLabel
#define velText                 UxRapicCtlPanelContext->UxvelText
#define dtimeText               UxRapicCtlPanelContext->UxdtimeText
#define DragToggleButton        UxRapicCtlPanelContext->UxDragToggleButton
#define toggleButton3           UxRapicCtlPanelContext->UxtoggleButton3
#define DistNELabel1            UxRapicCtlPanelContext->UxDistNELabel1
#define Units                   UxRapicCtlPanelContext->UxUnits
#define UseRef                  UxRapicCtlPanelContext->UxUseRef
#define SliderDrag              UxRapicCtlPanelContext->UxSliderDrag
#define AlwaysOnTop             UxRapicCtlPanelContext->UxAlwaysOnTop
#define TitleFontStr            UxRapicCtlPanelContext->UxTitleFontStr
#define TitleFontStr2           UxRapicCtlPanelContext->UxTitleFontStr2

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_RapicCtlPanel();

#endif	/* _RAPICCTLPANEL_INCLUDED */
