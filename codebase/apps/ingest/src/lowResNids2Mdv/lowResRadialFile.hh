// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

/////////////////////////////////////////////////////////////
// lowResRadialFile.hh
//
// Defines lowResRadialFile class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef HI_R_NIDS2MDV_H
#define HI_R_NIDS2MDV_H

#include "Params.hh"
#include <dataport/port_types.h>
#include <vector>

using namespace std;

class lowResRadialFile {
  
public:

  // Constructor. Makes a copy of a pointer to the TDRP parameters.
  // Reads the high res radial file named. One can tell
  // if it went OK because getNRadials() will return non-zero.
  lowResRadialFile (const char *FilePath, Params *P, bool verbose = false);

  // Destructor. Frees memory.
  ~lowResRadialFile ();

  // Data access methods.

  // Get number of radials
  int getNradials();

  // Get number of gates
  int getNgates();

  // Get azimuth of Nth radial
  fl32 getAz(int n);

  // Get pointer to all radials, bytes, Gate changes fastest in memory.
  ui08 *getAllRadialBytes();

  // Get pointer to all radials, fl32s.
  fl32 *getAllRadialFl32s();

  // Get pointer to nth radial, bytes
  ui08 *getRadialBytes(int n);

  // Get pointer to nth radial, fl32s
  fl32 *getRadialFl32s(int n);

  // General geography, setup etc.
  fl32 getLat();
  fl32 getLon();
  fl32 getAlt();

  // Note - getElevAngle() is not very accurate. The data
  // in the file is the cosine of the elevation angle, from
  // 0.001 to 1.0 in steps of 0.001. This is OK if you want
  // the cosine of the angle for range calculation purposes
  // but not if you want the actual angle.
  fl32 getElevAngle();

  //
  // Due to the issues with the evelvation angle, we may
  // want to override it.
  //
  void setElevAngle(double useThisAngle);

  fl32 getGateSpacing();
  fl32 getFirstGateDist();
  fl32 getMissingFl32();
  fl32 getDelAz();
  ui08 getMissingByte();
  time_t getTime();

  // NOAA product code
  int getProdCode();

  // Remap polar data to cartestian grid
  //
  // delta is the cartesian grid size in Km, if it is negative
  // then the polar gate spacing is used.
  //
  // dist is the range in Km to cover, ie. setting range
  // to 100Km would make the cartesian grid a 200Km square
  // centered on the radar. If it is negative then
  // the maximum range of the radar is used, that is
  // dist =  firstGateDist + nGates * gateSpacing
  //
  void remapToCart(double delta = -1.0, double dist=-1.0);

  // Get the nx by ny data after remapping
  fl32 *getCartData();

  // Get dxy (Km) and nxy (dx==dy and nx==ny here);
  int getNxy();
  fl32 getDxy();

  // Get a short field name, units based on the product code.
  char *getFieldName();
  char *getUnits();

protected:
  
private:

  Params *_params;

  int _nGates, _nRadials, _noaaProdCode;

  fl32 _lat, _lon, _alt;
  fl32 _elevAngle;
  fl32 _gateSpacing, _firstGateDist;
  fl32 _delAz;

  time_t _time;

  ui08 *_radialBytes;
  fl32 *_radialFl32s;

  fl32 _missingFl32;
  ui08 _missingByte;

  fl32 *_cartData;
  int _nxy;
  fl32 _dxy;

  double _physValue[16]; // Always 16 data values for these data
  void _getLUT(ui08 *headerBytes, bool verbose); // Decode bytes into _physValue array

  int _getAzIndex( double az );
  double _getMaxDist();

  // Data types from file - packed tightly

  typedef struct __attribute__ ((__packed__)) {
    si16 msgCode;
    ui16 msgDate;
    ui32 msgTime;
    ui32 msgLen;
    ui16 msgSrc;
    ui16 msgDest;
    ui16 nBlcks;
    si16 blckDiv;
  } msgHdrBlk_t;

  typedef struct __attribute__ ((__packed__)) {
    si32 lat;
    si32 lon;
    si16 elevFeet;
    si16 prodCode;
    ui16 opMode;
    ui16 vcp;
    si16 seqNum;
    ui16 volScanNum;
    ui16 volScanDate;
    ui32 volScanTime;
    ui16 genProdDate;
    ui32 genProdTime;
    ui16 prodDependent27;
    ui16 prodDependent28;
    ui16 elNum;
    ui16 prodDependent30;
    si16 prodDependent31;
    ui16 prodDependent32;
    ui16 prodDependent33;
    ui16 prodDependent34;
    ui16 prodDependent35;
    ui16 prodDependent36;
    ui16 prodDependent37;
    ui16 prodDependent38;
    ui16 prodDependent39;
    ui16 prodDependent40;
    ui16 prodDependent41;
    ui16 prodDependent42;
    ui16 prodDependent43;
    ui16 prodDependent44;
    ui16 prodDependent45;
    ui16 prodDependent46;
    ui16 prodDependent47;
    ui16 prodDependent48;
    ui16 prodDependent49;
    ui16 prodDependent50;
    ui16 prodDependent51;
    ui16 prodDependent52;
    ui16 prodDependent53;
    ui08 version;
    ui08 spotBlank;
    ui32 symbOff;
    ui32 grphOff;
    ui32 tabOff;
  } graphicProductMsg_t;


  typedef struct __attribute__ ((__packed__)) {
    si16 div;
    ui16 id;
    ui32 len;
    ui16 nLayers;
    si16 layerDiv;
    ui32 dataLayerLen;
  } prodSymbBlk_t;

  typedef struct __attribute__ ((__packed__)) {
    ui16 msgCode;
    ui16 firstIndex;
    ui16 nBins;
    ui16 iCent;
    ui16 jCent;
    ui16 rangeScaleFactor;
    ui16 nRadials;
    ui16 nBytes;
    ui16 az;
    ui16 delAz;
  } radial_t;

  typedef struct __attribute__ ((__packed__)) {
    ui16 num;
    ui16 az;
    ui16 dAz;
  } smallBlock_t;

  void _swapMsgHdrBlk(msgHdrBlk_t *m);
  void _printMsgHdrBlk(msgHdrBlk_t m);

  void _swapGraphicProductMsg(graphicProductMsg_t *g);
  void _printGraphicProductMsg(graphicProductMsg_t g);

  void _swapProdSymbBlk(prodSymbBlk_t *p);
  void _printPrdSymbBlk(prodSymbBlk_t p);

  void _swapRadial(radial_t *r);
  void _printRadial(radial_t r);

  void _swapSmallBlock(smallBlock_t *b);
  void _printSmallBlock(smallBlock_t b);

  void _swap2(void *c );
  void _swap4(void *c );


};

#endif





