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
 *  SouthwestEdrReport.hh :
 *                    class for decoding Southwest specific reports.
 *                    There two classifications of Southwest reports,
 *                    Heartbeat and Triggered - Functionality will need
 *                    to be added to fill in null reports using an ASDI
 *                    feed.
 *
 *                    Southwest reports use the ARINC 620 specification.
 *
 *                    This is an implementation of the EdrReport base class
 *
 **************************************************************************/

#include <stdio.h>
#include <string>
#include "SouthwestEdrReport.hh"

using namespace std;


const string SouthwestEdrReport::out_string[] = {
                            "\tVersion", "\tPeak Edr", "\tMean Edr",
                            "\tPeak Edr Confidence", "\tMean Edr Confidence",
                            "\tPeak Location", "\tNum Good Edrs"};

/**************************************************************************
 * Constructor.
 */


  SouthwestEdrReport::SouthwestEdrReport(Params *parameters, 
                       DateTime &msgtime,
                       char* tailNumber, 
                       char* flightId,
                       char* aircraftRegistryNum,
		       downlink_t &downlink_time) : EdrReport()

 {
    sprintf(sourceName,"SWA 737");
    sprintf (sourceFmt, "ACARS ASCII");   
    sprintf (tail_number, tailNumber); 
    sprintf (flight_id, flightId); 
    sprintf (aircraft_registry_num, aircraftRegistryNum);
    msgTime = msgtime;
    params = parameters;
    dnlnk_time = downlink_time;

 }


 /**************************************************************************
  * Destructor
  */

  SouthwestEdrReport::~SouthwestEdrReport(void)
  {
     

  }


 /**************************************************************************
  * setAndBufferEdr
  */

  int SouthwestEdrReport::setAndBufferEdr(DateTime msgTime)
  {

        Edr::edr_t edrData;
        memset ((void *)&edrData, 0, sizeof(Edr::edr_t));

	int time_dif = 0;

        //
        // set edr_t struct vals
        //
        edrData.time = recordTime.utime();
        edrData.fileTime = msgTime.utime();

	// Do QC on time values
	if (dnlnk_time.day != 1)
	{
           int dnlnk_secs = dnlnk_time.day * 24 * 3600 +  dnlnk_time.hour * 3600 + dnlnk_time.minute * 60;
           int msg_secs = recordTime.getDay() * 24 * 3600 +  recordTime.getHour() * 3600 + recordTime.getMin() * 60;

	   time_dif = dnlnk_secs - msg_secs;
	   if (time_dif > 3600 || time_dif < -60)
	   {
	      cerr << _className() <<  "::setAndBufferEdr Downlink time / record time check failed. Tail number: " << aircraft_registry_num  << endl;
              return 1;
	   }
	}

	time_dif = edrData.fileTime - edrData.time;

	if (time_dif  > 43200 || time_dif < -60)
	{
	   cerr << _className() <<  "::setAndBufferEdr message time / file time check failed. Tail number: " << aircraft_registry_num  << endl;
	   return 1;
	}

        edrData.edrPeak = edr_peak[0];
        edrData.edrAve = edr_ave[0];

        if (!(edrData.edrPeak >= 0.0 && edrData.edrPeak <= 1.0))
        {
           cerr << _className() <<  "::setAndBufferEdr Peak is not >= 0 and <= 1.0.  Tail number: "
                               << aircraft_registry_num  <<  " value is " << edrData.edrPeak << endl;
           return BAD_DATA;
        }

        if (!(edrData.edrAve >= 0.0 && edrData.edrAve <= 1.0))
        {
           cerr << _className() <<  "::setAndBufferEdr Mean is not >= 0 and <= 1.0.  Tail number: "
                               << aircraft_registry_num  <<  " value is " << edrData.edrAve << endl;
           return BAD_DATA;
        }

        if (edr_qc_flag[0] == Edr::QC_GOOD_VALUE)
	{
	  if (strncmp(&runningMinConf, ":", 1) == 0)
	    edrData.runningMinConf = Edr::VALUE_UNKNOWN;
	  else
	  {
	    if (strncmp(&runningMinConf, "A", 1) == 0)
	      edrData.runningMinConf = 0;
	    else
	      edrData.runningMinConf = 1 - ((runningMinConf - '0') / 10.);
	  }

	  if (strncmp(&runningNumBad, "A", 1) == 0)
	    edrData.maxNumBad = 40;
	  else
	  {
	    if (strncmp(&runningNumBad, ":", 1) == 0)
	      edrData.maxNumBad = (int) Edr::VALUE_UNKNOWN;
	    else
	      edrData.maxNumBad = 4 * (runningNumBad - '0');
	  }
	}
	else
	{
	  edrData.runningMinConf = Edr::VALUE_UNKNOWN;
	  edrData.maxNumBad = (int) Edr::VALUE_UNKNOWN;
	}

        edrData.PeakConf = peakConf;
	edrData.MeanConf = meanConf;
	edrData.PeakLocation = peakLocation;
	edrData.NumGood = numGood;

        edrData.wspd = wind_speed;
        edrData.wdir = wind_dir;
        edrData.sat = sat;
        edrData.edrAlgVersion = edrAlgVersion;
        edrData.qcVersion = Edr::VALUE_UNKNOWN;
        edrData.qcConf = Edr::VALUE_UNKNOWN;
        edrData.qcThresh = Edr::VALUE_UNKNOWN;

	// Initialize to Edr::VALUE_UNKNOWN
        edrData.mach = mach;
	edrData.rms = rms;
	edrData.computedAirspeed = computedAirspeed;

        for (int j = 0; j < Edr::NUM_QC_INDS; j++)
        {
              edrData.edrAveQcInds[j] = Edr::VALUE_UNKNOWN;
        }

        edrData.edrPeakQcFlags[0] = edr_qc_flag[0];
        edrData.edrAveQcFlags[0] = edr_qc_flag[0];

        for (int j = 1; j < Edr::NUM_QC_FLAGS; j++)
        {
              edrData.edrPeakQcFlags[j] = (int)Edr::VALUE_UNKNOWN;
              edrData.edrAveQcFlags[j] = (int)Edr::VALUE_UNKNOWN;
        }

        for (int j = 0; j < Edr::NUM_SPARE_INTS - 3; j++)
            edrData.spareInts[j] = (int)Edr::VALUE_UNKNOWN;

        for (int j = 0; j < Edr::NUM_SPARE_FLOATS -3; j++)
            edrData.spareFloats[j] = Edr::VALUE_UNKNOWN;

	edrData.QcDescriptionBitFlags = QcDescriptionBitFlags;

        sprintf(edrData.flightNum, "%s", flight_id);
        sprintf(edrData.aircraftRegNum, "%s", aircraft_registry_num);
        sprintf(edrData.sourceFmt, "%s", sourceFmt);
        sprintf(edrData.sourceName, "%s", sourceName);
        sprintf(edrData.origAirport, "%s", orig_airport);
        sprintf(edrData.destAirport, "%s", dest_airport);

        sprintf(edrData.airlineId, "%s", "SWA");

	// We need to find out if Southwest wants to encode tail numbers
        //sprintf(edrData.encodedAircraftRegNum, "%s", aircraft_registry_num);
	sprintf(edrData.encodedAircraftRegNum, "%s", tail_number);
        //        encodedTails.ual2fslTailnums[string(aircraft_registry_num)].c_str());

        edrData.lat = lat;
        edrData.lon = lon;
        edrData.alt = alt;

        if (!(edrData.alt >= 0.0 && edrData.edrAve <= 50000.0))
        {
           cerr << _className() <<  "::setAndBufferEdr Altitude is not between 0 and 50000.  Tail number: "
                               << aircraft_registry_num  <<  " value is " << edrData.alt << endl;
           return BAD_DATA;
        }

	//
        // set flag to indicate that data has been not been interpolated
        // Note: Edr::QC_INTERP = 1 indicates an interpolated value, a zero
        // indicates no interpolation was used.
        //
        edrData.interpLoc = 0;

	_edrBuffer.push_back(edrData);

	// create WFS output and, if indicated, put report in a Web Feature Service data store
	if (params->createWfsData)
           writeWfsData(edrData);
	
        //writeEdr(edrData);

        return ALL_OK;
}


 /**************************************************************************
  * Decode
  */

  EdrReport::status_t SouthwestEdrReport::decodeAscii (char* tokenptr)
  {

    string turb_str;
    string subStr;
    string latChar, lonChar, satSign;

    char rep_char[8];
    int latDeg, latMin, lonDeg, lonMin;
    int hour, minute, second;
    int num_fields;

    bool badMsg = FALSE;

    fl32 outfields[8];

    cerr << "Decoding Southwest Message" << endl;

    for (int i = 0; i < 8; i++)
    {
      outfields[i] = Edr::VALUE_UNKNOWN;
    }

    //
    // Text line 5: "-  /WX string" 620 report format
    //              "-  /239 string" MDCRS report format.
    //

    if (params->debug == Params::DEBUG_VERBOSE)
    {

      cerr << "\tScanning report format.\n";
      cerr << "\t" << tokenptr << "\n" << endl;

    }
    sscanf(tokenptr,"%*s %s", &format[0]);

    if (params->debug == Params::DEBUG_VERBOSE)
       cerr << "\tformat: " << format << "\n" << endl;

    if (strncmp (format, "7640", 4) == 0)
    {
       if (params->debug == Params::DEBUG_VERBOSE)
            cerr << "\tSouthwest 620 format " << endl;

       // How do we determing trigggered vs heartbeat for southwest
       sprintf(sourceFmt, "620 ASCII");
    }
    else
    {
        cerr << "\tUnrecognized  format" << endl;
        return FAIL;
    }

    //
    // Go to next line
    //
    tokenptr = strtok(NULL,"\n\r");

    // Example: 02E09KSLCKOAK
    //   02 - version number
    //   E  - enroute
    //   09 - day
    //   KSLC - flight destination -> from OAK to SLC
    //   KOAK - flight origination

    if (strlen(tokenptr) < 12)
    {
        cerr << "ERROR: No origination or destination airport defined " << endl;
        strcpy((char *) orig_airport, "XXX");
        strcpy((char *) dest_airport, "XXX");

    }
    else
    {
       strncpy((char *) orig_airport, tokenptr + 6, 3);
       strcpy((char *) dest_airport, tokenptr + 10);
    }

    if (params->debug == Params::DEBUG_VERBOSE)
    {
       cerr << "\torigination airport: " << orig_airport << endl;
       cerr << "\tdestination airport: " << dest_airport << endl;
    }

    //if (params->debug == Params::DEBUG_VERBOSE)
    //{
    //   cerr << "\torigination airport: " << orig_airport << endl;
    //   cerr << "\tdestination airport: " << dest_airport << endl;
    //}


    _spdb.clearPutChunks();
    _edrBuffer.clear();

    // May have up to 6 independent messages
    for (int i = 0; i < 6; i++)
    {
       //
       // Go to next line
       //
       tokenptr = strtok(NULL,"\n\r");

       if (tokenptr != NULL && (strncmp(tokenptr, "\003", 1) != 0) && strlen(tokenptr) == MAX_TOKENPTR_LEN)
       {
          cerr << tokenptr << endl;

          // Up to 6 lines similar to:
          //
          // N37610W12235519040008P1072750110XXXX2200XO20)
          //
          //      TRANSLATES TO
          //           N37610 - Latitude N==North - 37 degrees 61  0 DDMMTT -> TT is tenths of minutes
          //           W122355  - longitude W==West - 122 degrees 35 5 tenths of minutes DDDMMTT
          //           1904  - time of obs HHMM
          //           0008  - pressure altitude
          //           P107 - static air temperature P==plus  M==minus
          //           275 - wind direction
          //           011 - wind speed
          //           0   - rool angle flag - greater than 5 degrees set to B otherwise G - 0 undefined
          //           XXXX - mixing ratio - left undefined
          //           2200XO20)  2 followed by a 10s of seconds character, followed by 7 characters of 
          //                      compressed turbulence
          //

          //string inputString = "N34363W11808419461052P088108006GXXXX22::A0/";
          //string inputString = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX2X009X/9W";

	  string inputString = tokenptr;
	  
          // using find to allow some flexibility in the location of the lat/lon characters
          if (inputString.find('N') == string::npos)
          {
	      //assume South
	      cout << "N not found, assuming South" << endl;
              latChar[0] = 'S';
          }
          else
          {
	      //Found North - check location
	      if (inputString.find('N') < 3)
                 latChar[0] = 'N';
              else
              {
                 badMsg = TRUE;
                 cerr << "ERROR: Illegal lat character" << endl;
	         cerr << "position found at " << inputString.find('N') << endl;
              }
          }

	  subStr = inputString.substr(1,2);
	  if (is_number(subStr))
	     latDeg = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing latitude degree (" << subStr << ") is not a number " << endl;
          }

	  subStr = inputString.substr(3,3);
	  if (is_number(subStr))
	     latMin = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing latitude degree (" << subStr << ") is not a number " << endl;
          }
    

          if (!badMsg)
          {
            if (latChar[0] == 'N')
               lat = latDeg + latMin/600.;
            else
               lat = -1 * (latDeg + (latMin/600.));

	    cerr << "latitude direction character " << latChar[0] << endl;
	    cerr << "latitude degree " << latDeg << endl;
	    cerr << "latitude minutes " << latMin << endl;
	    cerr << "latitude set to " << lat << endl;
          }

          if (inputString.find('W') == string::npos)
          {
	      //assume East
	      cout << "W not found, assuming East" << endl;
              lonChar[0] = 'E';
          }
          else
          {
	      //Found West - check location
	      if (inputString.find('W') < 13)
                 lonChar[0] = 'W';
              else
              {
                 badMsg = TRUE;
                 cerr << "ERROR: Illegal lon character" << endl;
              }
          }

	  subStr = inputString.substr(7,3);
	  if (is_number(subStr))
	     lonDeg = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing longitude degree (" << subStr << ") is not a number " << endl;
          }

	  subStr = inputString.substr(10,3);
	  if (is_number(subStr))
	     lonMin = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing longitude minutes (" << subStr << ") is not a number " << endl;
          }
	  
          if (!badMsg)
          {
            if (lonChar[0] == 'W')
               lon = -1 * (lonDeg + (lonMin/600.));
            else
              lon = lonDeg + (lonMin/600.);

	    cerr << "longitude direction character " << lonChar << endl;
	    cerr << "longitude degree " << lonDeg << endl;
	    cerr << "longitude minutes " << lonMin << endl;
	    cerr << "longitude set to " << lon << endl;
          }

	  subStr = inputString.substr(13,2);
	  if (is_number(subStr))
	     hour = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing hour (" << subStr << ") is not a number " << endl;
          }

	  subStr = inputString.substr(15,2);
	  if (is_number(subStr))
	     minute = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing minute (" << subStr << ") is not a number " << endl;
          }

	  cerr << "hour " << hour << endl;
	  cerr << "minutes " << minute << endl;

	  subStr = inputString.substr(17,4);
	  if (is_number(subStr))
	     alt = atof(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing altitude (" << subStr << ") is not a number " << endl;
          }

	  //cerr << "Pressure altitude " << alt << endl;

	  satSign = inputString.substr(21,1);
	  subStr = inputString.substr(22,3);
	  if (is_number(subStr))
	     sat = atof(subStr.c_str());
          else
          {
             // Leaving message good since this field is not considered critical
             cerr << "Value representing static air temperature (" << subStr << ") is not a number " << endl;
          }

          if (satSign[0] == 'P')
             sat = sat/10.0;
          else 
             if (satSign[0] == 'M')
                sat = -sat/10.0;

	  //cerr << "sat sign " << satSign << endl;
	  //cerr << "sat " << sat << endl;

	  subStr = inputString.substr(25,3);
	  if (is_number(subStr))
	     wind_dir = atof(subStr.c_str());
          else
          {
             // Leaving message good since this field is not considered critical
             cerr << "Value representing wind direction (" << subStr << ") is not a number " << endl;
          }

	  subStr = inputString.substr(28,3);
	  if (is_number(subStr))
	     wind_speed = atof(subStr.c_str());
          else
          {
             // Leaving message good since this field is not considered critical
             cerr << "Value representing wind speed (" << subStr << ") is not a number " << endl;
          }

	  //cerr << "Wind direction " << wind_dir << endl;
	  //cerr << "Wind speed " << wind_speed << endl;

	  subStr = inputString.substr(37,1);
	  if (is_number(subStr))
	     second = atoi(subStr.c_str());
          else
          {
             badMsg = TRUE;
             cerr << "Value representing seconds (" << subStr << ") is not a number " << endl;
          }


	  //cerr << "Seconds " << second << endl;

	  turb_str = inputString.substr(38,7);
	  cout << "Raw turb_str = " << turb_str << endl;

	  cerr << "Turb characters " << turb_str[0] << " " 
	                             << turb_str[1] << " "
	                             << turb_str[2] << " "
	                             << turb_str[3] << " "
	                             << turb_str[4] << " "
	                             << turb_str[5] << " "
	                             << turb_str[6] << endl;

          if (badMsg)
            continue;

	  if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) 
          {
            cerr << _className() << "::decodeEdrMsg(): Date out of range. Tail number: " << aircraft_registry_num << endl;
            return FAIL;
          }
          
          alt*=10;

          second *= 10;

	  cerr << "hour " << hour << " minute " << minute << " seconds " << second << endl;
          getRecordTime(msgTime, hour, minute, second);

          cerr << "\n" << _className() << ":Decoded Edr message fragment: \n" << endl;
          cerr << "\tmsgTime: " << msgTime.dtime() <<  " or " << msgTime.utime() << endl;
          cerr << "\trecordTime: " << recordTime.dtime() << " or " << recordTime.utime() << endl;
          cerr << "\tlat: " << lat << endl;
          cerr << "\tlon: " << lon << endl;
          cerr << "\thour " << hour << endl;
          cerr << "\tminute " << minute << endl;
          cerr << "\tsecond " << second << endl;
          cerr << "\talt " << alt << endl;
          cerr << "\tsatsign " << satSign << endl;
          cerr << "\tsat " << sat << endl;
          cerr << "\twind dir " << wind_dir<< endl ;
          cerr << "\twind_speed " << wind_speed<< endl ;
          cerr << "\trep data: "<< turb_str[0] << " " << turb_str[1] <<  " " << turb_str[2];
	  cerr << " " << turb_str[3] << " " << turb_str[4] << " " << turb_str[5] <<  " " << turb_str[6] << endl;
          cerr << "\n\tTens of seconds = " << second / 10 << endl;
          cerr << "\tRunning conf max = " << turb_str[0] << endl;
          cerr << "\tRunning number of bad = " << turb_str[1] << endl;

          runningMinConf = turb_str[0];
          runningNumBad  = turb_str[1];
         
	  
	  strncpy (rep_char, turb_str.c_str(), 7);
          if (decode_turb_chars(outfields, &num_fields, &rep_char[2]) != FAIL)
          {
            
            // outfields contents = {Version, Peak Edr, Mean Edr, Peak Edr Confidence,
            //                       "Mean Edr Confidence, Peak Location, Num Good Edrs};
            for (int f = 0; f < num_fields; f++)
              cerr << out_string[f] << "  " <<  outfields[f] << endl;

            cerr << "\n" << endl;

            edrAlgVersion = outfields[0];

            edr_peak[0] =  outfields[1];
            edr_ave[0]  =  outfields[2];

            peakConf = outfields[3];
            meanConf = outfields[4];
            peakLocation = outfields[5];
            numGood = outfields[6];

            if (checkForDuplicateEntry())
	    {

		cerr << "\n" << _className() << "::decode: Duplicate edr entry detected for flight id: "
			<< flight_id << " tail number: " << aircraft_registry_num <<
			" at record time: " << recordTime.utime() << " and File time "
					<< msgTime.dtime() << endl << endl;

		continue;
	    }
	    if (checkForRepeatEntry(3, recordTime.utime() - 1))
	    {
		cerr << "\nSouthwestEdrReport::decode: Repeat edr entry detected for flight id: "
				<< flight_id << " tail number: " << aircraft_registry_num <<
				" at record time: " << recordTime.utime() << " and File time "
				<< msgTime.dtime() << endl << endl;
		continue;
	    }

	    if (checkForNonMinuteTime())
            {

               cerr << "\nSouthwestEdrReport::decode: Unexpected time interval between reports detected for flight id: "
                          << flight_id << " tail number: " << aircraft_registry_num <<
                          " at record time: " << recordTime.utime() << " and File time "
                                                  << msgTime.dtime() << endl << endl;
	       //Remove all reports from _edrBuffer
	       _edrBuffer.clear();
               break;
            }

            if (setAndBufferEdr(msgTime))
               cerr << _className() << "::decode: Error writing edr data.\n";

          }
          else
            cerr << "Decode failed" << endl;
       }
       else
	  break;
    }
    return (writeBufferedVectorData());
  }

 /**************************************************************************
  * decode_turb_chars
  */

  EdrReport::status_t SouthwestEdrReport::decode_turb_chars (float fields[], 
                                                             int *num_fields, 
                                                             char encoded_turb[])
  {
    int index;
    int i;
    int idx[MAX_FIELDS];
    int out_idx[MAX_FIELDS];
    const int num_encoded_vals[ENCODED_NUM_FIELDS] = {8, 41, 41, 11, 11, 11, 6};
    const float step_size[ENCODED_NUM_FIELDS] = {1., 0.02, 0.02, 0.1, 0.1, 0.1, 2.};
    const int save_missing[ENCODED_NUM_FIELDS] = {0, 1, 1, 0, 0, 0, 0};
    const int num_char[5] = {41, 41, 41, 41, 41};
    const char char_set[41] =
            {'0','1','2','3','4','5','6','7','8','9',
            'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
            ',','-',';','/',':' };
    //unused variable
    //const char X_value[1] = {'X'};


    *num_fields = ENCODED_NUM_FIELDS;

    if (strncmp(encoded_turb, &char_set[NUM_OUTPUT_CHAR-1], 5) == 0)
    {
      cerr << "Unusable message - message contains :::::" << endl;
      edr_qc_flag[0] = Edr::QC_BAD_OTHER;
      return(BAD_DATA);
    }


    if (strncmp(encoded_turb, "XXXXX", 5) == 0)
    {
      cerr << "Unusable message - message contains XXXXX" << endl;
      edr_qc_flag[0] = Edr::QC_BAD_OTHER;
      return(BAD_DATA);
    }

    /* First extract indices of characters */
    for (index = 0; index < NUM_CHAR; index++)
    {
      i = 0;
      while (encoded_turb[index] != char_set[i] && i < NUM_OUTPUT_CHAR)
        i++;

      if (i == NUM_OUTPUT_CHAR)
      {
          cerr << "Could not find " << encoded_turb[index] << " in character set" << endl;
          edr_qc_flag[0] = Edr::QC_BAD_CHR;

          return(BAD_DATA);
      }

      idx[index] = i;
    }

    /* Second, change basis to extract data fields indices */
    if (FAIL == change_basis(NUM_CHAR, 
                           (int *)num_char, 
                           idx, 
                           ENCODED_NUM_FIELDS, 
                           (int *)num_encoded_vals, 
                           out_idx))
    {
      edr_qc_flag[0] = Edr::QC_BAD_BASIS;
      return(BAD_DATA);
    }

    /* Third, map from indices to values */
    for (index = ENCODED_NUM_FIELDS-1; index >= 0; index--)
    {
      if (save_missing[index] && out_idx[index] == num_encoded_vals[index]-1)
      {
        fields[index] = MISSING_DATA;
        edr_qc_flag[0] = Edr::QC_NO_VALUE;
      }
      else
      {
        fields[index] = out_idx[index] * step_size[index];
        edr_qc_flag[0] = Edr::QC_GOOD_VALUE;
      }

      if (params->debug == Params::DEBUG_VERBOSE)
        cerr << "output value " << index << "  " << fields[index] << endl;
    }

    return(ALL_OK);
  }

 /**************************************************************************
  * determine if a string represents a number
  */

  bool SouthwestEdrReport::is_number(const std::string& s)
  {
     std::string::const_iterator it = s.begin();
     while (it != s.end() && std::isdigit(*it)) ++it;

     return !s.empty() && it == s.end();
  }


