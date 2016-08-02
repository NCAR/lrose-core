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
#include "AdvectPolygon.hh"
using namespace std;

//
// Constructor
//
AdvectPolygon::AdvectPolygon(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "AdvectPolygon";
  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
AdvectPolygon::~AdvectPolygon()

{

  _clear();

  //
  // unregister process
  //
  PMU_auto_unregister();

  
}

void AdvectPolygon::_clear()
{
  vector <GenPoly *> :: const_iterator i;

  for (i = _polygons.begin(); i != _polygons.end(); i++)
    delete (*i);

  _polygons.erase(_polygons.begin(), _polygons.end());

  _dbzGrid.clear();
  
  _windGrid.clear();

}


//////////////////////////////////////////////////
// 
// Run
//
int AdvectPolygon::Run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");


  DsTrigger *trigger = NULL;
  
  
  //
  // set up triggering
  //
  if (_params.mode == Params::ARCHIVE)
    {
      //
      // Create archive timelist trigger
      //
      if (_params.debug)
	{
	  cerr << "AdvectPolygon::Run(): Creating ARCHIVE trigger\n"  
	       << "     start time: " << _args.startTime 
	       << "     end time:   " << _args.endTime << endl;
	}
      
      DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();
      
      if (archive_trigger->init(_params.wind_url,
				_args.startTime,
				_args.endTime) != 0)
	{
	  cerr << archive_trigger->getErrStr();
	  trigger = 0;
	  return 1;
	}
      else
	{
	  trigger = archive_trigger;
	}
    }
  else if (_params.mode == Params::REALTIME)
    {
      if (_params.debug)
	{
	  cerr << "AdvectPolygon::Run(): Creating REALTIME trigger" << endl;
	}
      
      //
      // realtime mode
      //
      DsLdataTrigger *realtime_trigger = new DsLdataTrigger();
      if (realtime_trigger->init(_params.wind_url,
				 _params.max_valid_realtime_age,
				 PMU_auto_register))
	{
	  cerr << realtime_trigger->getErrStr();
	  trigger = 0;
	  return 1;
	}
      else
	{
	  trigger = realtime_trigger;
	}
      
    }

  //
  //
  // process data
  //
  time_t inputTime;

  while (!trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      inputTime = trigger->next(triggerInfo);   
      if (_processData(triggerInfo.getIssueTime(), triggerInfo.getFilePath()))
	{
          cerr << "AdvectPolygon::Run" <<"  Errors in processing time: " <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  return 1;
        }
    } // while
  
  delete trigger;

  return 0;
  
}


///////////////////////////////////
//
//  process data at trigger time
//
int AdvectPolygon::_processData(time_t input_time, const string filepath)
{

  if (_params.debug)
    {
      cerr << "AdvectPolygon::_processData: Processing time: " << input_time  
	   << " file trigger: " << filepath << endl;
    }

  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  //
  // Get the relevant polygons from the database
  //
  time_t time_begin = input_time - _params.lookback;
  
  time_t time_end = input_time;

  //
  // read in dbz data
  //
  if (_dbzGrid.init(_params, input_time) )
    {
      cerr << "AdvectPolygon::_processData(): ERROR: Failure to read Mdv radar data!"
	   << " (Check URL). Request time: " << input_time << endl;
    }

  //
  // read in wind data
  //
  if (_windGrid.init(_params, input_time) )
    {
      cerr << "AdvectPolygon::_processData(): ERROR: Failure to read Mdv wind data!"
	   << " (Check URL). Request time: " << input_time << endl;
    }
  
  if ( _readSpdb( time_begin, time_end) )
    {
      cerr << "AdvectPolygon::_processData(): ERROR: Failure to read database!"
	   << " (Check URL). Input time: " << input_time << endl;
    }

  if( (int) _polygons.size() > 0 )  
    {
      _advectPolygons(input_time);

      _writePolygons();
    }

  //
  // Cleanup
  //
  _clear();

  return 0;

}

