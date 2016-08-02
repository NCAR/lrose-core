/*

	RdrGlobals.c

*/

#include "rdr.h"
#include "palette.h"
#include "utils.h"
#include "log.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

const float EarthRad = 6371.64;      // Earth' radius in km
const float KMPerDeg = 111.240;
const float DegPerKM = 1/KMPerDeg;
const float DegPerRdn = 57.295779513;
const float RdnPerDeg = 1/DegPerRdn;
const float KMPerNM = 1.854;
const float NMPerKM = 1/KMPerNM;
const float FtPerM = 3.28084;
const float MPerFt = 1/FtPerM;
const float KFtperKm = 3.28084;
const float MperStoKMH = 3.6;

float ZRa = 200;      // Z-R co-efficients Z = aR^b
float ZRb = 1.6;
char zr_filename[] = "rp_zr_coeff.ini";

void load_zr(char *zr_fname)
{
  char *fname = zr_fname;
  char localstr[256];
  float fl;

  if (!fname)
    fname = zr_filename;
  if (!fname)
    return;
  FILE *zr_file = fopen(fname, "r");
  if (!zr_file)
    return;
  while (fgets(localstr, 256, zr_file))
    {
      if (localstr[0] && (localstr[0] != '#'))
	{
	  if (sscanf(localstr, "ZRa=%f", &fl) == 1)
	    ZRa = fl;
	  if (sscanf(localstr, "ZRb=%f", &fl) == 1)
	    ZRb = fl;
	}
    }
  fclose(zr_file);
}

void load_new_zr(char *zr_fname)
{
  char new_fname[256];
  char current_fname[256];
  char logmssg[256];
  float old_zra, old_zrb;

  if (zr_fname)
    strncpy(new_fname, zr_fname, 256);
  else
    strncpy(new_fname, zr_filename, 256);
  strcpy(current_fname, new_fname);
  strcat(new_fname, ".new");
  strcat(current_fname, ".current");
  if (!new_fname)
    return;
  if (FileExists(new_fname))
    {
      old_zra = ZRa;
      old_zrb = ZRb;
      load_zr(new_fname);
      rename(new_fname, current_fname);
      sprintf(logmssg, "New ZR Co-efficients loaded. ZRa=%1.2f (previously %1.2f), ZRb=%1.2f (previously %1.2f)", 
	      ZRa, old_zra, ZRb, old_zrb);
      RapicLog(logmssg, LOG_CRIT);
    }
}

/*
	default level to dBZ translation tables in 0.5 dBZ steps
*/
/*
char   dBZTbl6L[8] = {0, 24, 56, 78, 88, 98, 110, 140};

char    dBZTbl16L[16] = { 0, 24, 46, 56, 62, 68, 74, 80,
			86, 92, 98,104,110,116,122,128};
*/

// Default threshold tables - in units of dbZ
float   Dflt_dBZThresh6Lvl[8] = {0.0, 12.00, 28.00, 39.00, 44.00, 49.00, 55.00, 70.00};

float    Dflt_dBZThresh16Lvl[16] = { 0.0, 12.00, 24.00, 28.00, 31.00, 34.00, 37.00, 40.00,
				     43.00, 46.00, 49.00, 52.00, 55.00, 58.00, 61.00, 64.00};

/*
	default color map basis points in 0.5 dBZ steps
*/

/*
float STDAccumThresh32[32] = 
    {  0.0,  1.0,  2.0,  4.0,  7.0, 10.0, 14.0, 18.0, 
      24.0, 30.0, 40.0, 50.0, 65.0, 80.0, 100.0, 125.0, 
      150.0, 180.0, 220.0, 280.0, 350.0, 450.0, 600.0, 800.0, 
      1000.0, 1300.0, 1700.0, 2200.0, 3000.0, 4000.0, 6000.0, 10000.0};
*/

float STDAccumThresh32[32] = 
{  0.0,   0.2,   0.5,   1.0,   2.0,   4.0,   7.0,  10.0, 
   14.0,  18.0,  24.0,  30.0,  40.0,  50.0,  65.0,  80.0, 
   100.0, 125.0, 150.0, 180.0, 220.0, 280.0, 350.0, 450.0, 
   600.0, 800.0,1000.0,1300.0,1700.0,2200.0,3000.0,4000.0};

