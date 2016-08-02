#ifndef	__MAPS_H
#define __MAPS_H

/*
 *
 *	maps.h
 *
 */

#include "displ.h"
#include "latlong.h"
// #define Windows

#include "siteinfo.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifndef Windows
#include "rdr.h"
#include "rdrutils.h"
#endif

#ifdef Windows
// #include <winnt.h>
#include <mem.h>
#include <owl\dc.h>
#include <io.h>
#define FALSE 0
#define TRUE 1
typedef int bool;
#endif

#include "graphutils.h"
#include "draw.h"

#ifdef STDCPPHEADERS
#include <string>
using namespace std;
#else
#include <string.h>
#include <bitset.h>
#endif

enum MapRng { MR_NOMAP = 0, MR_GT2048, MR_GT1024, MR_GT512, MR_GT256, 
    MR_GT128, MR_GT64, MR_GT32, MR_GT16};

enum	RpMapRng {RpMap128km,RpMap256km,RpMap512km};

// Could use bits masked by 0x1e00 for map "layer" id
// ie bits9-12 inclusive, maybe use 9,10 for 4 layers and leave
// other 2 (11,12) for something later, possibly more layers
#define MapLayers 16
#define MapLayerMask 0x1e00
#define MapDfltAddLayer MapLayers - 1

enum MapEditMode {Pick, PickandDrag, Draw};

//	MAP STRINGS > 31 chars in length ARE DANGEROUS, AVOID

struct rpmap_vect {
	short	XVect,YVect;
	char	str[32];
	void 	Clear();
	int 	Size();
	void 	SetText(char *text);
	bool	CopyText(char *DestStr);	// copy the text string to dest.
	int		TextLength();
	bool StrCmp(char *text);
	void	SetXY(short xpos, short ypos);
	void	GetXY(short &xpos, short &ypos);
	void	SetLayer(int layer);
	void	SetPenDown(bool flag);
	int 	GetLayer();
	void	SetBase(rpmap_vect *newbase);	// set the XVect &YVect to newbase
	inline bool IsMapEnd();
	inline bool IsText();
	inline bool IsPenDown();
	bool IsSame(rpmap_vect *dup);
	inline void Copy(rpmap_vect *Src);
	};

#define MaxMapSize 0x4000		// size of total RawMap
#define EditMapSize 0x4000	// size of each indiv edit map
#define maxmapstrlen 31
// if size changed, check LoadMap for appropriate size load and masks

// RpRawMap suits direct read of PCRapic format maps
// RawMapStnID is an add-on, if lowest mapofs = 8, assume RawMapStnID is defined
// When loaded the lowest MapOfs will the be 2, ie RawMapStnID is 1st 2 bytes of MapData

struct RpRawMap {
	unsigned short int MapOfs[3];	// 0-125km map ofs, 1-256km map ofs 2-512km mapofs
	union {
		unsigned short int MapStnID;	// do not use this routinely, may not be defined
		char	MapData[0xc000];	// actual Map data NOTE: ALLOW 48k buffer
		};
	unsigned int RawMapStnID; 		// 0 if undefined, bytes 6&7 if lowest mapofs >= 8
	int		MapSizes[3];	// size in bytes of each rng map
	unsigned int TotalSize;		// valid data size (Sz of file read)
	bool			MapAvail;
	RpRawMap();
	~RpRawMap();
	void New();
	void Clear(int rng);
	};

extern int MapDefaultColor,MapHiliteColor;

struct rpmap_prop {
	bool Switch;				// if true display this map
	int			MapColIndex;  // MapColor Index
	int			Thickness;		// Map Line Thickness
  bool DrawToBMP;		// if true draw this layer to default plane when drawing BMP image
	// line pattern could live here
// font properties could also live here
	};

class rpmap {
//	rpmap_vect	ThisVect; 		// vector pnts
//	int		CharOfs;			// byte offset into MapData
	friend class rpmap_edit;
	RpRawMap	*RawMap;			// raw map data
	bool		MapError;			// true if map invalid
	rpmap_vect	*ThisVect,    // vector pnts
			*PrevVect;		// use prevvect with caution, not always valid
	char		*BuffPnt,*RawMapBase;	// pointer to map buffer array
	float		PrevX,PrevY,PrevZ,x,y,z;
	bool		PenDown;
	bool		MapEnd;
	char		MapTextStr[32];
	int		MapTextSize;
	int		Layer;
	bool		LayerSwitch;
	rpmap_prop	LayerProp[MapLayers];		// properties of each layer

