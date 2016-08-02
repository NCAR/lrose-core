#ifndef __PALETTE_H
#define __PALETTE_H


#include <stdio.h>

class RGBA {
 public:
  uchar val[4];
  RGBA() { setRGBA(255,255,255,255); };
  RGBA(uchar r, uchar g, uchar b, uchar a = 255) 
    { 
      setRGBA(r,g,b,a); 
    };
  RGBA(RGBA &initcolor) 
    { 
      setRGBA(initcolor); 
    };
  uchar r() { return val[0]; };
  uchar g() { return val[1]; };
  uchar b() { return val[2]; };
  uchar a() { return val[3]; };
  void setRGBA(uchar r, uchar g, uchar b, uchar a = 255)
    {
      val[0] = r; val[1] = g; val[2] = b; val[3] = a;
    };
  void setRGB(uchar r, uchar g, uchar b)
    {
      val[0] = r; val[1] = g; val[2] = b; val[3] = 255;
    };
  void setRGBA(const RGBA &initcolor)
    {
      for (int i = 0; i < 4; i++)
	val[i] = initcolor.val[i];
    };
  void setRGBAv(uchar *rgba) // MUST BE 4 array elements
    {
      val[0] = rgba[0]; val[1] = rgba[1]; val[2] = rgba[2]; val[3] = rgba[3];
    };
  void setRGBv(uchar *rgb)
    {
      val[0] = rgb[0]; val[1] = rgb[1]; val[2] = rgb[2]; val[3] = 255;
    };
  void getRGBA(uchar &r, uchar &g, uchar &b, uchar &a)
    {
      r = val[0]; g = val[1]; b = val[2]; a = val[3];
    };
  void getRGB(uchar &r, uchar &g, uchar &b)
    {
      r = val[0]; g = val[1]; b = val[2];;
    };
  uchar *getRGBAv() { return val; };
  // interpolate btwn this color and other - into this
  // otherfactor<=0.0 all thiscolor
  // otherfactor >= 1.0 all othercolor
  void interp(RGBA &othercolor, float otherfactor);
};

enum palTypes
  { PAL_UNDEFINED, PAL_COMPOSITE, PAL_REFL, PAL_VEL, PAL_HT, PAL_VIL, PAL_ACCUM, 
    PAL_CUSTOM, PAL_TERRAINHT, PAL_LAST};
// if a pal type is added here, add before PAL_LAST and add string to palTypeStrings in palette.C
    
char* getPalTypeString(palTypes paltype);

// not used
struct RGBPalParams
{
  RGBPalParams(int baseindex = 0, int topindex = 1,
	       float baseval = 0, float topval = 1.0)
  {
    base_index = baseindex;
    top_index = topindex;
    base_val = baseval;
    top_val = topval;
    if (top_index - base_index != 0)
      dval_per_unit = (top_val - base_val) / (top_index - base_index + 1);
  };
  int base_index, top_index; // base and top values for this palette
  float base_val, top_val; // base and top values for this palette
  float dval_per_unit;     
};  