// typically this table will be interpolated to 96 levels 
float STDAccumThresh24[24] = 
{     0.0,  // idx=0
      0.4,  // idx=4
      1.0,  // idx=8   
      2.0,  // idx=12 
      4.0,  // idx=16
      6.0,  // idx=20
      8.0,  // idx=24
      10.0,  // idx=28
      14.0,  // idx=32
      20.0,  // idx=36
      28.0,  // idx=40
      36.0,  // idx=44
      48.0,  // idx= 48
      60.0,  // idx= 52 
      80.0,  // idx= 56
      100.0,  // idx= 60
      140.0,  // idx= 64
      180.0,  // idx= 68
      240.0,  // idx= 72
      300.0,  // idx=76
      400.0,  // idx=80
      600.0,  // idx=84
     1000.0,  // idx=88
     1400.0   // idx=92
};

bool Displ8Bit = FALSE;

char   dBZCMapBasis[9] = {0, 1, 24, 56, 78, 88, 98, 110, 140};
int	MaxHt = 20000;  // height of max height color palette value
float terrainHtIncrement = 10;  // set default terrainHtIncrement to 10m
// float terrainHtIncrement = 20;  // set default terrainHtIncrement to 20m Malaysia
// float terrainHtIncrement = 30;  // set default terrainHtIncrement to 30m USA
int	VILTop = 100;  // max VIL color palette value
char	HtCMapBasis[9] = {0,1,20,40,60,80,100,120,140};
char	VCMapBasis[7] = {0, 1,69, 70, 72, 73, 141}; 

int    zmapsize = 256;
int    vmapsize = 142;
float  vmap_nyquist = 10;
bool   vmap_absolute = false;  // if true use absolute vmap, default is nyquist
bool   draw_absolute_vmap = true;  // allow switch drawing btwn abs and scan nyquist
float  vmap_abs_nyquist = 50;  // nyquist to use for absolute vel pal
bool   drawAbsVmap()
{ return vmap_absolute && draw_absolute_vmap; };
  ;  // allow switch drawing btwn abs and scan nyquist
int    htmapsize = 160;
int    vilmapsize = 160;
int    accummapsize = 256;  // may be smaller, but allow max size
int    terrainhtmapsize = 256;

int    CMapBase  = 512;
int    BGUndefined  = CMapBase;
int    BGIncomplete = BGUndefined + 1;
int    BGNormal = BGUndefined + 2;
int    BGCovered2 = BGUndefined + 3;
int    BGCovered1 = BGUndefined + 4;
int    PalIndColor = BGUndefined + 5;
int    AngleIndColor = BGUndefined + 6;
int    DataTextColor = BGUndefined + 7;
int    MapCol = BGUndefined + 8;
int    RingCol = BGUndefined + 9;
int    RHIGridCol = BGUndefined + 10;
int    RHI3DGridCol = BGUndefined + 11;
int    LatLongCol = BGUndefined + 12;
int    ImageTitleColor = BGUndefined + 13;
bool   ImageTitleBGSolid = false;
int    titanCurrentShapeCol = BGUndefined + 14;
int    titanPastShapeCol = BGUndefined + 15;
int    titanFcstShapeCol = BGUndefined + 16;
int    titanTrackCol = BGUndefined + 17;
int    titanFcastTrackCol = BGUndefined + 18;
int    annotCol = BGUndefined + 19;
int    worldCoreCol = BGUndefined + 20;

int    CMapBase8  = 0;
int    BGUndefined8  = CMapBase8;
int    BGIncomplete8 = BGUndefined8 + 1;
int    BGNormal8 = BGUndefined8 + 2;
int    PalIndColor8 = BGUndefined8 + 3;
int    AngleIndColor8 = BGUndefined8 + 4;
int    DataTextColor8 = BGUndefined8 + 3;
int    MapCol8 = BGUndefined8 + 5;
int    RingCol8 = BGUndefined8 + 6;
int    RHIGridCol8 = BGUndefined8 + 6;
int    RHI3DGridCol8 = BGUndefined8 + 6;
int    LatLongCol8 = BGUndefined + 6;

/*
int    BGUndefined8Bit	= 0;
int    BGIncomplete8Bit = BGUndefined8Bit + 1;
int    BGNormal8Bit = BGIncomplete8Bit + 1;
int    PalIndColor8Bit = BGNormal8Bit + 3;
*/


#define StdColorCount 23

