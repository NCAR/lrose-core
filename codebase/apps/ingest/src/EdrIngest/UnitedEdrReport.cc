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
/*************************************************************************
 *  UnitedEdrReport.cc :
 *                    class for decoding and United specific reports.
 *                    United reports come in bundled with up to 16 Edr reports
 *                    as follows:
 *                        upto 4 reports be "line"
 *                        upto 4 lines per downlink or
 *                    16 minutes of data.
 *                    Interpolation is required to determine location and 
 *                    timestamps of individual reports.
 *
 *                    Turbulence values are encoded.
 *
 *                    This is an implementation of the EdrReport base class
 *                    
 **************************************************************************/

#include "UnitedEdrReport.hh"

using namespace std;

/**************************************************************************
 * Constructors.
 */


UnitedEdrReport::UnitedEdrReport(Params *parameters, 
                           DateTime &msgtime,
                           char* tailNumber, 
                           char* flightId,
                           char* aircraftRegistryNum,
                           downlink_t &downlink_time) : EdrReport()

{
    static bool ual2flsEncode = FALSE;

    msgTime = msgtime;
    sprintf(sourceName,"UAL 757");
    sprintf (sourceFmt, "ACARS ASCII");   
    sprintf (tail_number, tailNumber); 
    sprintf (flight_id, flightId); 
    sprintf (aircraft_registry_num, aircraftRegistryNum); 
    params = parameters;
    dnlnk_time = downlink_time;

    if (!ual2flsEncode) {
      if (createTailnumList())
         cerr << "ERROR:  reading encoded tail number list:  Continuing anyway" << endl;
      ual2flsEncode = TRUE;
    }
}

UnitedEdrReport::UnitedEdrReport(Params *parameters, 
                           DateTime &msgtime,
                           char* flightId,
                           char* aircraftRegistryNum) : EdrReport()

{
    static bool ual2flsEncode = FALSE;

    msgTime = msgtime;
    sprintf(sourceName,"UAL 757");
    sprintf (sourceFmt, "ACARS BUFR");   
    sprintf (flight_id, flightId); 
    sprintf (aircraft_registry_num, aircraftRegistryNum); 
    params = parameters;

    if (!ual2flsEncode) {
      if (createTailnumList())
         cerr << "ERROR:  reading encoded tail number list:  Continuing anyway" << endl;
      ual2flsEncode = TRUE;
    }
 }

/**************************************************************************
* Destructor
*/

UnitedEdrReport::~UnitedEdrReport(void)
{ 


}


/**************************************************************************
* setAndBufferEdr
*/

