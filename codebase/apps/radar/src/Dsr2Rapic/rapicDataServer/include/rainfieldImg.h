#ifndef __RAINFIELD_H__
#define __RAINFIELD_H__

/*
 * rainfield.h
 *
 */

#include "levelTable.h"
#include "drawOGlLLTexture.h"
#include "CImage.hh"


typedef unsigned char uchar;

enum e_rf_data_mode {RF_REALTIME, RF_DB, RF_REPLAY};

// Old convention was rainfall products < 1hr to be rainrate(mm/hr), >1hr rainaccum(mm total)
// New convention is all rainfall products to be rainaccum(mm total)
enum e_rf_rainfall_mode {RF_RAINRATE, RF_RAINACCUM};

bool cimageIsBaseProd(Ref<CImage>& cimage);
bool cimageIsSince9am(Ref<CImage>& cimage);

class rainfieldImg : public virtual RefBase
{
 public:
  rainfieldImg();
  rainfieldImg(Ref<CImage>& _cimage, string _creatorstr, 
	       void *_creator = NULL, 
	       float  pxlscalefactor = 1.0, 
	       e_rf_rainfall_mode _rainfallmode = RF_RAINACCUM); 
  virtual ~rainfieldImg();
  void    setImage();
  void    setImage(Ref<CImage>& cimage);
  void    threshImage();             // threshold image
  bool    threshImageAvail() { return getThreshArraySize() > 0; };
  Ref<CImage> cimage;         // Ref to cimage object;
  float   _pxlScaleFactor;            // typically the pixel array is in scaled floating point - 0.1mm/hr
  float   pxlScaleFactor();            // typically the pixel array is in scaled floating point - 0.1mm/hr
  Array2<ImagePixel> pxl_array;      // rainfields floating point pixel array
  void    unpackPixelArray();         // allocate floating point pixel array
  void    deallocPixelArray();       // deallocate floating point pixel array
  void    encodeThreshArray();       // encode thresh_array to rapic RLE string
  void    unpackThreshArray();       // allocate and unpack rapic RLE string to thresh_array
  void    allocThreshArray();        // allocate thresh_array
  void    deallocThreshArray();      // deallocate thresh_array
  Ref<Projection>    proj;           // proj object
  Array2<uchar>      thresh_array;   // rainfields thresholded array
  string  threshRLEString;           // Rapic RLE array string
  Array2<uchar>      test_thresh_array;   // rainfields thresholded array
  void     deallocAll();
  void     makeTestPattern(Array2<uchar> &_array, 
			   int levels); // write a test patterm into the Array2
  size_t   rows, cols;               // cimage dimensions
  double   resolution;               // cell resolution in metres
  double   km_width, km_height;
  float    maxVal;                   // max value in array
  float    getMaxVal();              // traverse the array for the max value
  bool     getVal_kmne(float &retval, float kmn, float kme, // get value at given point
		       bool useFloat = false, int *idx = NULL); // if true, unpack pxl_array
  bool     getVal_rowcol(float &retval, size_t row, size_t col, // get value at given point
			 bool useFloat = false, int *idx = NULL); // if true, unpack pxl_array
  int      getStn();                 // return the stn id
  LevelTable *thresh_table;          // threshold table for thresholded array;
  void     *creator;                  // pointer to "creator" 
  string   creatorStr;
  string   keyStr;
  time_t   imgTime;     // use gendate from cimage
  e_rf_data_mode data_mode;
  e_rf_rainfall_mode rainfallMode;   // rainrate(mm/hr) or rainaccum(mm)
  bool since9am;   // true if product is since9am rainfall
  void     setKeyStr(); // key is string of - stnid datetime interval datatype
  /*
      sprintf(tempstr, "%03d %s %06d %03d", 
	      int(cimage->getProjn()->getStation()),
	      cimage->getGenDate().toSQL().c_str(),
	      int(cimage->getInterval()), 
	      int(cimage->getDataType()));
  */

  
  bool    getRFKey(string& keystr);  // get the key string, return true if valid
  char*   getRFKey();                // get the key string as char*, return NULL if not valid
  //enum rf1__ImageDataType {IDTUnknown = 0, RawReflectivity = 1, 
  // Rainfall = 2, ForecastRainfall = 3, ForecastReflectivity = 4, 
  // AdvectionXVel = 5, AdvectionYVel = 6};
  rf1__ImageDataType getDataType();  // get type - 
  time_t  getInterval();             // 
  double  getCalFactor();
  bool    isBaseProd();
  bool    isSince9am();
  rf1__ImageCalStatus	getCalStatus();
  char    *getCalStatusStr();
  char    *rfImgFlagStr;
  char    *getRfImgFlagStr();
  int     getCappi();
  void    setLLProjGrid(llProjGrid &projGrid);  // set llProjGrid for this
  void    setLLProjGridRes(int kmsperpt);  // set llProjGrid for this
  int     llGridRes;                 // llprojgrid res in kms (default 64km)
  void    setTextureTile(oglTextureTile &textile);
  int     getMemSize();              // return size of this object
  int     getRlzSize();              // get size of cimage rlz string
  int     getPxlArrayCells();        // get size of the pxl_array in cells
  int     getPxlArraySize();         // get size of the pxl_array in bytes
  int     getThreshArrayCells();      // get size of thresh array in cells
  int     getThreshArraySize();      // get size of thresh array in bytes
  void    setRealTime(bool state = true); // set data mode
  bool    isRealTime()
    {
      return data_mode == RF_REALTIME; 
    }; // get realtime state
  bool    debug;
};

#endif /* __RAINFIELD_H__ */