// FIRST ENTRY MUST BE BASE INDEX
// OTHER VALUES CAN BE OUT OF ORDER AND OUT OF RANGE 
// e.g. RdrHtMapOverflow lives above radar height palette
cmapentry StdColors[StdColorCount] = 
{{BGNormal,30,60,50,255}, 
 {BGIncomplete,80,100,80, 255}, 
 {BGUndefined,60,80,90, 255}, 
 {RdrHtMapOverflow, 255, 0, 0, 255}, 
 {RdrVILMapOverflow, 255, 0, 0, 255}, 
 {PalIndColor, 255, 0, 0, 255}, 
 {MapCol, 80, 120, 140, 255}, 
 {RingCol, 0, 50, 0, 255}, 
 {RHIGridCol, 128, 128, 0, 255}, 
 {RHI3DGridCol, 128, 128, 0, 255}, 
 {LatLongCol, 110, 120, 120, 255}, 
 {AngleIndColor, 0, 180, 180, 255}, 
 {ImageTitleColor, 0, 214, 214, 255}, 
 {DataTextColor, 180, 180, 180, 255}, 
 {BGCovered1,0,0,0, 255}, 
 {BGCovered2,10,30,20, 255},
 {titanCurrentShapeCol, 220, 0, 0, 255},
 {titanPastShapeCol, 220, 80, 80, 255},
 {titanFcstShapeCol, 160, 0 , 0, 255},
 {titanTrackCol, 0, 0, 220, 255},
 {titanFcastTrackCol, 0, 170, 220, 255},
 {annotCol, 200, 40, 255, 255},
 {worldCoreCol, 0, 20, 60, 255}
}; 

#define StdColorCount8 13
cmapentry StdColors8[13] = 
{{BGNormal8,0,0,0}, 
 {BGIncomplete8,50,50,50}, 
 {BGUndefined8,0,32,32}, 
 {RdrHtMapOverflow8, 255, 0, 0}, 
 {RdrVILMapOverflow8, 255, 0, 0}, 
 {PalIndColor8, 255, 0, 0}, 
 {MapCol8, 0, 80, 128}, 
 {RingCol8, 0, 50, 0}, 
 {RHIGridCol8, 128, 128, 0}, 
 {RHI3DGridCol8, 128, 128, 0}, 
 {LatLongCol8, 128, 128, 180}, 
 {AngleIndColor8, 0, 180, 180}, 
 {DataTextColor8, 180, 180, 180}};

StdColorTbl StdColorTable = {StdColorCount, StdColors};
StdColorTbl StdColorTable8 = {StdColorCount8, StdColors8};

int    RdrZCMapBase = CMapBase + 32;
int    zmapbase = RdrZCMapBase;
int    RdrVCMapBase = RdrZCMapBase + zmapsize;
int    vmapbase = RdrVCMapBase;
int    RdrHtMapBase = RdrVCMapBase + vmapsize;
int    htmapbase = RdrHtMapBase;
int    RdrHtMapOverflow = RdrHtMapBase + htmapsize;
int    htmapoflow = RdrHtMapOverflow;
int    RdrVILMapBase = RdrHtMapOverflow + 1;
int    vilmapbase = RdrVILMapBase;
int    RdrVILMapOverflow = RdrVILMapBase + vilmapsize;
int    vilmapoflow = RdrVILMapOverflow;
int    TerrainHtMapBase = RdrVILMapOverflow + 1;
int    RdrAccumMapBase = TerrainHtMapBase + terrainhtmapsize;
int    accummapbase = RdrAccumMapBase;
// always modify CMapTop to be last cmap color 
int    CMapTop = RdrAccumMapBase + accummapsize;    

// following definitions for 8bit displays
int    zmapsize8_16 = 16;
int    zmapsize8_16D = 9;
int    zmapsize8_6 = 8;
int    vmapsize8 = 16;
int    htmapsize8 = 16;
int    vilmapsize8 = 16;
int    accummapsize8 = 32;


// Double Buffered ZMap colors
// Only space for 9 colors, use 0, 1, 2/3, 4/5, 6/7, 8/9, 10/11, 12/13, 14/15
int    RdrZCMapBase8_16D = RHI3DGridCol8 + 1;	

