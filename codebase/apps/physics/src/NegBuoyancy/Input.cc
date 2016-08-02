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
 *
 * @file Input.cc
 *
 * @class Input
 *
 * Base class for classes that retrieve input files.
 *  
 * @date 6/10/2010
 *
 */

#include <iostream>

#include <toolsa/os_config.h>
#include <Mdv/DsMdvx.hh>

#include "Input.hh"


// Global variables


/*********************************************************************
 * Constructor
 */

Input::Input(const bool debug_flag) :
  _debug(debug_flag),
  _remap(false)
{
}


/*********************************************************************
 * Destructor
 */

Input::~Input()
{
}


/*********************************************************************
 * readField()
 */

MdvxField *Input::readField(const TriggerInfo &trigger_info,
			    const string &url,
			    const string &field_name,
			    const int field_num,
			    const int max_input_valid_secs) const
{
  static const string method_name = "Input::readField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  _setReadTime(input_file, url, field_name, max_input_valid_secs, trigger_info);
  
  if (field_name.length() > 0)
    input_file.addReadField(field_name);
  else
    input_file.addReadField(field_num);
  
  input_file.setReadNoChunks();
  
  // I think we really want to use all of the input data in the
  // calculations, but just do the calculations between these
  // levels.  Didn't want to actually delete this until I knew
  // for sure.

  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Set the remapping projection

  if (_remap)
    input_file.setReadRemap(_inputProj);
  
  // Now read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  // Make sure the data in the volume is ordered in the way we expect.

  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();
  
//  if (master_hdr.grid_orientation != Mdvx::ORIENT_SN_WE)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Input grid has incorrect orientation" << endl;
//    cerr << "Expecting " << Mdvx::orientType2Str(Mdvx::ORIENT_SN_WE) <<
//      " orientation, got " <<
//      Mdvx::orientType2Str(master_hdr.grid_orientation) <<
//      " orientation" << endl;
//    cerr << "Url: " << url << endl;
//    
//    return 0;
//  }
  
  if (master_hdr.data_ordering != Mdvx::ORDER_XYZ)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input grid has incorrect data ordering" << endl;
    cerr << "Expectin " << Mdvx::orderType2Str(Mdvx::ORDER_XYZ) <<
      " ordering, got " << Mdvx::orderType2Str(master_hdr.data_ordering) <<
      " ordering" << endl;
    cerr << "Url: " << url << endl;
    
    return 0;
  }
  
  // Pull out the appropriate field and make a copy to be returned.
  // We must make a copy here because getField() returns a pointer
  // into the DsMdvx object and the object is automatically deleted
  // when we exit this method.

  MdvxField *field = input_file.getField(0);
  
  if (field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving field from volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  return new MdvxField(*field);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
