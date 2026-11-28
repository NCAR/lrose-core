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
 *  DeltaEdrReport.cc :
 *                    class for decoding Delta specific reports.
 *                    There two classifications of Delta reports, 
 *                    Heartbeat and Triggered - Functionality will need 
 *                    to be added to fill in null reports using an ASDI
 *                    feed.
 *
 *                    Delta reports use a ARINC 620 specification hybrid for
 *                    triggered reports; differences are centered around how 
 *                    lat/lon values are stored.  Delta used existing MDCRS
 *                    reports for heartbeat messages.  These reports are
 *                    formatted using ARINC 239 specifications.
 *
 *                    This is an implementation of the EdrReport base class
 *                    
 **************************************************************************/

#include <stdio.h>
#include "DeltaEdrReport.hh"

using namespace std;

const string DeltaEdrReport::out_string[] = {
                            "\tVersion", "\tPeak Edr", "\tMean Edr", 
                            "\tPeak Edr Confidence", "\tMean Edr Confidence", 
                            "\tPeak Location", "\tNum Good Edrs"};

/**************************************************************************
 * Constructor.
 */


  DeltaEdrReport::DeltaEdrReport (Params *parameters, 
                                 DateTime &msgtime,
                                 char* tailNumber, 
                                 char* flightId,
                                 char* aircraftRegistryNum,
                                 downlink_t &downlink_time) : EdrReport()


 {
    sprintf(sourceName,"DAL EDR data");
    sprintf (sourceFmt, "ACARS ASCII");   
    sprintf (tail_number, tailNumber); 
    sprintf (flight_id, flightId); 
    sprintf (aircraft_registry_num, aircraftRegistryNum);
    msgTime = msgtime;
    params = parameters;
    dnlnk_time = downlink_time;

    // Option to insert data into a CSS-WX WFS server
    if (params->createWfsData)
    {
       _wfs._init(params->soap_cmd);
    }

 }


 /**************************************************************************
  * Destructor
  */

  DeltaEdrReport::~DeltaEdrReport(void)
  {
     

  }


 /**************************************************************************
  * setAndBufferEdr
  */

  int DeltaEdrReport::setAndBufferEdr (DateTime msgTime)
  {

        if (edrAlgVersion  < 2 || edrAlgVersion > 7)
        {
           cerr << _className() <<  "::setAndBufferEdr algorithm version out of range: " 
                                 << edrAlgVersion  << endl;

           return BAD_DATA ;
         }

        Edr::edr_t edrData;
        memset ((void *)&edrData, 0, sizeof(Edr::edr_t));

        int time_dif = 0;

        //
        // set edr_t struct vals
        //
        edrData.time = recordTime.utime();
        edrData.fileTime = msgTime.utime();
        if (dnlnk_time.day != 1)
        {
           int dnlnk_secs = dnlnk_time.day * 24 * 3600 +  dnlnk_time.hour * 3600 + dnlnk_time.minute * 60;
           int msg_secs = recordTime.getDay() * 24 * 3600 +  recordTime.getHour() * 3600 + recordTime.getMin() * 60;

           time_dif = dnlnk_secs - msg_secs;
           if (time_dif > 3600 || time_dif < -60)
           {
              cerr << _className() <<  "::setAndBufferEdr Downlink time / record time check failed. Tail number: " << aircraft_registry_num  << endl;
              return BAD_DATA;
           }
        }

        time_dif = edrData.fileTime - edrData.time;

        if (time_dif  > 43200 || time_dif < -60) 
        {
           cerr << _className() <<  "::setAndBufferEdr message time / file time check failed. Tail number: " << aircraft_registry_num  << endl;
           return BAD_DATA;
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

        for (int j = 0; j < Edr::NUM_SPARE_FLOATS - 3; j++)
            edrData.spareFloats[j] = Edr::VALUE_UNKNOWN;

        edrData.QcDescriptionBitFlags = QcDescriptionBitFlags;

        sprintf(edrData.flightNum, "%s", flight_id);
        sprintf(edrData.aircraftRegNum, "%s",aircraft_registry_num);
        sprintf(edrData.sourceFmt, "%s", sourceFmt);
        sprintf(edrData.sourceName, "%s", sourceName);
        sprintf(edrData.origAirport, "%s", orig_airport);
	//cerr << "orig_airport " << orig_airport << endl;
	//cerr << "EdrData.origAirport is " <<  edrData.origAirport << endl;

        sprintf(edrData.destAirport, "%s", dest_airport);
	//cerr << "dest_airport " << dest_airport << endl;
	//cerr << "EdrData.destAirport is " <<  edrData.destAirport << endl;

        sprintf(edrData.airlineId, "%s", "DAL");

        // Delta (as per Christian Amaral 1-30-2009) wants the downlinked
        // aircraft number viewable on the in-situ display
        //sprintf (edrData.encodedAircraftRegNum, "%s", aircraft_registry_num);
	sprintf (edrData.encodedAircraftRegNum, "%s", tail_number);

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

        //writeEdr(edrData);
        //bufferEdrReport(edrData);
	_edrBuffer.push_back(edrData);

	// put report in a Web Feature Service data store if indicated

	if (params->createWfsData)
	{
	   // Delta is seeing problems with their routine report not being null
	   if (strncmp(edrData.aircraftRegNum, "N155DL", 6) != 0)
	   {
	      writeWfsData(edrData);
	   }
	   else
		cerr << "Filtering tail number N155DL as per Delta March 24, 2014" << endl;
	}
        return ALL_OK;
}



 /**************************************************************************
  * decode620format
  */

  int DeltaEdrReport::decode620format (char *tokenPtr, char *format, DateTime &msgTime)
  {

    char rep_char[8];
    char lat_char, lon_char, sat_sign;
    int hour, minute, second;
    int num_fields;
    fl32 outfields[8];
    int n;

    cerr << "Delta 620 Message " << endl;

    for (int i = 0; i < 8; i++) 
    {
      outfields[i] = Edr::VALUE_UNKNOWN;
    }

    // Example 620 text: "-  /WX02GN07KATLMSLP"
    //                             TRANSLATES TO:
    //                          VVFFDDdestOrig  VV (02) - version = 02
    //                                          FF (GN) - format (should be A,E, or D) Diverges from 
    //                                                    actual 620 specification -
    //                                                    should be only 1 character
    //                                           07 - Day
    //                                         dest - KATL
    //                                         Orig - MSLP
    //
    //    Up to 6 lines that look like:
    //
    //                    N13458W08900718470106P2771770100XXXX2210O830)    -> see below
    //

    int len = strlen(format);

    if (len < 12)
    {
       cerr << "ERROR: No origination or destination airport defined " << endl;
       strcpy((char *) orig_airport, "UNK");
       strcpy((char *) dest_airport, "UNK");
    }
    else
    {
       strncpy((char *) orig_airport, format + (len - 7), 3);
       strcpy((char *) dest_airport, format + (len - 3));
    }

    //if (params->debug == Params::DEBUG_VERBOSE)
    if (1)
    {
       cerr << "\torigination airport: " << orig_airport << endl;
       cerr << "\tdestination airport: " << dest_airport << endl;
    }

    // Decode 620 body - may have up to 6 independent messages

    _spdb.clearPutChunks();
    
    for (int i = 0; i < 6; i++)
    {
       //
       // Go to next line
       //
       tokenPtr = strtok(NULL,"\n\r");

       if (tokenPtr != NULL && (strncmp(tokenPtr, "\003", 1) != 0))
       {
          cerr << tokenPtr << endl;

          if (!isdigit(tokenPtr[13]))
          {
             cerr << _className() << "::decodeEdrMsg(): Bad date found. Tail number: " << aircraft_registry_num << endl;
             return BAD_DATA;
          }

          //     0123456789012
          //     N13458W089007 18470106P2771770100XXXX2210O830) 

          //     Up to 6 lines similar to:
          //
          //     N13458W08900718470106P2771770100XXXX2210O830) 
          //
          //                     TRANSLATES TO:
          //                           "N13458"  Lat N==North  - 134 deg 5.8 min - does not follow 620 standard
          //                           "W089007" Lon W==West   - 089 deg 0.7 min - does not follow 620 standard
          //                           "1847"    Time of obs   - HHMM
          //                           "0106"    Pressure altitude 
          //                           "P277"    Static Air Temperature P==plus  M==minus
          //                           "177"     Wind direction 
          //                           "010"     Wind speed
          //                           "0"       Roll angle flag  greater than 5 degrees set to B otherwise G - 0 undefined
          //                           "XXXX"    Wind Vector Mixing Ration - always left undefined 
          //                           "2210O830)"  2 followed by 8 encoded turbulence values
          //

          n  = sscanf(tokenPtr,"%c%5f%c%6f%2d%2d%4f%c%3f%3f%3f%*6c%1d%c%c%c%c%c%c%c",
                         &lat_char,
                         &lat,
                         &lon_char,
                         &lon,
                         &hour,
                         &minute,
                         &alt,
                         &sat_sign,
                         &sat,
                         &wind_dir,
                         &wind_speed,
                         &second,
                         &rep_char[1],
                         &rep_char[2],
                         &rep_char[3],
                         &rep_char[4],
                         &rep_char[5],
                         &rep_char[6],
                         &rep_char[7]);

          if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) 
          {
            cerr << _className() << "::decodeEdrMsg(): Date out of range. Tail number: " << aircraft_registry_num << endl;
            return BAD_DATA;
          }

          if (n == 1)  // read end of text character
          {
             cerr << _className() << "::decodeEdrMsg(): Read End of Txt" << endl;
             return BAD_DATA;
          }

          if ( n < 19 && !isspace((int)lat_char) && !iscntrl((int)lat_char) )
          {
             cerr << _className() << "::decodeEdrMsg(): Error reading data line - number read - " << n << endl;
             return BAD_DATA;
          }

          if (!((lon_char == 'W' || lon_char == 'E') && 
                 (lat_char == 'N' || lat_char == 'S')))
          {
             cerr << _className() << "::decodeEdrMsg(): Bad latitude/longitude. Tail number: " << aircraft_registry_num << endl;
             return BAD_DATA;
          }

          if (lon_char == 'W')
             // Delta doesn't follow the 620 spec
             lon = -1 * (lon / 1000.);
          else
             lon = lon/1000.;

          if (lat_char == 'N')
             lat = lat/1000.;
          else
             lat = -1 * (lat / 1000.);

          if(sat_sign == 'P')
            sat = sat/10.0;
          else
            sat = -sat/10.0;

          alt*=10;

          // Delta sends a 620 message at the beginning and ending
          // of each flight.  Sometimes the ending 620 has an altitude
          // close to USHRT_MAX
          if (alt > 65000)
          {
            alt = 0;
            cerr << "Altitude above 65000 Ft - setting to 0" << endl;
          }

          second *= 10;

          getRecordTime(msgTime, hour, minute, second);

          //cerr << "\n" << _className() << "Decoded Edr message fragment: \n" << endl;
          //cerr << "\tmsgTime: " << msgTime.dtime() <<  " or " << msgTime.utime() << endl;
          //cerr << "\trecordTime: " << recordTime.dtime() << " or " << recordTime.utime() << endl;
          //cerr << "\tlat: " << lat << endl;
          //cerr << "\tlon: " << lon << endl;
          //cerr << "\thour " << hour << endl;
          //cerr << "\tminute " << minute << endl;
          //cerr << "\tsecond " << second << endl;
          //cerr << "\talt " << alt << endl;
          //cerr << "\tsatsign " << sat_sign << endl;
          //cerr << "\tsat " << sat << endl;
          //cerr << "\twind dir " << wind_dir<< endl ;
          //cerr << "\twind_speed " << wind_speed<< endl ;
          //cerr << "\trep data: " << rep_char[1] << " " << rep_char[2] <<  " " << rep_char[3];
          //cerr << " " << rep_char[4] << " " << rep_char[5] << " " << rep_char[6] <<  " " << rep_char[7] << endl;
          //cerr << "\n\tSeconds (tens of seconds * 10) = " << second * 10 << endl;
          //cerr << "\tRunning min conf = " << rep_char[1] << endl;
          //cerr << "\tRunning max number of bad = " << rep_char[2] << endl;

          runningMinConf = rep_char[1];
          runningNumBad  = rep_char[2];

          // Initialize outputfields;

          if (decode_turb_chars(outfields, &num_fields, &rep_char[3]) != FAIL)
          {

            // outfields contents = {Version, Peak Edr, Mean Edr, Peak Edr Confidence,
            //                       "Mean Edr Confidence, Peak Location, Num Good Edrs};
            //for (int f = 0; f < num_fields; f++)
            //  cerr << out_string[f] << "  " <<  outfields[f] << endl;;

            //cerr << endl;

            edrAlgVersion = outfields[0];

            edr_peak[0] =  outfields[1];
            edr_ave[0]  =  outfields[2];

            peakConf = outfields[3];
            meanConf = outfields[4];
            peakLocation = outfields[5];
            numGood = outfields[6];

            // The following check determines if there is an exact duplicate message.
            // Sometimes two independent radio receivers picks up the same signal
            // from an aircraft - skip the entire set if true
            //if (i == 0)
            //{
            //  if (checkForDuplicateFile())
            //  {

            //     cerr << "\nDeltaEdrReport::decode620format: Duplicate edr file detected for flight id: "
            //              << flight_id << " tail number: " << aircraft_registry_num <<
            //              " at record time: " << recordTime.utime() << " and File time "
            //                                      << msgTime.dtime() << endl << endl;

            //     return BAD_DATA;
            //  }
            //}

            if (checkForDuplicateEntry())
            {

               cerr << "\nDeltaEdrReport::decode620format: Duplicate edr entry detected for flight id: "
                          << flight_id << " tail number: " << aircraft_registry_num <<
                          " at record time: " << recordTime.utime() << " and File time "
                                                  << msgTime.dtime() << endl << endl;

                 continue;
            }
	    if (checkForRepeatEntry(3, recordTime.utime() - 1))
            {

               cerr << "\nDeltaEdrReport::decode620format: Repeat edr entry detected for flight id: "
                          << flight_id << " tail number: " << aircraft_registry_num <<
                          " at record time: " << recordTime.utime() << " and File time "
                                                  << msgTime.dtime() << endl << endl;

                 continue;
            }
	    
	    if (checkForNonMinuteTime())
            {

               cerr << "\nDeltaEdrReport::decode620format: Unexpected time interval between reports detected for flight id: "
                          << flight_id << " tail number: " << aircraft_registry_num <<
                          " at record time: " << recordTime.utime() << " and File time "
                                                  << msgTime.dtime() << endl << endl;
	       //Remove all reports from _edrBuffer
	       _edrBuffer.clear();
               break;
            }

            if (setAndBufferEdr(msgTime) == BAD_DATA)
               cerr << _className() << "::decodeEdrMessage(): Error writing edr data.\n";
          }
          else
            cerr << "Decode failed" << endl;
       }
    }

    return(writeBufferedVectorData());
  }

 /**************************************************************************
  * decodeMDCRSformat
  */

  int DeltaEdrReport::decodeMDCRSformat (char *tokenPtr, char *format, DateTime &msgTime)
  {

    char *sectionPtr;
    fl32 outfields[8];
    int num_fields;

    for (int i = 0; i < 8; i++) 
    {
      outfields[i] = Edr::VALUE_UNKNOWN;
    }

    _spdb.clearPutChunks();

    // save the second to last line for processing if required
    sectionPtr = tokenPtr + 12;

    //
    // Go to next line
    //
    tokenPtr = strtok(NULL,"\n\r");
    if (strlen(tokenPtr) < 18)
    {
      cerr << "Skipping MDCRS message - contains no turbulence info - " << tail_number << endl;

      return BAD_DATA;
    }

    // Example 239 MDCRS text: "-  239N3749 17000118082358 9792 4317 -7864320-16-44254135Y -506 35720"
    //
    //                             TRANSLATES TO:
    //                                        "N3749" - Ship number
    //                                        "17000118082358 9792 4317 -7864320"
    //                                        "1700" -> flight number
    //                                        "011808" -> MMDDYY
    //                                        "235809" -> HHMMSS
    //                                        "792"  -> Mach -> .SSS
    //                                        " 4317" -> +43.17 Latitude
    //                                        " -7864320" Long/alt -> -7864 (-78.64); 
    //                                                         Altitude -> 320 (100s of feet -> 32000 ft)
    //                                        "-16-44254135Y -506"
    //                                        "-16" ->   TAT
    //                                        "-44" -> Saturated air temp
    //                                        "254" -> wind direction
    //                                        "135" -> wind speed
    //                                        "Y -506" -> software identifier
    //                                        "35720" -> total fuel quanity
    //
    //                                NEXT LINE:
    //                                        "287 33600082783::::::::XXXXXXXX"
    //
    //                             TRANSLATES TO:
    //                                        "287" -> computed airspeed
    //                                        "336" -> ground speed
    //                                        "0008" -> Max rms
    //                                        "2783" -> Magnetic Heading (NNN.N) 278.3
    //                                        "::::::::" -> 8 chars of encoded EDR values
    //                                        "XXXXXXXX" -> ORIG/DEST info, if there.

    // Format uses spaces to denote zeros - change all spaces to zeros.
    // Don't change the 8 chars of encoded EDR values.

    // skip to beginning of flight number (Example of skipped MDCRS text: "-  239N3739 "

    si08 length = strlen(sectionPtr);


    //replace spaces with zeros - example:
    //17000118082358 9792 4317 -7864320-16-44254135Y -506 35720" becomes
    //1700011808235809792043170-7864320-16-44254135Y0-506035720"
    //
    char space = ' ';
    for (int i = 0; i < length; i++ )
    {
      if (strncmp (&sectionPtr[i], &space, 1) == 0)
      {
        sectionPtr[i] = '0';
      }
    }

    char dash = '-';

    // Hack to fix problem when tat originally was ' -7' - the replace space
    // section above converts that to '0-7' which makes the sscanf read
    // out of alignment
    if (sectionPtr[33] == '0' && sectionPtr[34] == '-')
    {
	sectionPtr[33] = space;
    }

    // Yet Another Hack (YAH) to fix problem when lon and lon sign are wrong
    // An example (different from above case) is 
    // ' -1417' for longitude - is changed '-01417'
    // section above converts that to '0-1417' which again makes the sscanf read
    // out of alignment
    if (sectionPtr[24] == '0' && sectionPtr[25] == '-' )
    {
	sectionPtr[24] = dash;
	sectionPtr[25] = '0';
    }

    // Yet Another Another Hack (YAAH) to fix problem when lon and lon sign are wrong
    // An example (different from above case) is 
    // '  -999' for longitude - is changed '-00999'
    // section above converts that to '00-999' which again makes the sscanf read
    // out of alignment
    //
    // At this point the developers initiated the WePo leap


    // YAAAH to fix problem when lat and lat sign are wrong
    if (sectionPtr[19] == '0' && sectionPtr[20] == '-')
    {
	sectionPtr[19] = dash;
	sectionPtr[20] = '0';
    }

    // Yet Another Another Hack (YAAH) to fix problem when lon and lon sign is wrong
    // An example (different from above case) is 
    // '  -999' for longitude - is changed '-00999'
    // section above converts that to '00-999' which again makes the sscanf read
    // out of alignment
    //
    // At this point the developers initiated the WePo leap
    if (sectionPtr[24] == '0' && sectionPtr[25] == '0' && sectionPtr[26] == '-')
    {
	sectionPtr[24] = dash;
	sectionPtr[25] = '0';
	sectionPtr[26] = '0';
    }

    for (int i = 0; i < length - 1; i++ )
    {
      if ((strncmp (&sectionPtr[i], &dash, 1) == 0) 
                && (strncmp (&sectionPtr[i+1], &dash, 1) == 0))
      {
    
        cerr << _className() << "::decodeEdrMsg(): bad message, found consecutive dashes in message for tail number: " << aircraft_registry_num<< endl;
        cerr << sectionPtr << endl;
        return BAD_DATA;
  
      }
    }
    si32 flight_number = 0;
    si32 month, day, year, hour, minute, sec;
    char lat_sign, lon_sign;
    si32 tat;
    si32 total_fuel;

    strncpy(orig_airport, "MDC", 3);
    strncpy(dest_airport, "MDC", 3);


    //cerr << "New sectionPtr looks like "  <<  sectionPtr << endl;

    if (!isdigit(sectionPtr[4]))
    {
        cerr << _className() << "::decodeEdrMsg(): Bad date found. Tail number: " << aircraft_registry_num<< endl;
        return BAD_DATA;
    }

    int n  = sscanf(sectionPtr,"%4d%2d%2d%2d%2d%2d%2d%3f%c%4f%c%5f%3f%3d%3f%3f%3f%*10c%5d",
                      &flight_number,
                      &month,
                      &day,
                      &year,
                      &hour,
                      &minute,
                      &sec,
                      &mach,
                      &lat_sign,
                      &lat,
                      &lon_sign,
                      &lon,
                      &alt,
                      &tat,
                      &sat,
                      &wind_dir,
                      &wind_speed,
                      &total_fuel);

    if (n != 18)
    {
      cerr << _className() << "::decodeEdrMsg(): not all sscanf variables read, sscanf returned " << n  << endl;
    }
    if (hour < 0 || hour >= 24 || minute >= 60 || sec >= 60) 
    {
      cerr << _className() << "::decodeEdrMsg(): Date out of range. Tail number: " << aircraft_registry_num  << endl;
      return BAD_DATA;
    }

    if (lat_sign == '-')
      lat *= -1;
    if (lon_sign == '-')
      lon *= -1;

    lat /= 100.;
    lon /= 100.;

    alt*= 100;

    // Sometime Delta sends a MDCRS message after the aircraft
    // has landed.  We have noticed often the altitude is
    // close to USHRT_MAX - we set these to zero
    if (alt > 65000)
    {
       alt = 0;
       cerr << "Altitude above 65000 Ft - setting to 0" << endl;
    }

    mach /= 1000;

    int ground_speed;
    float mag_heading;
    char rep_char[8];
    char orig[3];
    char dest[3];

    //cerr << "TokenPtr looks like "  <<  tokenPtr << endl;

    //                                        plus "287 33600082783::::::::XXXXXXXX"
    //                                        "287" -> computed airspeed
    //                                        "336" -> ground speed
    //                                        "0008" -> Max rms
    //                                        "2783" -> Magnetic Heading (NNN.N) 278.3
    //                                        "::::::::" -> 8 chars of encoded EDR values
    //                                        XXXXXXXX -> ORIG/DEST, if there.

    for (int i = 5; i < 16; i++ )
    {
      if (strncmp (&tokenPtr[i], &space, 1) == 0)
      {
        tokenPtr[i] = '0';
      }

    }

    cerr << "TokenPtr looks like "  <<  tokenPtr << endl;

    //n  = sscanf(tokenPtr,"%3d%3d%4f%4f%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
    n  = sscanf(tokenPtr,"%3d%3d%4f%4f%c%c%c%c%c%c%c%c%*c%3c%*c%3c",
                      &computedAirspeed,
                      &ground_speed,
                      &rms,
                      &mag_heading,
                      &rep_char[0],
                      &rep_char[1],
                      &rep_char[2],
                      &rep_char[3],
                      &rep_char[4],
                      &rep_char[5],
                      &rep_char[6],
		      &rep_char[7],
		      orig,
                      dest);

   if (n < 12) 
   {
      cerr << _className() << "::decodeEdrMsg(): No Turbulence data " << endl;
      return BAD_DATA;
   }

   for (int i = 0; i < 8; i++ )
   {
      if (strncmp (&rep_char[i], &space, 1) == 0)
      {
         cerr << _className() << "::decodeEdrMsg(): All Spaces for Turbulence data " << endl;
         return BAD_DATA;
      }

   }
   
   
   cerr << _className() << "::decodeEdrMsg(): Number read from sccanf is " << n << endl;

   if (n != 14)
   {
     cerr << _className() << "::decodeEdrMsg(): No Orig/Dest info " << endl;
   }
   else
   {
     strncpy((char *) orig_airport, orig, 3);
     strncpy((char *) dest_airport, dest, 3);
     cerr << _className() << "::decodeEdrMsg(): Orig/Dest info " << orig << " " << dest << endl;
   }


   rms /= 1000;
   mag_heading /= 10;

   //cerr << "\n" << _className() << "::decodeEdrMessage(): \n" << endl;
   //cerr << "\tflight number: " << flight_number << endl;
   //cerr << "\tmonth " << month << endl;
   //cerr << "\tday " << day << endl;
   //cerr << "\tyear " << year << endl;
   //cerr << "\thour " << hour << endl;
   //cerr << "\tminute " << minute << endl;
   //cerr << "\tsecond " << sec << endl;
   //cerr << "\tmach " << mach << endl;
   //cerr << "\tlat val: " << lat << endl;
   //cerr << "\tlon val: " << lon << endl;
   //cerr << "\talt " << alt << endl;
   //cerr << "\ttat " << tat << endl;
   //cerr << "\tsat " << sat << endl;
   //cerr << "\twind dir " << wind_dir<< endl ;
   //cerr << "\twind_speed " << wind_speed<< endl ;
   //cerr << "\ttotal fuel " << total_fuel<< endl ;
   //cerr << "\tair speed " << air_speed<< endl ;
   //cerr << "\tground speed " << ground_speed<< endl ;
   //cerr << "\tmax rms " << max_rms<< endl ;
   //cerr << "\tmagnetic heading " << mag_heading<< endl ;
   //cerr << "\tencoded turb data: " << rep_char[0] << rep_char[1] << rep_char[2] << rep_char[3];
   //cerr << rep_char[4] << rep_char[5] << rep_char[6] << rep_char[7] << endl;
   //cerr << "\n\tTens of seconds = " << rep_char[0] << endl;
   //cerr << "\tRunning conf max = " << rep_char[1] << endl;
   //cerr << "\tRunning number of bad = " << rep_char[2] << endl;

   runningMinConf = rep_char[1];
   runningNumBad  = rep_char[2];

   getRecordTime(msgTime, month, day, hour, minute, sec);

   // check to see if we have a 620 message for this point

   if (decode_turb_chars(outfields, &num_fields, &rep_char[3]) != FAIL)
   {
       for (int i = 0; i < num_fields; i++)
          cerr << out_string[i] << "  " <<  outfields[i] << endl;;

       cerr << "\n" << endl;

       edrAlgVersion = outfields[0];
       edr_peak[0] =  outfields[1];
       edr_ave[0]  =  outfields[2];

       peakConf = outfields[3];
       meanConf = outfields[4];
       peakLocation = outfields[5];
       numGood = outfields[6];
            
      // The following check determines if there is an exact duplicate message.
      // Sometimes two independent radio receivers picks up the same signal
      // from an aircraft - skip the entire set if true

       if (checkForDuplicateFile())
       {

           cerr << "\nDeltaEdrReport::decodeMDCRSformat: Duplicate edr file detected for flight id: "
                      << flight_id << " tail number: " << aircraft_registry_num <<
                      " at record time: " << recordTime.utime() << " and File time "
                                          << msgTime.dtime() << endl << endl;

           return BAD_DATA;
       }

       // Remove duplicates
       
       if (checkForDuplicateEntry())
       {

          cerr << "\nDeltaEdrReport::decodeMDCRSformat: Duplicate edr entry detected for flight id: "
                     << flight_id << " tail number: " << aircraft_registry_num <<
                     " at record time: " << recordTime.utime() << " and File time "
                                          << msgTime.dtime() << endl << endl;

           return BAD_DATA;
       }

       if (checkForRepeatEntry(3, recordTime.utime() - 1))
       {

          cerr << "\nDeltaEdrReport::decodeMDCRSformat: Repeat edr entry detected for flight id: "
                     << flight_id << " tail number: " << aircraft_registry_num <<
                     " at record time: " << recordTime.utime() << " and File time "
                                          << msgTime.dtime() << endl << endl;

           return BAD_DATA;
       }
       // Not actually necessary to buffer this record since it is routine or
       // heartbeat which by default the report should only have one record.
       if (setAndBufferEdr(msgTime) == BAD_DATA)
       {
          cerr << _className() << "::decodeEdrMessage(): Error writing edr data.\n";
          return BAD_DATA;
       }

       return (writeBufferedVectorData());
   }
   else
       cerr << "Decode failed" << endl;
   return BAD_DATA;

 }

 /**************************************************************************
  * Decode
  */

  EdrReport::status_t DeltaEdrReport::decodeAscii (char* tokenptr)

  {

     cerr << "Decoding Delta Message" << endl;

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

     bool isSevenSixSeven = isDelta767( aircraft_registry_num);

     if (isSevenSixSeven)
     {
         cerr << "\tDelta 767 aircraft found" << endl;
         sprintf(sourceName, "DAL 767");
     }
     else
     {
         sprintf(sourceName, "DAL 737");
     }

     if (params->debug == Params::DEBUG_VERBOSE)
       cerr << "\tformat: " << format << "\n" << endl;

     if (strncmp (format, "/WX", 3) == 0)
     {
        sprintf(sourceFmt, "Triggered ASCII");
        decode620format (tokenptr, format, msgTime);
        return (ALL_OK);

     }
     else if (strncmp (format, "239", 3) == 0)
     {
       //if (params->debug == Params::DEBUG_VERBOSE)
       cerr << "\tDelta MDCRS format - heartbeat message" << endl;
       sprintf(sourceFmt, "Heartbeat ASCII");

        decodeMDCRSformat (tokenptr, format, msgTime);
        return (ALL_OK);

     }
     else
     {
        cerr << "\tUnrecognized  format" << endl;
        return (ALL_OK);
     }
          
     return FAIL;

  }

  /**************************************************************************
  * decode_turb_chars
  */

  EdrReport::status_t DeltaEdrReport::decode_turb_chars (float fields[], int *num_fields, char encoded_turb[])
  {
    int index;
    int i = 0;
    int idx[MAX_FIELDS];
    int out_idx[MAX_FIELDS];
    char missing_values[5]; 
    const int num_encoded_vals[ENCODED_NUM_FIELDS] = {8, 41, 41, 11, 11, 11, 6};
    const float step_size[ENCODED_NUM_FIELDS] = {1., 0.02, 0.02, 0.1, 0.1, 0.1, 2.};
    const int save_missing[ENCODED_NUM_FIELDS] = {0, 1, 1, 0, 0, 0, 0};
    //const int num_char[NUM_CHAR] = {41, 41, 41, 41, 41};
    const int num_char[5] = {41, 41, 41, 41, 41};
    const char char_set[41] =
               {'0','1','2','3','4','5','6','7','8','9',
               'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
               '(','-',')','/',':' };

    *num_fields = ENCODED_NUM_FIELDS;

    for (i = 0; i < 5; i++)
      missing_values[i] = char_set[NUM_OUTPUT_CHAR-1];

    /* Check to see if message is usable */
    if (strncmp(encoded_turb, missing_values, 5) == 0)
    {
      cerr << "Unusable message" << endl;
      edr_qc_flag[0] = Edr::QC_BAD_OTHER;
      return (BAD_DATA);
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
                             idx, ENCODED_NUM_FIELDS, 
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
        fields[index] = EdrReport::MISSING_DATA;
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
  * isDelta767
  */

  bool DeltaEdrReport::isDelta767 (const char *tailNumber)
  {

     char tail[10];
     int tnumber = 0;
  
     cout << "Checking tail number " << tailNumber << endl;

     // try to limit processing,  767 tail numbers start with "N1" "N9" or "N3" 
     if ((strncmp (tailNumber, "N1", 2) != 0) && (strncmp (tailNumber, "N8", 2) != 0) &&
		(strncmp (tailNumber, "N3", 2) != 0))
     {
        return FALSE;

     }

     strncpy (tail, &tailNumber[1],3);
     tnumber = atoi(tail);
     if (strncmp (tailNumber, "N1", 2) == 0) 
     {
        if (strncmp (&tailNumber[4], "DN", 2) == 0)
        {
           // N171DN-N199DN 
           if ((tnumber >= 171 && tnumber <= 199) || (tnumber == 169))
              return TRUE;
        }
        if (strncmp (&tailNumber[4], "DZ", 2) == 0)
        {
           // N171DZ-N178DZ and N169DZ
           if ((tnumber >= 171 && tnumber <= 178) || (tnumber == 169))
              return TRUE;
        }
        if (strncmp (&tailNumber[4], "DL", 2) == 0)
        {
           // N152DL-N156DL or N137DL-N139DL
           if ((tnumber >= 152 && tnumber <= 156) || (tnumber >= 137 && tnumber <= 139))
              return TRUE;

        }

        // N1604R, N16065, N1607B
        if (tnumber == 160)
        {
	   if (strncmp (&tailNumber[4], "4R", 2) == 0 ||
		    strncmp (&tailNumber[4], "65", 2) == 0 ||
		    strncmp (&tailNumber[4], "7B", 2) == 0)
                return TRUE;
        }

        // N1610D, N1611B, N1612T, N1613B
        if (tnumber == 161)
        {
	   if (strncmp (&tailNumber[4], "0D", 2) == 0 ||
		    strncmp (&tailNumber[4], "1B", 2) == 0 ||
		    strncmp (&tailNumber[4], "2T", 2) == 0 ||
		    strncmp (&tailNumber[4], "3B", 2) == 0)
                return TRUE;
        }

        strncpy (tail, &tailNumber[1],4);
        tnumber = atoi(tail);
        if (tnumber == 1200 || tnumber == 1201)
        {
           if (strncmp(&tailNumber[5], "K",1) == 0)
               return TRUE;
           if (strncmp(&tailNumber[5], "P",1) == 0)
               return TRUE;
        }

        //  N1602, N1603, N1605, N1608, N1609
        if (strlen(tailNumber) == 5)
           if (tnumber == 1602 || tnumber == 1603 || tnumber == 1605 ||
           		tnumber == 1608 || tnumber == 1609)
	       return TRUE;

        //  N140LL, N1402A, N143DA, N144DA, N1501P
	if (strncmp (&tailNumber[2], "40LL", 4) == 0 ||
		    strncmp (&tailNumber[2], "402A", 4) == 0 ||
		    strncmp (&tailNumber[2], "43DA", 4) == 0 ||
		    strncmp (&tailNumber[2], "44DA", 4) == 0 ||
		    strncmp (&tailNumber[2], "501P", 4) == 0)
                return TRUE;

     }
     // N394DL
     else 
     {
       
       if (tnumber == 394)
       {
          // N394DA is a 737 - filter out this tail number
          if (strncmp (&tailNumber[4], "DL", 2) == 0)
            return TRUE;
	  else
            return FALSE;

       }
       // N825MH-N845MH
       else if (strncmp (tailNumber, "N8", 2) == 0) 
       {
         if (strncmp (&tailNumber[4], "MH", 2 ) == 0)
         {
            strncpy (tail, "\0\0\0\0\0\0\0", 7);
            strncpy (tail, &tailNumber[1], 3);

            tnumber = atoi(tail);
            if (tnumber >= 825 && tnumber <= 845) 
               return TRUE;

         }
       }

     }
     return false;
  }