int    RdrZCMapBase8_16 = RdrZCMapBase8_16D + zmapsize8_16D;
int    RdrZCMapBase8_6 = RdrZCMapBase8_16 + zmapsize8_16;
int    RdrVCMapBase8 = RdrZCMapBase8_6 + zmapsize8_6;
int    RdrHtMapBase8 = RdrVCMapBase8 + vmapsize8;
int    RdrHtMapOverflow8 = RdrHtMapBase8 + htmapsize8;
int    RdrVILMapBase8 = RdrHtMapOverflow8 + 1;
int    RdrVILMapOverflow8 = RdrVILMapBase8 + vilmapsize8;
int    RdrAccumMapBase8 = RdrVILMapOverflow8 + 1;
// always modify CMapTop8 to be last cmap color
int    CMapTop8  = RdrAccumMapBase8 + accummapsize8;

int 	 dBZOffset = 0;		// 0 offset of dBZ colormap (0.5dBZ steps)
// e.g. 20 gives -10dBZ base
float	MINDBZ = -32.0;
float	MAXDBZ = 96.0;

cmapbasis *GMSIR1CMapBasis = satcmapbasis_IR1;
cmapbasis *GMSIR2CMapBasis = satcmapbasis_IR2;
cmapbasis *GMSIR3CMapBasis = satcmapbasis_IR3;
cmapbasis *GMSVisCMapBasis = satcmapbasis_Vis;

// short    DBZCMAP[160][3];  // dBZ color look up table (0-80dBZ 0.5dBZ steps)
float   rdrcmap_16L[16][3]=
{{0.0,0.0,0.0},{0.0,0.0,0.4},{0.0,0.0,0.7},{0.0,0.5,0.8},
 {0.0,1.0,1.0},{0.5,1.0,0.5},{1.0,1.0,0.0},{0.5,0.9,0.0},
 {0.0,0.8,0.0},{0.5,0.4,0.5},{1.0,0.0,1.0},{1.0,0.0,0.5},
 {1.0,0.0,0.0},{1.0,1.0,1.0},{1.0,1.0,1.0},{1.0,1.0,1.0}};

cmapbasis   rdrcmapbasis_16L[16] = 
{{0, 0, 0, 0},{1, 0, 0, 100},{2, 0, 0, 180},{3, 0, 128, 200},
 {4, 0, 255, 255},{5, 128, 255, 128},{6, 255, 255, 0},{7, 128, 230, 0},
 {8, 0, 200, 0},{9, 128, 100, 128},{10, 255, 0, 255},{11, 255, 0, 128},
 {12, 255, 0, 0},{13, 255, 255, 255},{14, 255, 255, 255},{15, 255, 255, 255}};

/* float   rdrcmap_6L[8][3]=
	{{0.0,0.0,0.0},{0.0,0.0,0.7},
	 {0.0,1.0,1.0},{1.0,1.0,0.0},
	 {0.0,0.8,0.0},{1.0,0.0,1.0},
	 {1.0,0.0,0.0},{1.0,1.0,1.0}};
*/
float   rdrcmap_6L[9][3]=
{{0.0,0.0,0.0},
 {0.0,0.0,0.5},{0.0,0.0,1.0},
 {0.0,1.0,1.0},{1.0,1.0,0.0},
 {0.0,1.,0.0},{1.0,0.0,1.0},
 {1.0,0.0,0.0},{1.0,1.0,1.0}};

cmapbasis   rdrcmapbasis_6L[9]=
{{0, 0, 0, 0},
 {1, 0, 0, 128},{24, 0, 0, 255},
 {56, 0, 255, 255},{78, 255, 255, 0},
 {88, 0, 255, 0},{98, 255, 0, 255},
 {110, 255, 0, 0},{140, 255, 255, 255}};

cmapbasis   satcmapbasis_1[9]=
{{0, 0, 0, 0},
 {1, 0, 0, 128},{96, 0, 0, 255},
 {140, 0, 255, 255},{170, 255, 255, 0},
 {190, 0, 255, 0},{210, 255, 0, 255},
 {235, 255, 0, 0},{250, 255, 255, 255}};

int greyscalebasis_points = 2;
cmapbasis   greyscalebasis[2]=
{{0, 0, 0, 0},
 {255, 255, 255, 255}};

int default_satcmapbasis_IR1_points = 9;
cmapbasis   default_satcmapbasis_IR1[9]=
{{0, 80, 14, 8},
 {40, 46, 19, 5}, 
 {85, 82, 95, 41}, 
 {90, 0, 29, 67},
 {105, 0, 76, 114},
 {115, 90, 90, 90},
 {180, 140, 140, 140},
 {220, 255, 255,255}, 
 {255, 190, 255, 255},};
