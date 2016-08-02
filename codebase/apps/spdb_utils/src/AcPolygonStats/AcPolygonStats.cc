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
#include "AcPolygonStats.hh"
using namespace std;

//
// Constructor
//
AcPolygonStats::AcPolygonStats(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "AcPolygonStats";
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
AcPolygonStats::~AcPolygonStats()

{
  _clear();

  //
  // unregister process
  //
  PMU_auto_unregister();  
}

void AcPolygonStats::_clear()
{

  //
  // Delete the GenPolys in each of the time series,
  // the delete the vector holding each time series.
  //
  vector < vector <GenPoly * >* > :: const_iterator i;

  for (i = _timeSeries.begin(); i != _timeSeries.end(); i++)
    {
       vector <GenPoly *> :: const_iterator j;

       for ( j = (*i)->begin(); j !=  (*i)->end(); j++)
	 delete (*j);

       (*i)->erase((*i)->begin(), (*i)->end());
	 
       delete (*i);
    }

  _timeSeries.erase(_timeSeries.begin(), _timeSeries.end());
}


//////////////////////////////////////////////////
// 
// Run
//
int AcPolygonStats::Run ()
{
  //
  // register with procmap
  //
  PMU_auto_register("Run");

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
	  cerr << "AcPolygonStats::Run(): Creating ARCHIVE trigger\n"  
	       << "     start time: " << _args.startTime 
	       << "     end time:   " << _args.endTime << endl;
	}      
    }
  else
    {
      cerr << "Presently there is only an ARCHIVE mode. Sorry. Please play again\n";
      return 1;
    }
    
  //
  //
  // process data
  //
  if ( _processData(_args.startTime, _args.endTime))
    {
      cerr << "AcPolygonStats::Run(): Errors in processing time: [" <<  _args.startTime << ", " 
	   << _args.endTime << "] " << endl; 
      return 1;
    }

  return 0; 
}


///////////////////////////////////
//
//  process data 
//
int AcPolygonStats::_processData(time_t start_time, time_t end_time)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");
  
  if ( _readSpdb( start_time, end_time) )
    {
      cerr << "AcPolygonStats::_processData(): ERROR: Failure to read database! "
      << " (Check URL). Time interval:  [" <<  _args.startTime << ", " 
      << _args.endTime << "] " << endl; 
    }

  _writeTimeSeries();

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
int AcPolygonStats::_readSpdb(time_t time_begin, time_t time_end)
{

  //
  // Read the spdb database of GenPoly objects.
  //
  DsSpdb spdbMgr;

  if (spdbMgr.getInterval(_params.polygon_url, time_begin, time_end))
    {
      cerr << "AcPolygonStats::_processData(): getInterval failed for " << _params.polygon_url << ", ["
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
      cerr << "AcPolygonStats::readSpdb(): "
           << nChunks << " found in interval [" << time_begin << "," << time_end << "]" << endl;
    }

  //
  // Loop through the chunks,disassmeble, and save. Note we will create unique time series
  // as we save the data.
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
         cerr << "AcPolygonStats::_readSpdb " << polygon->getErrStr() << endl;
	 delete polygon;
         continue;
       }

     //
     // Get current number of time series
     //
     int numTimeSeries = (int) _timeSeries.size();
     
     bool addedToTimeSeries = false;
     
     //
     // Add polygon to existing time series if possible
     //
     for (int j = 0; j < numTimeSeries ; j++)
       {
	 //
	 // If the aircraft ids match and the initiation times match then add to time series
	 // We will comapre to the first element in the time series.
	 //
	 vector <GenPoly*> *vec = _timeSeries[j];
	 
	 if ( (strcmp(polygon->getName().c_str(), (*vec)[0]->getName().c_str()))== 0 &&
	      polygon->getId() == (*vec)[0]->getId())
	   {
	     vec->push_back(polygon);
	     j = numTimeSeries;
	     addedToTimeSeries = true;
	   }
       }
     
     if (! addedToTimeSeries)
       {
	 //
	 // Create a new vector to hold the new time series and push on the first polygon.
	 //
	 vector <GenPoly*> *vec = new vector <GenPoly*>;
	 _timeSeries.push_back( vec);
	 _timeSeries[numTimeSeries]->push_back(polygon);
       }  
   }
  
  return 0;
  
}

int AcPolygonStats::_writeTimeSeries()
{

  cerr << "************************** TIME SERIES **********************\n\n\n";

  int count = 0;
  for (int i = 0; i < (int)_timeSeries.size(); i++)
    {
      vector <GenPoly*> polygons = *(_timeSeries[i]);

      for (int j = 0; j < (int)polygons.size(); j++)
	{
	  double area = _computePolygonArea( *polygons[j]);
	  
	  cerr << "acId: " << polygons[j]->getName()
	       << " init_t: " << polygons[j]->getId() << " "
	       << " t: " <<  polygons[j]->getTime()  << " " 
	       << polygons[j]->getFieldName(0) << ": " 
	       << polygons[j]->get1DVal(0) << " " 
	       << polygons[j]->getFieldUnits(0) << " " 
	       << " area: " << area << endl;
	   
	   
	   count++;
	}
      cerr << "\n\n" << endl;
    }
    
  return 0;
}

double AcPolygonStats::_computePolygonArea( GenPoly &polygon)
{
  //
  // Get the centroid
  //
  float centroidLon, centroidLat;

  polygon.calcCentroid(centroidLat,centroidLon);

  //
  // Initialize a Pjg object at the centroid. We'll use Pjg methods to 
  // give us distance in km between two points in lat lon.
  //
  Pjg grid;

  grid.initFlat(centroidLat,centroidLon);

  //
  // Get the dx and dy in km from the centroid to each point in the polygon.
  // We'll calulate the area of the polygon with the centroid as the origin and 
  // the corresponding dx and dy's as coordinates of points.
  //
  int numVertices = polygon.getNumVertices();
  
  Point_d polyPts[numVertices + 1];

  for (int j = 0; j < numVertices; j++)
    {
      GenPoly::vertex_t vertex = polygon.getVertex(j);
      
      double theta;
      //
      // Get dx between pts (centroidLon, centroidLat), and  (vertex.lon, vertex.lat)
      //
      grid.latlon2RTheta( centroidLat,  vertex.lon, centroidLat, centroidLon, polyPts[j].x, theta);  
      
      //
      // Get dy between pts (centroidLon, centroidLat), and  (vertex.lon, vertex.lat)
      //
      grid.latlon2RTheta(centroidLat,  vertex.lon, vertex.lat, vertex.lon, polyPts[j].y, theta);

    }

  polyPts[numVertices].x = polyPts[0].x;
  
  polyPts[numVertices].y = polyPts[0].y;

  //
  // Calulate the area
  //
  double area = EG_polygon_area_d(polyPts, numVertices + 1);

  return area;

}
