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
////////////////////////////////////////////////////////////////////////////////
//
//  Class for storing a beam of 1000m data
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: KmData.hh,v 1.3 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _KM_DATA_INC_
#define _KM_DATA_INC_


class KmData
{
public:
   KmData();
  ~KmData(){};

   //
   // The two types of 1km data we handle
   //
   enum type_t { DBZ_DATA,
                 SNR_DATA };

   //
   // Saving the data at a specified azimuth
   // Although you can fetch dbz and snr separately, the two must be
   // saved together at a single azimuth.
   //
   void     setData( float azimuth, 
                     ui08* dbzData, ui08* snrData, size_t nbytes );

   //
   // Access methods
   //
   float    getAzimuth(){ return azimuth; }

   ui08*    getDbz(){ return kmDbz; }
   ui08*    getSnr(){ return kmSnr; }

private:

   //
   // Stored 1km data
   //
   ui08     kmDbz[NEX_MAX_GATES];
   ui08     kmSnr[NEX_MAX_GATES];

   //
   // Remember the azimuth at which the data were collected
   //
   float    azimuth;
};

#endif