cmapbasis *satcmapbasis_IR1 = default_satcmapbasis_IR1;
int satcmapbasis_IR1_points = default_satcmapbasis_IR1_points;

int default_satcmapbasis_IR2_points = 9;
cmapbasis   default_satcmapbasis_IR2[9]=
{{0, 80, 14, 8},
 {40, 46, 19, 5}, 
 {85, 82, 95, 41}, 
 {90, 0, 29, 67},
 {105, 0, 76, 114},
 {115, 90, 90, 90},
 {180, 140, 140, 140},
 {220, 255, 255,255}, 
 {255, 190, 255, 255}};
cmapbasis *satcmapbasis_IR2 = default_satcmapbasis_IR2;
int satcmapbasis_IR2_points = default_satcmapbasis_IR2_points;

int default_satcmapbasis_IR3_points = 9;
cmapbasis   default_satcmapbasis_IR3[9]=
{{0, 12, 14, 8},
 {180, 29, 80, 134}, 
 {200, 40, 100, 130}, 
 {205, 40, 100, 100}, 
 {215, 60, 130, 131},
 {225, 130, 130, 130},
 {235, 180, 180, 180},
 {245, 220, 220,220}, 
 {255, 190, 255, 255}};
cmapbasis *satcmapbasis_IR3 = default_satcmapbasis_IR3;
int satcmapbasis_IR3_points = default_satcmapbasis_IR3_points;


int default_satcmapbasis_Vis_points = 9;
cmapbasis   default_satcmapbasis_Vis[9]=
{{0, 0, 10, 10},
 {2, 0, 9, 80}, 
 {44, 0, 40, 120}, 
 {46, 23, 80, 0},
 {66, 73, 102, 0},
 {84, 113, 44, 0},
 {88, 80, 80, 80},
 {210, 255, 255,255}, 
 {255, 190, 255, 255}};
cmapbasis *satcmapbasis_Vis = default_satcmapbasis_Vis;
int satcmapbasis_Vis_points = default_satcmapbasis_Vis_points;

cmapbasis   rdrcmapbasis_6L8[8]=
{{0, 0, 0, 0},{24, 0, 0, 255},
 {56, 0, 255, 255},{78, 255, 255, 0},
 {88, 0, 255, 0},{98, 255, 0, 255},
 {110, 255, 0, 0},{140, 255, 255, 255}};

float   rdrcmap_6Lvl[8][3]=
{{0.0,0.0,0.0},{0.0,0.0,1.0},
 {0.0,1.0,1.0},{1.0,1.0,0.0},
 {0.0,1.,0.0},{1.0,0.0,1.0},
 {1.0,0.0,0.0},{1.0,1.0,1.0}};

float   velcmap[16][3]=
{{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,1.0},{0.0,0.0,0.83},
 {0.0,0.0,0.66},{0.0,0.0,0.5},{0.0,0.0,0.33},{0.0,0.0,0.16},
 {0.16,0.0,0.0},{0.33,0.0,0.0},{0.5,0.0,0.0},{0.66,0.0,0.0},
 {0.83,0.0,0.0},{1.0,1.0,1.0},{1.0,1.0,1.0},{1.0,1.0,1.0}};
	
float   velcmap1[8][3]=
{{0.0,0.0,0.0},{0.0,0.0,1.0},
 {0.0,0.0,0.66},{0.0,0.0,0.33},
 {0.33,0.0,0.0},{0.66,0.0,0.0},
 {1.0,0.0,0.0},{1.0,1.0,1.0}};

float   RdrPal[8][3]= 
{{0.0,0.0,0.0},{0.0,0.0,0.7},{0.0,1.0,1.0},{1.0,1.0,0.0},
 {0.0,0.8,0.0},{1.0,0.0,1.0},{1.0,0.0,0.0},{1.0,1.0,1.0}};

cmapbasis   VelPalBasis[8]= 
{{0, 0, 0, 0},{1, 0, 0, 100},{69, 128, 230, 255},
 {70, 200, 200, 200}, {71, 255, 255, 255}, {72, 200, 200, 200}, 
 {73, 255, 255, 0}, {141, 255, 0, 0}};

float   VelPal[7][3]= 
{{0.0,0.0,0.0},{0.0,0.0,0.4},{0.5,0.9,1.0},
 {1.0,1.0,1.0}, {1.0,1.0,1.0}, {1.0,1.0,0.0},{1.0,0.0,0.0}};

