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
#include "AcTrack2Polygon.hh"
#include <toolsa/TaArray.hh>
#include <toolsa/TaArray2D.hh>
using namespace std;

//
// Constructor
//
AcTrack2Polygon::AcTrack2Polygon(int argc, char **argv)

{

  isOK = true;

  //
  // set programe name
  //
  _progName = "AcTrack2Polygon";
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
  _paramsPath = (char *) "unknown";
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
AcTrack2Polygon::~AcTrack2Polygon()

{

  //
  // unregister process
  //
  PMU_auto_unregister();

}

void AcTrack2Polygon::clear()

{
  vector <ac_posn_wmod_t *> :: const_iterator i;

  for (i = _acPosVec.begin(); i != _acPosVec.end(); i++)
    delete (*i);

  _acPosVec.erase(_acPosVec.begin(), _acPosVec.end());

  genPoly.clear();
}


//////////////////////////////////////////////////
// 
// Run
//
int AcTrack2Polygon::Run ()
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
	  cerr << "AcTrack2Polygon::Run(): Creating ARCHIVE trigger\n"  
	       << "     start time: " << _args.startTime 
	       << "     end time:   " << _args.endTime << endl;
	}
      
      DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();
      
      if (archive_trigger->init(_params.trigger_url,
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
	  cerr << "AcTrack2Polygon::Run(): Creating REALTIME trigger" << endl;
	}
      
      //
      // realtime mode
      //
      DsLdataTrigger *realtime_trigger = new DsLdataTrigger();
      if (realtime_trigger->init(_params.trigger_url,
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
          cerr << "Error - AcTrack2Polygon::Run" << endl;
          cerr << "  Errors in processing time: " <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
          cerr << " inputTime: " << DateTime::strm(inputTime) << endl;
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
int AcTrack2Polygon::_processData(time_t input_time, const string file_path)

{
  if (_params.debug)
    {
      cerr << "AcTrack2Polygon::_processData: Processing time: " << input_time 
	   << " file : " << file_path << endl;
    }

  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  //
  // Get the aircraft position points from the database
  //
  time_t time_begin = input_time - _params.lookback;
  
  time_t time_end = input_time;

  if ( _readSpdb( time_begin, time_end) )
    {
      cerr << "AcTrack2Polygon::_processData(): ERROR: Failure to read aircraft position database!"
	   << " (Check URL). Input time: " << input_time << endl;
    }

  //
  // Create polygon around points if we have 3 or more, expand/inflate polygon if desired
  // then write data.
  //
  if ( _acPosVec.size() > 2 )
    {

      if (_createConvexHull(input_time))
	{
	  cerr << "AcTrack2Polygon::_processData(): ERROR: Failure to create polygon from flight track!"
	       <<  " Input time: " << input_time << endl;
	}

      if (_params.expansion_factor > 0)
	_expandTrack();
 
      
      //
      // Create polygon around points
      //
      if (_writeGenPoly())
	{
	  cerr << "AcTrack2Polygon::_processData(): ERROR: Failure to create polygon from flight track!"
	       <<  " Input time: " << input_time << endl;
	}
    }

  //
  // Cleanup
  //
  clear();

  return 0;

}

/////////////////////////////////////////////////////////////////
//
// Read aircraft position points from database for time interval
// and store in class member vector _acPosVec.
//
int AcTrack2Polygon::_readSpdb(time_t time_begin, time_t time_end)
{

  //
  // Construct two data types from the tailnumber. We will query the database for 
  // only points with matching tailnumber or call sign to the tailnum in the parameter file.
  //
  int dataType1 = Spdb::hash4CharsToInt32(_params.tailnum );

  char *strPtr = _params.tailnum + 4;

  int dataType2 = Spdb::hash4CharsToInt32(strPtr);
  
  //
  //
  // Read the spdb database of ac_posn points.
  //
  DsSpdb spdbMgr;

  if (spdbMgr.getInterval(_params.ac_posn_url, time_begin, time_end, dataType1, dataType2))
    {
      cerr << "AcTrack2Polygon::_processData(): getInterval failed for " << _params.ac_posn_url << ", ["
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
      cerr << "AcTracks2Polygon::readSpdb(): "
           << nChunks << " aircraft position points found." << endl;
    }

  //
  // Loop through the chunks and disassmeble.
  //  
  for (int i = 0; i < nChunks; i++)
   {

     ac_posn_wmod_t *acPos = new ac_posn_wmod_t;

     if ( sizeof( chunks[i].data ) == sizeof( ac_posn_wmod_t) )
       {
	 BE_to_ac_posn_wmod((ac_posn_wmod_t*)chunks[i].data);
	 memcpy(acPos, chunks[i].data,  sizeof( ac_posn_wmod_t));
       }
     else
       {
	 //
	 // Note: we will use the ac_posn_wmod_t struct to store data
	 // since it contains a superset of the ac_posn_t elements.
	 //
	 BE_to_ac_posn((ac_posn_t*)chunks[i].data);
	 acPos->lat = ((ac_posn_t*)chunks[i].data)->lat;
	 acPos->lon = ((ac_posn_t*)chunks[i].data)->lon;
	 acPos->alt = ((ac_posn_t*)chunks[i].data)->alt;
	 memcpy(acPos->callsign,((ac_posn_t*)chunks[i].data)->callsign,AC_POSN_N_CALLSIGN);  
       }

     _acPosVec.push_back(acPos);

     if (_params.debug == Params::DEBUG_VERBOSE)
       {
	 cerr << "AcTrack2Polygon::_readSpdb(): acPosn lat: " << acPos->lat << " lon: " << acPos->lon 
	      << " alt: " << acPos->alt << " callsign: " << acPos->callsign << endl;
       } 
   }  

  return 0;
  
}

///////////////////////////////////////////////////////
// 
// Create polygon around aircraft position points
//
int AcTrack2Polygon::_createConvexHull(time_t dataTime)
{

  int numPoints = _acPosVec.size();

  //
  // Declare and create arguments for libeuclid convex hull routines
  //

  TaArray<double *> pointPtr_;
  double **pointPtr = pointPtr_.alloc(numPoints + 1);
  
  TaArray2D<double> points_;
  double **points = points_.alloc(numPoints, 2);

  for (int i = 0; i < numPoints; i++) {
    points[i][0] = (double)_acPosVec[i]->lon;
    points[i][1] =(double) _acPosVec[i]->lat;
    pointPtr[i] = points[i];
  }

  int numPtsInHull = EG_ch2d(pointPtr, numPoints); 

  if (_params.debug == Params::DEBUG_VERBOSE)
    EG_print_hull(pointPtr, points[0], numPtsInHull);

  //
  // Create the GenPoly of convex hull points
  // Note:  The indices of the actual points in the hull are listed by
  // the expression (pointPtr[i]-points[0])/2 for i = 0 through numPtsInHull.
  //   
  //
  genPoly.setName(_acPosVec[0]->callsign);
  
  genPoly.setTime(dataTime);
  
  genPoly.setExpireTime(dataTime + _params.expire_seconds);

  genPoly.setClosedFlag(1);

  genPoly.setNLevels(1);

  genPoly.setId(dataTime);

  for (int i = 0; i < numPtsInHull; i++)
    {
      int index = (pointPtr[i]-points[0])/2;
      
      GenPoly::vertex_t vertex;

      vertex.lon = (float)points[index][0];

      vertex.lat = (float)points[index][1];

      genPoly.addVertex(vertex);

    }
   
  bool success = genPoly.assemble();

  if ( !success)
    {
      cerr << "AcTrack2Polygon::_createConvexHull(): GenPoly failed to assemble: " 
	 <<  genPoly.getErrStr() << endl;
      return 1;
    }
   
  return 0;
}


/////////////////////////////////////////////////////////////////////////
//
// Expand or inflate polygon by a (configurable) percent.
// 
int AcTrack2Polygon::_expandTrack( )
{

  if (_params.debug)
    {
      cerr << "AcTrack2Polygon::_expandTrack(): Expanding polygon by  " << _params.expansion_factor << " %" << endl; 
    }

  int numVertices = genPoly.getNumVertices();
  TaArray<GenPoly::vertex_t> vertices_;
  GenPoly::vertex_t *vertices = vertices_.alloc(numVertices);

  //
  //  Get the vertices and find the centroid of the unexpanded polygon
  //
  float centroid_lat = 0, centroid_lon = 0;
   
  for ( int i = 0; i < numVertices; i++ )
    {
      vertices[i] = genPoly.getVertex(i);
      centroid_lon += vertices[i].lon;
      centroid_lat += vertices[i].lat;
    }
  
  centroid_lon = centroid_lon/numVertices;
  
  centroid_lat = centroid_lat/numVertices;
  
  //
  // Expand the polygon along the radials from the 
  // polygon centroid to polygon vertices using trigonometry.
  //
  for ( int i = 0; i < numVertices; i++ )
    { 
      float dx = centroid_lon - vertices[i].lon;

      float dy = centroid_lat - vertices[i].lat ;

      float theta = atan(dy/dx);

      float radius = sqrt(dx*dx + dy*dy);
      
      //
      // New hypotenuse, legs of triangle
      //
      float R = radius + _params.expansion_factor/100 * radius;
	    
      float Dx = R * fabs(cos(theta));

      float Dy = R * fabs(sin(theta));

      if (dx > 0)
	vertices[i].lon = centroid_lon - Dx;
      else
	vertices[i].lon = centroid_lon + Dx;
      if (dy > 0)
	vertices[i].lat = centroid_lat - Dy;
      else 
	vertices[i].lat = centroid_lat + Dy;
    }

  //
  // Clear out the old vertices and add the vertices of the expanded polygon
  //
  genPoly.clearVertices();

  for (int i = 0; i < numVertices; i++)
      genPoly.addVertex(vertices[i]);
    
  bool success = genPoly.assemble();

  if ( !success)
    {
      cerr << "AcTrack2Polygon::_expandTrack(): GenPoly failed to assemble: " 
	 <<  genPoly.getErrStr() << endl;
      return 1;
    }
   
  return 0;
}


int AcTrack2Polygon::_writeGenPoly()
{

  if (_params.debug)
    {
      cerr << "AcTrack2Polygon::_writeGenPoly(): Writing output to " << _params.output_url << endl;
    }

  //
  // Construct the data types from the tail number
  //
  int dataType1, dataType2;

  dataType1 = Spdb::hash4CharsToInt32( _acPosVec[0]->callsign);

  char *strPtr =_acPosVec[0]->callsign  + 4;

  dataType2 = Spdb::hash4CharsToInt32(strPtr);


  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  int ret = spdb.put(_params.output_url,
		     SPDB_GENERIC_POLYLINE_ID,
		     SPDB_GENERIC_POLYLINE_LABEL,
		     dataType1,
		     genPoly.getTime(),
		     genPoly.getExpireTime(),
		     genPoly.getBufLen(),
		     genPoly.getBufPtr(),
		     dataType2);
  if (ret)
    {
      cerr << "AcTrack2Polygon::_writeGenPoly(): Write failed to url: " 
	   <<  _params.output_url << endl;
      
      return 1;
    }

  return 0;
}

