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
/////////////////////////////////////////////////////////////////////////
//
// Class:	MergerInt8
//
// Author:	G M Cunning
//
// Date:	Tue Feb  6 12:59:19 2001
//
// Description: this class perform INT8 based data mergers.

// C++ include files

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cassert>

// Local include files
#include "MergerInt8.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include "Params.hh"
using namespace std;



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MergerInt8::MergerInt8(string prog_name, Params *params)
  : Merger(prog_name, params)
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
MergerInt8::~MergerInt8()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MergerInt8::mergeField
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

bool 
MergerInt8::mergeField(InputFile *in_file, const int& i, OutputFile* out_file)
{
  // Get the needed information for the input field

  DsMdvx *inHandle = in_file->handle();
  MdvxField *inField = inHandle->getFieldByNum(i);
  assert(inField != 0);

  // make sure encoding type is INT8
  if(inField->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_NONE, 
			  Mdvx::SCALING_DYNAMIC) != 0) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << inField->getErrStr();
    cerr << "*** Exiting ***" << endl << endl;
    return false;
  }

  inField->setPlanePtrs();
  Mdvx::field_header_t in_fhdr = inField->getFieldHeader();
  double in_scale = in_fhdr.scale;
  double in_bias = in_fhdr.bias;

  // Get the needed information for the output field

  DsMdvx *outHandle = out_file->handle();
  MdvxField *outField = outHandle->getFieldByNum(i);
  assert(outField != 0);

  outField->setPlanePtrs();
  Mdvx::field_header_t out_fhdr = outField->getFieldHeader();
  double out_scale = out_fhdr.scale;
  double out_bias = out_fhdr.bias;

  // compute lookup table for changing from input scale and bias
  // to output scale and bias

  ui08 scaleLut[256];
  for (int i = 0; i < 256; i++) {
    if (i == (int)in_fhdr.missing_data_value) {
      scaleLut[i] = (int)out_fhdr.missing_data_value;
    }
    else if (i == (int)in_fhdr.bad_data_value)
    {
      scaleLut[i] = (int)out_fhdr.bad_data_value;
    }
    else
    {
      double val = i * in_scale + in_bias;
      int outByte = (int) ((val - out_bias) / out_scale + 0.49999);

      if (outByte > 255)
	scaleLut[i] = 255;
      else if (outByte < 3)
	scaleLut[i] = 3;
      else
	scaleLut[i] = outByte;
    }
  }

  // loop through the input planes, setting values in the output planes
  // where appropriate

  MdvxProj *inGrid = in_file->grid();
  Mdvx::coord_t input_coords = inGrid->getCoord();

  vector<int>* zLut = in_file->zLut();
  for (int input_z = 0; input_z < input_coords.nz; input_z++) {

    int output_z = (*zLut)[input_z];
    if (output_z < 0)
      continue;

    // set up pointers to planes and XY lookup table

    ui08 *inPlane = (ui08 *) inField->getPlane(input_z);
    ui08 *outPlane = (ui08 *) outField->getPlane(output_z);
    vector<long>::iterator xylut = (in_file->xyLut())->begin();
    
    // loop through points in the input plane, updating the output
    // plane where appropriate.  Note that we don't have to check
    // for bad/missing data values in this loop because these values
    // are taken care of in the scaleLut.

    ui08 *inbp = inPlane;

    for (int i = 0; i < input_coords.nx * input_coords.ny;
	 i++, inbp++, xylut++) {

      int index = *xylut;
      if (index < 0)
	continue;
      
      int inb = *inbp;
      int outb = scaleLut[inb];
      if (_params->merge_operator == Params::MERGE_SUM) {
	if((outPlane[index] > 2) && (outb > 2))
	    outPlane[index] = outPlane[index] + outb;
	else if((outPlane[index] < 3) && (outb > 2))
	    outPlane[index] =  outb;
      }
      else if (_params->merge_operator == Params::MERGE_MAX) {
	if (outPlane[index] <= outb)
	  outPlane[index] = outb;
      }
      else {
	// If outPlane[index] is 0 or 1, it is a missing/bad data value.
	// I'm guessing that the 2 value is a buffer between the missing
	// data and the actual data.

	if ((outPlane[index] < 3) || (outPlane[index] > outb))
	  outPlane[index] = outb;
      }
    } // i

  } // input_z

  return true;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

