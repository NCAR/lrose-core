/*

  demTexture.h

  Utilities to read and manipulate the demTexture digital terrain data

*/

#ifndef __DEMTEXTURE_H
#define __DEMTEXTURE_H

#include <vector>
#include <string>
#include "drawOGlLLTexture.h"
#include "rdrscan.h"

using namespace std;

class demTexture
{
 public:
  string headerName, dataName, pathName;
  vector<short> databuff;        // only one instance of demTexture has databuff
  boundingBox data_LLBounds;
  vector<uchar> dataidx_buff;    // ht values converted to ht map index values
  vector<float> worldXYZCoords;  // lat/long locations converted to world XYZ kms coords, 
                                 // anticipate generally only needed for 3d quad rendering, not for 2D texture
  short *data(long pos = 0);     // returns pointer to global demTexture databuff
  short data(int row, int col); // returns value from demTexture databuff
  short *pdata(int row, int col); // returns pointer to global demTexture databuff
  uchar *dataidx(long pos = 0);  // returns pointer to global demTexture dataidx_buff
  uchar dataidx(int row, int col);  // returns pointer to global demTexture dataidx_buff
  float *coord_3d(long pos = 0); // returns pointer to global demTexture worldXYZCoords
  float maxHeightInBox(int row1, int col1, int row2, int col2, 
		       int &highrow, int &highcol, float &maxheight); // height in metres
  float maxHeightInBox(float tl_lat, float tl_lng,    // get hieghest point from box - top/left lat long adn deg_w&deg_h
		       float deg_h, float deg_w, 
		       float &highlat, float &highlong, float &maxheight); // height in metres
  int rows, cols;
  int bands, bits;
  int bandRowBytes, totalRowBytes;
  int bandGapBytes;
  int noDataVal;
  float originLat, originLong;
  float latRes, longRes;
  bool needNtoH;
  bool forceByteSwap;
  int  resReduction;  // dem texture res reduction factor : 1 in n sampling
  bool valid;   // true id
  bool isOpen();
  LevelTable *heightTable;
  RGBPalette    *palette;
  bool projectHeights;

 /*

  For texture mapping need to use 2^n width and height texel tiles
  
  Tile No. lat1/long1    lat2/long2   row1/col1     row2/col2   h/w
  0         -6.4/107.4   -57.6/158.6  -344/-184     1703/1863  1024/1024  // half res (1/20deg) > full area
  1        -13.0/112.5   -38.0/125.0   120/20       1143/531   1024/512
  2        -10.5/125.0   -35.5/150.0   20/532       1043/1555  1024/1024 
  3        -15.0/150.0   -40.0/156.25  200/1556     1223/1811  1024/256    ** exceeds source array size
  4        -35.5/136.0   -48.0/148.5   1044/960     1555/1471  512/512     ** exceeds source array size
*/
  vector<oglTextureTile*> textureTiles; // break array into tiles (2^n width height) for texture mapping
                                       // textureTiles[0] is half res 1024x1024 full area tile
  vector<tileDefParams*> stdTileDefs; // std tile parameters - tiles 0-4 see above
  void clear();
  demTexture(char *headername = NULL, 
	   char *dataname = NULL, 
	   char *pathname = NULL);
  ~demTexture();
  void init();
  
  bool readFile(const char *headername = NULL, 
		const char *dataname = NULL, 
		const char *pathname = NULL);
  bool readHeader(const char *headername = NULL);
  bool readData(const char *dataname = NULL);  
  /*
  bool getSubset(int row1, int col1, int &h, int &w, );
  bool getSubset(float &lat1, float &lng1, float &lat2, float &lng2);
  */
  bool getLatLong(int Row, int Col, float &lat, float &lng);
  bool getRowCol(int &Row, int &Col, float lat, float lng);

  bool getTileProjGrid(oglTextureTile &tile, 
		       float lat1, float lng1, float lat2, float lng2, 
		       float projgridspacing = 0.5);   // get tile from row1/col1
  bool getTile(oglTextureTile &tile, int row1, int col1, int h, int w, int spacing = 1,
	       float projgridspacing = 0.5);   // get tile from row1/col1
  bool getTile(oglTextureTile &tile, tileDefParams &tiledefs,
	       float projgridspacing = 0.5);   // get tile from row1/col1
  bool getTile(oglTextureTile &tile, float lat, float lng, int h, int w, int spacing = 1,
	       float projgridspacing = 0.5); // get tile centred on lat/long
  void getStdTiles();  // get std tiles
  drawOGlLLTexture *oglTexRenderer;
  void setTileReloadFlags();
  void renderTexture(int tile_no, float atten = 1.0, 
		     bool blend = true);
  void renderTexture(oglTextureTile &tile, float atten = 1.0, 
		     bool blend = true);  // render as 2D texture to 3d world space
  void renderQuads(oglTextureTile &tile);    // render as 3D quad strips to 3d world space
  void renderQuads(float lat, float lng, int h, int w, int spacing = 1);    // render as 3D quad strips to 3d world space
  GLuint    quad_dlist;              // display list ident, 0 if no display list
  bool      quad_useDlist, quad_dlistValid;         // if false new display list should be
  void writeASCIIFile(char *fname = NULL);
};

extern demTexture *globalDemTexture;
extern string globalDemTextureDataName1;
extern string globalDemTextureHdrName1;
extern string globalDemTextureDataName2;
extern string globalDemTextureHdrName2;
extern string globalDemTexturePath;
extern int globalDemTextureRes;
extern bool globalDemTextureSwap;
demTexture* getGlobalDemTexture();

#endif
