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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:39:56 $
//   $Id: SndgSpdbWriter.cc,v 1.2 2016/03/07 01:39:56 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SndgSpdbWriter: Class that writes Sndg information to a Sndg
 *                 SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include <Spdb/SoundingPut.hh>

#include "SndgSpdbWriter.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

SndgSpdbWriter::SndgSpdbWriter(const string &output_url,
			       const int expire_secs,
			       const bool debug_flag) :
  SpdbWriter(output_url, expire_secs, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

SndgSpdbWriter::~SndgSpdbWriter()
{
}


/*********************************************************************
 * writeSndg() - Write the given sounding to the output database.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool SndgSpdbWriter::writeSndg(const Sndg &sounding)
{
  static const string method_name = "SndgSpdbWriter::writeSndg()";
  
  // Construct the output sounding

  Sndg::header_t sndg_hdr = sounding.getHeader();
  
  SoundingPut sounding_put;
  vector< string* > output_urls;
  
  output_urls.push_back(&_outputUrl);
  
  sounding_put.init(output_urls, Sounding::DEFAULT_ID, "",
		    0, sndg_hdr.siteName,
		    sndg_hdr.lat, sndg_hdr.lon, sndg_hdr.alt,
		    Sndg::VALUE_UNKNOWN);
  
  vector< double > height;
  vector< double > u;
  vector< double > v;
  vector< double > w;
  vector< double > prs;
  vector< double > relHum;
  vector< double > temperature;
  
  vector< Sndg::point_t > sndg_pts = sounding.getPoints();
  vector< Sndg::point_t >::const_iterator sndg_pt;
  
  for (sndg_pt = sndg_pts.begin(); sndg_pt != sndg_pts.end(); ++sndg_pt)
  {
    height.push_back(sndg_pt->altitude);
    u.push_back(sndg_pt->u);
    v.push_back(sndg_pt->v);
    w.push_back(sndg_pt->w);
    prs.push_back(sndg_pt->pressure);
    relHum.push_back(sndg_pt->rh);
    temperature.push_back(sndg_pt->temp);
  } /* endfor - sndg_pt */
  
  sounding_put.set(sndg_hdr.launchTime,
		   &height, &u, &v, &w, &prs, &relHum, &temperature);
  
  // Write the sounding to the database

  if (sounding_put.writeSounding(sndg_hdr.launchTime,
				 sndg_hdr.launchTime + _expireSecs) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing sounding to output URL: " << _outputUrl << endl;
    
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
