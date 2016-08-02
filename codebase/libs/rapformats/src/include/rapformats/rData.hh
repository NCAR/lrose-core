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
//
// $Id: rData.hh,v 1.11 2016/03/03 19:23:53 dixon Exp $
//


#include <string>
#include <vector>
#include <cstdio>
#include <climits>
#include <dataport/port_types.h>
#include <rapformats/r_data.h>
using namespace std;

//
// defines for printing
//
#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld " \
"%.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

//
// Forward class declarations
//
class DsRadarMsg;
class DsRadarParams;
class DsRadarBeam;
class DsFieldParams;

class rData 
{
  public:
   
   rData();
   ~rData();

   void           setRay(DsRadarMsg& dsrMsg, int nGatesCopy = INT_MAX);
   void           printRay() const;

   char          *getRay(){ return rayBuf; }
   Ray_header    *getRayHdr() { return rayHdr; }
   Radar_header  *getRadarHdr() { return radarHdr; }
   Field_header **getAllFieldHdrs() { return fieldHdrPtr; }
   Field_header  *getFieldHdr(int idex) { return fieldHdrPtr[idex]; }
   ui08         **getFieldData(){ return dataPtr; }
   ui08          *getGateData();
   int            getRayLength(){ return rayLength; }

   
  private:
   
   char          *rayBuf;
   Ray_header    *rayHdr;
   Radar_header  *radarHdr;
   Field_header **fieldHdrPtr;
   ui08         **dataPtr;
   ui08          *gateData;

   int            rayLength;
   int            nFields;
   int            nGates;

   int            nFieldsAlloc;
   int            nBytesAlloc;

   int            fieldHdrLen;
   int            fieldDataLen;
   int            fieldTotalLen;
   int            radarHdrOffset;
   int            fieldHdrOffset;

   void allocRay();
   void setHdrs(const DsRadarParams& radarParams, const DsRadarBeam& radarBeam,
                const vector< DsFieldParams* >& fieldParams,
                int radarHdrOffset);
   
   
};

   
   
  