	LatLongConvert	LatLongCnvt;
	bool		LatPosSouth;
	virtual	void	SetLayerProps();
	virtual void	NewProps(int NewLayer = 0);
	float		MapScale;			// integer unit to kms converter
	bool		LineOpen;
	bool		CheckBuffLimits;
	virtual void	Reset(int rng=-1);
	virtual void 	EndVect(int rng=-1);			// ThisVEct will point to MapEnd vect
	virtual void 	LastVect(int rng=-1);			// ThisVEct will point to last point vector
	virtual int	GetMapSize(int rng);			// calc size of given rng map
	virtual int 	GetMapSize(char *MapPnt);	// calc the size of a given map
	virtual void	GetMapSizes(RpRawMap *InRawMap = 0);	// Get sizes of the 3 maps
	virtual void	StepVect();
	virtual void	NewVect();
	virtual void	DrawVect();
#ifdef Windows
	TDC	*MapTDC;
	TPen	*MapPen;
#endif
public:
	FONTHANDLE font;
	char	fontname[128];
	double	MapFontScale;
	int	MapLnW,GridLnW;
	bool	TextFlag,HideText;
	bool    ThickMaps;
	bool    antiAlias;
	char 	MapName[128];	// Name of Map file
	int	range,station;
	int	YTextOfs256,YTextOfs512;	// offsets for different text align
	virtual void kmNE2XY(float kmN, float kmE, int &x, int &y);
	virtual void XY2kmNE(int x, int y, float &kmN, float &kmE);	// x,y relative to origin ie +/- 256
	virtual bool kmNE2LatLng(float kmN, float kmE, float &rLat, float &rLong);
	virtual bool LatLng2kmNE(float lat, float lng, float &kmN, float &kmE);
	virtual int ThisMapSize();
	virtual int TotalMapSize();
	// return false if OriginLatLong not defined
	rpmap();
	virtual ~rpmap();
	virtual bool 		LoadMap(int stn);
	virtual bool 		LoadMap(char *mapname = 0);
#ifdef Windows
	virtual void SetTDC(TDC *MAPTDC);
	int  HalfScale;
#endif
	virtual void	DrawMap(int rng, int stn, int ZoomFactor);
	virtual void	DrawMap(int rng, char* mapname, int ZoomFactor);
	virtual void 	DrawMap(int rng, int ZoomFactor);
	bool 			BMPMapDraw;
//	void	DrawMap();
#ifndef Windows
	virtual FONTHANDLE SetFont(FONTHANDLE fh = 0);
	virtual FONTHANDLE ScaleFont(double scale = 0, FONTHANDLE fh = 0);
	virtual void MapStr(float x, float y, float z, char *MpStr);
	virtual void MapStr(char *MpStr);
	virtual void draw_RHI_grid(float ViewHt, float rng);
	virtual void draw_3DRHI_grid(float ViewHt,rdr_angle az);
#endif
	virtual void drawrngrings(float NorthOfs,float EastOfs,float RngInc,int NumRings);
	};


// rpmap constructor allocates and assign RpRawMap
// rpmap_edit should move it to OrigMap in the UNPack phase
// and use a RawEditMap instead

enum UndoEditType {Delete, Insert, ChgBase /*ChangeText*/};

class UndoEntry {

	friend class UndoStack;
	friend class rpmap_edit;

	rpmap_vect Vect1;				// deleted vector
	rpmap_vect *Vect1Pos;		// position in map
/*
	rpmap_vect Vect2;				// inserted vector
	rpmap_vect *Vect2Pos;		// position in map
*/
	UndoEditType Type;			// type of edit
	int				 UndoRng;			// rng undo data refers to
	UndoEntry *next,*prev;	// linked list pointers
	};

class UndoStack {

	friend class rpmap_edit;

