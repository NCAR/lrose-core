/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifndef METARX
#define METARX
 
#ifdef __cplusplus
 extern "C" {
#endif                                                                                        
/********************************************************************/
/*                                                                  */
/*  Title:         METAR H                                          */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          09 Jan 1995                                      */
/*  Programmer:    CARL MCCALLA                                     */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      METAR Decoder Header File.                       */
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
 
#include <rapformats/nws_oso_local.h>     /* standard header file */
 
 
/*********************************************/
/*                                           */
/* RUNWAY VISUAL RANGE STRUCTURE DECLARATION */
/*       AND VARIABLE TYPE DEFINITION        */
/*                                           */
/*********************************************/
 
typedef struct runway_VisRange {
   int  visRange;
   int  Min_visRange;
   int  Max_visRange;
   ubool above_max_RVR;
   ubool vrbl_visRange;
   ubool below_min_RVR;
   ubool spare;
   char runway_designator[8];
}  Runway_VisRange;
 
/***********************************************/
/*                                             */
/* DISPATCH VISUAL RANGE STRUCTURE DECLARATION */
/*       AND VARIABLE TYPE DEFINITION          */
/*                                             */
/***********************************************/
 
typedef struct dispatch_VisRange {
   int  visRange;
   int  Max_visRange;
   int  Min_visRange;
   ubool vrbl_visRange;
   ubool below_min_DVR;
   ubool above_max_DVR;
   ubool spare;
}  Dispatch_VisRange;
 
/*****************************************/
/*                                       */
/* CLOUD CONDITION STRUCTURE DECLARATION */
/*      AND VARIABLE TYPE DEFINITION     */
/*                                       */
/*****************************************/
 
typedef struct cloud_Conditions {
   int  cloud_hgt_meters;
   char cloud_type[8];
   char cloud_hgt_char[4];
   char other_cld_phenom[4];
}  Cloud_Conditions;
 
/*****************************************/
/*                                       */
/* WIND GROUP DATA STRUCTURE DECLARATION */
/*      AND VARIABLE TYPE DEFINITION     */
/*                                       */
/*****************************************/
 
typedef struct windstruct {
   int windDir;
   int windSpeed;
   int windGust;
   ubool windVRB;
   ubool windEstimated;
   char windUnits[ 4 ];
} WindStruct;
 
/*****************************************/
/*                                       */
/* RECENT WX GROUP STRUCTURE DECLARATION */
/*      AND VARIABLE TYPE DEFINITION     */
/*                                       */
/*****************************************/
 
typedef struct recent_wx {
   int  Bhh;
   int  Bmm;
   int  Ehh;
   int  Emm;
   char Recent_weather[ 8 ];
} Recent_Wx;
 
/***************************************/
/*                                     */
/* DECODED METAR STRUCTURE DECLARATION */
/*     AND VARIABLE TYPE DEFINITION    */
/*                                     */
/***************************************/
 
typedef struct decoded_METAR {
   int  ob_hour;
   int  ob_minute;
   int  ob_date;
   int minWnDir;
   int maxWnDir;
   int VertVsby;
   int temp;
   int dew_pt_temp;
   int QFE;
   int hectoPasc_altstng;
   int char_prestndcy;
   int minCeiling;
   int maxCeiling;
   int WshfTime_hour;
   int WshfTime_minute;
   int min_vrbl_wind_dir;
   int max_vrbl_wind_dir;
   int PKWND_dir;
   int PKWND_speed;
   int PKWND_hour;
   int PKWND_minute;
   int SKY_2ndSite_Meters;
   int Ceiling;
   int Estimated_Ceiling;
   int SNINCR;
   int SNINCR_TotalDepth;
   int SunshineDur;
   int ObscurAloftHgt;
   int VrbSkyLayerHgt;
   int Num8thsSkyObscured;
   int CIG_2ndSite_Meters;
   int snow_depth;
   int BTornadicHour;
   int BTornadicMinute;
   int ETornadicHour;
   int ETornadicMinute;
 
 
   float SectorVsby;
   float WaterEquivSnow;
   float VSBY_2ndSite;
   float prevail_vsbySM;
   float prevail_vsbyM;
   float prevail_vsbyKM;
   float prestndcy;
   float precip_amt;
   float precip_24_amt;
   float maxtemp;
   float mintemp;
   float max24temp;
   float min24temp;
   float minVsby;
   float maxVsby;
   float hourlyPrecip;
   float TWR_VSBY;
   float SFC_VSBY;
   float Temp_2_tenths;
   float DP_Temp_2_tenths;
   float SLP;
   float GR_Size;
 
   double inches_altstng;
 
   ubool CIGNO;
   ubool SLPNO;
   ubool ACFTMSHP;
   ubool NOSPECI;
   ubool FIRST;
   ubool LAST;
   ubool SunSensorOut;
   ubool AUTO;
   ubool COR;
   ubool NIL_rpt;
   ubool CAVOK;
   ubool RVRNO;
   ubool A_altstng;
   ubool Q_altstng;
   ubool VIRGA;
   ubool VOLCASH;
   ubool GR;
   ubool CHINO;
   ubool VISNO;
   ubool PNO;
   ubool PWINO;
   ubool FZRANO;
   ubool TSNO;
   ubool DollarSign;
   ubool PRESRR;
   ubool PRESFR;
   ubool Wshft_FROPA;
   ubool OCNL_LTG;
   ubool FRQ_LTG;
   ubool CNS_LTG;
   ubool CG_LTG;
   ubool IC_LTG;
   ubool CC_LTG;
   ubool CA_LTG;
   ubool DSNT_LTG;
   ubool AP_LTG;
   ubool VcyStn_LTG;
   ubool OVHD_LTG;
   ubool LightningVCTS;
   ubool LightningTS;
 
   char synoptic_cloud_type[ 8 ];
   char snow_depth_group[ 8 ];
   char codeName[ 8 ];
   char stnid[8];
   char horiz_vsby[8];
   char dir_min_horiz_vsby[4];
   char vsby_Dir[ 4 ];
   char WxObstruct[10][8];
   char autoIndicator[8];
   char VSBY_2ndSite_LOC[16];
   char SKY_2ndSite_LOC[16];
   char SKY_2ndSite[16];
   char SectorVsby_Dir[ 4 ];
   char ObscurAloft[ 16 ];
   char ObscurAloftSkyCond[ 16 ];
   char VrbSkyBelow[ 4 ];
   char VrbSkyAbove[ 4 ];
   char LTG_DIR[ 4 ];
   char CloudLow;
   char CloudMedium;
   char CloudHigh;
   char CloudSpare;
   char CIG_2ndSite_LOC[16];
   char VIRGA_DIR[4];
   char TornadicType[16];
   char TornadicLOC[16];
   char TornadicDIR[4];
   char CHINO_LOC[8];
   char VISNO_LOC[8];
   char PartialObscurationAmt[2][8];
   char PartialObscurationPhenom[2][12];
   char SfcObscuration[6][16];
   char charPrevailVsby[16];
   char charVertVsby[16];
 
   Runway_VisRange RRVR[16];
   Dispatch_VisRange DVR;
   Recent_Wx ReWx[3];
   WindStruct winData;
   Cloud_Conditions cldTypHgt[6];

}  Decoded_METAR;
 
#define MAXWXSYMBOLS 10       /*-- NOT TO EXCEED 10 PRES. WX GRPS --*/
#define MAXTOKENS    500      /*--  RPT NOT TO EXCEED 500 GRPS   --*/
 
#ifdef __cplusplus
}
#endif                                                        
 
#endif
