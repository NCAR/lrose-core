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
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  April 1998
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_FIELD_PARAMS_INC_
#define _DS_FIELD_PARAMS_INC_


#include <string>
#include <rapformats/ds_radar.h>
using namespace std;


class DsFieldParams
{
public:

  DsFieldParams();
  DsFieldParams( const char *fname, const char *funits,
		 float fscale, float fbias, int fbyteWidth = 1,
		 int fmissingDataValue = 0 );
  DsFieldParams( const DsFieldParams &inputParams ) { copy( inputParams ); }

  ~DsFieldParams(){};

  DsFieldParams& operator=( const DsFieldParams &inputParams);
   
  void    copy( const DsFieldParams &inputParams );
  void    print( FILE *out=stdout ) const;
  void    print( ostream &out ) const;

  void    decode( DsFieldParams_t *fparams_msg );
  void    encode( DsFieldParams_t *fparams_msg );

  // Public members

  int     byteWidth;
  int     missingDataValue;

  float   scale;
  float   bias;

  string  name;
  string  units;

};

#endif
