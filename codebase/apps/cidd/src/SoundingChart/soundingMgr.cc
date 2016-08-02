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
//
// Object that creates and stores the sounding objects.
//

#include "soundingMgr.hh"
#include <toolsa/DateTime.hh>

using namespace std;


//
// Constructor.  Determines which sounding to read.
// Reads the data and fills up the vector.
//
soundingMgr::soundingMgr(const Params &params,
                         time_t startTime,
			 time_t endTime,
			 char *url,
			 int dataType,
			 char *label) :
        _params(params)

{
  //
  //
  Sndg::point_t pt;  // Sounding Plus data format
  Sndg::header_t hdr;

  SNDG_spdb_product_t *s;

  _label = label;
  //
  // Do a get interval, getting the times only.
  //
  if (_params.debug) {
    cerr << "Getting sounding in interval" << endl;
    cerr << "  url: " << url << endl;
    cerr << "  Start time: " << DateTime::strm(startTime) << endl;
    cerr << "  End time: " << DateTime::strm(endTime) << endl;
    cerr << "  dataType: " << dataType << endl;
  }
  DsSpdb theTimes;
  // if(dataType > 0) {
  if (theTimes.getInterval(url, startTime, endTime, dataType)) {
    cerr << "ERROR - soundingMgr::soundingMgr" << endl;
    cerr << " Cannot get soundings in interval" << endl;
    cerr << theTimes.getErrStr() << endl;
    cerr << "  url: " << url << endl;
    cerr << "  Start time: " << DateTime::strm(startTime) << endl;
    cerr << "  End time: " << DateTime::strm(endTime) << endl;
    cerr << "  dataType: " << dataType << endl;
  }
  // }
  
  const vector<Spdb::chunk_t> &chunks = theTimes.getChunks();

  if (_params.debug) {
    cerr << "Found " << theTimes.getNChunks() << " soundings" << endl;
  }

  for (int ichunk = 0; ichunk < theTimes.getNChunks(); ichunk++){
    // Get the sounding time.
    //
    //time_t soundingTime = chunks[ichunk].valid_time;
    // cerr << " Data Chunk " << ichunk  << " Len " << chunks[ichunk].len << " ADD: " << chunks[ichunk].data << endl;

    //
    // Alloc Sounding object.
    //
    Sndg *sg = new Sndg();

	int p_id = theTimes.getProdId();
	switch (p_id) {
	  default:
          cerr << "Data not in SPDB_SNDG or SPDB_SNDG_PLUS Format" << endl;
	  break;

	  case SPDB_SNDG_ID: // Promote sounding point to sounding plus point.
		  s = (SNDG_spdb_product_t *) chunks[ichunk].data;
		  SNDG_spdb_product_from_BE(s);

		  for(int i=0; i < s->nPoints; i++) {
			  pt.time = Sndg::VALUE_UNKNOWN;
			  pt.pressure = s->points[i].pressure;
			  pt.altitude = s->points[i].altitude;
			  pt.u = s->points[i].u;
			  pt.v = s->points[i].v;
			  pt.w = s->points[i].w;
			  pt.rh = s->points[i].rh;
			  pt.temp = s->points[i].temp;
			  pt.dewpt = Sndg::VALUE_UNKNOWN;
			  if(s->points[i].u == SNDG_VALUE_UNKNOWN ||
			    s->points[i].u == SNDG_VALUE_UNKNOWN) {
				pt.windSpeed = Sndg::VALUE_UNKNOWN;
				pt.windDir = Sndg::VALUE_UNKNOWN;
			  } else {
			  	pt.windSpeed = PHYwind_speed(s->points[i].u,s->points[i].v);
			  	pt.windDir =  PHYwind_dir(s->points[i].u,s->points[i].v);
			  }

			  pt.ascensionRate = Sndg::VALUE_UNKNOWN;
			  pt.longitude = Sndg::VALUE_UNKNOWN;
			  pt.latitude = Sndg::VALUE_UNKNOWN;
			  pt.pressureQC = Sndg::VALUE_UNKNOWN;
			  pt.tempQC = Sndg::VALUE_UNKNOWN;
			  pt.humidityQC = Sndg::VALUE_UNKNOWN;
			  pt.uwindQC = Sndg::VALUE_UNKNOWN;
			  pt.vwindQC = Sndg::VALUE_UNKNOWN;
			  pt.ascensionRateQC = Sndg::VALUE_UNKNOWN;

			  for(int j = 0; j < SNDG_PNT_SPARE_FLOATS; j++) {
			      pt.spareFloats[j] = s->points[i].spareFloats[j];
			  }
			  for(int j = SNDG_PNT_SPARE_FLOATS; j < Sndg::PT_SPARE_FLOATS; j++) {
			      pt.spareFloats[j] = Sndg::VALUE_UNKNOWN;
			  }

			sg->addPoint(pt);
		  }
		  hdr.launchTime = s->launchTime;
		  hdr.nPoints = s->nPoints;
		  hdr.sourceId = s->sourceId;
		  hdr.leadSecs = s->leadSecs;

		  for(int j = 0; j < Sndg::HDR_SPARE_INTS; j++) {
		    hdr.spareInts[j] = s->spareInts[j];
		  }

		  hdr.lat = s->lat;
		  hdr.lon = s->lon;
		  hdr.alt = s->alt;

		  for(int j = 0; j < Sndg::HDR_SPARE_FLOATS; j++) {
		    hdr.spareFloats[j] = s->spareFloats[j];
		  }

		  strncpy(hdr.sourceName,s->sourceName,Sndg::SRC_NAME_LEN);

		  strncpy(hdr.sourceFmt,s->sourceFmt,Sndg::SRC_FMT_LEN);

		  strncpy(hdr.siteName,s->siteName,Sndg::SITE_NAME_LEN);

		  sg->setHeader(hdr);

          _soundings.push_back( sg );
	  break;


	  case SPDB_SNDG_PLUS_ID:
	    if(chunks[ichunk].data != NULL &&
		   sg->disassemble(chunks[ichunk].data,chunks[ichunk].len) == 0 ) {
          // cerr << "Adding Observed  Sounding to Vector" << endl;
          _soundings.push_back( sg );
	    }
	  break;
	}
  }

}
  
//
// Destructor
//
soundingMgr::~soundingMgr(){
  for (unsigned is=0; is < _soundings.size(); is++){
    delete _soundings[is];
  }
}

//
// Get the number of soundings.
//
int soundingMgr::getNumSoundings(){
  return _soundings.size();
}
//
// Get the nth sounding.
//
Sndg *soundingMgr::getSoundingData(int n){
  return _soundings[n];
}
//
// Get the label in use.
//
char *soundingMgr::getLabel(){
  return _label;
}
