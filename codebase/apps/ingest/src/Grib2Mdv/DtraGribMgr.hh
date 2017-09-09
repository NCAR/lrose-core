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
/////////////////////////////////////////////////////////
// DtraGribMgr
//
// $Id: DtraGribMgr.hh,v 1.5 2017/06/09 16:27:58 prestop Exp $
//
////////////////////////////////////////////////////////
#ifndef _DTRA_GRIB_MGR
#define _DTRA_GRIB_MGR

#include "GribMgr.hh"
#include "AvnGribMgr.hh"

using namespace std;

//
// Forward class declarations
//

/**
 *  DtraGribMgr
 *  This class converts DTRA-type grib files to MDV.
 */
class DtraGribMgr : public AvnGribMgr {

public:

  DtraGribMgr();
  virtual ~DtraGribMgr();

  /// DTRA grib files have some kind of header before the first 'GRIB' indicator:
  ///   ####018004559####
  ///   HTME85 KWBC 220000 
  virtual void findFirstRecord(FILE *fp);

 // we store the Dtra grib table in our own structure
  typedef struct {
    int paramId;
    const char *name;
    const char *long_name;
    const char *units;
  } DtraParmTable;

  static const DtraParmTable _dtraParmTable[];

  // last param index in the standard grib table
  static int _kLastStandardParam;

  // override the NWS/NCEP usage
  virtual string getLongName();
  virtual string getName();
  virtual string getUnits();


};

#endif

