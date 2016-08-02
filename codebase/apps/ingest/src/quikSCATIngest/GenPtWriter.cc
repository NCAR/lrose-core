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

/*********************************************************************
 * GenPtWriter: Class that writes observation information to a GenPt
 *              SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <cstdio>

#include "GenPtWriter.hh"

using namespace std;

const string GenPtWriter::WIND_SPEED_FIELD_NAME = "speed";
const string GenPtWriter::WIND_SPEED_UNITS_NAME = "m/s";
const string GenPtWriter::WIND_DIRECTION_FIELD_NAME = "direction";
const string GenPtWriter::WIND_DIRECTION_UNITS_NAME = "degrees";
const string GenPtWriter::RAIN_FLAG_FIELD_NAME = "rain";
const string GenPtWriter::RAIN_FLAG_UNITS_NAME = "bool";
const string GenPtWriter::NSOL_FLAG_FIELD_NAME = "nsol";
const string GenPtWriter::NSOL_FLAG_UNITS_NAME = "bool";



/*********************************************************************
 * Constructors
 */

GenPtWriter::GenPtWriter(const string &output_url,
		       const int expire_secs,
		       const bool debug_flag) :
  SpdbWriter(output_url, expire_secs, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

GenPtWriter::~GenPtWriter()
{
}


/*********************************************************************
 * addInfo(quikSCATObs obs) - Add observation information
 *
 * Returns true on success, false otherwise.
 */

bool GenPtWriter::addInfo(const quikSCATObs &obs)
{
  static const string method_name = "GenPtWriter::addInfo()";

  GenPt outputGenPt;

  outputGenPt.setTime(obs.getObsTime().utime());
  outputGenPt.setLat(obs.getLatitude());
  outputGenPt.setLon(obs.getLongitude());
  outputGenPt.addFieldInfo(WIND_SPEED_FIELD_NAME, WIND_SPEED_UNITS_NAME);
  outputGenPt.addFieldInfo(WIND_DIRECTION_FIELD_NAME, WIND_DIRECTION_UNITS_NAME);
  outputGenPt.addFieldInfo(RAIN_FLAG_FIELD_NAME, RAIN_FLAG_UNITS_NAME);
  outputGenPt.addFieldInfo(NSOL_FLAG_FIELD_NAME, NSOL_FLAG_UNITS_NAME);

  outputGenPt.addVal(obs.getWindSpeed());
  outputGenPt.addVal(obs.getWindDir());
  outputGenPt.addVal(obs.getRainFlag());
  outputGenPt.addVal(obs.getNsolFlag());

  if (_debug)
    {
      cerr << endl;
      cerr << "Printing outputGenPt object" << endl;
      outputGenPt.print(cerr);
    }

  if (outputGenPt.assemble() != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error with outputGenPt.assemble(). Error string: " << outputGenPt.getErrStr() << endl;
      return false;
    }

  _outputSpdb.addPutChunk(0, outputGenPt.getTime(), outputGenPt.getTime() + _expireSecs, outputGenPt.getBufLen(),outputGenPt.getBufPtr());

  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
