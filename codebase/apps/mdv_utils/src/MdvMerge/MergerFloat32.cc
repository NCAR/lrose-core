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
// Class:	MergerFloat32
//
// Author:	G M Cunning
//
// Date:	Tue Feb  6 12:59:19 2001
//
// Description: this class perform Float32 based data mergers.

// C++ include files

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cassert>
#include <cmath>

// Local include files
#include "MergerFloat32.hh"
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

MergerFloat32::MergerFloat32(string prog_name, Params *params)
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
  
MergerFloat32::~MergerFloat32()
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
// Method Name:	MergerFloat32::mergeField
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
MergerFloat32::mergeField(InputFile *in_file, const int& i, OutputFile* out_file)
{
  // compute lookup table for changing from input scale and bias
  // to output scale and bias
  DsMdvx *inHandle = in_file->handle();
  MdvxField *inField = inHandle->getFieldByNum(i);
  assert(inField != 0);

  // make sure encoding type is INT8
  if(inField->convertType() != 0) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << inField->getErrStr();
    cerr << "*** Exiting ***" << endl << endl;
    return false;
  }

  inField->setPlanePtrs();
  Mdvx::field_header_t in_fhdr = inField->getFieldHeader();


  DsMdvx *outHandle = out_file->handle();
  MdvxField *outField = outHandle->getFieldByNum(i);
  assert(outField != 0);

  // make sure encoding type is INT8
  //  if(outField->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_NONE, 
  //			   Mdvx::SCALING_DYNAMIC) != 0) {
  //  cerr << "ERROR - " <<  _progName << endl;
  //  cerr << outField->getErrStr();
  //  cerr << "*** Exiting ***" << endl << endl;
  //  return false;
  //}

  outField->setPlanePtrs();
  Mdvx::field_header_t out_fhdr = outField->getFieldHeader();

  // loop through output planes
  MdvxProj *inGrid = in_file->grid();
  Mdvx::coord_t input_coords = inGrid->getCoord();

  vector<int>* zLut = in_file->zLut();
  for (int input_z = 0; input_z < input_coords.nz; input_z++) {

    int output_z = (*zLut)[input_z];
    if (output_z < 0) {
      continue;
    }

    // set up pointers to planes and XY lookup table

    fl32 *inPlane = (fl32 *) inField->getPlane(input_z);
    fl32 *outPlane = (fl32 *) outField->getPlane(output_z);
    vector<long>::iterator xylut = (in_file->xyLut())->begin();
    
    // loop through points in input plane, merging in the input
    // data if it exceeds the value already in the plane and if it
    // is in a location that is within the output plane.

    //    ui08 *outbp = outPlane;
    fl32 *inbp = inPlane;
    for (int i = 0; i < input_coords.nx * input_coords.ny;
	 i++, inbp++, xylut++) {

      int index = *xylut;
      if (index < 0)
	continue;
      
      // take care of missing and bad data values
      if (_isValid(out_fhdr, outPlane[index])) {
	if (_isValid(in_fhdr, *inbp)) {
	  if (_params->merge_operator == Params::MERGE_SUM) {
	      outPlane[index] = outPlane[index] + *inbp;
	  }
	  else if (_params->merge_operator == Params::MERGE_MAX) {
	    if (outPlane[index] <= *inbp)
	      outPlane[index] = *inbp;
	  }
	  else {
	    if (outPlane[index] > *inbp)
	      outPlane[index] = *inbp;
	  }
	}
      }
      else {
	if (_isValid(in_fhdr, *inbp)) {
	  outPlane[index] = *inbp;
	}
	else {
	  if (_isMissing(in_fhdr, *inbp))
	    outPlane[index] = out_fhdr.missing_data_value;  
	  else if (_isBad(in_fhdr, *inbp))
	    outPlane[index] = out_fhdr.bad_data_value;  
	}
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

bool MergerFloat32::_isValid(const Mdvx::field_header_t& fhdr, const fl32& val) 
{
  if (_isMissing(fhdr, val) || _isBad(fhdr, val)) {
    return false;
  }

  return true;
}

bool MergerFloat32::_isMissing(const Mdvx::field_header_t& fhdr, const fl32& val) 
{
  if (fabs(fhdr.missing_data_value - val) < 0.00001) {
    return true;
  }

  return false;
}

bool MergerFloat32::_isBad(const Mdvx::field_header_t& fhdr, const fl32& val) 
{
  if (fabs(fhdr.bad_data_value - val) < 0.00001) {
    return true;
  }

  return false;
}


