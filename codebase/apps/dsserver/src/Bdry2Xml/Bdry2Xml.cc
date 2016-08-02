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
// Bdry2Xml.cc
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <rapformats/Bdry.hh>
#include <rapformats/BdryPolyline.hh>
#include <rapformats/BdryPoint.hh>

#include "Bdry2Xml.hh"
using namespace std;

// Constructor

Bdry2Xml::Bdry2Xml(int argc, char **argv) :
  _progName("Bdry2Xml"),
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
  
  _paramsPath = "unknown";
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

Bdry2Xml::~Bdry2Xml()

{
   // unregister

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// 
// Run
//
int Bdry2Xml::Run()
{
  PMU_auto_register("In Bdry2Xml main loop");
  
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
       // Convert the Bdry objects at this time to Xml
       //
       _convert(time_begin, time_end);
    }
  else //REALTIME mode
    {
      time_t lastTime = 0;
      
      while (true) 
	{
	  PMU_auto_register("Running REALTIME mode");

	  string urlStr(_params.input_url);
	  DsURL dsUrl(urlStr);

	  DsLdataInfo ldata(dsUrl, _params.debug);

          if (!(ldata.read((int)_params.max_valid_age)))
            {
              time_t dataTime = ldata.getLatestTime();
	
	      if (_params.debug)
		cerr << "dataTime = " << dataTime << endl;

              if ( dataTime <= lastTime)
                {
		  if (_params.debug)
		    cerr << "Going to sleep, this is not new data." << endl;

                  sleep(1);
		   
                  PMU_auto_register("Bdry2Xml: Waiting for new data.");
                }
              else
                {
		  if (_params.debug)
		    cerr << "Processing data" << endl;
		  //
		  // Convert the Bdry object at this time to Xml
		  //
                  _convert(dataTime, dataTime);

                  lastTime = dataTime;
                }
            }
          else
             sleep(1);
	} // while
    }
  
  return 0;

}

