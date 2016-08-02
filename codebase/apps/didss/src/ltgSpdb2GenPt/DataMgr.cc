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
////////////////////////////////////////////////////////////////////////////////
//
//  Server, ingest, and data management
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <unistd.h>
#include <float.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <rapformats/GenPt.hh>
#include <rapformats/kavltg.h>
#include "LtgSpdb2GenPt.hh"
#include "DataMgr.hh"
using namespace std;


DataMgr::DataMgr()
{
  ltgSpdbServer = NULL;
}

DataMgr::~DataMgr()
{
  if (ltgSpdbServer)
    delete ltgSpdbServer;
}


int DataMgr::init( Params &parameters )
{
  params = parameters; 

  // 
  // Create the ltg  data server
  //
  ltgSpdbServer = new LtgSpdbServer(params);

  runMode = params.runMode;
  
  sleepTime = params.sleepTime;

  return( 0 );
}

int DataMgr::processData()
{

 
  switch (runMode)
    {
      case Params::ARCHIVE:
	{
	  POSTMSG(INFO,"Processing data in ARCHIVE mode...");

	  if ( ltgSpdbServer->readData() )
	    {
	      //
	      // Error reading data.
	      //
	      return (-1);
	    }
	  
	  nChunks = ltgSpdbServer->getNChunks();
	  
	  if ( nChunks > 0)
	    {
	      POSTMSG(INFO, "%d strikes found\n", nChunks);
	  
	      chunks =  ltgSpdbServer->getChunks();
	  
	      if (writeGenPtSpdb() )
		{
		  //
		  // Error writing data.
		  //
		  return (-1);
		}
	    }
	  else 
	    POSTMSG(INFO, "No data found\n");
	}
      break;
      
    case Params::REALTIME:
      {

	POSTMSG(INFO, "Processing data in REALTIME mode.\n");
	
	//
	// On start up, beginTime for getting an interval of 
        // data is the current time. 
	//
	time_t beginTime = time(0);

	while(1)
	  {
	    POSTMSG(INFO, "Sleeping for %d seconds...", sleepTime);
	    sleep(sleepTime);
	    
	    time_t currentTime = time(0);
	    
	    if ( ltgSpdbServer->readData( beginTime , currentTime) )
	      //
	      // Error reading data.
	      //
	      continue;
	    
	    nChunks = ltgSpdbServer->getNChunks();
 
	    //
	    // If there is data, get it and write in GenPt format.
	    //
	    if (nChunks >0)
	      {
		POSTMSG(INFO, "%d strikes found\n", nChunks);
  
		chunks =  ltgSpdbServer->getChunks();
   
		if ( writeGenPtSpdb() )
		  {
		    //
		    // Error writing data. Dont reset beginTime.
		    //
		    continue;
		    
		  }
		else
		  //
		  // data successfully written, reset begin time.
		  //
		  beginTime  = currentTime + 1;
	      }
	    else
	      POSTMSG(INFO, "No data found in interval.");

	  }

      }
      break;

    default:
      {
	POSTMSG(ERROR, "%s DataMgr::getData():", PROGRAM_NAME );
	POSTMSG(ERROR, "runMode not recognized. \n");
	return (-1);
      }
      
    }

  return (0);
}

//
// writeGenPtSpdb():
// Convert chunk data in LTG_strike_t format to GenPt format.
// Use DsSpdb object to write the data to output url.
//

int DataMgr::writeGenPtSpdb()
{

  POSTMSG(INFO, "Reformatting and writing data to %s\n", params.outputUrl);

  //
  // Reformat LTG_strike_t data into GenPt data and output Spdb 
  //
  for(int i = 0; i < nChunks; i++)
     {
       if (i%1000 == 0 && i > 0 )
	 POSTMSG(DEBUG, " Processing strike %d", i);
       
       //
       // Extract ltg data from chunk.
       //
       LTG_strike_t *ltg_strike = ( LTG_strike_t *)chunks[i].data;
       LTG_from_BE(ltg_strike);
       time_t time = ltg_strike->time;
       float latitude = ltg_strike->latitude;
       float longitude = ltg_strike->longitude;
       float amplitude = ltg_strike->amplitude;
       float type = ltg_strike->type;
       
       //
       // create GenPt object from strike data
       // 
       GenPt genPt;
       
       string nameStr("ltg strike");
       genPt.setName(nameStr);
       
       const int id = 0;
       genPt.setId( id );
       
       genPt.setTime(time);
       
       genPt.setLat(latitude);
       
       genPt.setLon(longitude);
       
       genPt.setNLevels(1);
       
       //
       // Add amplitude field to GenPt object.
       //
       string ampStr("amplitude");
       string ampUnits("kiloamps");
       genPt.addFieldInfo(ampStr,ampUnits);
       genPt.addVal(amplitude);
       
       //
       // Add strike type field  GenPt object.
       //
       string typeStr("srtike type");
       string typeUnits("KAVLTG_GROUND_STROKE or KAVLTG_CLOUD_STROKE");
       genPt.addFieldInfo(typeStr,typeUnits);
       genPt.addVal(type);
       
       //
       // Check that GenPt object has been created correctly.
       //
       if ( !genPt.check() )
	 {
	   string errStr = genPt.getErrStr();
	   POSTMSG( ERROR, "%s\n", errStr.c_str());
	 }
       
       //
       // Assemble GenPt object
       //
       genPt.assemble();
       
       //
       // Write the data to Spdb file. If error, return -1.
       //
       if ( ltgSpdbServer->writeGenPt(genPt) )
	   return (-1);
     } 
  
  return( 0);

}