class RGBPalette {
 public:
  palTypes pal_type;
  int pal_size;            // number of entries in palette
  int pal_alloc;           // space allocated for this palette
  float base_val, top_val; // base and top values for this palette
  float dval_per_unit;       
  float indexVal(int index); // get val at index
  int valIndex(float val);   // get index for val
  int   palMark1, palMarkStep, palMarkSteps; // index vals of palette marker locations
  void setPalMarkerPoints(int mark1, int markstep, int markcount)
    {
      palMark1 = mark1; palMarkStep = markstep; palMarkSteps = markcount;
    };
  uchar *rgba_palette;     // store all as rgba
  uchar *atten_palette(float attenval = 1.0);
  uchar *atten_rgba_palette;
  RGBA padval;
  float rgbAtten;
  void setRGBAtten(float attenval);
  void clearRGBAtten();
  float getRGBAtten() { return rgbAtten; };
  uchar nullval[4];        // null palette entry to use if no palette exists
  uchar attenval[4];       // attenuated palette entry to use if atten != 1.0
  char Title[32];          // short title of this pal
  void setTitle(char *newTitle);
  bool globalPalette;   // if this is set, palette should not be deleted by users
  void setRGBv(int index, uchar *rgb);  // set value at index to values at rgb 
  uchar *getRGBv(int index);  // get pointer to rgba array at index
  void copyRGBv(int index, uchar *dest);  // copy rgb vals to dest
  void setRGBAv(int index, uchar *rgba);  // set value at index to values at rgb
  uchar *getRGBAv(int index);  // get pointer to rgba array at index
  void copyRGBAv(int index, uchar *dest);  // // copy rgba vals to dest
  void setRGB(int index, uchar r, uchar g, uchar b);
  void getRGB(int index, uchar &r, uchar &g, uchar &b);
  void setRGBA(int index, uchar r, uchar g, uchar b, uchar a);
  void getRGBA(int index, uchar &r, uchar &g, uchar &b, uchar &a);
  void setA(int index, uchar a);
  void getA(int index, uchar &a);
  uchar getA(int index);

  void delPal();
  void clearPal(uchar r = 0, uchar g = 0, uchar b = 0, uchar a = 0);
  void setPalSize(int size);     // resize palette, recalc increments
  void padPalSize(int padsize);  // resize allocation to padsize, 
                                 // don't recalc increments
                                 // pad from pal_size to pal_alloc with padval
  void padPalette(int padfromidx = -1);
  void setPadRGBA(uchar r, uchar g, uchar b, uchar a = 255)
    {
      padval.setRGBA(r, g, b, a);
    };
  int  Size() { return pal_size; };

  void getValRGBA(float val, uchar &r, uchar &g, uchar &b, uchar &a);
  void getValRGB(float val, uchar &r, uchar &g, uchar &b);

  void setPalType(palTypes paltype) { pal_type = paltype; };
  void setBaseTop(float pbase, float ptop);
  void newVals();
  
  void init(palTypes paltype,
	     float baseval, float topval, 
	     int size,
	    char *title = 0);

  RGBPalette(palTypes paltype,
	     float baseval, float topval, 
	     int size,
	     char *title = 0);
  RGBPalette();
  ~RGBPalette();
  void dump(FILE *dumpfile);
};


extern RGBPalette RGBPal;            // contains all palettes
extern int        RGBPalSize;
extern bool       rgbaMode;
extern bool       useGlobalRGBPal;
extern RGBPalette RGBPal_Refl;
extern RGBPalette RGBPal_Vel;
extern RGBPalette RGBPal_Height;
extern RGBPalette RGBPal_Accum;
extern RGBPalette RGBPal_VIL;
extern RGBPalette RGBPal_TerrainHt;
extern RGBPalette RGBPal_Std;



struct cmapbasis {
    short	    index;
    short	    r, g, b;
};

class CMapBasisPalette {
public:
    CMapBasisPalette(int numpoints, cmapbasis *initarray = 0);
    ~CMapBasisPalette();
    void	SetCMapArray(unsigned char outRGBarray[][3]);
    void	SetCMapArray(RGBPalette *outRGBPal);
    int		NumPoints;
    cmapbasis	*BasisArray;
};

struct cmapentry {
    int	    index;
    int	    r, g, b, a;
};

struct StdColorTbl {
    int numentries;
    cmapentry *entryarray;
};

extern float	blackvec[3];
extern float   whitevec[3];
extern float   OlayC[3];
extern int	OLayCol;
extern float	MapC[3];
extern float	RingC[3];
extern float	RHIGridC[3];
extern float	RHI3DGridC[3];
extern float   rdrcmap_16L[][3];
extern float   rdrcmap_6L[][3];
extern float   rdrcmap_6Lvl[][3];
extern float   heightcmap[][3];
extern float   heightcmap16[][3];
extern float   velcmap[][3];
extern float   velcmap1[][3];
extern float   VelPal[][3];