int Bdry2Xml::_convert(time_t start_time, time_t end_time)
{
  DsSpdb spdbIn;

  if (_params.debug)
    {
      cerr << "Bdry2Xml::_convert :" << endl;
      cerr << "Reading data from " << _params.input_url << " in interval " 
	   << "[ " <<  start_time << ", " << end_time << " ]" << endl;
    }

  if ( spdbIn.getInterval( _params.input_url, start_time, end_time) != 0 )
     {
       cerr << "Bdry2Xml::_convert(): Problem reading data at "
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
      
   //
   // Convert each boundary to xml and write it to the 
   // database.
   //
   for (int i = 0; i < nChunks; i++)
    {

      if (_params.debug)
         cerr << "Processing chunk " << i << endl;

      Bdry bdry;
      
      DateTime bdryTime;

      DateTime expTime;

      double bdrySpeed;

      double bdryDirection;

      int bdryId;

      vector< BdryPolyline > polylines;
	
      vector< BdryPoint > polyLnPts;

      bool success = bdry.disassemble(chunks[i].data, chunks[i].len);
      
      if (!success)
	cerr << "Bdry2Xml::_convert(): Unable to disassemble chunk data\n";

      polylines = bdry.getPolylines();
      
      bdryTime = bdry.getDataTime();
     
      expTime = bdry.getExpireTime();
      
      bdrySpeed = bdry.getSpeed();

      bdryDirection = bdry.getDirectionPjg();

      bdryId = bdry.getBdryId();
      
      char outfile[256];

      sprintf( outfile, "%s/niwot_bdry_%s.xml", _params.output_url, bdryTime.getStrPlain().c_str());

      cerr << "outfile : " << outfile << endl;

      FILE *fptr;

      if ( (fptr = fopen(outfile, "w")) == NULL)
	{
	  cerr << "Bdry2Xml::_convert(): ERROR opening file " << outfile << " Exiting." << endl;
	  return 1;
	}

      //
      // Print version
      //
      fprintf(fptr, "<?xml version=\"1.0\"?>\n");
      
      //
      // Print root element
      //
      fprintf(fptr, "<wxml\n");
      
      fprintf(fptr, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version =\"1.0\"\n");
      
      fprintf(fptr, "noNoNamespaceSchemaLocation=\"schema/wxml.xsd\">\n");
      
      //
      // Head
      //
      fprintf(fptr, "<head>\n");
      
      fprintf(fptr, "\t<product concise-name=\"line\" operational-mode=\"experimental\">\n");
      
      fprintf(fptr, "\t\t<system>niwot</system>\n");
      
      fprintf(fptr, "\t\t<title>Niwot Boundary</title>\n");
      
      fprintf(fptr, "\t\t<category>analysis</category>\n");
      
      fprintf(fptr, "\t\t<creation-date>%sT%s</creation-date>\n", 
	      bdryTime.getDateStr().c_str(), bdryTime.getTimeStr(false).c_str());
      
      fprintf(fptr, "\t</product>\n");
      
      fprintf(fptr, "\t<data-source>\n");
      
      fprintf(fptr, "\t</data-source>\n");
      
      fprintf(fptr, "\t<product-source>\n");
      
      fprintf(fptr, "\t\t<production-center> National Center For Atmospheric Research\n");
      
      fprintf(fptr, "\t\t\t<sub-center> Research Applications Laboratory\n");
      
      fprintf(fptr, "\t\t\t</sub-center>\n");
      
      fprintf(fptr, "\t\t</production-center>\n");

      fprintf(fptr, "\t</product-source>\n");

      fprintf(fptr, "</head>\n");

      //
      // Nowcast Data
      //
      fprintf(fptr, "<nowcast-data>\n");
      
      fprintf(fptr, "\t<time-layout time-coordinate=\"UTC\">\n");

      fprintf(fptr, "\t\t<start-valid-time>%sT%s</start-valid-time>\n",
           bdryTime.getDateStr().c_str(), bdryTime.getTimeStr(false).c_str());

      fprintf(fptr, "\t\t<end-valid-time>%sT%s</end-valid-time>\n",
           expTime.getDateStr().c_str(), expTime.getTimeStr(false).c_str());

      fprintf(fptr, "\t</time-layout>\n");
      
      fprintf(fptr, "\t<event id=\"%d\">\n", bdryId);      

      fprintf(fptr, "\t\t<case>\n");
      
      fprintf(fptr, "\t\t\t<location>\n");

      for ( int k = 0; k < (int)polylines.size(); k++)
      {	
	  fprintf(fptr, "\t\t\t\t<polyline n-points=\"%d\">\n", polylines[k].getNumPoints() );
	  
	  polyLnPts  = polylines[k].getPoints();

	  for (int j = 0; j <  (int)polyLnPts.size(); j++)
	    {

	      fprintf(fptr,"\t\t\t\t\t<moving-point type=\"vertex\">\n");
	      
	      fprintf(fptr,"\t\t\t\t\t\t<latitude>%f</latitude>\n", polyLnPts[j].getLat() );
	      
	      fprintf(fptr,"\t\t\t\t\t\t<longitude>%f</longitude>\n", polyLnPts[j].getLon() );
	      
	      fprintf(fptr,"\t\t\t\t\t\t<cartesian_motion>\n");

	      fprintf(fptr,"\t\t\t\t\t\t\t<u_motion units=\"m s-1\">%f</u_motion>\n", polyLnPts[j].getUComp());

	      fprintf(fptr,"\t\t\t\t\t\t\t<v_motion units=\"m s-1\">%f</v_motion>\n", polyLnPts[j].getVComp());
	      
	      fprintf(fptr,"\t\t\t\t\t\t</cartesian_motion>\n");
	      
	      fprintf(fptr,"\t\t\t\t\t</moving-point>\n");
	    }

	  fprintf(fptr, "\t\t\t\t</polyline>\n");
      }

      fprintf(fptr, "\t\t\t</location>\n");

      fprintf(fptr, "\t\t</case>\n");

      fprintf(fptr, "\t</event>\n");

      fprintf(fptr, "</nowcast-data>\n");

      fprintf(fptr,"</wxml>\n");

      fclose(fptr);

      if (_params.debug)
	{
	  cerr << "Bdry2Xml::_convert(): bdryId =    " << bdryId << endl; 
	  cerr << "Bdry2Xml::_convert(): bdryTime =  " << bdryTime.dtime() << endl;
	  cerr << "Bdry2Xml::_convert(): expTime =   " << expTime.dtime() << endl;
	  cerr << "Bdry2Xml::_convert(): speed =     " << bdrySpeed << endl;
	  cerr << "Bdry2Xml::_convert(): direction = "  << bdryDirection << endl;
	  cerr << "Bdry2Xml::_convert(): number of polylines = " << polylines.size() << endl;
      
	  for ( int k = 0; k < (int)polylines.size(); k++)
	    {	      
	      cerr << "Bdry2Xml::_convert(): Number of points in polyline [" << k << "] = " 
		   <<  (int) polylines[k].getNumPoints() << endl;

	      for (int j = 0; j <  (int)polyLnPts.size(); j++)
		{
		  double lat = polyLnPts[j].getLat();
		  double lon = polyLnPts[j].getLon();
		  double uComp = polyLnPts[j].getUComp();
		  double vComp = polyLnPts[j].getVComp();
		    
		  cerr << "Bdry2Xml::_convert(): point[" << j << "] :"
		       <<  " lat = " << lat  
		       <<  " lon = " << lon 
		       <<  " u   = " << uComp
		       <<  " v   = " << vComp << endl;
		}
	    }
	}

      polylines.erase(polylines.begin(), polylines.end());
      
      polyLnPts.erase(polyLnPts.begin(), polyLnPts.end());

    }

   chunks.erase(chunks.begin(), chunks.end());

   return 0;
}