void UnitedEdrReport::setAndBufferEdr(DateTime msgTime, bool buffer_data)
{

    //
    // Get closest report to current edr point to set the edr point lat, lon, alt
    // possibly by interpolation if possible.
    //
    Edr::edr_t nearRep;

    bool foundExactRep = false;
    int  numNextRep = 0;
    bool gotNearRep = true;

    DateTime recTime;

    int ret = getClosestRep(nearRep);

    if (ret)
      gotNearRep = false;

    if (params->message_format == Params::ASCII)
      {

	foundExactRep = findExactRep();

	if (foundExactRep)
	  {
	    numNextRep = findNextRep();
	  }

	// If there is an exact record adjust record time
	if ( numNextRep == 2 )
	  {
	    recTime.set(recordTime.utime()+52);
	  }
	else
	  {
	    if(numNextRep == 1)
	      {
		recTime.set(recordTime.utime()+45);
	      }
	    else
	      {
		if ( foundExactRep )
		  {
		    recTime.set(recordTime.utime()+30);
		  }
		else
		  {
		    recTime.set(recordTime.utime());
		  }
	      }
	  }
      }
    else
      {
	recTime.set(recordTime.utime());
      }


    double speed, dAltDt, theta;
    if ( gotNearRep )
      getAircraftSpeed( speed, dAltDt, theta, nearRep, recTime.utime());

    //
    // lastData is a flag to mark whether the character representing edr
    // peak and ave is the last data point in the line. If it is, these
    // data will correspond to the recorded lat, lon, alt, and time.
    // The lats, lons, alts and times for other edr vals will have to be
    // estimated or set to missing.
    //
    bool lastData = true;

    int k = 0;

    if (params->debug == Params::DEBUG_VERBOSE)
      cerr << "\n" << _className() << "::setAndBufferEdr: Writing up to 4 edr points "
           << "from edr message line in order 3, 2, 1, 0 "
           << "(the reverse order of edr reps on the message line).\n" << endl;

    // Determine the type of data - ASCII or BUFR
    // to set the rep_size accordingly

    int rep_size;
    
    if (params->message_format == Params::ASCII)
      {
	rep_size = 0;
    
	for(int i = 0; i < 4 ; i++)
	  {
	    if (rep_data[i] != '\0')
	      rep_size++;
	  }
      }
    // BUFR format
    else
      {
	rep_size = 4;
      }


    if (params->debug == Params::DEBUG_VERBOSE)
      cerr << "size of rep_data " << rep_size << endl;

      
    for(int i = 3; i >=0 ; i--)
      {
      //
      // if rep_data[i] is valid, ie. not '\0' for ascii or > 20 and <=63 for bufr
      //
      if ( (rep_data[i] == '\0' && params->message_format == Params::ASCII))
      {
          if (params->debug == Params::DEBUG_VERBOSE) 
            cerr << "\n" << _className() << "::setAndBufferEdr: No edr point: " << i << "\n" << endl;
      }
      else // ( rep_data[i] != '\0' if the data is ASCII format)
      {
        if (params->debug == Params::DEBUG_VERBOSE)
            cerr << "\n" <<_className() << "::setAndBufferEdr: Setting edr point: " << i << "\n" << endl;

        Edr::edr_t edrData;
        memset ((void *)&edrData, 0, sizeof(Edr::edr_t));

        //
        // set edr_t struct vals
        //
        if (lastData)
          edrData.time = recTime.utime();
        else
        {
          time_t deltaTime;
          if (gotNearRep)
                 deltaTime = (time_t) (recTime.utime() - nearRep.time)/ rep_size;

          if (gotNearRep && deltaTime <= 60 && 
	      (recTime.utime() - nearRep.time - 60*(rep_size-1) <= 0) &&
	      (params->message_format == Params::ASCII))
             edrData.time = recTime.utime() - (k * deltaTime);
          else
             edrData.time = recTime.utime() - (k * 60);
        } 

        for (int j = 0; j < Edr::NUM_QC_INDS; j++)
        {
           edrData.edrAveQcInds[j] = Edr::VALUE_UNKNOWN;
        }

        edrData.edrPeakQcFlags[0] = edr_qc_flag[i];
        edrData.edrAveQcFlags[0] = edr_qc_flag[i];

        for (int j = 1; j < Edr::NUM_QC_FLAGS; j++)
        {
           edrData.edrPeakQcFlags[j] = (int)Edr::VALUE_UNKNOWN;
           edrData.edrAveQcFlags[j] = (int)Edr::VALUE_UNKNOWN;
        }
        for (int j = 0; j < Edr::NUM_SPARE_INTS; j++)
           edrData.spareInts[j] = (int)Edr::VALUE_UNKNOWN;

        for (int j = 0; j < Edr::NUM_SPARE_FLOATS; j++)
           edrData.spareFloats[j] = Edr::VALUE_UNKNOWN;

        edrData.fileTime = msgTime.utime();
        edrData.edrPeak = edr_peak[i];
        edrData.edrAve = edr_ave[i];

        // these are all set to VALUE_UNKNOWN in clearAll()
        edrData.PeakConf = peakConf;
        edrData.MeanConf = meanConf;
        edrData.PeakLocation = peakLocation;
        edrData.NumGood = numGood;

        edrData.wspd = wind_speed;
        edrData.wdir = wind_dir;
        edrData.sat = sat;
        edrData.edrAlgVersion = edrAlgVersion;
        edrData.qcVersion = Edr::VALUE_UNKNOWN;
        edrData.mach = Edr::VALUE_UNKNOWN;
        edrData.rms = Edr::VALUE_UNKNOWN;
	edrData.runningMinConf =  Edr::VALUE_UNKNOWN;
        edrData.qcConf = Edr::VALUE_UNKNOWN;
        edrData.qcThresh = Edr::VALUE_UNKNOWN;
	edrData.computedAirspeed = (si32) Edr::VALUE_UNKNOWN;
	edrData.maxNumBad = (si32) Edr::VALUE_UNKNOWN;
	edrData.QcDescriptionBitFlags = (si32) Edr::VALUE_UNKNOWN;

        sprintf(edrData.flightNum, "%s", flight_id);
        sprintf(edrData.aircraftRegNum, "%s",aircraft_registry_num);
        sprintf(edrData.sourceFmt, "%s", sourceFmt);
        sprintf(edrData.sourceName, "%s", sourceName);
        sprintf(edrData.origAirport, "%s", orig_airport);
        sprintf(edrData.destAirport, "%s", dest_airport);
        sprintf(edrData.airlineId, "%s", "UAL");
        //sprintf(edrData.encodedAircraftRegNum, "%s",
        //          ual2fslTailnums[string(aircraft_registry_num)].c_str());
	sprintf(edrData.encodedAircraftRegNum, "%s", tail_number);
        k++;

        if (lastData)
        {
           //
           // The lat and lon in the record line correspond to this rep
           //
           edrData.lat = lat;
           edrData.lon = lon;
           edrData.alt = alt;

           //
           // set flag to indicate that data has been not been interpolated
           // Note: Edr::QC_INTERP = 1 indicates an interpolated value (position
           // and time set by ingest),
           // Edr::QC_INTERP = 0 indicates no interpolation was used (position
           // and time set by aircraft),
           // Edr::QC_INTERP = 2 indicates the position was set by the aircraft,
           // but the time was set by ingest since there were mulitiple reports
           // within the same minute
           //
           if(foundExactRep)
           {
              edrData.interpLoc = 2;
           }
           else
           {
              edrData.interpLoc = 0;
           }

           lastData = false;

           if (params->debug == Params::DEBUG_VERBOSE)
           {
              cerr << "\tAssigning lat and lon from line:" << endl;
              cerr << "\trep lat is  " << edrData.lat << endl;
              cerr << "\trep lon is " << edrData.lon << endl;
              cerr << "\trep alt is " << edrData.alt << endl;
           }
        }
        else
        {
           //
           // Use the near rep to interpolate position in time and space
           //
           if (gotNearRep)
           {
              //
              // Estimate the lat and lon
              //
              time_t tDiff = (recTime.utime() - (time_t)edrData.time);

              float dist = tDiff * speed;

              double lat2, lon2;

              PjgCalc::latlonPlusRTheta(lat, lon, dist, theta, lat2, lon2);

	      if (std::isnan(lat2) || std::isnan(lon2))
	      {
                 edrData.lat = Edr::VALUE_UNKNOWN;
                 edrData.lon = Edr::VALUE_UNKNOWN;
                 edrData.alt = Edr::VALUE_UNKNOWN;
	      }
	      else
	      {
		edrData.lat = lat2;
		edrData.lon = lon2;
		edrData.alt = alt + (-tDiff) * dAltDt;		
	      }

              //
              // set flag to indicate that data has been interpolated
              //
              edrData.interpLoc = 1;

              if (params->debug == Params::DEBUG_VERBOSE)
              {
                  cerr << "\tInterpolating lat, lon, alt:\n";
                  cerr << "\tThe distance travelled between reps " << dist
                           << " km" << endl;
                  cerr << "\tThe estimated rep lat is " << edrData.lat << endl;
                  cerr << "\tThe estimated rep lon is " << edrData.lon << endl;
                  cerr << "\tThe estimated rep alt is " << edrData.alt << endl;
              }
           }
           else
           {
              //
              // No near rep available for interpolation.
              // lat, lon and alt set to unknown
              //
              edrData.lat = Edr::VALUE_UNKNOWN;
              edrData.lon = Edr::VALUE_UNKNOWN;
              edrData.alt = Edr::VALUE_UNKNOWN;

              //
              // set flag to indicate that data has been interpolated
              //
              edrData.interpLoc = 1;

              if (params->debug == Params::DEBUG_VERBOSE)
              {
                  cerr << "\tSetting lat, lon, and alt to Edr::VALUE_UNKNOWN "
                           << "since we have no previous reference point " << endl;
              }
           }
        }
        bufferEdrReport(edrData);
      }
    }
    writeBufferedSpdbData();
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: findExactRep( )
//
// Description:
//
// Returns: true if there is a report with the exact time, false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////

bool UnitedEdrReport::findExactRep() {

  DsSpdb test_spdb;


  //
  // Construct the data types from the tail number
  //
  int dataType1, dataType2;

  dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

  char *strPtr = aircraft_registry_num + 4;

  dataType2 = Spdb::hash4CharsToInt32(strPtr);

  //
  // Search the database for a report with the exact time from
  // this tail number
  //
  if (params->debug == Params::DEBUG_VERBOSE) {
      cerr << "\nUnitedEdrReport::findExactRep: Looking for report with exact "
                                    << "time in database.\n" << endl;

      cerr << "\tCalling test_spdb.getExact for time "
                                    << recordTime.utime() << " for exact edr report" << endl;
  }

  if (test_spdb.getExact(params->output_url,
                          recordTime.utime(),
                          dataType1,
                          dataType2) )
  {
      cerr << "\ttest_spdb.getExact failed: " << endl;
      return false;
  }
  else
  {
    int nChunks = test_spdb.getNChunks();
    string str1, str2;

    str1 = Spdb::dehashInt32To4Chars(dataType1);
    str2 = Spdb::dehashInt32To4Chars(dataType2);
    string tailnum = str1 + str2;

    if(nChunks <= 0)
    {
       if (params->debug == Params::DEBUG_VERBOSE)
       {
          cerr << "\ttest_spdb.getNChunks() = " << nChunks
                           << ", no exact data for " << tailnum << endl;
       }
       return false;
    }
    else //(nChunks > 0)
    {
       if (params->debug == Params::DEBUG_VERBOSE)
       {
          cerr << "\ttest_spdb.getNChunks() = " << nChunks
                   << ", there is a report with exact time for " << tailnum.c_str() << endl;
       }
       return true;
    }

  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: findNextRep  ()
//
// Description:
//
// Returns: true is there is a report with time within a minute of current report,
//          false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////


int UnitedEdrReport::findNextRep()
{

  DsSpdb test_spdb;


  //
  // Construct the data types from the tail number
  //
  int dataType1, dataType2;

  dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

  char *strPtr = aircraft_registry_num + 4;

  dataType2 = Spdb::hash4CharsToInt32(strPtr);

  //
  // Search the database for a report with the next time from
  // this tail number
  //
  if (params->debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\nUnitedEdrReport::findNextRep: Looking for report with time "
           << "within 60 seconds after current report.\n\n";

      cerr << "\tCalling test_spdb.getInterval for time "
           << recordTime.dtime() << "."
           << endl;
  }

  if (test_spdb.getInterval(params->output_url,
                          recordTime.utime()+1,
                          recordTime.utime()+59,
                          dataType1,
                          dataType2) )
  {
      cerr << "\ttest_spdb.getInterval failed: " << endl;
      return 0;
  }
  else
  {
    int nChunks = test_spdb.getNChunks();
    string str1, str2;

    str1 = Spdb::dehashInt32To4Chars(dataType1);
    str2 = Spdb::dehashInt32To4Chars(dataType2);
    string tailnum = str1 + str2;

    if(nChunks <= 0)
    {
       if (params->debug == Params::DEBUG_VERBOSE)
       {
          cerr << "\ttest_spdb.getNChunks() = " << nChunks
                    << ", no next report for " << tailnum << endl;
       }
       return 0;
    }
    else //(nChunks > 0)
    {
       if (params->debug == Params::DEBUG_VERBOSE)
       {
          cerr << "\ttest_spdb.getNChunks() = " << nChunks
                    << ", there is a next report(s) for " << tailnum << endl;
       }
       return nChunks;
    }
  }
}

 /**************************************************************************
  * getClosestRep
  */

  int UnitedEdrReport::getClosestRep(Edr::edr_t &nearRep)
  {

    DsSpdb test_spdb;

    vector <Edr*>    edrVec;

    //
    // Construct the data types from the tail number
    //
    int dataType1, dataType2;

    dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

    char *strPtr = aircraft_registry_num + 4;

    dataType2 = Spdb::hash4CharsToInt32(strPtr);

    //
    // Search the database for the latest from
    // this tail number
    //
    if (params->debug == Params::DEBUG_VERBOSE)
    {
      cerr << "\n" << _className() << "::getClosestRep: Retrieving previous edr pt for "
           << "possible interpolation of edr position\n\n";

      cerr << "\tCalling test_spdb.getFirstBefore for time "
           << recordTime.dtime() << " for previous edr reference point"
           << endl;
    }

    if (test_spdb.getFirstBefore(params->output_url,
                          recordTime.utime()-1,
                          params->lookback,
                          dataType1,
                          dataType2) )
    {
      cerr << "\ttest_spdb.getFirstBefore failed: " << endl;
      return 1;
    }
    else
    {
      int nChunks = test_spdb.getNChunks();

      if(nChunks <= 0)
      {
         string str1, str2;

         //
         // Just checking whether the full tailnum is being encoded
         // in the data types.
         //
         str1 = Spdb::dehashInt32To4Chars(dataType1);
         str2 = Spdb::dehashInt32To4Chars(dataType2);
         string tailnum = str1 + str2;

         if (params->debug == Params::DEBUG_VERBOSE)
         {
            cerr << "\ttest_spdb.getNChunks() = " << nChunks
                   << ", no recent data for " << tailnum << endl;
         }
         return 1;
      }
      else //(nChunks > 0)
      {
         vector <Spdb::chunk_t> chunks  = test_spdb.getChunks();

         //
         // Disassemble chunks. Store in soundings vector.
         //
         for( int i = 0; i < nChunks; i++ )
         {
              Edr *edrPtr = new Edr();

              edrPtr->disassemble( chunks[i].data, chunks[i].len );

              edrVec.push_back(edrPtr);
         }

         //
         // We might need to interpolate the lats and lons for
         // the first 3 edr peaks and averages, so grab the the most
         // recent edr point.
         //
         vector < Edr *> :: const_iterator i;

         for ( i = edrVec.begin(); i != edrVec.end(); i++)
         {
            Edr::edr_t lastRep = (*i)->getRep();

            //
            // Make sure we have vals for lat lon and alt.
            //
            if ( (fabs (lastRep.lat - Edr::VALUE_UNKNOWN) > .0001) &&
                   (fabs (lastRep.lon - Edr::VALUE_UNKNOWN) > .0001) &&
                   (fabs (lastRep.alt - Edr::VALUE_UNKNOWN) > .0001))
            {
               if (params->debug == Params::DEBUG_VERBOSE)
               {
                 cerr << "\tFound reference edr point in database:" << endl;
                 (*i)->print(cerr, "\t");
               }
               memcpy((void*)&nearRep,(const void*)&lastRep, sizeof( Edr::edr_t ));
               break;
             }

         }

         //
         // clean up
         //
         if ( (int) edrVec.size() >0 )
         {
           for ( i = edrVec.begin(); i != edrVec.end(); i++)
             delete *i;
         }
         edrVec.erase(edrVec.begin(), edrVec.end());

         return 0;
      }
    }
  }

 /**************************************************************************
  * GetAircraftSpeed
  */
  int UnitedEdrReport::getAircraftSpeed( double &speed, double &dAltDt,
                                  double &theta, Edr::edr_t &nearRep,
                                  time_t recTime)
  {
    double r, deltaT, deltaAlt;
    //
    // We might need to interpolate the position of individual edr reps
    // First we find the distance, time difference, and estimate the speed
    // of the aircraft in km/sec. We estimate change in alt w/r to time as
    // well.
    //
    PjgCalc::latlon2RTheta((double)lat, (double)lon, (double)nearRep.lat, (double)nearRep.lon, r, theta);

    deltaT = (double)recTime - (double)nearRep.time;

    speed = r/deltaT;

    deltaAlt = (double)(alt - nearRep.alt);

    dAltDt =   deltaAlt/deltaT;

    if (params->debug == Params::DEBUG_VERBOSE)
    {
      cerr << "\n" << _className() << "::getAircraftSpeed():\n\n"
           << "\tDistance between reference point and this report is "
           << r << " km" << endl;
      cerr   << "\tTime difference in points is " << deltaT << " seconds" << endl;
      cerr << "\tEstimated speed of aircraft is " << speed <<  " km/sec" << endl;
      cerr << "\tAltitude difference in reference point and this report is "
           << deltaAlt << " ft" << endl;
      cerr << "\tChange in altitude with repsect to time is " << dAltDt << " ft\n" << endl;
    }
    return 0;
  }

 /**************************************************************************
  * createTailnumList()
  */

  int UnitedEdrReport::createTailnumList()
  {
      string tailnumFilename = params->encodedTailnumFile;

      ifstream infile( tailnumFilename.c_str(), ios::in );

      if (! infile)
      {
            cerr << " UnitedEdrReport::createTailnumList():  Unable to open input file: "
                 << params->encodedTailnumFile << endl;
            return 1;
      }

      string textline;

      while ( getline( infile, textline, '\n' ))
      {
          ual2fslTailnums [textline.substr(0,6)] = textline.substr(18,22);

          //if (params.debug == Params::DEBUG_VERBOSE)
          //  {
          //    cerr << "UnitedEdrReport::createTailnumList " 
          //       << textline.substr(0,6) << "\tfsl:  "
          //       << ual2fslTailnums[ textline.substr(0,6) ] << endl;
          // }
      }

      return 0;
  }


 /**************************************************************************
  * Decode ASCII
  */

  EdrReport::status_t UnitedEdrReport::decodeAscii (char* tokenptr)
  {

     int second = 0;

     //
     // Go to next line
     //
     edrAlgVersion = 1.0;
     tokenptr = strtok(NULL,"\n\r");

     // Text line 6: Origination airport in positions 12,13,14 and
     //                    destination airport in positions 15,16,17 read from the
     //                    beginning of the string with the first element in
     //                    the line at position zero.
     // Example text: 5260C0120/23LAXORD
     //

     if (params->debug == Params::DEBUG_VERBOSE)
       cerr << "\tScanning origination and destination airports: \n";

     strncpy((char*)orig_airport, tokenptr+12, 3);
     strcpy((char*)dest_airport, tokenptr+15);

     if (params->debug == Params::DEBUG_VERBOSE)
     {
       cerr << "\torigination airport: " << orig_airport << endl;
       cerr << "\tdestination airport: " << dest_airport << endl;
     }

     // Flag used to help determine bad times in data
     bool last_min_zero = true;

     _spdb.clearPutChunks();

     //
     // Extract data from acars reports:
     // Lat+Lon+Alt+StaticAirTemp+WindDir+WindSpeed+AcarsReport
     //
     for(int i = 0; i < 4; i++)
     {
       //
       // if next line exists....
       //
       if ( (tokenptr = strtok(NULL,"\n\r")) != NULL)
       {
          int lat_deg, lat_min, lon_deg, lon_min, hour, minute;
          //int second = 0;

          char lat_char, lon_char, sat_sign;

          memset(rep_data, '\0', 4);

          int n  = sscanf(tokenptr,"%c%2d%3d %c%3d%3d %2d %2d %*c%5f %c%3f "
                               "%3f %3f %c%c%c%c",
                      &lat_char,
                      &lat_deg,
                      &lat_min,
                      &lon_char,
                      &lon_deg,
                      &lon_min,
                      &hour,
                      &minute,
                      &alt,
                      &sat_sign,
                      &sat,
                      &wind_dir,
                      &wind_speed,
                      &rep_data[0],
                      &rep_data[1],
                      &rep_data[2],
                      &rep_data[3]);
          if ( n < 13 &&  !isspace((int)lat_char) && !iscntrl((int)lat_char)  )
          {
              cerr << _className() << "::decodeAscii(): Error reading data line "
                   << i << " \n";
              return (EdrReport::FAIL);

          }

          if( lon_char == 'W')
              lon = -lon_deg + -lon_min/600.0;
          else
              lon = lon_deg + lon_min/600.0;

          if( lat_char == 'N')
            lat = lat_deg + lat_min/600.0;
          else
            lat = -lat_deg + -lat_min/600.0;

          if(sat_sign == 'P')
            sat = sat/10.0;
          else
            sat = -sat/10.0;

          // Check to see if lat and lon data is not zero.
          if (lat == 0 || lon == 0 || lat == MISSING_DATA || lon == MISSING_DATA)
          {
               cerr << "\nEdrIngest::decodeAsciiMsg(): Error with position data. lat: " << lat << " lon: " << lon << endl;
               cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl << endl;
               return (BAD_DATA);
          }

          // Use message time to help set record time
          // Based on the following assumptions:
          // The downlink time is either the same day as the
          // data time or the next day.  Also it is always
          // later than the datatime.
          //
          if ((hour < 24 && hour >= 0) &&
                                 (minute < 60 && minute >= 0))
          {

             //
             // Use message time to help set record time
             //
             getRecordTime(msgTime, hour, minute, second);

             int rec_day = recordTime.getDay();


             // Check for bad downlink day (only when downlink day != 1)
             if (dnlnk_time.day != 1)
             {
               if (!( (dnlnk_time.day - rec_day == 1) || (dnlnk_time.day - rec_day == 0) ))
               {
                 cerr << "\nEdrIngest::decodeAsciiMsg(): Error with downlink day being out of range of file time." << endl;
                 cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl << endl;
                 return (BAD_DATA);
               }
             }

             // Check to see if there is a significant difference in the downlink
             // time and the data time.  For cases when the data time minute is 00
             // more bad data times have been observed thus when the minute is 00
             // a difference of 1 hour is considered significant, otherwise 2 hrs
             // is considered significant.
             if (minute == 0 && last_min_zero)
             {
                if (rec_day == dnlnk_time.day)
                {
                   if (abs((dnlnk_time.hour * 60 + dnlnk_time.minute) - (hour * 60 + minute)) > 60)
                   {
                      cerr << "\nEdrIngest::decodeAsciiMsg(): Error with data time not matching downlink time." << endl;
                      cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl;
                      return (BAD_DATA);
                   }
                }
                else
                {
                  if (abs(((dnlnk_time.hour + 24) * 60 + dnlnk_time.minute) - (hour * 60 + minute)) > 60)
                  {
                      cerr << "\nEdrIngest::decodeAsciiMsg(): Error with data time not matching downlink time." << endl;
                      cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl << endl;
                      return (BAD_DATA);
                  }
                }

             }
             else
             {
                 if(minute != 0)
                 {
                   last_min_zero = false;
                 }

                 if (rec_day == dnlnk_time.day)
                 {
                   if (abs((dnlnk_time.hour * 60 + dnlnk_time.minute) - (hour * 60 + minute)) > 120)
                   {
                      cerr << "\nEdrIngest::decodeAsciiMsg(): Error with data time not matching downlink time." << endl;
                      cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl;
                      return (BAD_DATA);
                   }
                 }
                 else
                 {
                  if (abs(((dnlnk_time.hour + 24) * 60 + dnlnk_time.minute) - (hour * 60 + minute)) > 120)
                  {
                      cerr << "\nEdrIngest::decodeAsciiMsg(): Error with data time not matching downlink time." << endl;
                      cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl << endl;
                      return (BAD_DATA);
                  }
                 }

             }

          }
          else
          {
             cerr << "\nEdrIngest::decodeAsciiMsg(): Invalid data time. hour: " << hour << " minute: " << minute << endl;
             cerr << "Skipping file with file time: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl << endl;
             return (BAD_DATA);
          }

          //
          // Decode edr characters representing peaks and averages
          //
          decodeEdrDataChars();

          //
          // Print debug message
          //
          if (params->debug == Params::DEBUG_VERBOSE)
          {
              cerr << "\n" << _className() << "::decodeAscii(): Data read from "
                   << "line " << i << " of edr msg: \n" << endl;
              cerr << "\tlat: " << lat << endl;
              cerr << "\tlon: " << lon << endl;
              cerr << "\tmsgTime: " << msgTime.dtime() <<  " or " << msgTime.utime() << endl;
              cerr << "\trecordTime: " << recordTime.dtime() << " or "
                 << recordTime.utime() << endl;
              cerr << "\talt " << alt << endl;
              cerr << "\tsat " << sat << endl;
              cerr << "\twind dir " << wind_dir<< endl ;
              cerr << "\twind_speed " << wind_speed<< endl ;
              cerr << "\tturb encoded data: " << rep_data[0] << rep_data[1] << rep_data[2] << rep_data[3] << endl;

              for (int k = 0; k < 4; k ++)
              {
                  //
                  // If rep_data[k] is not '\0', then print the character and the decoded value
                  //
                  if (rep_data[k] != '\0')
                    {
                      cerr << "\tedr_ave[ " << k << " ] = " << edr_ave[k];
                      cerr << ", edr_peak[ " << k << " ] = " << edr_peak[k] << endl;
                    }
              }
           }

           // The following check determines if there is an exact duplicate message.
           // Sometimes two independent radio receivers picks up the same signal
           // from an aircraft - skip the entire set if true
           if (i == 0)
           {
             if (checkForDuplicateFile())
             {

               cerr<<"\nEdrIngest::decodeAsciiMsg: Duplicate edr file detected for flight id: "
                          << flight_id << " tail number: " << aircraft_registry_num <<
                          " at record time: " << recordTime.dtime() << " and File time "
                                                  << msgTime.dtime() << endl << endl;

               return (BAD_DATA);
             }
           }

           // set edr data in and write to database

           setAndBufferEdr(msgTime);

        }
    }
    
    // Chunks have been added to a buffer - put them in the SPDB database
    

    return ALL_OK;
  }

 /**************************************************************************
  * decodeEdrDataChars()
  */

void UnitedEdrReport::decodeEdrDataChars()
{
  //
  // Decode the Edr characters from message
  //
  int ave, peak;

  for (int i = 0; i < 4; i++)
  {
      edr_peak[i] = 0;
      edr_ave[i] = 0;
  }

  for (int j = 0; j < 4; j++)
  {
    ACT_interpret_report_char(rep_data[j], &ave, &peak);

    //
    // For UAL data, if one field is bad, the other has the
    // the same error reported
    //
    if( ave == BAD_VALUE )
    {
            edr_ave[j] = Edr::VALUE_UNKNOWN;
            edr_peak[j] = Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] = Edr::QC_NO_VALUE;
            rep_data[j] =  '\0';
    }
    else if( ave == LNA )
    {
            edr_ave[j] = Edr::VALUE_UNKNOWN;
            edr_peak[j] = Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] = Edr::QC_BAD_CHR;
    }
    else if (ave == LEPS_BADCGA )
    {
            edr_ave[j] = Edr::VALUE_UNKNOWN;
            edr_peak[j] = Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] = Edr::QC_BAD_CGA;
    }
    else if (ave == LEPS_BADALT )
    {
            edr_ave[j] =  Edr::VALUE_UNKNOWN;
            edr_peak[j] =  Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] =  Edr::QC_BAD_ALT;
    }
    else if (ave == LEPS_BADMACH )
    {
            edr_ave[j] = Edr::VALUE_UNKNOWN;
            edr_peak[j] = Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] =  Edr::QC_BAD_MACH;
    }
    else if (ave == LEPS_TROUBLE )
    {
            edr_ave[j] = Edr::VALUE_UNKNOWN;
            edr_peak[j] = Edr::VALUE_UNKNOWN;
            edr_qc_flag[j] = Edr::QC_BAD_OTHER;
    }
    else
    {
            edr_ave[j] = (float)ave/EPSILON_SCALE;
            edr_peak[j] = (float)peak/EPSILON_SCALE;
            edr_qc_flag[j] = Edr::QC_GOOD_VALUE;
    }
  }
}

 /**************************************************************************
  * Decode BUFR
  */

