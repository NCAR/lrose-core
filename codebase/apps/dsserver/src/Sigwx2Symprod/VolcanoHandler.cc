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
 * @file VolcanoHandler.cc
 *
 * @class VolcanoHandler
 *
 * Class for handling volcano data.
 *  
 * @date 10/10/2009
 *
 */

#include <cmath>

#include <xmlformats/SigwxVolcanoBuffer.hh>

#include "Server.hh"
#include "VolcanoHandler.hh"

#include "CycloneDisplayItem.hh"
#include "RadiationDisplayItem.hh"
#include "SandstormDisplayItem.hh"
#include "VolcanoDisplayItem.hh"

using namespace std;



/*********************************************************************
 * Constructors
 */

VolcanoHandler::VolcanoHandler(Params *params,
			       const int debug_level) :
  ProductHandler(params, debug_level)
{
}


/*********************************************************************
 * Destructor
 */

VolcanoHandler::~VolcanoHandler()
{
}


/*********************************************************************
 * convertToSymprod()
 */

int VolcanoHandler::convertToSymprod(const string &dir_path,
				     const int prod_id,
				     const string &prod_label,
				     const Spdb::chunk_ref_t &chunk_ref,
				     const Spdb::aux_ref_t &aux_ref,
				     const void *spdb_data,
				     const int spdb_len,
				     MemBuf &symprod_buf,
				     const BoundingBox &bbox) const
{
  static const string method_name = "VolcanoHandler::convertToSymprod()";
  
  _printEntryInfo("volcano",
		  dir_path, prod_id, prod_label,
		  chunk_ref,
		  spdb_data, spdb_len, bbox);

  // Initialize the symprod object

  time_t curTime = time(0);
  Symprod symprod(curTime,    // generate time
		  curTime,    // received time
		  chunk_ref.valid_time,     // start time
		  chunk_ref.expire_time,    // expire time
		  chunk_ref.data_type,
		  chunk_ref.data_type2,
		  SPDB_WAFS_SIGWX_VOLCANO_LABEL);

  // Get the volcano information

  SigwxVolcanoBuffer volcano_buffer(_debugLevel >= 1, _debugLevel >= 5);
  
  if (!volcano_buffer.disassemble(spdb_data, spdb_len))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling SPDB buffer" << endl;
    cerr << "Skipping product" << endl;
    
    return 0;
  }
  
  // Display all of the volcanos in the buffer

  vector< SigwxVolcano > volcanos = volcano_buffer.getVolcanos();
  vector< SigwxVolcano >::const_iterator volcano;
  
  for (volcano = volcanos.begin(); volcano != volcanos.end(); ++volcano)
  {
    DisplayItem *ditem = 0;
    
    switch (volcano->getType())
    {
    case SigwxVolcano::TYPE_CYCLONE :
      ditem = new CycloneDisplayItem(volcano->getLatitude(),
				     volcano->getLongitude(),
				     volcano->getName(),
				     _params->cyclone_line_color,
				     _params->cyclone_line_width,
				     _params->cyclone_text_color,
				     _params->cyclone_text_bg_color,
				     _params->font_name,
				     _params->font_size,
				     _debugLevel);
      break;
      
    case SigwxVolcano::TYPE_VOLCANO :
      ditem = new VolcanoDisplayItem(volcano->getLatitude(),
				     volcano->getLongitude(),
				     volcano->getName(),
				     _params->volcano_line_color,
				     _params->volcano_line_width,
				     _params->volcano_text_color,
				     _params->volcano_text_bg_color,
				     _params->font_name,
				     _params->font_size,
				     _debugLevel);
      break;
      
    case SigwxVolcano::TYPE_SANDSTORM :
      ditem = new SandstormDisplayItem(volcano->getLatitude(),
				       volcano->getLongitude(),
				       _debugLevel);
      break;
      
    case SigwxVolcano::TYPE_RADIATION :
      ditem = new RadiationDisplayItem(volcano->getLatitude(),
				       volcano->getLongitude(),
				       volcano->getName(),
				       _params->radiation_line_color,
				       _params->radiation_line_width,
				       _params->radiation_text_color,
				       _params->radiation_text_bg_color,
				       _params->font_name,
				       _params->font_size,
				       _debugLevel);
      break;
    
    case SigwxVolcano::TYPE_UNKNOWN :
      // do nothing
      break;
      
    }

    if (ditem != 0)
    {
      ditem->draw(symprod, bbox, _params->volcano_icon_size);
    }

  } /* endfor - volcano */
  
  if (_debugLevel >= 5)
  {
    cerr << "ConvertVolcanoToSymprodMain: final symprod:" << endl;
    symprod.print(cerr);
  }
  symprod.serialize(symprod_buf);

  if (_debugLevel >= 1) 
    cerr << "\nConvertVolcanoToSymprod: exit" << endl;

  return 0;
} // end ConvertVolcanoToSymprod


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
