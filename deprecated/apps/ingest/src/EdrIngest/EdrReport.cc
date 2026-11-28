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

// EdrReport.cc : implementation for the EdrReport class.
//                    The EdrReport class is an abstract base
//                    class for various airline carrier objects
//                    (United, Southwest, Delta) responsible for
//                    decoding and writing EDR reports.
//
//  Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  Added ability to insert data into WFS - Nov 2012
//                    
///////////////////////////////////////////////////////////////

#include <stdio.h>
#include "EdrReport.hh"

using namespace std;

const int EdrReport::NUM_CHAR = 5;
const int EdrReport::MAX_FIELDS = 32;
const float EdrReport::EDR_MAX_VAL = 2.0;
const float EdrReport::CONF_MAX_VAL = 1.0;
const int EdrReport::NUM_OUTPUT_CHAR = 41;
const int EdrReport::MAX_DIM = 50;
const int EdrReport::MISSING_DATA = -9999;
const unsigned int EdrReport::MAX_TOKENPTR_LEN = 45;


/**************************************************************************
 * Constructor.
 */

  EdrReport::EdrReport(void)
 {
    // Do nothing.
 }


 /**************************************************************************
  * Destructor
  */

  EdrReport::~EdrReport(void)
  {
     // Do nothing.

  }


 /**************************************************************************
  * setAndWriteEdr
  */

  int EdrReport::setAndWriteEdr(DateTime msgTime)
  {
     // Do nothing.
     return -1;
  }


 /**************************************************************************
  * decode
  */


  EdrReport::status_t EdrReport::decodeAscii (char* buffer)

  {
     // Do nothing.
     return FAIL;

  }


 /**************************************************************************
  * decode
  */


  EdrReport::status_t EdrReport::decodeBufr (BUFR_Val_t &bv)

  {
     // Do nothing.
     return FAIL;

  }

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: getRecordTime(DateTime &msgTime, 
//                              int month,
//                              int day,
//                              int hour,
//                              int minute,
//                              int second)
//
// Description:  This function defines the year and sets the month, day hour minute
//               and second for the for the DateTime object for a given record.
//
// Returns:
//
// Globals:
//
// Notes: The edr reports do not contain the year of the individual reports. 
//        The report header does contain the day of the month, the hour, minute 
//        and seconds.
//
//        Therefore we use the file time stamp on the message to ascertain
//        the actual date of the records. That time is recored when the message
//        arrives at NCAR. 
//
//        Problems may occur when close to the end of the year.
//
void EdrReport::getRecordTime(DateTime msgTime, 
                              int month, 
                              int day, 
                              int hour, 
                              int minute, 
                              int second)
{


  if (msgTime.getMonth() == 1 && month == 12)
     recordTime.set(msgTime.getYear() -1,  month, day, hour, minute, second);
  else
     recordTime.set(msgTime.getYear(),  month, day, hour, minute, second);

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: getRecordTime(DateTime &msgTime, int hour, int minute, int second)
//
// Description:  This function defines the year, month, day for the DateTime
//               object for a given record.  This is the legacy approach
//               used for United downlinks until the software is updated.
//
// Returns:
//
// Globals:
//
// Notes: The edr reports do not contain the year, month, and day
//        of the individual reports. The report header does contain
//        the day of the month, the hour, and the minute (ddhhmm); some
//        reports also have a notion of seconds.
//
//        Therefore we use the time stamp on the message to ascertain
//        the date of the records. We do record the time that the message
//        arrives at NCAR.
//
//        Assuming realtime data flow, these times should be close.  The
//        the year and day of the data contained in the file, however, might be
//        different than the year and day of the reports since, taking into account
//        downlink times and the number of reports that may be contain in each
//        downlinked report.
//
//        We first assume that the year, month, and day of the edr records are the year,
//        month, day of the message arrival time.
//
//        Then we compare the unix times of the record and the message arrival time;
//        if the (time in filename) - (report time) is less than 12hrs, we need to
//        to subtract a day from the report time.
//

void EdrReport::getRecordTime(DateTime msgTime, int hour, int minute, int second)
{
  //
  // Set the record time, use the message time as a first guess
  // to year, month, and day.
  //

  recordTime.set(msgTime.getYear(),  msgTime.getMonth(), msgTime.getDay(),
                 hour, minute, second);

  time_t recordT, messageT;

  recordT = recordTime.utime();

  messageT = msgTime.utime();

  //
  // If the message time - the report time is less than -12 hours,
  // then the report time is off by 24 hours so subtract 1 day
  // and reset record
  //
  if(messageT - recordT < -43200)
    {
      recordT = recordT - 86400;
      recordTime.set(recordT);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
//  bufferEdrReport is used to add multiple reports to SPDB to minimize IO and to eliminate
//  dropped reports by the downstream QC programs who may process a particular fileTime
//  before all data has been processed.
//
void EdrReport::bufferEdrReport(Edr::edr_t &edrData)
{

  Edr edr;
  edr.setEdr(edrData);
  edr.assemble();

  if (params->debug == Params::DEBUG_VERBOSE)
  {
      cerr << endl;
      cerr << _className() << "::addEdrData:\n" << endl;
      cerr << "\tWriting the following data point to " << params->output_url << endl;
      edr.print(cerr, "\t");
  }

  _spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Construct the data types from the tail number
  //

  int dataType1, dataType2;

  dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

  char *strPtr = aircraft_registry_num + 4;

  dataType2 = Spdb::hash4CharsToInt32(strPtr);

  _spdb.addPutChunk(
                dataType1,
                (time_t)edr.getTime(),
                (time_t)edr.getTime() + params->expire_sec,
                edr.getBufLen(),
                edr.getBufPtr(),
                dataType2);
}
 
////////////////////////////////////////////////////////////////////////////////////////////////
//  writeWfsData is used to put a single EDR report into a Web Feature Service (WFS) data
//  store 
//
void EdrReport::writeWfsData(Edr::edr_t &edrData)
{
  bool written = _wfs._writeXmlEdr(edrData, params->output_XML_url, params->sendToWfsServer);
  if (!written)
  {
     cerr << "ERROR writing XML output" << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//  WriteBufferedSpdbData - small method used to write buffered SPDB EDR data 
//
EdrReport::status_t EdrReport::writeBufferedSpdbData()
{
   if (_spdb.put(params->output_url,
             SPDB_EDR_POINT_ID,
             SPDB_EDR_POINT_LABEL))
   {
       cerr << _className() << "::writeBufferedData: Couldn't write buffered EDR data to SPDB "
                            << params->output_url << endl;
       cerr << "  " << _spdb.getErrStr() << endl;
       return (BAD_DATA);
   }

   return ALL_OK;

}



////////////////////////////////////////////////////////////////////////////////////////////////
//  WriteBufferedVectorData - small method used to write buffered EDR data from a vector:
//
EdrReport::status_t EdrReport::writeBufferedVectorData()
{
   

   if (_edrBuffer.size() > 0)
   {
     for (unsigned int i = 0;  i < _edrBuffer.size(); i++)
     {

      bufferEdrReport(_edrBuffer[i]);

     }

     if (_spdb.put(params->output_url,
		SPDB_EDR_POINT_ID,
		SPDB_EDR_POINT_LABEL))
     {
	cerr << _className() << "::writeBufferedData: Couldn't write buffered EDR data to SPDB "
		<< params->output_url << endl;
	cerr << "  " << _spdb.getErrStr() << endl;
	return (BAD_DATA);
     }

   }
   _edrBuffer.clear();
   return ALL_OK;
    
}

////////////////////////////////////////////////////////////////////////////////////////////////
//  Write edrData to spdb data store.   put_mode_unique is true by default - if set to false
//  the new record will replace the previously stored record at the same valid time
//
void EdrReport::writeEdr(Edr::edr_t &edrData, bool put_mode_unique /* true */)
{

  Edr edr;
  edr.setEdr(edrData);
  edr.assemble();

  if (params->debug == Params::DEBUG_VERBOSE)
  {
      cerr << endl;
      cerr << _className() << "::writeEdr:\n" << endl;
      cerr << "\tWriting the following data point to " << params->output_url << endl;
      edr.print(cerr, "\t");
  }

  if (put_mode_unique)
    _spdb.setPutMode(Spdb::putModeAddUnique);
  else
    _spdb.setPutMode(Spdb::putModeOver);

  //
  // Construct the data types from the tail number
  //

  int dataType1, dataType2;

  dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

  char *strPtr = aircraft_registry_num + 4;

  dataType2 = Spdb::hash4CharsToInt32(strPtr);

  if ( _spdb.put(params->output_url,
                SPDB_EDR_POINT_ID,
                SPDB_EDR_POINT_LABEL,
                dataType1,
                (time_t)edr.getTime(),
                (time_t)edr.getTime() + params->expire_sec,
                edr.getBufLen(),
                edr.getBufPtr(),
                dataType2))
  {
      cerr << "\n" << _className() << "::writeEdr: ERROR: Spdb Put failed to "
           << params->output_url << endl;
      cerr << "\t time " << (time_t) edr.getTime() << " Buffer Length " << edr.getBufLen() << endl;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////

int EdrReport::change_basis( int in_num_dim,
                             int in_dim[],
                             int in_val[],
                             int out_num_dim,
                             int out_dim[],
                             int out_val[])
{
  int base_prod, sum;
  int i, fld;
  int max_output_sum;

  for (i=0; i< out_num_dim; i++)
    if (out_dim[i] <= 0 || out_dim[i] >= EdrReport::MAX_DIM)
    {
        cerr << "Dimension " << i << " is " << out_dim[i] << ", Not allowed." << endl;
        return(FAIL);
    }

  base_prod = 1;
  sum = 0;

  for (fld = 0; fld < in_num_dim; fld++)
  {
      if (in_val[fld] < 0 || in_val[fld] >= in_dim[fld])
      {
          cerr << "invalid field value) " << endl;
          return(FAIL);
      }
      sum += base_prod * in_val[fld];
      base_prod *= in_dim[fld];

      if (params->debug == Params::DEBUG_VERBOSE)
          cerr << "fld " << fld << " index " << in_val[fld] << " sum " << sum
                               << " base_prod " <<  base_prod << endl;
  }
  if (params->debug == Params::DEBUG_VERBOSE)
    cerr << "encoded sum " <<  sum << endl;

  max_output_sum = 1;
  for (i=0; i< out_num_dim; i++)
    max_output_sum *=  out_dim[i];

  if (sum > max_output_sum)
  {
      cerr << " sum too big " << sum << " > " << max_output_sum << endl;
      return(FAIL);
  }

  base_prod = 1;
  for (i=0; i< out_num_dim-1; i++)
    base_prod *= out_dim[i];
  if (params->debug == Params::DEBUG_VERBOSE)
    cerr << "base_prod " <<  base_prod << endl;

  for (i=0; i< out_num_dim; i++)
  {
      out_val[out_num_dim-i-1] = sum / base_prod;
     sum = sum % base_prod;

      if (out_num_dim-i-2 >= 0)
        base_prod /= out_dim[out_num_dim-i-2];
      if (params->debug == Params::DEBUG_VERBOSE)
        cerr << "i " << i << "  " << out_num_dim-i-1  << " index " << out_val[out_num_dim-i-1]
                       <<  " out_dim " <<  out_dim[out_num_dim-i-2] << " sum "
                       << sum << " base_prod " <<  base_prod << endl;
  }

  return(ALL_OK);
}


///////////////////////////////////////////////////////////////////
// clearAll:
//        Initialize/clear data members
//
/////////////////////////////////////////////////////////////////

void EdrReport::clearAll()
{
  //memset(aircraft_registry_num, 0, Edr::TAILNUM_NAME_LEN);
  memset(encoded_aircraft_registry_num, 0, Edr::TAILNUM_NAME_LEN);
  memset(encoded_registry_num, 0, Edr::TAILNUM_NAME_LEN);

  recordTime.set(DateTime::NEVER);
  lat = Edr::VALUE_UNKNOWN;
  lon = Edr::VALUE_UNKNOWN;
  alt = Edr::VALUE_UNKNOWN;

  mach = Edr::VALUE_UNKNOWN;  // Delta MDCRS (Heartbeat) specific info
  rms  = Edr::VALUE_UNKNOWN;  // Delta MDCRS specific info
  //computedAirspeed = (si32)Edr::VALUE_UNKNOWN;  // Delta MDCRS specific info
  //maxNumBad = (si32) Edr::VALUE_UNKNOWN;
  computedAirspeed = -9999;  // Delta MDCRS specific info
  maxNumBad = -9999;

  peakConf = Edr::VALUE_UNKNOWN;
  meanConf = Edr::VALUE_UNKNOWN;
  peakLocation = Edr::VALUE_UNKNOWN;
  numGood = Edr::VALUE_UNKNOWN;
  QcDescriptionBitFlags = (si32)Edr::VALUE_UNKNOWN;

  runningMinConf = ':';
  runningNumBad = ':';

  // memset(flight_id, 0, Edr::FLIGHTNUM_NAME_LEN);

  for(int i = 0; i < 4; i++)
    {
      rep_data[i] = '\0';
      bufr_rep_data[i] = 63;
      edr_peak[i] = Edr::VALUE_UNKNOWN;
      edr_ave[i] = Edr::VALUE_UNKNOWN;
      edr_qc_flag[i] = (int)Edr::VALUE_UNKNOWN;
    }
  wind_dir = Edr::VALUE_UNKNOWN;
  wind_speed = Edr::VALUE_UNKNOWN;
  sat = Edr::VALUE_UNKNOWN;
  memset(orig_airport, 0, sizeof(orig_airport));
  memset(dest_airport, 0, sizeof(dest_airport));

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: checkForDuplicateFile()
//
//
//
//
// Description: Sometimes two independent radio stations picks up an identical
//              report from one source.  Several checks are made since we may
//              have a legal duplicate record.
//
//
// Returns: true an illegal report is found, false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////


bool EdrReport::checkForDuplicateFile()
{

   DsSpdb test_spdb;
   bool retVal = false;

   //
   // Construct the data types from the tail number
   //
   int dataType1, dataType2;

   dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

   char *strPtr = aircraft_registry_num + 4;

   dataType2 = Spdb::hash4CharsToInt32(strPtr);

   // First check to see if there is a record already in the database
   // with the exact time from this tail number

   if (params->debug == Params::DEBUG_VERBOSE)
   {
       cerr << "\ncheckDuplicateFile: Looking for report with exact "
                                    << "time in database.\n" << endl;

       cerr << "\tCalling test_spdb.getExact for time "
                                    << recordTime.utime() << " for exact edr report" << endl;
				   
   }

   if (test_spdb.getExact(params->duplicate_report_url,
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
      else // nChunks > 0
      {
         // There is a report in the database with the exact time of
	 // this report. This could be because there are multiple
	 // reports within one minute (can be up to three) or this file
	 // is a duplicate file sent in by two radio stations.
	 //
	 // Next get last report within this minute of this report for
	 // comparison.
      
         if (test_spdb.getInterval (params->duplicate_report_url,
	                               recordTime.utime() - 1,
				       recordTime.utime() + 59,
				       dataType1,
				       dataType2))
	 {
	    cerr << "\ttest_spdb.getInterval failed: " << endl;
	    return false;
         }
	 else
	 {
	    nChunks = test_spdb.getNChunks();
	    string str1, str2;
	    vector <Edr*>    edrVec;

	    str1 = Spdb::dehashInt32To4Chars(dataType1);
	    str2 = Spdb::dehashInt32To4Chars(dataType2);
	    string tailnum = str1 + str2;

	    if(nChunks <= 0)
	    {
	       if (params->debug == Params::DEBUG_VERBOSE)
	       {
	          cerr << "\ttest_spdb.getNChunks() = " << nChunks
					<< ", no data for " << tailnum << endl;
	       }
	       return false;
	    }
	else // nChunks > 0
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
          // We need to check the most recent edr point.
	  //
	  vector < Edr *> ::reverse_iterator r;

	  // for loop may be unnessessary...this way it checks every record
	  // within 1 minute of the current report for a match.

          for ( r = edrVec.rbegin(); r != edrVec.rend(); ++r)
	  {
	    Edr::edr_t lastRep = (*r)->getRep();

	    //
	    // Check if lastRep has same values
	    //
            if ((lastRep.lat == lat)  &&
	                      (lastRep.lon == lon) &&
			      (lastRep.alt == alt))
	    {
	      if (params->debug == Params::DEBUG_VERBOSE)
	      {
	        cerr << "\tFound exact edr point in database, Duplicate File" << endl;
	      }
	      retVal = true;
	      break;
            }
          }
        }
        //
        // clean up
        //
        vector < Edr *> :: const_iterator i;
	    
        if ( (int) edrVec.size() >0 )
        {
	   for ( i = edrVec.begin(); i != edrVec.end(); i++)
	   delete *i;
        }
        edrVec.erase(edrVec.begin(), edrVec.end());
     }	    
    }
  }

  return retVal;

}
/////////////////////////////////////////////////////////////////////////////
//
// Function Name: checkForDuplicateEntry()
//
//
// Description: Gennerally 620 reports contain 1 report with the current
//              time and 5 more one minute reports previous to that.  It
//              is possible that that time interval will overlap a MDCRS
//              heartbeat message already inserted into the SPDB database.
//
//              This method will detect this event so we can avoid inserting
//              a duplicate report.  The report that comes in first generally
//              contains more QC specific information so we will retain that
//              report.
//
// Returns: true a duplicate MDCRS report is found, false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////


bool EdrReport::checkForDuplicateEntry()
{

   DsSpdb test_spdb;
   bool retVal = false;
   fl32 delta_lat = 5.0;
   fl32 delta_lon = 5.0;
   fl32 delta_alt = 130.0;
   int nChunks = 0;


   // Construct the data types from the tail number
   //
   int dataType1, dataType2;

   dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

   char *strPtr = aircraft_registry_num + 4;

   dataType2 = Spdb::hash4CharsToInt32(strPtr);

   // First check to see if there is a record already in the database
   // with the exact time from this tail number

   if (params->debug == Params::DEBUG_VERBOSE)
   {
      cerr << "\nEdrReport::checkForDuplicateEntry: Looking for duplicate report with "
	                              << "time " << recordTime.utime() << " in database.\n" << endl;

   }


   if (test_spdb.getInterval (params->duplicate_report_url,
				recordTime.utime() - 15,
				recordTime.utime() + 15,
				dataType1,
				dataType2))
   {
      cerr << "\ttest_spdb.getInterval failed: " << endl;
      return retVal;
   }
   else
   {
      nChunks = test_spdb.getNChunks();
      string str1, str2;
      vector <Edr*>    edrVec;

      str1 = Spdb::dehashInt32To4Chars(dataType1);
      str2 = Spdb::dehashInt32To4Chars(dataType2);
      string tailnum = str1 + str2;

      if (nChunks <= 0)
      {
         if (params->debug == Params::DEBUG_VERBOSE)
         {
           cerr << "\ttest_spdb.getNChunks() = " << nChunks
                                 << ", no data for " << tailnum << endl;
         }
	 return retVal;
      }
      else //(nChunks > 0)
      {

         vector <Spdb::chunk_t> chunks  = test_spdb.getChunks();

         //
         // Disassemble chunks. Store in a vector.
         //
         for ( int i = 0; i < nChunks; i++ )
         {

            Edr *edrPtr = new Edr();
            edrPtr->disassemble( chunks[i].data, chunks[i].len );
            edrVec.push_back(edrPtr);

         }

         // We need to check the most recent edr point. 
         vector < Edr *> ::reverse_iterator r;

         // for loop may be unnessessary...this way it checks every record
         // within 1 minute of the current report for a match.
         for ( r = edrVec.rbegin(); r != edrVec.rend(); ++r)
         {
            Edr::edr_t lastRep = (*r)->getRep();
            //
            //
            // Check if lastRep has same values
            //
            if ((fabs(lastRep.lat - lat) <= delta_lat)  &&
                                       (fabs(lastRep.lon - lon) <= delta_lon) &&
                                       (fabs(lastRep.alt - alt) <= delta_alt) &&
                                       lastRep.edrPeak == edr_peak[0] &&
                                       lastRep.edrAve == edr_ave[0] )

            {
               if (params->debug == Params::DEBUG_VERBOSE)
               {
                  cerr << "\tFound duplicate entry in edr database" << endl;
               }

               retVal = true;
               break;

             }
	 }
	 //
	 // clean up
	 //
	 vector < Edr *> :: const_iterator i;

	 if ( (int) edrVec.size() >0 )
	 {
           for ( i = edrVec.begin(); i != edrVec.end(); i++)
           delete *i;
         }
	 edrVec.erase(edrVec.begin(), edrVec.end());
      }
   }
   return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: checkForRepeatEntry(int repeatNum, time_t checkTime)
//
//
// Description: We have seen occurances where it appears that the Delta Airlines
//              software was not initialized properly and thus the same encoded
//              EDR string is repeatedly reported, sometimes for several flights. 
//
//              This method will detect this event so we can avoid inserting
//              these reports. Once there is a third occurance of the same info
//              all other reports will be removed until a new EDR string is 
//              observed.
//
// Returns: true the last repeatNum reports contained the same info, false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////


bool EdrReport::checkForRepeatEntry(int repeatNum, time_t checkTime)
{

   DsSpdb test_spdb;
   bool retVal = false;
   //fl32 delta_lat = 5.0;
   //fl32 delta_lon = 5.0;
   //fl32 delta_alt = 130.0;
   int nChunks = 0;


   // Construct the data types from the tail number
   //
   int dataType1, dataType2;

   dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

   char *strPtr = aircraft_registry_num + 4;

   dataType2 = Spdb::hash4CharsToInt32(strPtr);

   // First grab the previous report from this tail number in the 
   //database

   if (params->debug == Params::DEBUG_VERBOSE)
   {
     cerr << "\nEdrReport::checkForRepeatEntry: Looking for previous report before "<< checkTime << endl;
   }
   if (_edrBuffer.size() > 0)
   {
     // Go backwards through _edrBuffer to see if rep with previous time
     // in vector
     for (int i = _edrBuffer.size()-1; i >= 0; i--)
     {
       if(_edrBuffer[i].time >= checkTime)
       {
	 continue;
       }	 
       
       // Found a rep with an earlier time
       if (_edrBuffer[i].edrAlgVersion == edrAlgVersion  &&
		           _edrBuffer[i].edrPeak == edr_peak[0] &&
                           _edrBuffer[i].edrAve == edr_ave[0]   &&
		           _edrBuffer[i].PeakConf == peakConf &&
		           _edrBuffer[i].MeanConf == meanConf &&
                           _edrBuffer[i].PeakLocation ==  peakLocation &&
                           _edrBuffer[i].NumGood == numGood)
       {
	 if (params->debug == Params::DEBUG_VERBOSE)
         {
	   cerr << "\tFound Repeat entry in edr database" << endl;	  
	 }
	 repeatNum -= 1;
	 if (repeatNum <= 0)
	 {
	   retVal = true;
	   return retVal;
	 }
	 else
	 {
	   if (params->debug == Params::DEBUG_VERBOSE)
	   {
	     cerr << "\tLooking for another Repeat entry in edr database" << endl;
	   }
	   retVal = checkForRepeatEntry(repeatNum,_edrBuffer[i].time - 1);
	 }
       }
       else
       {
	 if (params->debug == Params::DEBUG_VERBOSE)
         {
	   cerr << "\tPrevious Report not Repeat entry." << endl;  
	 }
	 return retVal;
       }

     }
   } 

   //_edrBuffer empty or reps in _edrBuffer must already be repeats
   //Now look in database for repeat
   
   // Currently set to look back 10 days
   if (test_spdb.getFirstBefore (params->duplicate_report_url,
				checkTime,
				864000,
				dataType1,
				dataType2))
   {
      cerr << "\ttest_spdb.getFirstBefore failed: " << endl;
      return retVal;
   }
   else
   {
      nChunks = test_spdb.getNChunks();
      string str1, str2;
      vector <Edr*>    edrVec;

      str1 = Spdb::dehashInt32To4Chars(dataType1);
      str2 = Spdb::dehashInt32To4Chars(dataType2);
      string tailnum = str1 + str2;

      if (nChunks <= 0)
      {
         if (params->debug == Params::DEBUG_VERBOSE)
         {
           cerr << "\ttest_spdb.getNChunks() = " << nChunks
                                 << ", no data for " << tailnum << endl;
         }
	 return retVal;
      }
      else //(nChunks > 0)
      {

         vector <Spdb::chunk_t> chunks  = test_spdb.getChunks();
	 
         //
         // Disassemble chunks. Store in a vector.
         //
         for ( int i = 0; i < nChunks; i++ )
         {

            Edr *edrPtr = new Edr();
            edrPtr->disassemble( chunks[i].data, chunks[i].len );
            edrVec.push_back(edrPtr);

         }

         // We need to check the most recent edr point. 
         vector < Edr *> ::reverse_iterator r;

         // for loop may be unnessessary...since only one rep
	 // should have been returned
         for ( r = edrVec.rbegin(); r != edrVec.rend(); ++r)
         {
            Edr::edr_t lastRep = (*r)->getRep();
            //
            //
            // Check if lastRep has same values
            // for edrAlgVersion, edr_peak[0], edr_ave[0]
	    // peakConf, meanConf, peakLocation, numGood
	    
            if (lastRep.edrAlgVersion == edrAlgVersion  &&
		           lastRep.edrPeak == edr_peak[0] &&
                           lastRep.edrAve == edr_ave[0]   &&
		           lastRep.PeakConf == peakConf &&
		           lastRep.MeanConf == meanConf &&
                           lastRep.PeakLocation ==  peakLocation &&
                           lastRep.NumGood == numGood)

            {
               if (params->debug == Params::DEBUG_VERBOSE)
               {
                  cerr << "\tFound Repeat entry in edr database" << endl;
		  
	       }
	       repeatNum -= 1;
	       if (repeatNum <= 0)
		 {
		   retVal = true;
		   break;
		 }
	       else
		 {
		   if (params->debug == Params::DEBUG_VERBOSE)
		     {
		       cerr << "\tLooking for another Repeat entry in edr database" << endl;
		     }
		   retVal = checkForRepeatEntry(repeatNum,lastRep.time - 1);
		 }
	    }
	    else
	    {
	      if (params->debug == Params::DEBUG_VERBOSE)
              {
		cerr << "\tPrevious Report not Repeat entry." << endl;  
	      }
	      break;
	    }
	       

	 }
      
	 //
	 // clean up
	 //
	 vector < Edr *> :: const_iterator i;

	 if ( (int) edrVec.size() >0 )
	 {
           for ( i = edrVec.begin(); i != edrVec.end(); i++)
           delete *i;
         }
	 edrVec.erase(edrVec.begin(), edrVec.end());
      }
   }
   return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: checkForNonMinuteTime()
//
//
// Description: We have seen occurances where it appears that the Delta Airlines
//              software is not reporting EDR every minute, but every 2 minutes and
//              a few  (~10) seconds. 
//
//              This method will detect this event for 620 reports so we can avoid 
//              inserting these reports into the database. 
//
// Returns: true if report times are not 1 minute apart in bundled message (+/- 10 seconds), 
//          false otherwise.
//
// Globals:
//
/////////////////////////////////////////////////////////////////////////////////


bool EdrReport::checkForNonMinuteTime()
{


   DsSpdb test_spdb;
   bool retVal = false;
   int nChunks = 0;


   // Construct the data types from the tail number
   //
   int dataType1, dataType2;

   dataType1 = Spdb::hash4CharsToInt32(aircraft_registry_num);

   char *strPtr = aircraft_registry_num + 4;

   dataType2 = Spdb::hash4CharsToInt32(strPtr);


   if (_edrBuffer.size() > 0)
   {
     if (params->debug == Params::DEBUG_VERBOSE)
     {
       cerr << "\nEdrReport::checkForNonMinuteTime: Checking time difference between reports."<< endl;
     }
     //Allow for 10 seconds off + or - from 60 seconds
     if( (((recordTime.utime() - _edrBuffer[_edrBuffer.size()-1].time) % 60) != 0) && !( ((recordTime.utime() - _edrBuffer[_edrBuffer.size()-1].time) <= 80 ) && ((recordTime.utime() - _edrBuffer[_edrBuffer.size()-1].time) >= 40 ))) 
     {
       //Timing off.
       
       //Make sure there wasn't a heartbeat report already in database:
       //Look back 80 seconds
       if (test_spdb.getFirstBefore (params->duplicate_report_url,
				recordTime.utime(),
				80,
				dataType1,
				dataType2))
       {
	 // If this call fails still consider the report time to be unexpected
	 cerr << "\ttest_spdb.getFirstBefore failed in checkForNonMinuteTime. " << endl;
	 if (params->debug == Params::DEBUG_VERBOSE)
	 {
	   cerr << "\tFound non-minute time difference between reports: " << (recordTime.utime() - _edrBuffer[_edrBuffer.size()-1].time) << endl;	  
	 }
	 retVal = true;
	 return retVal;
       }
       else
       {
	 nChunks = test_spdb.getNChunks();
	 if (params->debug == Params::DEBUG_VERBOSE)
	 {
	   cerr << "\tFound previous reports: " << nChunks << endl;
	 }

	 if (nChunks <= 0)
	 {
	   if (params->debug == Params::DEBUG_VERBOSE)
	   {
	     cerr << "\tFound non-minute time difference between reports: " << (recordTime.utime() - _edrBuffer[_edrBuffer.size()-1].time) << endl;
	   }
	   return true;
	 }
	 else
	 {
	   //Report in database means not unexpected interval
	   if (params->debug == Params::DEBUG_VERBOSE)
	   {
	     cerr << "\tTime interval not unexpected." << endl;
	   }
	   return false;
	 }  
       }	 
   
     }
   }
   return false;
}