EdrReport::status_t UnitedEdrReport::decodeBufr ( BUFR_Val_t &bv )
{
  
  edrAlgVersion = 1.0;

  int ret;
  
  //
  // Check for type of station: fxy:0-02-001 
  // Don't record
  //
  ret = checkBufrVar((char *)"0-02-001",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check station type\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for station type, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  
	  return (END_OF_FILE);
	}
      else 
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for type of instrumentation for wind measurement. fxy: 0-02-002
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check wind measurement instrumentation type\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for tail number, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }

  //
  // Check for precision of temperature observed. fxy: 0-02-005
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-005",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check precision of observed temp\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for precision of temp, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
      
  //
  // Check for type of aircraft data relay system. fxy: 0-02-062
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-062",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check type of aircraft data relay system\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for type of aircraft, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else 
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for original specification of latitude/longitude. fxy: 0-02-070
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-070",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for original specification of latitude/longitude\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for original specification of latitude/longitude, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }

  //
  // Check for ACARS ground receiving station. fxy: 0-02-065
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-065",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for ACARS ground receiving station\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking ACARS ground receiving station, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get year. fxy: 0-04-001
  //
  int year;
  ret = getBufrVar(year, (char *)"0-04-001",   bv); 
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get year\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for year, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  
  //
  // Get month. fxy: 0-04-002
  //
  int month;
  ret = getBufrVar(month, (char *)"0-04-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get month\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for month, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get day. fxy: 0-04-003
  //
  int day;
  ret = getBufrVar(day, (char *)"0-04-003",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get day\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for day, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  
  
  //
  // Get hour. fxy: 0-04-004
  //
  int hour;
  ret = getBufrVar(hour, (char *)"0-04-004",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get hour\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for hour, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
      
  //
  // Get minute. fxy: 0-04-005
  //
  int minute;
  ret = getBufrVar(minute, (char *)"0-04-005",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get minute\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for minute, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get second. fxy: 0-04-006
  //
  int second;
  ret = getBufrVar(second, (char *)"0-04-006",   bv);
  
  //
  // Check for missing value
  //
  if (second >= 60)
    second = 0;
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get second\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for second, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
    
  //
  // Check to be sure we have good values for the time variables.
  //
  if (month > 12 || day > 31 || hour >= 24 || hour < 0 || minute >= 60
     || minute < 0 )
    {
      cerr << "\nEdrIngest::decodeBufrMsg(): Error with time variables. month: " << month << " day: " << day 
	   << " hour: " << hour << " minute: " << minute << endl
	   << "msgTime: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl;
      return (BAD_DATA);
    }
  
  //
  // Set record time.
  //
  recordTime.set(year, month, day, hour, minute, second);
  
  //
  // Check to be sure that the file time and data time aren't too far off.
  //
  if ( ((msgTime.utime() - recordTime.utime()) > 3*60*60) ||
       ((msgTime.utime() - recordTime.utime()) < -5*60) )
    {
      cerr << "\nEdrIngest::decodeEdrMsg(): Error with msgTime vs. recordTime. msgTime: " << msgTime.dtime() 
	   << " recordTime: " << recordTime.dtime() << " Tail number: " << aircraft_registry_num << endl;
      return (BAD_DATA);
    }

  //
  // Get latitude. fxy: 0-05-002
  //
  ret = getBufrVar(lat, (char *)"0-05-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get latitude\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for latitude, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get longitude. fxy: 0-06-002
  //
  ret = getBufrVar(lon, (char *)"0-06-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get longitude\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for longitude, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check to see if lat and lon data is not zero.
  //
  if (lat == 0 || lon == 0 )
    {
      cerr << "\nEdrIngest::decodeEdrMsg(): Error with position data. lat: " << lat << " lon: " << lon << endl
	   << "msgTime: " << msgTime.dtime() << " Tail number: " << aircraft_registry_num << endl;
      return (BAD_DATA);
    }

  //
  // Check for pressure fxy: 0-07-004
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-07-004",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for pressure\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for pressure, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for Aircraft roll angle quality. fxy: 0-02-064
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-02-064",   bv); 
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for Aircraft roll angle quality\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for roll angle, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else 
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for Phase of Aircraft flight. fxy: 0-08-004
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-08-004",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for phase of Aircraft flight\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for phase, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get altitude. fxy: 0-01-070
  //
  int altitude;
  ret = getBufrVar(altitude, (char *)"0-10-070",   bv);
  //
  // Convert meters to feet.
  //
  alt = altitude * 3.2808399;
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get altitude\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for alt, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get wind_direction. fxy: 0-11-001
  //
  int dir;
  ret = getBufrVar(dir, (char *)"0-11-001",   bv);
  wind_dir = dir;
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get wind direction\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for wind dir, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
      
  //
  // Get wind speed. fxy: 0-11-002
  //
  ret = getBufrVar(wind_speed, (char *)"0-11-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to get wind speed\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for wind speed, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  // Covert wind speed to knots
  wind_speed = wind_speed * 1.944;
  
  // 
  //
  // Check for dry bulb temperature. fxy: 0-12-001
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-12-001",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for dry bulb temperature\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for dry bulb temp, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for mixing ratio. fxy: 0-13-002
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-13-002",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for mixing ratio\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for mixing ratio, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }

  //
  // Check for relative humidity. fxy: 0-13-003
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-13-003",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for relative humidity\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for rh, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for ACARS interpolated values. fxy: 0-33-025
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-33-025",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for ACARS interpolated values\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for ACARS interpolated values, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for Mixing ratio quality. fxy: 0-33-026
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-33-026",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for mixing ratio quality\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for mixing ratio quality, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Check for time increment. fxy: 0-04-015
  // Don't record.
  //
  ret = checkBufrVar((char *)"0-04-015",   bv);
  if (ret)
    {
      cerr << "\n" << _className() << "::decodeBufr: Unable to check for time increment\n";
      if ( ret == BUFR_EOM || ret == BUFR_EOF)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): Looking for time increment, got EOF, destroying BUFR, exiting\n";
	  BUFR_Destroy(1);  

	  return (END_OF_FILE);
	}
      else
	return (INCORRECT_SEQUENCE);
    }
  
  //
  // Get the 4 turb indices
  //
  bool gotEdrData = true;
  for (int i = 0; i < 4; i++)
    {	  
      //
      // Check for duration of time relating to following value. fxy: 0-04-032
      // Don't record.
      //
      ret = checkBufrVar((char *)"0-04-032",   bv);
      if (ret)
	{
	  cerr << "\n" << _className() << "::decodeBufr: Unable to check for duration of time "
	       << "relating to following value.\n";
	  if ( ret == BUFR_EOM || ret == BUFR_EOF)
	    {
	      cerr << "EdrIngest::decodeBufrMsg(): Looking for duration of time for edr val, got EOF, destroying BUFR, exiting\n";
	      BUFR_Destroy(1);  
	      return (END_OF_FILE);
	    }
	  else
	    {
	      //
	      // Check for error
	      //
	      if (ret == 1)
		{
		  gotEdrData = false;
		  break;
		}
	    }
	}
	  
      //
      // Get TURBULENCE INDEX. fxy: 0-11-235
      // Note : filling in bufr_rep_data from right to left
      //        based on the way bufr reps are decoded.
      //
      ret = getBufrVar(bufr_rep_data[3-i], (char *)"0-11-235",   bv);
      if (ret)
	{
	  if ( ret == BUFR_EOM || ret == BUFR_EOF)
	    {
	      cerr << "EdrIngest::decodeBufrMsg(): Looking for turb index, got EOF, destroying BUFR, exiting\n";
	      BUFR_Destroy(1);  
	      return (END_OF_FILE);
	    }
	  else
	    {
	      //
	      // Check for error
	      //
	      if (ret == 1)
		{
		  cerr << "EdrIngest::decodeBufrMsg: Unable to get turbulence index.\n";
		  gotEdrData = false;
		  break;
		}
	    }
	}
    }

  if(!gotEdrData)
    return (INCORRECT_SEQUENCE);

  //
  // decode BUFR rep data
  //
  decodeBufrRepData();

  //
  // set edr data and write to database
  //
  setAndBufferEdr(msgTime, false);
  
  // Chunks have been added to a buffer - put them in the SPDB database
    
  // Temporary change - uncomment these when dropped report problem is fixed
  // and we can begin buffering spdb data again.  Might use writeBufferedData
  // instead
  //if (_spdb.put(params->output_url,
  //               SPDB_EDR_POINT_ID,
  //               SPDB_EDR_POINT_LABEL))
  //{
  //   cerr << _className() << "::setAndBufferEdr: Couldn't put EDR data to "
  //               << params->output_url << ". Tail number: " << aircraft_registry_num << endl;
  //   cerr << "  " << _spdb.getErrStr() << endl;
  //   return (BAD_DATA);
  //}

  return (ALL_OK);

}

 /**************************************************************************
  * 
  */

void UnitedEdrReport::decodeBufrRepData()
{
  //Average value of eddy dissipation rate     Peak value of eddy dissipation rate
  //                               ((m**(2/3))/s)                          ((m**(2/3))/s)
  //			 
  //      0                        < 0.1                                    < 0.1
  //      1                        < 0.1                                < 0.2 and > 0.1
  //      2                    < 0.2 and > 0.1                          < 0.2 and > 0.1
  //      3                        < 0.1                                < 0.3 and > 0.2
  //      4                    < 0.2 and > 0.1                          < 0.3 and > 0.2
  //      5                    < 0.3 and > 0.2                          < 0.3 and > 0.2
  //      6                        < 0.1                                < 0.4 and > 0.3
  //      7                    < 0.2 and > 0.1                          < 0.4 and > 0.3
  //      8                    < 0.3 and > 0.2                          < 0.4 and > 0.3
  //      9                    < 0.4 and > 0.3                          < 0.4 and > 0.3
  //     10                        < 0.1                                < 0.5 and > 0.4
  //     11                    < 0.2 and > 0.1                          < 0.5 and > 0.4
  //     12                    < 0.3 and > 0.2                          < 0.5 and > 0.4
  //     13                    < 0.4 and > 0.3                          < 0.5 and > 0.4
  //     14                    < 0.5 and > 0.4                          < 0.5 and > 0.4
  //     15                        < 0.1                                   > 0.5
  //     16                    < 0.2 and > 0.1                             > 0.5
  //     17                    < 0.3 and > 0.2                             > 0.5
  //     18                    < 0.4 and > 0.3                             > 0.5
  //     19                    < 0.5 and > 0.4                             > 0.5
  //     20                       > 0.5                                    > 0.5
  //   21-62      Reserved
  //     63       Missing value

  //
  // We choose midpoint of a bin for edr peak and mean values.
  // 
  for (int i = 0; i < 4; i++)
    {
      if ( bufr_rep_data[i] == 0)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.05;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 1)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.15;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 2)
	{
	  edr_ave[i] = 0.15;
	  edr_peak[i] = 0.15;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 3)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.25;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 4)
	{
	  edr_ave[i] = 0.15;
	  edr_peak[i] = 0.25;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 5)
	{
	  edr_ave[i] = 0.25;
	  edr_peak[i] = 0.25;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 6)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.35;	  
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 7)
	{
	  edr_ave[i] = 0.15;
	  edr_peak[i] = 0.35;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 8)
	{
	  edr_ave[i] = 0.25;
	  edr_peak[i] = 0.35;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 9)
	{
	  edr_ave[i] = 0.35;
	  edr_peak[i] = 0.35;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 10)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.45;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 11)
	{
	  edr_ave[i] = 0.15;
	  edr_peak[i] = 0.45;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 12)
	{
	  edr_ave[i] = 0.25;
	  edr_peak[i] = 0.45;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 13)
	{
	  edr_ave[i] = 0.35;
	  edr_peak[i] = 0.45;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 14)
	{
	  edr_ave[i] = 0.45;
	  edr_peak[i] = 0.45;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 15)
	{
	  edr_ave[i] = 0.05;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 16)
	{
	  edr_ave[i] = 0.15;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 17)
	{
	  edr_ave[i] = 0.25;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 18)
	{
	  edr_ave[i] = 0.35;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 19)
	{
	  edr_ave[i] = 0.45;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else if ( bufr_rep_data[i] == 20)
	{
	  edr_ave[i] = 0.55;
	  edr_peak[i] = 0.55;
	  edr_qc_flag[i] = Edr::QC_GOOD_VALUE;
	}
      else // if ( 20  < bufr_rep_data[i] && bufr_rep_data[i] <= 63)
	{
	  edr_ave[i] = Edr::VALUE_UNKNOWN;
	  edr_peak[i] = Edr::VALUE_UNKNOWN;
	  edr_qc_flag[i] = Edr::QC_NO_VALUE;
	} 
    }

}


 /**************************************************************************
  * 
  */

