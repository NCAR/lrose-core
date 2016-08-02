
/*******************************************************************************
       DatabaseOps.h
       This header file is included by DatabaseOps.c

*******************************************************************************/

#ifndef	_DATABASEOPS_INCLUDED
#define	_DATABASEOPS_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <X11/Shell.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/RepType.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/Label.h>
#include <Xm/BulletinB.h>

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
	Widget	UxDatabaseOps;
	Widget	Uxlabel3;
	Widget	Uxlabel4;
	Widget	Uxlabel5;
	Widget	UxscrolledWindow3;
	Widget	UxTimeScrollList;
	Widget	UxscrolledWindow2;
	Widget	UxDateScrollList;
	Widget	UxscrolledWindow4;
	Widget	UxStationScrollList;
	Widget	UxDBMenu;
	Widget	UxLoadDB;
	Widget	UxLoadLatest;
	Widget	UxLoadTo;
	Widget	UxLoadFrom;
	Widget	UxLoadCentred;
	Widget	UxLoadImg1Img2;
	Widget	UxLoadDB_b5;
	Widget	UxLoadDB_SpDepth;
	Widget	Uxmenu1_top_b2;
	Widget	UxCopyDB;
	Widget	UxDBCopyRec_b;
	Widget	UxDBCopyRecAllStn_b;
	Widget	UxDBReadFile1;
	Widget	Uxmenu1_top_b3;
	Widget	UxUtils;
	Widget	UxMrgDB;
	Widget	UxUtils_SeqSpdDpth;
	Widget	UxDBReadFile;
	Widget	Uxmenu1_top_b4;
	Widget	UxDBMenu_Exit;
	Widget	UxDBMenu_Exit_button;
	Widget	UxDBMenu_Exit_RealTm;
	Widget	UxDBMenu_top_b1;
	Widget	UxrowColumn4;
	Widget	UxSelImg1;
	Widget	UxSelImg2;
	Widget	UxSelImg1Lbl;
	Widget	UxSelImg2Lbl;
	Widget	UxSrcDBName;
	Widget	UxDestDBName;
	Widget	UxDBLoadSpacing;
	Widget	UxDBCopySpacing;
	Widget	UxDBSelSrcM;
	Widget	UxDBSelSrcMP;
	Widget	UxDBSelSrc_MAIN;
	Widget	UxDBSelSrc_Simple;
	Widget	UxDBSelSrc_Full;
	Widget	Uxmenu1_top_b5;
	Widget	UxDBSelDestM1;
	Widget	UxDBSelDest_MP;
	Widget	UxDBSelDest_MAIN;
	Widget	UxDBSelDest_Simple;
	Widget	UxDBSelDest_Full;
	Widget	UxDBSelSrcM1_top_b1;
	Widget	UxAutoLoad;
	Widget	UxMultiStn;
} _UxCDatabaseOps;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCDatabaseOps         *UxDatabaseOpsContext;
#define DatabaseOps             UxDatabaseOpsContext->UxDatabaseOps
#define label3                  UxDatabaseOpsContext->Uxlabel3
#define label4                  UxDatabaseOpsContext->Uxlabel4
#define label5                  UxDatabaseOpsContext->Uxlabel5
#define scrolledWindow3         UxDatabaseOpsContext->UxscrolledWindow3
#define TimeScrollList          UxDatabaseOpsContext->UxTimeScrollList
#define scrolledWindow2         UxDatabaseOpsContext->UxscrolledWindow2
#define DateScrollList          UxDatabaseOpsContext->UxDateScrollList
#define scrolledWindow4         UxDatabaseOpsContext->UxscrolledWindow4
#define StationScrollList       UxDatabaseOpsContext->UxStationScrollList
#define DBMenu                  UxDatabaseOpsContext->UxDBMenu
#define LoadDB                  UxDatabaseOpsContext->UxLoadDB
#define LoadLatest              UxDatabaseOpsContext->UxLoadLatest
#define LoadTo                  UxDatabaseOpsContext->UxLoadTo
#define LoadFrom                UxDatabaseOpsContext->UxLoadFrom
#define LoadCentred             UxDatabaseOpsContext->UxLoadCentred
#define LoadImg1Img2            UxDatabaseOpsContext->UxLoadImg1Img2
#define LoadDB_b5               UxDatabaseOpsContext->UxLoadDB_b5
#define LoadDB_SpDepth          UxDatabaseOpsContext->UxLoadDB_SpDepth
#define menu1_top_b2            UxDatabaseOpsContext->Uxmenu1_top_b2
#define CopyDB                  UxDatabaseOpsContext->UxCopyDB
#define DBCopyRec_b             UxDatabaseOpsContext->UxDBCopyRec_b
#define DBCopyRecAllStn_b       UxDatabaseOpsContext->UxDBCopyRecAllStn_b
#define DBReadFile1             UxDatabaseOpsContext->UxDBReadFile1
#define menu1_top_b3            UxDatabaseOpsContext->Uxmenu1_top_b3
#define Utils                   UxDatabaseOpsContext->UxUtils
#define MrgDB                   UxDatabaseOpsContext->UxMrgDB
#define Utils_SeqSpdDpth        UxDatabaseOpsContext->UxUtils_SeqSpdDpth
#define DBReadFile              UxDatabaseOpsContext->UxDBReadFile
#define menu1_top_b4            UxDatabaseOpsContext->Uxmenu1_top_b4
#define DBMenu_Exit             UxDatabaseOpsContext->UxDBMenu_Exit
#define DBMenu_Exit_button      UxDatabaseOpsContext->UxDBMenu_Exit_button
#define DBMenu_Exit_RealTm      UxDatabaseOpsContext->UxDBMenu_Exit_RealTm
#define DBMenu_top_b1           UxDatabaseOpsContext->UxDBMenu_top_b1
#define rowColumn4              UxDatabaseOpsContext->UxrowColumn4
#define SelImg1                 UxDatabaseOpsContext->UxSelImg1
#define SelImg2                 UxDatabaseOpsContext->UxSelImg2
#define SelImg1Lbl              UxDatabaseOpsContext->UxSelImg1Lbl
#define SelImg2Lbl              UxDatabaseOpsContext->UxSelImg2Lbl
#define SrcDBName               UxDatabaseOpsContext->UxSrcDBName
#define DestDBName              UxDatabaseOpsContext->UxDestDBName
#define DBLoadSpacing           UxDatabaseOpsContext->UxDBLoadSpacing
#define DBCopySpacing           UxDatabaseOpsContext->UxDBCopySpacing
#define DBSelSrcM               UxDatabaseOpsContext->UxDBSelSrcM
#define DBSelSrcMP              UxDatabaseOpsContext->UxDBSelSrcMP
#define DBSelSrc_MAIN           UxDatabaseOpsContext->UxDBSelSrc_MAIN
#define DBSelSrc_Simple         UxDatabaseOpsContext->UxDBSelSrc_Simple
#define DBSelSrc_Full           UxDatabaseOpsContext->UxDBSelSrc_Full
#define menu1_top_b5            UxDatabaseOpsContext->Uxmenu1_top_b5
#define DBSelDestM1             UxDatabaseOpsContext->UxDBSelDestM1
#define DBSelDest_MP            UxDatabaseOpsContext->UxDBSelDest_MP
#define DBSelDest_MAIN          UxDatabaseOpsContext->UxDBSelDest_MAIN
#define DBSelDest_Simple        UxDatabaseOpsContext->UxDBSelDest_Simple
#define DBSelDest_Full          UxDatabaseOpsContext->UxDBSelDest_Full
#define DBSelSrcM1_top_b1       UxDatabaseOpsContext->UxDBSelSrcM1_top_b1
#define AutoLoad                UxDatabaseOpsContext->UxAutoLoad
#define MultiStn                UxDatabaseOpsContext->UxMultiStn

#endif /* CONTEXT_MACRO_ACCESS */


/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	popup_DatabaseSelect();

#endif	/* _DATABASEOPS_INCLUDED */
