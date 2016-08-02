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
// The euroSfc2Spdb class does most of the work for the euroSfc2Spdb application.
// An ASCII file of surface station data is read, and the results are written
// to an SPDB database.
//
#ifndef _EURO_SFC_2_SPDB_HH
#define _EURO_SFC_2_SPDB_HH


#include <Spdb/DsSpdb.hh>
#include "Params.hh"
using namespace std;

class euroSfc2Spdb {

  public :
  //
  // Constructor.
  //
  euroSfc2Spdb();

  //
  // Destructor.
  //
  ~euroSfc2Spdb();

  //
  // Main method - read an input SAMS file and write SPDB.
  //
  int ReadSamFile(char *FileName, Params *TDRP);

private :
  //
  // Internal methods.
  //
  int _FindStation(int ID, double *Lat, 
		  double *Lon, double *Alt, char **Desc);




  DsSpdb _OutputDsSpdbObject;


  void _checkStatus(int status, char *exitStr);


};

#endif