float   RdrPal2[8][3]= 
{{0.0,0.0,0.0},{0.0,0.2,0.2},{0.0,0.4,0.4},{0.0,0.5,0.5},
 {0.0,0.6,0.6},{0.0,0.7,0.7},{0.0,0.8,0.8},{0.0,1.0,1.0}};

/*float heightcmap[8][3]=
	{{0.0,0.0,0.6},{0.0,0.0,0.8},{0.0,0.4,0.0},{0.0,0.8,0.0},
	 {1.0,0.9,0.0},{0.5,0.2,0.0},{0.3,0.1,0.0},{0.8,0.8,0.8}}; */
float heightcmap16[16][3]=
{{0.0,0.0,0.0},{0.0,0.3,0.0},{0.0,0.4,0.0},{0.0,0.5,0.0},
 {0.0,0.6,0.0},{0.0,0.7,0.0},{0.5,0.7,0.0},{1.0,0.9,0.0},
 {0.7,0.5,0.0},{0.5,0.2,0.0},{0.4,0.2,0.0},{0.3,0.1,0.0},
 {0.35,0.15,0.2},{0.4,0.4,0.4},{0.7,0.7,0.7},{1.0,1.0,1.0}};

cmapbasis heightcmapbasis16[16]=
{{0, 0, 0, 0},{1, 0, 75, 0},{2, 0, 100, 0},{3, 0, 128, 0},
 {4, 0, 155, 0},{5, 0, 180, 0},{6, 128, 180, 0},{7, 255, 230, 0},
 {8, 180, 128, 0},{9, 128, 50, 0},{10, 100, 50, 0},{11, 75, 25, 0},
 {12, 87, 37, 50},{13, 100, 100, 100},{14, 180, 180, 180},{15, 255, 255, 255}};
float heightcmap[9][3]=
{{0.0,0.0,0.0},
 {0.0,0.2,0.0},{0,0.3,0.0},{0.0,0.5,0.0},{1.0,0.9,0.0},
 {0.5,0.2,0.0},{0.3,0.1,0.0},{0.4,0.4,0.4},{1.0,1.0,1.0}};

cmapbasis heightcmapbasis[9]=
{{0, 0, 0, 0},
 {1, 0, 50, 0},{20, 0, 75, 0},{40, 0, 128, 0},{60, 255, 230, 0},
 {80, 128, 50, 0},{100, 75, 25, 0},{120, 100, 100, 100},{140, 255, 255, 255}};

cmapbasis terrainhtcmapbasis[7]=
{{0, 0, 0, 100},
 {1, 255, 187, 114},{20, 255, 171, 81},{40, 139, 255, 94},{60, 64, 188, 16},
 {100, 100, 100, 100},{220, 255, 255, 255}};

// accumcmapbasis for 32 level accum level table
cmapbasis   accumcmapbasis_32[15]=
{{0, 0, 0, 0},     
 {1, 122, 91, 29},    //   0.2mm
 {3, 247, 192, 92},   //   1.0mm
 {5, 236, 155, 5},    //   4.00mm
 {7, 251, 245, 46},   //  10.0mm
 {10, 182, 247, 67},  //  24.0mm
 {13, 89, 255, 66},   //  50.0mm
 {16, 85, 213, 167},  // 100.0mm
 {19, 59, 155, 225},  // 180.0mm
 {21, 78, 54, 225},   // 280.0mm
 {22, 152, 24, 197},  // 350.0mm
 {24, 237, 0, 214},   // 600.0mm
 {25, 225, 112, 112}, // 800.0mm
 {28, 222, 20, 20},   //1700.0mm 
 {31, 255, 255, 255}};//4000.0mm

// accumcmapbasis for 96 level accum level table
// refer to STDAccumThresh24 for thresholds (must multiply by 4 for indexes here)
cmapbasis   accumcmapbasis_96[15]=
{{0, 24, 36, 36}, 
 {1, 122, 91, 29},      //   0.1mm  
 {8, 247, 192, 92},     //   1.0mm
 {14, 236, 155, 5},     //   3.0mm 
 {24, 251, 245, 46},    //   8.0mm
 {32, 182, 247, 67},    //  14.0mm
 {38, 89, 255, 66},     //  24.0mm
 {48, 85, 213, 167},    //  48.0mm
 {56, 59, 155, 225},    //  80.0mm
 {64, 78, 54, 225},     // 140.0mm
 {72, 152, 24, 197},    // 240.0mm
 {80, 237, 0, 214},     // 400.0mm
 {85, 225, 112, 112},   // 700.0mm
 {90, 222, 20, 20},     // 1200.0mm
 {95, 255, 255, 255}};  // >1700.0mm

