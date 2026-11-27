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
// Sounding2Sndg.cc
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2005
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Spdb/sounding.h>
#include <rapformats/Sndg.hh>
#include "Sounding2Sndg.hh"
using namespace std;

// Constructor

Sounding2Sndg::Sounding2Sndg(int argc, char **argv) :
  _progName("Sounding2Sndg"),
  _args(_progName)
  
{
  
  isOK = TRUE;
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with command line args." << endl;
    isOK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    isOK = FALSE;
    return;
  }
  
  // process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Sounding2Sndg::~Sounding2Sndg()

{
   // unregister

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// 
// Run
//
int Sounding2Sndg::Run()
{
  PMU_auto_register("In Sounding2Sndg main loop");
  
  if ( _params.mode == Params::ARCHIVE )
    {
       PMU_auto_register("Running ARCHIVE mode");
      //
      // Set up the time interval
      //
      DateTime start( _params.start_time);

      DateTime end( _params.end_time);

      time_t time_begin = start.utime();

      time_t time_end = end.utime();

       if (_params.debug)
         {
            cerr << "Converting input data for time interval ["
                 << time_begin << " , " << time_end << "]." << endl;
         }

       //
       // Convert the Sounding object at this time to Sndg
       //
       _convert(time_begin, time_end);
    }
  else //REALTIME mode
    {
      string urlStr(_params.input_url);
      DsURL dsUrl(urlStr);
	  
      DsLdataInfo ldata(dsUrl, _params.debug);

      while (true) 
	{
	  PMU_auto_register("Running REALTIME mode");
	  
	  ldata.readBlocking((int)_params.max_valid_age,_params.sleep_time,PMU_auto_register);
	   
	  time_t dataTime = ldata.getLatestTime();
	      
	  _convert(dataTime, dataTime);
	
	}
    }
  
  return 0;

}

int Sounding2Sndg::_convert(time_t start_time, time_t end_time)
{
  DsSpdb spdbIn;

  if (_params.debug)
    {
      cerr << "Sounding2Sndg::_convert :" << endl;
      cerr << "Reading data from " << _params.input_url << " in interval " 
	   << "[ " <<  start_time << ", " << end_time << " ]" << endl;
    }

  if ( spdbIn.getInterval( _params.input_url, start_time, end_time) != 0 )
     {
       cerr << "Sounding2Sndg::_convert(): Problem reading data at "
	    << _params.input_url << " in interval " << "[ " 
	    <<  start_time << ", " << end_time << " ]" << endl;
       return( -1 );
     }

   //
   // Get number of chunks
   // 
   int nChunks = spdbIn.getNChunks();

   vector <Spdb::chunk_t> chunks = spdbIn.getChunks();

   if (_params.debug)
     cerr << nChunks << " chunk(s) found." << endl;

   Spdb::chunk_ref_t *refs = spdbIn.getChunkRefs();   
      
   //
   // Convert each sounding to a Sndg object and write it to the 
   // database.
   //
   for (int i = 0; i < nChunks; i++)
   {

      if (_params.debug)
         cerr << "Processing chunk " << i << endl;

      //
      // Swap data retrieved from database if necessary
      //
      SNDG_spdb_product_t *sounding = (SNDG_spdb_product_t *)chunks[i].data;
      
      SNDG_spdb_product_from_BE(sounding);

      //
      // Set Sndg header data
      //
      Sndg::header_t sndgHdr;

      sndgHdr.launchTime = sounding->launchTime;
      
      sndgHdr.nPoints = sounding->nPoints;
 
      sndgHdr.sourceId = sounding->sourceId;

      sndgHdr.leadSecs = sounding->leadSecs;

      sndgHdr.lat = sounding->lat;

      sndgHdr.lon = sounding->lon;

      sndgHdr.alt  = sounding->alt;

      sprintf( sndgHdr.sourceName, "%s", sounding->sourceName );
      
      sprintf( sndgHdr.sourceFmt, "%s", sounding->sourceFmt );
      
      sprintf( sndgHdr.siteName, "%s", sounding->siteName );
     
      for (int j = 0; j < Sndg::HDR_SPARE_INTS; j++)
	sndgHdr.spareInts[j] = (int) Sndg::VALUE_UNKNOWN;

      for (int j = 0; j < Sndg::HDR_SPARE_FLOATS; j++)
	sndgHdr.spareFloats[j] = Sndg::VALUE_UNKNOWN;
      
      Sndg sndg;
      
      sndg.setHeader(sndgHdr);

      // 
      // Get pointer to first sounding point
      //
      int pointOffset = sizeof( SNDG_spdb_product_t ) - sizeof( SNDG_spdb_point_t );
      
      SNDG_spdb_point_t *dataPtr = (SNDG_spdb_point_t *) ((char *) sounding + pointOffset);

      //
      // Copy point data to Sndg::point_t format
      //
      vector <Sndg::point_t*> sndgPts;

      for( int j = 0; j < sounding->nPoints; j++ ) 
	{
	  dataPtr = (SNDG_spdb_point_t *)((char *) sounding + pointOffset );

	  Sndg::point_t *point = new Sndg::point_t;
	  memset(point, 0, sizeof(Sndg::point_t));
	  point->time = Sndg::VALUE_UNKNOWN;
	  point->pressure = dataPtr->pressure ;
	  point->altitude =  dataPtr->altitude;
	  point->u = dataPtr->u;
	  point->v = dataPtr->v;
	  point->w = dataPtr->w;
	  point->rh = dataPtr->rh;
	  point->temp = dataPtr->temp ;
	  point->dewpt = Sndg::VALUE_UNKNOWN ;
	  point->windSpeed = Sndg::VALUE_UNKNOWN ;
	  point->windDir = Sndg::VALUE_UNKNOWN ;
	  point->ascensionRate = Sndg::VALUE_UNKNOWN ;
	  point->longitude = Sndg::VALUE_UNKNOWN ;
	  point->latitude = Sndg::VALUE_UNKNOWN ;
	  point->pressureQC = Sndg::VALUE_UNKNOWN ;
	  point->tempQC = Sndg::VALUE_UNKNOWN ;
	  point->humidityQC = Sndg::VALUE_UNKNOWN ;
	  point->uwindQC = Sndg::VALUE_UNKNOWN ;
	  point->vwindQC = Sndg::VALUE_UNKNOWN ;
	  point->ascensionRateQC = Sndg::VALUE_UNKNOWN ;
	  for (int l = 0; l < Sndg::PT_SPARE_FLOATS; l++)
	    point->spareFloats[l] = Sndg::VALUE_UNKNOWN;	  
	  sndg.addPoint(*point);
	  sndgPts.push_back(point);

	  //
	  // Move on to the next point
	  // 
	  pointOffset += sizeof( SNDG_spdb_point_t );
	}

      sndg.assemble();

      DsSpdb spdbOut;
      spdbOut.setPutMode( Spdb::putModeOver );

      if (spdbOut.put( _params.output_url,
		       SPDB_SNDG_PLUS_ID,
		       SPDB_SNDG_PLUS_LABEL,
		       refs[i].data_type,
		       refs[i].valid_time,
		       refs[i].expire_time,
		       (int)sndg.getBufLen(),
		       (const void *)sndg.getBufPtr(), 
		       refs[i].data_type2) != 0 )
	cerr << "Could not write Sndg output to url " << "_params.output_url\n";
      else if (_params.debug)
 	cerr << "Writing Sndg data to " << _params.output_url << endl;

      // Optionally, delay. This is unfortunate and hopefully
      // can go away, but for now it may be needed to avoid
      // overloading the DsSpdbServer.
      //
      for (int idelay=0; idelay < _params.postWriteDelay; idelay++){
	PMU_auto_register("Delaying");
	sleep(1);
      }


      //
      // Free sounding point memory
      //
      vector<Sndg::point_t*>::iterator k ;
      
      for (k = sndgPts.begin(); k != sndgPts.end(); k++)
	delete *k;
       
      sndgPts.erase(sndgPts.begin(), sndgPts.end());
       
   }

   chunks.erase(chunks.begin(), chunks.end());

   return 0;
}