	int UndoEntries,MaxEntries;
	UndoEntry *head,*tail;	// stack pointers
	UndoEntry *listbase;		// pointer to first entry
	UndoStack(int depth);		// construct UndoStack
	~UndoStack();
	bool	Disabled;
	void Disable() {Disabled = TRUE;};
	void Enable() {Disabled = FALSE;};
	void Clear();
	void DelEntry();
	void NextEntry();				// set head to next in stack prior to adding an undo
	void Del(rpmap_vect *DelVect, int EditRng);
	void Ins(rpmap_vect *InsVect, int EditRng);
	void ChangeBase(rpmap_vect *ChgVect, int EditRng);
//	void ChangeText(rpmap_vect *OldVect, rpmap_vect *NewVect, int EditRng);
	};

class rpmap_edit : public rpmap {
	RpRawMap	*OrigRawMap;		// pointer to original map
	RpRawMap	*EditMap;			// pointer to edit map buffer
	int		EditRng;        		// map rng currently being edited
	int		EditZoom;						// current zoom factor
	rpmap_vect EditVect;			// Vector for modification
	rpmap_vect *PickedVect;		// 0 for none
	float MaxPickRng;					// max range for pick
	virtual void	DrawVect();				 // allow highlight of PickedVect
	UndoStack *UndoStk;				//	undo stack
public:
	bool EditPenDown;
	int 	ImportRes;				// pixel resolution for data import filter
	virtual void	DrawMap(int rng, int stn, int ZoomFactor);	// intercept the DrawMaps to
	virtual void	DrawMap(int rng, char *mapname, int ZoomFactor);	// avoid stn & range change
	virtual void	DrawMap(int rng, int ZoomFactor);
	rpmap_edit(char *MapNm=0,int rng=1, int ZoomFactor = 1);
	virtual ~rpmap_edit();					// ~rpmap will dealloc RawMap, must dealloc OrigMap
	virtual int PickVect(float x,float y); // pick the vect closest to x,y
	virtual void UnPickVect();			// unpick vector
	virtual void PickFirst();				// pick first in map
	virtual void PickLast();				// pick last in map
	virtual void StepVectFwd();			// step pickedvect fwd
	virtual void StepVectBwd();			// step pickedvect bwd
	virtual void HiliteVect();			// put a symbol around the pickedvect
	virtual void	SetEditRng(int editrng);
	virtual void	SetZoomFactor(int zoomfactor);
	virtual void SetStation(int stn);
	virtual int GetStation();
	virtual bool 		LoadMap(int stn);
	virtual bool LoadMap(char *MapNm = 0);
	virtual void	SaveMap(char *MapNm=0);
	virtual void	UnPackRaw();       // unpack the contiguous maps into 3 separate buffs
	virtual void	PackRaw();			//	repack the 3 map into contiguous
	virtual bool InsertVect(rpmap_vect *InsVect = 0, rpmap_vect *DestPos = 0, bool AfterDest = FALSE);
				// by default insert EditVect before PickedVect
	virtual bool InsertVect(int x, int y, char *text=0);
				// insert vect after picked, if no text, add pen down point, picked = new
				// if no picked, append new line segment, picked at new point
	virtual void DelVect(rpmap_vect *DelPos = 0);  // delete given vect, PickedVect if not defined
//	virtual void	AddPoint(int xpos, int ypos);	// if not EditPenDown assume new segment
					// Append to this map
					// if EditPenDown, insert after ThisVect
	virtual void	AddText(rpmap_vect *newtext=0);	// add this text
	virtual void	AddText(int xpos, int ypos, char *text);	// add this text
	virtual void 	MovePicked(int NewX, int NewY);
	virtual void 	SetPickedPenDown(bool pendown);
	virtual void 	SetPickedLayer(int newlayer);
	virtual void	SetBase(rpmap_vect *newbase, rpmap_vect *vectpos);	// set XVect &YVect of vectpos to newbase
//	virtual void 	SetSegmentLayer(int newlayer);
	virtual void	ChangePickedText(char *NewText); // replace Old with New
	virtual bool PickText(char *PickStr);		// true if FindStr found. Sets PickedVect
	virtual bool IsDup(rpmap_vect *dupvect); // true if this vect is present
	virtual void	DeleteText(char *text);				// delete given text
	virtual bool  CopyPickedText(char *pickedtext);	// copy text to pickedtext, return FALSE if not picked text
	virtual void DisableUndo();		// disable for floods of events e.g. drag
	virtual void EnableUndo();
	virtual bool Undo();		// undo last edit, false if no last edit
	virtual int UndoCount();
	virtual int MaxUndoCount();
	virtual void PickedVectString(char *outstr);
	bool NoDups;
	virtual int ImportLatLong(char *fname);	// import lat long [text] lines of data, return no of vectors read
	virtual int ExportLatLong(char *fname);	// export lat long [text] lines of data, return no of vectors written
	// if not 1st 2 elements numeric, assume pen-up
	// if no 3rd element, or 1st char of 3rd element numeris, assume point then pen-down
	// if 1st char of 3rd element is alpha, assume text, pen-up
	void	 ClearRng(int rng = -1);		// clear given rng, current rng if not defined
#ifdef Windows
	TPen *HilitePen;
#endif
	};

