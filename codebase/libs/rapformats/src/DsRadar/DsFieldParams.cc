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
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1998
//
////////////////////////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <rapformats/DsFieldParams.hh>
#include <iostream>
using namespace std;

DsFieldParams::DsFieldParams()
{
   scale            = 0.0;
   bias             = 0.0;
   byteWidth        = 1;
   missingDataValue = 0;

}

DsFieldParams::DsFieldParams( const char *fname, const char *funits,
                              float fscale, float fbias,
			      int fbyteWidth /* = 1*/,
                              int fmissingDataValue /* = 0*/ )
{
   name             = fname;
   units            = funits;
   scale            = fscale;
   bias             = fbias;
   byteWidth        = fbyteWidth;
   missingDataValue = fmissingDataValue;

}

DsFieldParams&
DsFieldParams::operator=( const DsFieldParams &inputParams )
{
   copy( inputParams );
   return( *this );
}

void
DsFieldParams::copy( const DsFieldParams &inputParams ) 
{
   name             = inputParams.name;
   units            = inputParams.units;
   scale            = inputParams.scale;
   bias             = inputParams.bias;
   byteWidth        = inputParams.byteWidth;
   missingDataValue = inputParams.missingDataValue;
}

void
DsFieldParams::print(FILE *out) const
{
   
   fprintf(out, "FIELD PARAMS\n");
   
   fprintf(out, "  field name:  %s\n", name.c_str());
   fprintf(out, "  units:  %s\n", units.c_str());
   fprintf(out, "  scale:  %f\n", scale);
   fprintf(out, "  bias:  %f\n", bias);
   fprintf(out, "  byte width: %d\n", byteWidth);
   fprintf(out, "  missing data value:  %d\n", missingDataValue);
 
   fprintf(out, "\n");
}

void
DsFieldParams::print(ostream &out) const
{
   
  out << "FIELD PARAMS" << endl;
   
  out << "  field name:  " << name << endl;
  out << "  units:  " << units << endl;
  out << "  scale:  " << scale << endl;
  out << "  bias:  " << bias << endl;
  out << "  byte width: " << byteWidth << endl;
  out << "  missing data value:  " << missingDataValue << endl;
 
  out << endl;

}

void
DsFieldParams::decode( DsFieldParams_t *fparams_msg )
{
  
  DsFieldParams_t fparams = *fparams_msg;
  BE_to_DsFieldParams(&fparams);
   
  name = fparams.name;
  units = fparams.units;
  scale = fparams.scale;
  bias = fparams.bias;
  byteWidth = fparams.byte_width;
  missingDataValue = fparams.missing_data_value;

}

void
DsFieldParams::encode( DsFieldParams_t *fparams_msg )
{

  memset( fparams_msg, 0, sizeof(DsFieldParams_t) );
  
  STRncopy( fparams_msg->name, name.c_str(), DS_FIELD_NAME_LEN );
  STRncopy( fparams_msg->units, units.c_str(), DS_FIELD_UNITS_LEN );
  
  fparams_msg->scale = scale;
  fparams_msg->bias = bias;
  fparams_msg->byte_width = byteWidth;
  fparams_msg->missing_data_value = missingDataValue;
  
  BE_from_DsFieldParams(fparams_msg);

}



