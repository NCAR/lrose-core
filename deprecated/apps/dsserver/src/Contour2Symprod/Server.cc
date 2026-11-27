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
// November 2000
//
///////////////////////////////////////////////////////////////


#include <contour/Contour.hh>
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

  if (prod_id != SPDB_CONTOUR_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_CONTOUR_ID: " << SPDB_CONTOUR_ID << endl;

    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // Convert the SPDB data to a weather hazards buffer

  Contour contour;
  
  contour.disassemble(spdb_data, spdb_len);
  
  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_CONTOUR_LABEL);

  // Display all of the polylines in all of the contour levels

  const ContourLevel *contour_level;
  
  for (contour_level = contour.getFirstLevel();
       contour_level != 0;
       contour_level = contour.getNextLevel())
  {
    const ContourPolyline *polyline;
    
    for (polyline = contour_level->getFirstPolyline();
	 polyline != 0;
	 polyline = contour_level->getNextPolyline())
    {
      vector< ContourPoint > *points =
	(vector< ContourPoint > *)polyline->getPoints();
      
      int npoints = points->size();
      Symprod::wpt_t *point_array = new Symprod::wpt_t[npoints];
      
      vector< ContourPoint >::iterator point_iter;
      int point_num;
      
      for (point_iter = points->begin(), point_num = 0;
	   point_iter != points->end();
	   ++point_iter, ++point_num)
      {
	point_array[point_num].lat = (*point_iter).getLat();
	point_array[point_num].lon = (*point_iter).getLon();
      }
      
      prod.addPolyline(npoints,
		       point_array,
		       serverParams->color);
      
    } /* endfor - polyline */
    
  } /* endfor - contour_level */
  
  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}
