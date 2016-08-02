/*

  tops.h

*/

#ifndef __TOPS_H
#define __TOPS_H

/*
  Typically this would be used as a global service object
  and creates a top rdr_scan object
  The topsarray will be created on first use
  360 x 1000 ushorts
*/
  
#include "rdrscan.h"

class tops : public scanProductCreator
{
 public:
  
  int rngDim, azDim;
  int angle_res; // in 10ths of degs
  int maxThreshHt,
    numLevels;
  float mindBZ;
  tops(int rngdim = 0, int azdim = 0, int angleres = 0);
  ~tops();
  
  void setMindBZ(float mindbz) { mindBZ = mindbz; };
  void makeTopsArray(rdr_scan *topsscan, float MindBZ = 0);
  rdr_scan* getTopsScan(rdr_scan *topsscan, float MindBZ = 0);
  virtual rdr_scan* createScanProduct(rdr_scan *src_scan, 
				     rdr_scan *scansetroot);
  void setDim(int rngdim, int azdim, int angleres);
};

extern ushort *globalTopsArray;
extern int globalTopsArraySize;  
extern int globalTopsArrayRefCount;
#endif

