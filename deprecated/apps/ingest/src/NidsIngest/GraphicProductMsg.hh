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
/**
 * @file GraphicProductMsg.hh
 * @brief GraphicProductMsg has component structs comprising the WSR88D 
 *        Level-III NEXRAD Information Dissemination Service (NIDS) data 
 *        files. It also contains byte-swapping and printing routines for 
 *        these components
 * 
 * @class GraphicProductMsg.hh
 * @brief GraphicProductMsg has component structs comprising the WSR88D 
 *        Level-III NEXRAD Information Dissemination Service (NIDS) data 
 *        files. It also contains byte-swapping and printing routines for 
 *        these components

 * @struct GraphicProductMsg::msgHdrBlk_t
 * @brief msgHdrBlk_t is a struct containing 
 * @struct GraphicProductMsg::productDescription_t
 * @brief productDescription_t 
 * @struct GraphicProductMsg::prodSymbBlk_t
 * @brief prodSymbBlk_t is a struct containing
 * 
 * The document used to create this code is Build 12.1 for 2620001 
 * (RPG to Class 1 User ICD) which can be found at:
 * http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx.
 */

#ifndef GRAPHIC_PRODUCT_MSG_HH
#define GRAPHIC_PRODUCT_MSG_HH

#include <dataport/port_types.h>
#include <vector>
#include <string>

using namespace std;

class GraphicProductMsg {

public:
  /**
   * Data types from file (with structure fields aligning on 
   * one-byte boundaries)
   * See ICD Figure 3-3 Message Header
   * Note that the block divider is attached to this struct
   */
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