/* DEFAULT 24bit display color nap variables */

extern int    zmapsize;
extern int    zmapbase;
extern int    vmapsize;
extern int    vmapbase;
extern float  vmap_nyquist;
extern bool   vmap_absolute;  // if true use absolute vmap, default is nyquist
extern bool   draw_absolute_vmap;  // allow switch drawing btwn abs and scan nyquist
extern bool   drawAbsVmap();  // allow switch drawing btwn abs and scan nyquist
extern float  vmap_abs_nyquist;  // nyquist to use for absolute vel pal
extern int    htmapsize;
extern int    htmapbase;
extern int    htmapoflow;
extern int    vilmapsize;
extern int    vilmapbase;
extern int    vilmapoflow;
extern int    accummapbase;
extern int    accummapsize;
extern int    terrainhtmapsize;

extern int    CMapBase;
extern int    BGNormal;
extern int    BGIncomplete;
extern int    BGUndefined;
extern int    BGCovered1;
extern int    BGCovered2;
extern int    PalIndColor;
extern int    AngleIndColor;
extern int    DataTextColor;
extern int    ImageTitleColor;
extern bool   ImageTitleBGSolid;
extern int    MapCol;
extern int    RingCol;
extern int    RHIGridCol;
extern int    RHI3DGridCol;
extern int    LatLongCol;
extern int    titanCurrentShapeCol;
extern int    titanPastShapeCol;
extern int    titanFcstShapeCol;
extern int    titanTrackCol;
extern int    titanFcastTrackCol;
extern int    annotCol;
extern int    worldCoreCol;

void setStdColorEnumerators(int cmapbase = -1);

extern int    RdrZCMapBase;
extern int    RdrVCMapBase;
extern int    RdrHtMapBase;
extern int    RdrHtMapOverflow;
extern int    RdrVILMapBase;
extern int    RdrVILMapOverflow;
extern int    RdrAccumMapBase;
extern int    TerrainHtMapBase;
extern int    CMapTop;

/* 8bit display color nap variables */
extern bool Displ8Bit;

extern int    zmapsize8_16;
extern int    zmapsize8_6;
extern int    vmapsize8;
extern int    htmapsize8;
extern int    vilmapsize8;
extern int    accummapsize8;

extern int    CMapBase8;
extern int    BGNormal8;
extern int    BGIncomplete8;
extern int    BGUndefined8;
extern int    PalIndColor8;
extern int    AngleIndColor8;
extern int    DataTextColor8;
extern int    MapCol8;
extern int    RingCol8;
extern int    RHIGridCol8;
extern int    RHI3DGridCol8;

extern int    RdrZCMapBase8_16D;    // Double-buffer 16lvl Z colors
extern int    RdrZCMapBase8_16;
extern int    RdrZCMapBase8_6;
extern int    RdrVCMapBase8;
extern int    RdrHtMapBase8;
extern int    RdrHtMapOverflow8;
extern int    RdrVILMapBase8;
extern int    RdrVILMapOverflow8;
extern int    RdrAccumMapBase8;
extern int    CMapTop8;

extern int		dBZOffset;			// 0.5 dbZ step offset of dBZ (e.g. 20 gives -10dBZ base)
extern float 	MAXDBZ;
extern float 	MINDBZ;


extern char		HtTbl6L[];

extern float 	Dflt_dBZThresh6Lvl[];
extern float 	Dflt_dBZThresh16Lvl[];
// extern float	STDAccumThresh32[];
extern float	STDAccumThresh24[];
extern char   dBZCMapBasis[];
extern char   HtCMapBasis[];
extern char   VCMapBasis[];