struct bswp {
	union {
		short int sh1;
		char b[2];
		};
	};

void LoadMap(char *MapNm = 0);
void InitMap();
void CloseMap();
void InitEditMap();
void draw_rapic_map(int rng, int stn, int ZoomFactor = 1);
void draw_rapic_map_bmp(int rng, int stn, int ZoomFactor = 1);
void draw_rapic_map(int rng, char *mapname, int ZoomFactor = 1);
extern rpmap *TestMap;
extern rpmap_edit *TestEditMap;
#ifdef Windows
void SetTDC(TDC *MAPTDC);
void SetHalfScale(int Flag);
#endif

void draw_RHI_grid(float ViewHt, float rng);
void draw_3DRHI_grid(float ViewHt,rdr_angle az);
void draw_Underlay(float ViewRng, int stn, bool OlayBG);
void draw_UnderlayCoast(float ViewRng, int stn, OverlayProperties *OlayProps);
void draw_UnderlayRings(float ViewRng, OverlayProperties *OlayProps);

int drawGlASCIILatLongMap(char *mapname, OverlayProperties *OlayProps = 0, bool SwapLatLongOrder = FALSE);
#ifdef IRIS_GL
GL_Object createGlASCIILatLongMapObj(char *mapname, OverlayProperties *OlayProps = 0, bool SwapLatLongOrder = FALSE);
#endif
#ifdef OPEN_GL
GLuint createGlASCIILatLongMapObj(char *mapname, OverlayProperties *OlayProps = 0, bool SwapLatLongOrder = FALSE);
#endif
void drawDefaultASCIILatLongMap(char *mapname = 0, OverlayProperties *OlayProps = 0, bool SwapLatLongOrder = TRUE);
extern bool defaultASCIILatLongMapExists;
MapRng asciiLatLongMapExists(int stnid, char *mapname, MapRng maprng);

struct covObjDlistRng {
  GLuint dlist1, dlist2;
  float maxRng;
  covObjDlistRng()
  {
    dlist1 = dlist2 = 0;
    maxRng = 0;
  };
};

class rdrCovObj {
 public:
  list<covObjDlistRng> dlistRng;
  int stn;
  rdrCovObj(int covstn);
  // getDlist will return the dlist for the matching range if it exists
  // or will create a dlist for the requested range if it doesn't
  GLuint getDlist(float covmaxrng, bool redraw = false);
  void doRender(GLuint dl1, float covmaxrng, bool blend = false);
  ~rdrCovObj();
};

/*
  A Global RpMapMng object will load and manage all maps for use within 
  3DRapic.
  Common maps may be used for multiple radars.
  Symbolic links or copies may be used. 
  The label of a newly read map will be checked against existing maps and if it matches
  the newly loaded map will be deleted and the previously load same map referenced, and
  its refCount will be incremented.
*/

/*
 A DrawDataVector object is created for the vector data of each map loaded and
 appended to the latLongVectorList
 vector<DrawDataList*> projVectorLists will return a projected version of latLongVectorList, 
 creating it if it doesn't exist already. These lists are indexed by the DrawCoordType enumerator

 DrawDataSymbol objects are created for the symbol data of each map loaded and added 
 to the latLongSymbolList
 As above, vector<DrawDataList*> projSymbolLists will contain projected copies of the SymbolData.
*/

struct mapSuffixLayer
{
  string suffix;
  int layer;
  mapSuffixLayer(string str, int l = -1)
  {
    suffix = str;
    layer = l;
  };
};

class rpMapPaths {
 public:
  vector<string> mapDirs;
  //  vector<string> asciiMapSuffixes;
  vector<mapSuffixLayer> asciiMapSuffixes;
  //  vector<string> legacyMapSuffixes;
  vector<mapSuffixLayer> legacyMapSuffixes;
  rpMapPaths(char *mapPathFile = 0);
  ~rpMapPaths();
  void load(char *mapPathFile);
};