  /** 
   *  See ICD Figure 3-6 Graphic Product Message (Sheet 2 and 6)
   * 
   */
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
    ui16 prodDependent31;
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
  } productDescription_t;
  
  /**
   * Product Symbology Block header
   * See ICD Figure 3-6 (Sheet 3 and Sheet 8) 
   */
  typedef struct __attribute__ ((__packed__)) {
    si16 div;
    ui16 id;
    ui32 len;
    ui16 nLayers;
    si16 layerDiv;
    ui32 dataLayerLen;
  } prodSymbBlk_t;
  
  /**
   * Graphic Alphanumeric Block header
   * See ICD Figure 3-6 (Sheet 4 and Sheet 9) 
   */
  typedef struct __attribute__ ((__packed__)) {
    si16 div;
    ui16 id;
    ui32 len;
    ui16 nPages;
  } graphicAlphaNumericBlk_t;

 
  /**
   * Digital Radial Array Packet header (code 16)
   * See ICD Figure 3-11c (Sheet1 and Sheet2)
   */
  typedef struct __attribute__ ((__packed__)) {
    si16 msgCode;
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

  /**
   * Header on each radial of Digital Radial Array Packet (code 16)
   * See ICD Figure 3-11c (Sheet1 and Sheet2)
   * number of bytes in radial, azimuth, azimuth delta
   */
  typedef struct __attribute__ ((__packed__)) {
    ui16 num;
    ui16 az;
    ui16 dAz;
  } smallRadBlock_t;

  /**
   * Generic Header Format: Appendix E of the ICD specifies packet
   * code 28 or 29 for generic products. 
   * From CODE orpg file packet_28.h, the header contains: half word packet 
   * code, half word alignment spacer, (int) numbytes num_bytes (not including 
   * self or packet code).
   */
  typedef struct __attribute__ ((__packed__)) {
    si16 code;
    ui16 notUsed;
    ui32 numBytes;
  } genericHeader_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 code;
    si16 colorIndicator;
    si16 colorValue;
  } setColorLevelHdr_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 code;
  } linkedVectorHeader_t;
  
  typedef struct __attribute__ ((__packed__)) {
    si16 code;
    ui16 numBytes;
  } specialGraphicSymbolPacket_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 I;
    si16 J;
    char char1;
    char char2;
  } stormID_t;
  
  typedef struct __attribute__ ((__packed__)) {
    si16 I;
    si16 J;
    si16 probOfHail;
    si16 probOfSevereHail;
    si16 maxHailSize;
  } hdaHail_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 I;
    si16 J;
    si16 type;
    si16 radius;
  } mesocyclone_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 pageNum;
    ui16 pageLen;
  } graphicPageHdr_t;

  typedef struct __attribute__ ((__packed__)) {
    si16 code;
    ui16 numBytes;
    si16  I;
    si16  J;
  } textPacket1_t;

 typedef struct __attribute__ ((__packed__)) {
    si16 code;
    ui16 numBytes;
    ui16 textColor;
    si16  I;
    si16  J;
  } textPacket8_t;

  

  static int readMessageHeaderBlock(msgHdrBlk_t &msgHdr, FILE *fp,  
				    bool byteSwap,bool debug, bool hasHdr); 
  static int readProdDescr(productDescription_t &graphProdDesc, FILE *fp, 
			   bool byteSwap, bool debug);

  static void readSymbologyBlockHdr(prodSymbBlk_t &psb,
				    unsigned char *decompressedBuf, 
				    unsigned long int &offset, bool byteSwap,
				    bool debug);

  static void readGraphAlphaNumHdr(graphicAlphaNumericBlk_t &g, 
				   unsigned char *decompressedBuf, 
				   unsigned long int &offset,
				   bool byteSwap,
				   bool debug);

  static void readRadialHdr(radial_t &r,  unsigned char *decompressedBuf, 
			     unsigned long int &offset, bool byteSwap,
			     bool debug);  
  
  static void readSmallRadHdr(smallRadBlock_t & sb, 
			      unsigned char *decompressedBuf, 
			      unsigned long int &offset, bool byteSwap, 
			      bool debug);

  static void readGenericHdr(genericHeader_t &genHdr, 
			     unsigned char *decompressedBuf,
			     unsigned long int &offset,  bool byteSwap, 
			     bool debug);

  static void readSetColorLevelHdr(setColorLevelHdr_t &cH,
				 unsigned char *decompressedBuf, 
				 unsigned long int &offset, bool byteSwap, 
				 bool debug);
  
  static void readLinkedVectorHdr( linkedVectorHeader_t &v,
				   unsigned char *decompressedBuf, 
				   unsigned long int &offset, bool byteSwap, 
				   bool debug);

  static void readGraphicAlphaNumericBlk(graphicAlphaNumericBlk_t &g, 
					 unsigned char *decompressedBuf, 
					 unsigned long int &offset,
					 bool byteSwap,
					 bool debug);

  static void readSpecialGraphicSymbolHdr(specialGraphicSymbolPacket_t &g, 
					  unsigned char *decompressedBuf, 
					  unsigned long int &offset,
					  bool byteSwap,
					  bool debug);

  static void readGraphicPageHdr(graphicPageHdr_t &g, 
				 unsigned char *decompressedBuf, 
				 unsigned long int &offset,
				 bool byteSwap,
				 bool debug);

  static void readStormID(stormID_t &s, 
			  unsigned char *decompressedBuf, 
			  unsigned long int &offset,
			  bool byteSwap,
			  bool debug);

  static void readHdaHail(hdaHail_t &h, 
			  unsigned char *decompressedBuf, 
			  unsigned long int &offset,
			  bool byteSwap,
			  bool debug);
  
  static void readMesocyclone(mesocyclone_t &m, 
			      unsigned char *decompressedBuf, 
			      unsigned long int &offset,
			      bool byteSwap,
			      bool debug);
  
  static void readTextPacket1(textPacket1_t &t, 
			      unsigned char *decompressedBuf, 
			      unsigned long int &offset,
			      bool byteSwap,
			      bool debug);

  static void  readTextPacket8(textPacket8_t &t, 
			       unsigned char *decompressedBuf, 
			       unsigned long int &offset,
			       bool byteSwap,
			       bool debug);
  
  static void printMsgHdrBlk(msgHdrBlk_t m);

  static void printProdDescr( productDescription_t);
  
  static void printPrdSymbBlk(prodSymbBlk_t p);

  static void printGraphAlphaNumHdr(graphicAlphaNumericBlk_t g);
  
  static void printRadial(radial_t r);
  
  static void printSmallRadBlock(smallRadBlock_t b);
  
  static void printGenericHdr(genericHeader_t h);

  static void printLinkedVectorHdr( linkedVectorHeader_t v);

  static void printSetColorLevelHdr( setColorLevelHdr_t cH );

  static void printGraphicAlphaNumericHdr(graphicAlphaNumericBlk_t g);

  static void printSpecialGraphicSympbolHdr(specialGraphicSymbolPacket_t g);

  static void printGraphicPageHdr(graphicPageHdr_t g); 

  static void printStormID(stormID_t s);
  
  static void printHdaHail(hdaHail_t h);

  static void printMesocyclone(mesocyclone_t m);
  
  static void printTextPacket1(textPacket1_t t);

  static void printTextPacket8(textPacket8_t t);

  static bool getContourInitPt(unsigned char *decompressedBuf, 
				unsigned long int &offset, 
				bool byteSwap,	bool debug, si16 &startI, 
				si16 &startJ);

  static si16 getContourNumVectors(unsigned char *decompressedBuf, 
				   unsigned long int &offset, bool byteSwap, 
				   bool debug);

  static void getContourVectorPt(unsigned char *decompressedBuf, 
				 unsigned long int &offset, bool byteSwap, 
				 bool debug, si16 &I, si16 &J);


public:
  
protected:
  
private:
  /**
   * Byte swapping object for handling byte order.
   */
  static void _swapMsgHdrBlk(msgHdrBlk_t *m);
  
  static void _swapProdDescr(productDescription_t *prodDescr);
  
  static void _swapProdSymbBlk(prodSymbBlk_t *p);

  static void _swapGraphAlphaNumHdr(graphicAlphaNumericBlk_t *g);
  
  static void _swapRadial(radial_t *r);
  
  static void _swapSmallRadBlock(smallRadBlock_t *b);
  
  static void _swapGenericHdr(genericHeader_t *h);
  
  static void _swapLinkedVectorHdr( linkedVectorHeader_t *v);

  static void _swapSetColorLevelHdr( setColorLevelHdr_t *v);

  static void _swapGraphicAlphaNumericHdr(graphicAlphaNumericBlk_t *g);

  static void _swapSpecialGraphicSymbolHdr(specialGraphicSymbolPacket_t *g);

  static void _swapGraphicPageHdr(graphicPageHdr_t *g); 

  static void _swapStormID(stormID_t *s); 

  static void _swapHdaHail(hdaHail_t *h);

  static void _swapMesocyclone(mesocyclone_t *m);

  static void _swapTextPacket1(textPacket1_t *t);

  static void _swapTextPacket8(textPacket8_t *t);

};

#endif
