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
///////////////////////////////////////////////////////////////
// Server.cc
//
// File Server object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////


#include <rapformats/GenPt.hh>
#include <toolsa/str.h>

#include "Server.hh"
using namespace std;



/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE)
{
  // Do nothing
}


/*********************************************************************
 * loadLocalParams() - Load local params if they are to be overridden.
 */

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)
{
  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  const string routine_name = "_allocLocalParams";

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
			_isVerbose) != 0)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  // Make sure the local params are correct

  if (localParams->field_names_n < 1)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Error in parameter file: " << paramFile << endl;
    cerr << "Must specify at least 1 field name to be displayed" << endl;
    
    delete localParams;
    return -1;
  }
  
  if (_isVerbose)
    localParams->print(stderr, PRINT_SHORT);

  *serverParams = (void*)localParams;
  return 0;
}


/*********************************************************************
 * convertToSymprod() - Convert the given data chunk from the SPDB
 *                      database to symprod format.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
			     const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
{
  // check prod_id

  if (prod_id != SPDB_GENERIC_POINT_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POINT_ID: " << SPDB_GENERIC_POINT_ID << endl;

    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // Convert the SPDB data to a weather hazards buffer

  GenPt point;
  
  point.disassemble(spdb_data, spdb_len);
  
  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_GENERIC_POINT_LABEL);

  // Construct the data string

  string text_string;
  bool first_field = true;
  
  for (int i = 0; i < serverParams->field_names_n; ++i)
  {
    char field_value[BUFSIZ];
    
    // Get the field number for the field

    int field_num = point.getFieldNum(serverParams->_field_names[i]);
    
    if (field_num < 0)
      STRcopy(field_value, serverParams->missing_value_string, BUFSIZ);
    else
      sprintf(field_value, serverParams->value_format_string,
	      point.get1DVal(field_num));
    
    // Add the field value to the text string

    if (!first_field)
      text_string += serverParams->field_delim;
    
    text_string += field_value;
    
    first_field = false;

  } /* endfor - i */

  // Add the string to the Symprod object

  prod.addText(text_string.c_str(),
	       point.getLat(), point.getLon(),
	       serverParams->text_color, "",
	       0, 0,
	       Symprod::VERT_ALIGN_CENTER, Symprod::HORIZ_ALIGN_CENTER,
	       0, Symprod::TEXT_NORM, serverParams->font_name);
  
  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}