/*
  IMPORTANT NOTE: data objects store coords as lat, long i.e. x=lat, y=long
*/
class RpLatLongMap {
 public:
  int stnId;
  char Label[64];
  int refCount; // number of times this map is refenced
  int vector_nodecount;
  int symbol_nodecount;
  bool valid;
  DrawDataList latLongVectorList;  // list of DrawDataVector objects, typically overlays
  // such as coast, etc. Each List entry can have different 
  // properties, e.g. color, thickness, minrng/maxrng
  vector<DrawDataList*> projVectorLists; // ProjVectorLists is a list of projected versions of 
  // latLongVectorList
  DrawDataList* getProjVectorList(DrawCoordType coordtype);

  DrawDataList latLongSymbolList;  
  vector<DrawDataList*> projSymbolLists;
  DrawDataList* getProjSymbolList(DrawCoordType coordtype,
				  bool clipOOBB = false,
				  llBoundingBox *bbox = NULL);
  void setProjSymbolListOOBB(DrawDataList* projlist,
			     llBoundingBox *bbox);
  llBoundingBox lastLLBB;
  virtual bool loadMaps(int stnid, rpMapPaths *mapPaths);
  virtual void clearLists();
  virtual bool readASCIIMap(char *mapname, int dfltlayer, bool SwapLatLongOrder = true); 
  RpLatLongMap(int stnid = -1, rpMapPaths *mapPaths = 0);
  virtual ~RpLatLongMap();
  void incRefCount();
  void decRefCount();
  int getRefCount();
  bool labelMatch(char *label);
  
  void renderMap(DrawCoordType coordType, float DisplRng, 
		 OverlayProperties *OlayProps = 0);
  overlayLayers olayLayers;
};

/*
  This class will load and store maps for given radar site id
*/
class RpRdrStnMap {
 public:
  int StnID;
  bool mapAvail(int layer = 0); // true if there is a map available
                                // if layer > 0, true if layer in this map
  RpRdrStnMap(int stnid, rpMapPaths *mapPaths);
  virtual ~RpRdrStnMap();
  rpmap *legacyMap;
  RpLatLongMap *latLongMap;
  bool loadMaps(int stnid, rpMapPaths *mapPaths);
  void deRefAsciiMap();

  void renderMap(DrawCoordType coordType, float DisplRng, 
		 OverlayProperties *OlayProps = 0);
};


// rpMaps[0] will be a "global" map which can be used if no map available for the
// requested rdr stnid

extern bool allowMapGroupText;
extern bool enableTextDlists;
extern bool useDefaultMapsByLayer;

class RPMapMng {
 protected:
  vector<RpRdrStnMap*> rpMaps;
  vector<rdrCovObj*> coverageObjs;
  rpMapPaths mapPaths;
  string inifile;
 public:
  RPMapMng(char *map_ini_fname = 0);
  void init(char *map_ini_fname = 0);
  void close();
  void reload();
  virtual ~RPMapMng();
  virtual bool loadMaps(int stnid);
  virtual RpRdrStnMap* getMaps(int stnid, int layer = 0);
  virtual bool mapAvail(int stnid, int layer = 0);
  virtual rdrCovObj* getCoverageObj(int stnid);
  virtual void checkASCIIMapDupl(RpRdrStnMap *newmap);
  
/*   bool allowGroupText; */
  overlayLayers olayLayers;
  void setLayerCount(int count) { olayLayers.setLayerCount(count); };
  int getLayerCount() { return olayLayers.getLayerCount(); };
  virtual void loadLayerDefs(char *fname) { olayLayers.loadLayerDefs(fname); };

  void setDefaultLayerState() { olayLayers.setDefaultLayerState(); }; // set the string "Layer n" if not already defined
  void setLayerString(int layer, char *newstr) 
    { olayLayers.setLayerString(layer, newstr); };
  const char* getLayerString(int layer) { return olayLayers.getLayerString(layer); };
  void setLayerState(int layer = -1, bool state = true)
    { olayLayers.setLayerState(layer, state); };
  bool getLayerState(int layer) { return olayLayers.getLayerState(layer); };
};  

extern RPMapMng *rpMapMng;



#endif	/* __MAPS_H */