int UnitedEdrReport::checkBufrVar(char *fxy, BUFR_Val_t &bv)
{
  int n;
  if ( ((n=BUFR_Get_Value(&bv, 1))  != BUFR_EOM) && (n!= BUFR_EOF))
    {
      char *c_fxy = FXY_String(bv.FXY_Val);  
      if (strcmp( c_fxy, fxy))
	return 1;
    }

  return n;
}

 /**************************************************************************
  * 
  */

int UnitedEdrReport::getBufrVar(char *stringVar, char* fxy,  BUFR_Val_t &bv)
{

  int n;
  if ( ((n=BUFR_Get_Value(&bv, 1))  != BUFR_EOM) && (n!= BUFR_EOF))
    {
      char *c_fxy = FXY_String(bv.FXY_Val);
      
      if ( strcmp( c_fxy, fxy ) == 0)
	sprintf(stringVar, "%s", bv.Val.string);
      else 
	return 1;
    }
  
  return n;
}

 /**************************************************************************
  * 
  */

int UnitedEdrReport::getBufrVar(int &intVar, char* fxy,  BUFR_Val_t &bv)
{
  
  int n;
  if ( ((n=BUFR_Get_Value(&bv, 1))  != BUFR_EOM) && (n!= BUFR_EOF))
    {
      char *c_fxy = FXY_String(bv.FXY_Val);
      
      if ( strcmp( c_fxy, fxy ) == 0)
	intVar = bv.Val.int_number;
      else 
	return 1;
    }
  
  return n;
}


 /**************************************************************************
  * 
  */

int UnitedEdrReport::getBufrVar(float &floatVar, char* fxy,  BUFR_Val_t &bv)
{

  int n;
  if ( ((n=BUFR_Get_Value(&bv, 1))  != BUFR_EOM) && (n!= BUFR_EOF))
    {
      char *c_fxy = FXY_String(bv.FXY_Val);
  
      if ( strcmp( c_fxy, fxy ) == 0)
	floatVar = bv.Val.number;
      else 
	return 1;
    }
  
  return n;
}