extern cmapbasis rdrcmapbasis_6L[];	// 6level basis for 24bit
extern cmapbasis rdrcmapbasis_6L8[];	// 6level basis for 8bit
extern cmapbasis VelPalBasis[];
extern cmapbasis heightcmapbasis[];
extern cmapbasis heightcmapbasis16[];
extern cmapbasis rdrcmapbasis_16L[];
extern cmapbasis accumcmapbasis_32[];
extern cmapbasis accumcmapbasis_96[];
extern cmapbasis terrainhtcmapbasis[];

extern cmapbasis greyscalebasis[];	
extern int greyscalebasis_points;

extern cmapbasis *satcmapbasis_IR1;	
extern int satcmapbasis_IR1_points;
extern cmapbasis *satcmapbasis_IR2;	
extern int satcmapbasis_IR2_points;
extern cmapbasis *satcmapbasis_IR3;	
extern int satcmapbasis_IR3_points;
extern cmapbasis *satcmapbasis_Vis;	
extern int satcmapbasis_Vis_points;

extern cmapbasis default_satcmapbasis_IR1[];	
extern int default_satcmapbasis_IR1_points;
extern cmapbasis default_satcmapbasis_IR2[];	
extern int default_satcmapbasis_IR2_points;
extern cmapbasis default_satcmapbasis_IR3[];	
extern int default_satcmapbasis_IR3_points;
extern cmapbasis default_satcmapbasis_Vis[];	
extern int default_satcmapbasis_Vis_points;

extern char cmapfname[];

enum	cmap_enum {NOCMAPTYPE, ZMAP, ZMAP8, VMAP, VMAP8, 
		   HTMAP, HTMAP8, VILMAP, VILMAP8, ACCUMMAP, ACCUMMAP8, 
		   TERRAINHTMAP,
		   SATMAP_GMSIR1, SATMAP_GMSIR2, SATMAP_GMSIR3, SATMAP_GMSVIS
    };
extern	char *cmap_type_str[];
  
void NewStdColor(cmapentry *StdColorMap, int entryno, 
		 int r, int g, int b, int a = 255);
void SetStdColorAlpha(cmapentry *StdColorMap, int entryno, int a);
void SetStdColorAlpha(int entryno, int a);

extern cmapentry StdColors[];
extern StdColorTbl StdColorTable;
extern StdColorTbl StdColorTable8;

void InitCMaps(char *FName = 0);
void SetCMap(float BasisColors[][3], char BasisPoints[], int NumBasisPoints,int mapbase, int mapsize);
void SetCMapBasis(cmapbasis BasisArray[], int NumBasisPoints,int mapbase, int mapsize);
// convert basis array to unsigned char RGB array, ie outRGBarray[outRGBsize][3]
void SetCharCMapArrayFromBasis(cmapbasis BasisArray[], int NumBasisPoints, int outRGBsize, unsigned char outRGBarray[][3]);
void SetRGBPaletteFromBasis(cmapbasis BasisArray[], int NumBasisPoints, RGBPalette *rgbpal, 
			    palTypes paltype, float baseval, float topval,
			    int mapbase, int mapsize, 
			    bool reset); 
// if newmapsize is < 0, the pal will be used as is

void SetStdColors(StdColorTbl *colortable = 0);

void SetEnumCMap(cmap_enum type, cmapbasis *BasisArray, int NumBasisPoints);
void SetHtMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetHtMap8(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetZMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetZMap8(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetVMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetVMap8(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetVILMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetVILMap8(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetAccumMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetAccumMap8(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetTerrainHtMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetGMSIR1Map(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetGMSIR2Map(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetGMSIR3Map(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void SetGMSVisMap(cmapbasis *BasisArray = 0, int NumBasisPoints = 0);
void RelabelVPal();

extern cmapbasis *GMSIR1CMapBasis;
extern cmapbasis *GMSIR2CMapBasis;
extern cmapbasis *GMSIR3CMapBasis;
extern cmapbasis *GMSVisCMapBasis;

/* return the cmap type enumeration corresponding to the given string */
cmap_enum GetCMapType(char *str);
extern char PrinterCMapFName[];


#endif	/* __PALETTE_H */