/////////////////////////////////////////////////////////////////
//
// Read polygons from database for time interval
// and store in class member vector _polygons.
//
int AdvectPolygon::_readSpdb(time_t time_begin, time_t time_end)
{

  //
  // Read the spdb database of ac_posn points.
  //
  DsSpdb spdbMgr;

  if (spdbMgr.getInterval(_params.polygon_url, time_begin, time_end))
    {
      cerr << "AdvectPolygon::_processData(): getInterval failed for " << _params.polygon_url << ", ["
           << time_begin << "," << time_end << "]" <<  endl;
      return 1;
    }

  //
  // Get chunk data
  //
  int nChunks = spdbMgr.getNChunks();
  
  vector <Spdb::chunk_t> chunks  = spdbMgr.getChunks();

  if (_params.debug)
    {
      cerr << "AdvectPolygon::readSpdb(): "
           << nChunks << " found in interval [" << time_begin << "," << time_end << "]" << endl;
    }

  //
  // Loop through the chunks,disassmeble, and save.
  //
  //  
  for (int i = 0; i < nChunks; i++)
   {
     //
     // Dissassemble the data into a GenPoly struct.
     //
     GenPoly *polygon = new GenPoly();

     bool success = polygon->disassemble((const void*) chunks[i].data, chunks[i].len);

     if (!success)
       {
         cerr << "AdvectPolygon::_readSpdb " << polygon->getErrStr() << endl;
	 delete polygon;
         continue;
       }
     
     //
     // If the polygon's life has exceeded its limit do not advect or process.
     //
     else  if ( (int)time_end  - polygon->getId() > _params.polygon_expire_seconds)
       {
	 delete polygon;
	 continue;
       }
     else
       _polygons.push_back(polygon);

     //
     // Debug message:
     // Construct and print tailnum from spdb data types, print initiation time 
     //
     string str1 = Spdb::dehashInt32To4Chars(chunks[i].data_type);

     string str2 = Spdb::dehashInt32To4Chars(chunks[i].data_type2);
     
     string tailnum = str1 + str2;

     DateTime polygonInitTime;

     polygonInitTime.set(polygon->getId());

     if (_params.debug == Params::DEBUG_VERBOSE)
       {
	 cerr << "AdvectPolygon::_readSpdb(): tailnum: " << tailnum 
	      << ". Init time " << polygon->getId() << " " << polygonInitTime.dtime() << endl;
       } 
   }  

  return 0;
  
}

//
// polygons advected to dataTime
//
int AdvectPolygon::_advectPolygons(time_t dataTime)
{
  for (int i = 0; i < (int)_polygons.size(); i++)
    { 

      //
      // Get the centroid
      //
      float centroidLon, centroidLat;

      int numVertices = _polygons[i]->getNumVertices();

      _polygons[i]->calcCentroid(centroidLat,centroidLon);
      
      if (_params.debug == Params::DEBUG_VERBOSE)
	{
	  cerr << "AdvectPolygon::_advectPolygons(): centroidLat, centroidLon " 
	       << centroidLat << " " << centroidLon << endl;
	}

      //
      // get the time
      //
      time_t polygonTime = _polygons[i]->getTime();

      //
      // Get the dx and dy from cetroid to each point in the polygon so that once
      // cetroid is advected, we can reconstruct the polygon from the dx and dy values
      //
      float dx[numVertices], dy[numVertices];
      
      for (int j = 0; j < numVertices; j++)
	{
	  GenPoly::vertex_t vertex = _polygons[i]->getVertex(j);

	  dx[j] = centroidLon - vertex.lon;

	  dy[j] = centroidLat - vertex.lat;
	}

      //
      // Advect centroid
      //
      float newCentroidLat, newCentroidLon;

      _windGrid.advectPoint(centroidLat, centroidLon, newCentroidLat, newCentroidLon, polygonTime);

      //
      // Clear out the old vertices and add the vertices of the advected polygon
      // with vertices constructed from the centroid and relevant dx and dy values.
      //
      _polygons[i]->clearVertices();
      
      for (int j = 0; j < numVertices; j++)
	{
	  GenPoly::vertex_t newVertex;

	  newVertex.lat = newCentroidLat - dy[j];

	  newVertex.lon = newCentroidLon -  dx[j];

	  _polygons[i]->addVertex(newVertex);
	  
	}
      
      _polygons[i]->clearVals();
      
      _polygons[i]->clearFieldInfo();

      double rainRate = _dbzGrid.calculateRainRate( _polygons[i]);

      _polygons[i]->addFieldInfo("rain rate", "mm/hr");

      _polygons[i]->addVal(rainRate);

      //
      // reset the time	
      //	
      _polygons[i]->setTime(dataTime);
      
      _polygons[i]->setExpireTime(dataTime + _params.expire_seconds);
    }

  return 0;
}


int AdvectPolygon::_writePolygons()
{
  if (_params.debug)
    {
      cerr << "AdvectPolygon::_writeGenPoly(): Writing output to " << _params.output_url << "\n" << endl;
    }

  for (int i = 0; i < (int)_polygons.size(); i++)
    {      
      //
      // Construct the data types from the tail number which is stored in the 
      // GenPoly name member.
      //
      int dataType1, dataType2;
      
      const char *tail = _polygons[i]->getName().c_str();
      
      dataType1 = Spdb::hash4CharsToInt32( tail );
      
      const char *strPtr = tail  + 4;
      
      dataType2 = Spdb::hash4CharsToInt32(strPtr);
      
      _polygons[i]->assemble();

      DsSpdb spdb;
      
      spdb.setPutMode(Spdb::putModeAddUnique);
      
      int ret = spdb.put(_params.output_url,
			 SPDB_GENERIC_POLYLINE_ID,
			 SPDB_GENERIC_POLYLINE_LABEL,
			 dataType1,
			 _polygons[i]->getTime(),
			 _polygons[i]->getExpireTime(),
			 _polygons[i]->getBufLen(),
			 _polygons[i]->getBufPtr(),
			 dataType2);
      if (ret)
	{
	  cerr << "AdvectPolygon::_writePolygons(): Write failed to url: "
	       <<  _params.output_url << endl;
	  
	  return 1;
	}
    }

  return 0;

}