char cmapfname[128] = "default.cmap";
char *cmap_type_str[] = 
{"NOCMAPTYPE", "Refl24bit", "Refl8bit", "Vel24bit", "Vel8bit", "Height24bit", "Height8bit", 
 "VIL24bit", "VIL8bit", "Accum24bit", "Accum8bit", "TerrainHtMap",
 "Sat24bit_GMSIR1", "Sat24bit_GMSIR2", "Sat24bit_GMSIR3", 
 "Sat24bit_GMSVis" }; 
     
float	blackvec[3] = {0.0,0.0,0.0};
float	whitevec[3] = {1.0,1.0,1.0};
float	OlayC[3]    = {0.0,0.7,0.0};
int	OLayCol     = 60;
// float   MapC[3]     = {0.0,0.7,0.0};
// int     MapCol     = 60;
// float   MapC[3]     = {0.5,0.15,0.25};
float   MapC[3]     = {1.0,1.0,1.0};
// int     MapCol     = 75;
// int MapCol = 7
// float   RingC[3]     = {0.4,0.4,0.4};
// int     RingCol     = 39;
float   RingC[3]     = {0.0,0.7,0.0};
// int     RingCol     = 61;
float   RHIGridC[3]     = {0.4,0.4,0.4};
// int     RHIGridCol     = 39;
// float   RHI3DGridC[3]     = {0.0,0.0,1.0};
// int     RHI3DGridCol     = 227;
float   RHI3DGridC[3]     = {1.0,1.0,0.0};
// int     RHI3DGridCol     = 3;
/*
int	BkGndCol = 0;
int	BkGndIncomplete = 9;
*/
// const float pi = M_PI;
const float DEG2RAD = M_PI/180.0;
const float RDRANG2RAD = M_PI/1800.0;
const float RAD2DEG = 180.0/M_PI;
const float HALF_PI = M_PI * 0.5;
const float EARTH_RAD = 6371.64;
const float EARTH_RAD_4_3 = 8495.52;
const float ECC_EARTHRAD43 = 0.5 / 8495.52; // consts for calc Earth
const float ECC_EARTHRAD = 0.5 / 6371.64;     // Curvature Correction
const float ReSqr = 72173860.;
// const float BEAM_CORR = (0.5/8495.5) - (0.5/6371.64); // Beam position corr
const float BEAM_REFR = (0.5/8495.5) - (0.5/6371.64); // Beam position corr

// float	AzCorrection = 700; // rdr_angle rotation kludge


const int JulDay1Jan70 = 2440601;   // julian day of Unix Day 1(1/1/70)
const int SecsPerDay = 86400;       // seconds per day

// struct timezone	tmzone = {0,0};

char *true_false_text[] = {
  "FALSE","TRUE"
};

char RapicTitle[64] = "3D-Rapic Version";
char RapicTitle1[64] = "3D-Rapic Version";
char RapicTitle2[64] = "Australian Bureau of Meteorology";
unsigned char AutoDBPreview = TRUE;
unsigned char DBMultiStnLoad = TRUE;
char PrinterCMapFName[] = "printer.cmap";

bool DebugMode = FALSE;

int LocalTime = 0;

char *cursorval1Labelstrings[] = {
  "Undef", "Refl", "RADLVel", "SpectW", "DiffZ", "Ht", 
  "VIL", "RRate Accum", "degK", "", "WV", "Blend",
  "RainRate", "RainAccum"
};
     
char *cursorval1Unitstrings[] = {
  "undef", "dBZ", "m/s", "", "dBZ", "m", 
  "kg/m^2", "mm acc", "degK", "", "WV", "Blend",
  "mm/hr", "mm"
};
     
char *cursorval2Labelstrings[] = {
  "undef", "RRate", "Nyquist", "SpectW", "DiffZ", "MindBZ", 
  "Max VIL", "RRate/Hr", "val", "val", "val", "val",
  "Max Val", "Max Val"
};
     
char *cursorval2Unitstrings[] = {
  "undef", "mm/hr", "m/s", "", "dBZ", "dBZ", 
  "kg/m^2", "mm/hr av", "val", "val", "val", "val",
  "mm/hr", "mm"
};
     
