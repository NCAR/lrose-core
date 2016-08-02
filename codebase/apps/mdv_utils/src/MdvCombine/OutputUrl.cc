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
/*
 *  $Id: OutputUrl.cc,v 1.11 2016/03/04 02:22:10 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	OutputUrl
//
// Author:	G M Cunning
//
// Date:	Wed Jan 17 17:04:03 2001
//
// Description: This class manages the output MDV data.
//
//
//
//
//
//


// C++ include files

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/str.h>

// Local include files
#include "OutputUrl.hh"
#include "Params.hh"
using namespace std;


// define any constants
const string OutputUrl::_className    = "OutputUrl";




/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
OutputUrl::OutputUrl(Params *params)
{
  _params = params;
  _mdvx = new DsMdvx();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
OutputUrl::~OutputUrl()
{
  delete _mdvx;
  _params = 0;
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
// Method Name:	OutputUrl::initHeaders
//
// Description:	This method initializes the headers for the ouptu MDV
//		data.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void 
OutputUrl::initMasterHeader(const Mdvx::master_header_t *in_mhdr)
{

  _mdvx->clearWrite();

  _mdvx->setWriteLdataInfo();

  Mdvx::master_header_t outMhdr;
  // clear the master header
  memset(&outMhdr, 0, sizeof(Mdvx::master_header_t));

  // fill the master header
  
  outMhdr.num_data_times = 1;
  if (in_mhdr->max_nz == 1) {
    outMhdr.data_dimension = 2;
  } else {
    outMhdr.data_dimension = 3;
  }
  outMhdr.data_collection_type = in_mhdr->data_collection_type;
  outMhdr.native_vlevel_type = in_mhdr->native_vlevel_type;
  outMhdr.vlevel_type = in_mhdr->vlevel_type;
  outMhdr.vlevel_included = 1;
  outMhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  outMhdr.data_ordering = Mdvx::ORDER_XYZ;
  outMhdr.n_fields = _params->field_info_n;
  outMhdr.max_nx = in_mhdr->max_nx;
  outMhdr.max_ny = in_mhdr->max_ny;
  outMhdr.max_nz = in_mhdr->max_nz;
  outMhdr.n_chunks = 0;
  outMhdr.field_grids_differ = 0;
  outMhdr.sensor_lon = in_mhdr->sensor_lon;
  outMhdr.sensor_lat = in_mhdr->sensor_lat;
  outMhdr.sensor_alt = in_mhdr->sensor_alt;

  // data set name and source
  
  STRncopy(outMhdr.data_set_name, _params->data_set_name, MDV_NAME_LEN);
  STRncopy(outMhdr.data_set_source, _params->data_set_source, MDV_NAME_LEN);
 
  _mdvx->setMasterHeader(outMhdr);

}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::addToInfo
//
// Description:	concatenates string to data_set_info field of master header.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void 
OutputUrl::addToInfo(const char *info_str)
{
  Mdvx::master_header_t mhdr = _mdvx->getMasterHeader();
  STRconcat(mhdr.data_set_info, info_str, MDV_INFO_LEN);
  _mdvx->setMasterHeader(mhdr);
  

}

void 
OutputUrl::addToInfo(const string &info_str)
{
  addToInfo(info_str.c_str());
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::clear
//
// Description:	clear out the current information in the output URL
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void
OutputUrl::clear(void)
{
  delete _mdvx;
  _mdvx = new DsMdvx();
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::addField
//
// Description:	~description
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void
OutputUrl::addField(MdvxField* in_field)
{
  _mdvx->addField(in_field);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::writeVol
//
// Description:	this method writes the new output volume.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

bool
OutputUrl::writeVol(const time_t& merge_time, const time_t& start_time, 
		    const time_t& end_time)
{
  Mdvx::master_header_t mhdr = _mdvx->getMasterHeader();


  if (_params->write_forecast == FALSE)
  {
        time_t genTime = merge_time;
        mhdr.time_gen = genTime;
  	mhdr.time_begin = start_time;
  	mhdr.time_end = end_time;
  	mhdr.time_centroid = merge_time;
  	mhdr.time_expire = mhdr.time_centroid +
    	(mhdr.time_end - mhdr.time_begin);
  
  	_mdvx->setMasterHeader(mhdr);

  }
  else if (_params->write_forecast == TRUE)
  {

	time_t genTime = start_time - _params->fcast_lead_time.lead_time_secs;
	time_t leadSecs = _params->fcast_lead_time.lead_time_secs;
    mhdr.time_gen = genTime;
	mhdr.time_begin = genTime + leadSecs;
	mhdr.time_end = genTime + leadSecs;
	mhdr.time_centroid = genTime + leadSecs;
	mhdr.time_expire   = mhdr.time_centroid + (mhdr.time_end - mhdr.time_begin);

  	_mdvx->setMasterHeader(mhdr);
	_mdvx->updateMasterHeader();
  	_mdvx->setWriteAsForecast();
  }

  if (_params->debug) {
    cerr << "Writing combined MDV file, time " << utimstr(mhdr.time_centroid) 
	 << ", to URL " << _params->output_url << endl;
  }

  if (_mdvx->writeToDir(_params->output_url) != 0) {
    cerr << "WARNING: " << endl;
    cerr << "Error writing data for time ";
    cerr << utimstr(mhdr.time_centroid);
    cerr << " to URL " << _params->output_url << endl;
    cerr << "*** Exiting ***" << endl << endl;
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

