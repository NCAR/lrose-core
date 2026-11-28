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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:11 $
//   $Id: GenptSpdbInput.cc,v 1.2 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GenptSpdbInput: Class for manipulating GenPt SPDB input dataset objects.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <rapformats/GenPt.hh>

#include "GenptSpdbInput.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

GenptSpdbInput::GenptSpdbInput(const string &input_url,
			       const string &data_field_name,
			       const bool debug_flag) :
  Input(debug_flag),
  _retriever(input_url),
  _dataFieldName(data_field_name),
  _dataUnits("")
{
}


/**********************************************************************
 * Destructor
 */

GenptSpdbInput::~GenptSpdbInput(void)
{
}
  

/**********************************************************************
 * getData() - Retrieve the input data for the given time range.
 */

vector< DataPoint > GenptSpdbInput::getData(const DateTime start_time,
					    const DateTime end_time)
{
  static const string method_name = "GenptSpdbInput::getData()";
  
  if (_debug)
  {
    cerr << "   Interval start time: " << start_time << endl;
    cerr << "   Interval end time: " << end_time << endl;
  }
  
  // Get the data from the SPDB database

  const vector< Spdb::chunk_t > chunks =
    _retriever.getData(start_time, end_time);
  
  if (_debug)
    cerr << "   Read " << chunks.size() << " chunks from SPDB database" << endl;
  
  // Convert the data to data points

  vector< DataPoint > data_pts;
  vector< Spdb::chunk_t >::const_iterator chunk;
  
  for (chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
  {
    GenPt gen_pt;
    DataPoint data_pt;
    
    gen_pt.disassemble(chunk->data, chunk->len);
    
    data_pt.setLocation(gen_pt.getLat(), gen_pt.getLon());
    
    int gen_pt_field_num = gen_pt.getFieldNum(_dataFieldName);
    if (gen_pt_field_num < 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting data field from GenPt object:" << endl;
      cerr << "   field name: " << _dataFieldName << endl;
      cerr << "   point lat: " << gen_pt.getLat() << endl;
      cerr << "   point lon: " << gen_pt.getLon() << endl;
      
      continue;
    }
    
    data_pt.setValue(gen_pt.get1DVal(gen_pt_field_num));
    
    data_pts.push_back(data_pt);

    _dataUnits = gen_pt.getFieldUnits(gen_pt_field_num);
  }
  
  return data_pts;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
