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
// Fields.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef Fields_HH
#define Fields_HH

////////////////////////
// This class

class Fields {
  
public:
  
  Fields();

  // set values to missing

  void initialize();

  // public data

  int flags;  // censoring flag

  double snr;
  double dbm; // uncalibrated power
  double dbz;
  double vel;
  double width;

  // clutter-filtered fields

  double clut;
  double dbzf;
  double velf;
  double widthf;

  // dual polarization fields

  double zdr;  // calibrated
  double zdrm; // measured - uncalibrated
  
  double ldrh;
  double ldrv;

  double rhohv;
  double phidp;
  //double kdp; // disabled until a better algorithm can be implemented

  double snrhc;
  double snrhx;
  double snrvc;
  double snrvx;

  double dbmhc; // uncalibrated power
  double dbmhx;
  double dbmvc;
  double dbmvx;

  double dbzhc;
  double dbzhx;
  double dbzvc;
  double dbzvx;

  static const double missingDouble;
  static const int missingInt;

protected:
private:

};

#endif

