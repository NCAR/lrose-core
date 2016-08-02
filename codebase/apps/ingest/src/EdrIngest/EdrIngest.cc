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
// EdrIngest.cc
//
// EdrIngest object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <toolsa/Socket.hh>
#include "EdrIngest.hh"
#include "EdrInput.hh"
#include "FileInput.hh"
#include "EdrDecoder.hh"

#include "EdrReport.hh"
#include "DeltaEdrReport.hh"
#include "SouthwestEdrReport.hh"
#include "UnitedEdrReport.hh"

using namespace std;

const ui32 EdrIngest::key[5] = {2657703306, 2541004132, 3816866951, 3829628205, 1965237371}; // uint32s
const si32 EdrIngest::shifts[5] = {3, -3, -1, -2, 2};
const ui32 EdrIngest::permutes[5] = {2, 1, 2, 1, 2};
const ui32 EdrIngest::iters = 5;                                 // number of encoding loops
const ui32 EdrIngest::ui32Nbytes = 4;

const ui32 perms[2][4] = {
	                     {2, 1, 4, 3}, 
			     {2, 4, 1, 3}};
//			     {3, 1, 4, 2}, 
//			     {3, 4, 1, 2}, 
//			     {3, 4, 2, 1},
//			     {4, 3, 1, 2}  };
//
// Constructor

EdrIngest::EdrIngest(int argc, char **argv) :
  _progName("EdrIngest"),
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

EdrIngest::~EdrIngest()

{
  // unregister

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// Run

int EdrIngest::Run()
{

 //
 // Create the input data stream
 //
 inputStream = new FileInput();
  
 if (inputStream->init(_params, _args.getInputFileList(),_progName) )
 {
     cerr << "EdrIngest::Run(): Unable to initialize data stream. Exiting\n";
     return 1;
 }

 //
 // Create buffer to hold edr message from input stream
 //
 ui08  *buffer;
 
 buffer = new ui08[2048];
 
 EdrReport::status_t status = EdrReport::ALL_OK;
 
 //
 // Read and decode messages from input stream
 //
 while (status != EdrReport::BAD_INPUT_STREAM && status != EdrReport::END_OF_DATA) {
   
   PMU_auto_register("EdrIngest::Run(): Reading message");
   
   //
   // Clear buffer
   //
   memset(buffer, 0, 1024);  
   
   DateTime msgTime;
   
   //
   // Read
   //
   status = inputStream->readFile(buffer, msgTime);
   
   //)
   // Decode
   //
   if (status == EdrReport::ALL_OK)
   {
       if (_params.debug == Params::DEBUG_VERBOSE)
       {
	   cerr << "EdrIngest::Run(): \n\n Processing EDR data:\n\n";
	   cerr << buffer << endl;
       }

       EdrReport::status_t status = decodeEdrMsg(buffer, msgTime);

       if (!(status == EdrReport::ALL_OK || status == EdrReport::END_OF_FILE))
          cerr << "Decoding failed" << endl;
   }
 } 
 
 //
 // Clean up
 //
 delete inputStream;
 
 delete [] buffer;
 
 return 0;
 
}


////////////////////////////////////////////////////////
// decodeEdrMsg
//         Method determines if the file format is 
//         BUFR or ASCII and calls the appropriate
//         decode method.
//
////////////////////////////////////////////////////////

EdrReport::status_t EdrIngest::decodeEdrMsg(ui08 *buffer, DateTime msgTime)
{
  
  EdrReport::status_t ret;

  if (_params.message_format == Params::ASCII)
    ret = decodeAsciiMsg(buffer, msgTime);

  else
    //
    // params.message_format == Params::BUFR
    //
    ret = decodeBufrMsg(buffer, msgTime);

  return (ret);
}


////////////////////////////////////////////////////////
// decodeAsciiMsg
//         Method decodes enough of the ASCII EDR report
//         to determine the carrier type.
//
////////////////////////////////////////////////////////

EdrReport::status_t EdrIngest::decodeAsciiMsg(ui08 *buffer, DateTime msgTime)
{

  PMU_auto_register("EdrIngest::decodeAsciiMsg(): Decoding EDR message");

  time_t now = time(0);
  cerr << utimstr(now) << ":  Processing ASCII file with time stamp, " << utimstr(msgTime.utime()) << endl;

  if (_params.debug == Params::DEBUG_VERBOSE)
      cerr << "\nEdrIngest::decodeAsciiMsg():Decoding message header: \n" << endl;


  char *tokenptr = strtok((char *) buffer, "\n\r");
  EdrReport::downlink_t dnlnk_time;
  //int downlink_day, downlink_hour, downlink_min;

  //
  // Text line 1: priority code/address.
  // Example text: ^AQU NCAR2XA
  //
  // Skip it.
  //
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\tSkipping priority code, address:\n";
      cerr << "\t" << tokenptr << "\n" << endl;
  }

  //
  // Go to the next line
  //
  tokenptr = strtok(NULL,"\n\r");

  //
  // Text line 2: Service provider tty address and
  //              day hour minute of report downlink time in form of ddhhmm.
  // Example text: .QXSXMXS 071855
  //
  // Scan day, hour, and minute.
  //

  if (_params.debug == Params::DEBUG_VERBOSE)
  {
       cerr << "\tScanning ACARS (downlink) day, hour, second token, DDHHSS:  \n";
       cerr << "\t" << tokenptr << "\n" << endl;
  }
  //sscanf(tokenptr,"%*s %2d%2d%2d", &downlink_day, &downlink_hour, &downlink_min);
  sscanf(tokenptr,"%*s %2d%2d%2d", &dnlnk_time.day, &dnlnk_time.hour, &dnlnk_time.minute);

  if (_params.debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\tdownlink day: " << dnlnk_time.day
           << "\n\t        hour: " << dnlnk_time.hour
           << "\n\t      minute: " << dnlnk_time.minute <<endl;
  }

  //
  // Go to next line
  //
  tokenptr = strtok(NULL,"\n\r");

  //
  // Text line 3: Message label. Skip it.
  // Example text: DFD
  //
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\tSkipping message label: \n";

      cerr << "\t" << tokenptr << "\n" << endl;
  }

  //
  // Go to next line
  //
  tokenptr = strtok(NULL,"\n\r");

  //
  // Text line 3: Flight ID token, flight ID, aircraft registry
  //              number token, aircraft registry number.
  // Example: FI DL0277/AN N3753
  //
  // Scan flight id, aircraft registry number
  //
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\tScanning flight id, aircraft registry number: \n";

      cerr << "\t" << tokenptr << "\n" << endl;
  }


  char arn_line[256];
  char tail_number[256];

  sprintf(arn_line,"%s", tokenptr + 3);
  sprintf(tail_number,"%s", tokenptr + 13);
  //strncpy(tail_number,"N251WN",6 );   // test tail number
  

  char *slashptr = strrchr(arn_line,'/');

  if (slashptr == NULL) {
    cerr << "EdrIngest::decodeEdrMsg:Downlinked message doesn't match specified format" << endl;
    return EdrReport::FAIL;
  }

  slashptr[0] = '\0';

  char  aircraft_registry_num[Edr::TAILNUM_NAME_LEN];
  char  flight_id[Edr::FLIGHTNUM_NAME_LEN];

  sprintf(flight_id, "%s ", arn_line);

  sscanf(tokenptr,"%*s %*s %s", aircraft_registry_num);

  //
  // Go to next line
  //
  tokenptr = strtok(NULL,"\n\r");

  //
  // Text line 4: "DT", Provider Code, Radio Station, day of month, hour, minute
  //               of report in form of ddhhmm(same as above), message sequence
  //               number.
  // Example text: DT DDL LNK 232359 D65A
  // Skip it.
  //
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
      cerr << "\tSkipping Provider Code, Radio Station, month day hour: \n";
      cerr << "\t" << tokenptr << "\n" << endl;
  }

  //
  // Go to next line
  //
  tokenptr = strtok(NULL,"\n\r");

  //string encoded_number = encrypt(tail_number);
  string encoded_number = encrypt(aircraft_registry_num);
  //cout << "Encoded tail number is " << encoded_number.c_str();
  char * encoded = new char[encoded_number.size() + 1];
  std::copy(encoded_number.begin(), encoded_number.end(), encoded);
  encoded[encoded_number.size()] = '\0'; // don't forget the terminating 0

  if (strncmp (flight_id, "DL", 2) == 0)
  {
    // Delta message
    _report = new DeltaEdrReport (&_params, 
                                   msgTime, 
                                   encoded, 
                                   flight_id, 
                                   aircraft_registry_num,
                                   dnlnk_time);

  }
  else
  {
    if (strncmp (flight_id, "UA", 2) == 0)
    {
      //edrAlgVersion = 1.0;

      //  UNITED message
      _report = new UnitedEdrReport (&_params, 
                                      msgTime, 
                                      encoded, 
                                      flight_id, 
                                      aircraft_registry_num,
                                      dnlnk_time);

      //sprintf(sourceName,"UAL EDR data");
    }
    else 
    {
      if (strncmp (flight_id, "WN", 2) == 0)
      {
        // SOUTHWEST message

	
        _report = new SouthwestEdrReport (&_params, 
	                                   msgTime, 
					   encoded, 
					   flight_id, 
					   aircraft_registry_num,
					   dnlnk_time);

        //sprintf(sourceName,"SWA EDR data");
      }
      else
      {
       cerr << "EdrIngest::decodeEdrMsg:Illegal flight ID - not supported" << endl;
       delete[] encoded;
       return EdrReport::FAIL;
      }
    }

  }
  // don't forget to free the string after finished using it
  delete[] encoded;

  //
  // Initialize/clear data members
  //
  _report->clearAll();
  
  EdrReport::status_t ret_val;
  
  ret_val =_report->decodeAscii(tokenptr);
  if (ret_val == EdrReport::ALL_OK)
     delete _report;

  return ret_val;

}                                                        

////////////////////////////////////////////////////////
// Encrypt
////////////////////////////////////////////////////////

string EdrIngest::encrypt (const char *orig_string)
{
  ui32 result;
  string strResult;
  
  result = bitify (orig_string, 0);
  if (result < 0)
    cerr << "ERROR: encrypting string " << orig_string << endl;
  
  ui32 bitEncoding = result;
  //Test section for converting back to original strings value
  //string debugStr;
  //debugStr = unbitify(result, 0);
  //cerr << "Bitify/unbitified string " << debugStr.c_str() << endl;
  
  for (int i = 0; i < (int)iters; i++)
  {
    bitEncoding = (bitEncoding ^ key[i]);
    bitEncoding = permute (bitEncoding, shifts[i], perms[permutes[i] - 1]);
  }
  
  strResult = unbitify(bitEncoding, 1);
  //cout << "Encoded Str " << strResult << endl;
  
  return (strResult);
}

////////////////////////////////////////////////////////
// Decrypt
////////////////////////////////////////////////////////
string EdrIngest::decrypt (const char * encoded_string)
{
  ui32 result;
  string strResult;

  result = bitify (encoded_string, 1);
  if (result < 0)
    cerr << "ERROR: encrypting string " << encoded_string << endl;

  //Test section for converting back to original strings value
  //string debugStr;
  //debugStr = unbitify(result, 1);
  //cerr << "Bitify/unbitified string " << debugStr.c_str() << endl;


  ui32 bitEncoding = result;
  for (int i = iters-1; i >= 0; i--)
    {
      bitEncoding = invpermute (bitEncoding, shifts[i], perms[permutes[i] - 1]);
      bitEncoding = (bitEncoding ^ key[i]);
    }
  
  strResult = unbitify(bitEncoding, 0);
  //cout << "Decoded Str " << strResult << endl;
  
  return (strResult);
}

////////////////////////////////////////////////////////
// unbitify
//         General method to take a unsigned int (32 bits) and 
//         convert it back to a string.
//         Allowed chars are 0-9,A-Z, if encoding is set to one
//         lowercase letters are also allowed.
//         
////////////////////////////////////////////////////////

string EdrIngest::unbitify (ui32 data, int encoding)
{
        int base = 0;

	// convert ui32 to array.  MSB stored in first characters.
	vector <fl64> numericVector; 

        base = 36 + encoding * 5;

	if (data > pow(base, 6))
        { 
	    cerr << "data is too large to convert to 6 characters" << endl;
	    string error = "999999";
	    return error;
	}
	for (int i = 6; i > 0; i--)
        {
           numericVector.push_back ((fl64) (data%base));
	   data = floor (double(data)/base); 
	}

	return ( numeric2Alpha(numericVector, encoding));
 
}

////////////////////////////////////////////////////////
// bitify
//         Encodes a text string and returns a clear text string.
//         Allowed chars are 0-9,A-Z,a-e.  Output characters 
//         will not contain a-e.  Further, note that all strings 
//         greater than 'b2cACa' (dictionary ordering) will generate 
//         an error (since that string is not storable in uint32
//         
////////////////////////////////////////////////////////

ui32 EdrIngest::bitify (const char *orig_string, int encoding)
{
	fl64 flt_result;       // Storage for numeric version of input
	ui32 result;           // Storage for numeric version of input
        int base = 0;          // how many bits to store the encrypted value in


	vector <fl64> numericVector = alpha2Numeric(orig_string, encoding);

        base = 36 + encoding * 5;
      
	// Convert array into ui32
	//
	//  NOTE:result here is a double.  we allow this since we want to check to see 
	//  if the number is bigger than intmax, which we couldn't do if res is uint32

	// put first "character" as MSBs
	flt_result = 0;
	for (ui32 i = 0; i < numericVector.size(); i++)
        {
           flt_result = flt_result * base + numericVector[i];

	}
	
	// error check.  This should only be possible for bitifying an encoded string
	// since the number of allowable characters is larger
	if (flt_result > numeric_limits<unsigned int>::max())
        {
           cerr << "string exceeds uint32 capacity" << endl;
	   result = 99999;
	}
        else
        {
            // store as ui32.
            result = ui32(flt_result);
	}

return (result);
 
}

////////////////////////////////////////////////////////
// alpha2Numeric
//         takes as input a alphanumeric string,  If encoding
//         = 1, the string can have lowercase letters as well as
//         0-9,A-Z.
//
//         Returns an float array.
//         
////////////////////////////////////////////////////////

vector<fl64>  EdrIngest::alpha2Numeric (const char *orig_string, int encoding)

{
   int length = strlen(orig_string);
   vector <fl64> output;

   for (int i = 0; i < length; i++)
   {
      if (orig_string[i]< fl64('A'))
	output.push_back(orig_string[i]-fl64('0'));
      else 
      {
         if (orig_string[i] < fl64('a'))
	    output.push_back((orig_string[i]) - fl64('A')+10);
	 else
         {
	    if (encoding == 1)
	       output.push_back(fl64('a')+10+26);
            else
               cerr << "Found data consitent with encoded data, but not using encoded bitify" << endl;
         }
      }
   }
   return (output);
}

////////////////////////////////////////////////////////
// numeric2Alpha
//         Useful for testing - allow user the ability to 
//         decode an encrypted value.   Converts an floating
//         point array and returns a string
//
//         INPUT: Floating point arrayinput a alphanumeric string,  
//              : encoding flag - if = 1, the output string 
//              can have lowercase letters as well as 0-9,A-Z.
//
//         Returns an string.
//         
////////////////////////////////////////////////////////

string EdrIngest::numeric2Alpha (vector <fl64> original, int encoding)
{
   int length = original.size();

   string output;
   output.resize(length, '0');
   int j = -1;

   for (int i = length; i >= 0; i--, j++)
   //for (int i = 0; i < length; i++-)
   {
      if (original[i]< 10)
	output[j] = char('0'+original[i]);
      else 
      {
         if (original[i] < 10+26)
	    output[j] = char('A'+original[i]-10);
	 else
         {
	    if (encoding == 1)
	       output[j] = char('a'+original[i]-10-26);
            else
               cerr << "Data implies encoded, but not using encoded bitify" << endl;
         }
      }
   }
   return output;
}

////////////////////////////////////////////////////////
// permute
//         Bitshift and permutes bytes in 32 bit unsigned
//         int
//
//         Returns an encoded 32 bit unsigned int.
//         
////////////////////////////////////////////////////////

ui32 EdrIngest::permute (ui32 inputNum, si32 shift, const ui32 *perms)

{

   ui32 value = inputNum;

   // performat bitshift first
   value = circularBitShift (inputNum, shift); 

   // break uint32 into 4 bytes.  LSBs go first.
   vector <ui32> byteVector; 
   vector <ui32> permuteVector; 
   for (int i = 0; i < (int)ui32Nbytes; i++)
   {
      byteVector.push_back(value % 256);   // 2^8
      value = value >> 8;
   }
    
   // permute - e.g. a vector with values of [4 3 2 1] will reorder so so a(4) a(3) a(2) a(1)
   for (ui32 i = 0; i < (int)ui32Nbytes; i++)
   {
      permuteVector.push_back(byteVector[perms[i]-1]);
   }

   //recombine back into uint32
   ui32 temp = 0;
   value = 0;
   for (int i = ui32Nbytes; i > 0; i--) 
   {
	 temp = value << 8;
	 value = permuteVector[i - 1] | temp;
   }
   return value;
}

////////////////////////////////////////////////////////
// invpermute
//         unpermutes and unBitshifts encoded 32 bit unsigned
//         int
//
//         Returns an 32 bit unsigned int.
//         
////////////////////////////////////////////////////////

ui32 EdrIngest::invpermute (ui32 inputNum, si32 shift, const ui32 *perms)

{
   ui32 value = inputNum;

   // break uint32 into 4 bytes.  LSBs go first.
   vector <ui32> byteVector; 
   ui32 permuteVector[(int)ui32Nbytes];
   for (int i = 0; i < (int)ui32Nbytes; i++)
   {
      byteVector.push_back(value % 256);   // 2^8
      value = value >> 8;
   }

   // unpermute - e.g. a vector with values of [4 3 2 1] will reorder so so a(4) a(3) a(2) a(1)
   for (int i = 0; i < (int)ui32Nbytes; i++)
   {
     permuteVector[perms[i]-1] = byteVector[i];
   }

   //recombine back into uint32
   ui32 temp = 0;
   value = 0;
   for (int i = ui32Nbytes; i > 0; i--) 
   {
	 temp = value << 8;
	 value = permuteVector[i - 1] | temp;
   }

   // performat bitshift last
   value = circularBitShift (value, shift*-1); 

   return value;
}

////////////////////////////////////////////////////////
// circularBitShift
//         Bitshift and permutes bytes in 32 bit unsigned
//         int
//
//         Returns an encoded 32 bit unsigned int.
//         
////////////////////////////////////////////////////////

ui32 EdrIngest::circularBitShift (ui32 data, si32 shift)

{
  ui32 result;
  ui32 numbits = 32;
  if (shift > 0)
  {
     result = (data << shift) | (data >> (numbits - shift));
  }
  else
  {
    result = (data >> abs(shift)) | (data << (numbits - abs(shift)));
  }
  return result;
}


////////////////////////////////////////////////////////
// decodeBufrMsg
//         Method decodes enough of the BUFR EDR report
//         to determine the carrier type.        
////////////////////////////////////////////////////////

EdrReport::status_t EdrIngest::decodeBufrMsg(ui08 *buffer, DateTime msgTime)
{

  BUFR_Info_t bInfo;

  BUFR_Val_t bv;

  //
  // The following variable type is a status return from the BUFR functions
  //
  int ret = 0;

  PMU_auto_register("EdrIngest::decodeBufrMsg(): Decoding UAL EDR BUFR message");

  // Close any open BUFR files
  BUFR_Close();

  //
  // Initialize mel_bufr structures
  //
  if ( BUFR_Info_Init(&bInfo))
    {
      cerr << "EdrIngest::BUFR_Info_Init failure.\n";
      
      return EdrReport::BAD_DATA; 
    }
  
  if ( BUFR_Init ( &bInfo, (char*)buffer, DECODING ) )
    {
      cerr << "EdrIngest::decodeBufrMsg(): BUFR_Init failure for " << buffer << ".\n";
      
      return EdrReport::BAD_DATA;
      
    }

  time_t now = time(0);
  cerr << utimstr(now) << ":  Processing BUFR file: " << buffer << endl;

  if (_params.debug == Params::DEBUG_VERBOSE)
  	cerr << "EdrIngest::decodeBufrMsg(): Initialization complete\n" << endl; 

  
  char flight_id[Edr::FLIGHTNUM_NAME_LEN];
  char tailnum[16];
  char aircraft_registry_num[Edr::TAILNUM_NAME_LEN];
  
  while ( true )
    {  
      
      PMU_auto_register("EdrIngest::decodeBufrMsg(): Decoding UAL EDR BUFR message");

      //
      // The BUFR EDR message starts with the flight number
      // We will keep processing fields until we find the flight
      // number. Then we will assume it is an EDR message. If
      // we fail to find the proper sequence of fields 
      // we continue and then search for the flight number
      // again.
      //
      // Flight id (or flight number) is fxy: 0-01-006
      //

      ret = gotFlightNumber(flight_id, bv);
      if (ret)
	{
	  if ( ret == BUFR_EOM || ret == BUFR_EOF)
	    {
	      if (_params.debug >= Params::DEBUG_NORM)
		cerr << "EdrIngest::decodeBufrMsg(): Looking for  FlightNumber, got EOF, destroying BUFR, exiting\n";
	      BUFR_Destroy(1);
	      if (_params.debug >= Params::DEBUG_NORM)
		cerr << "EdrIngest::decodeBufrMsg(): End of processing " << buffer << ".\n";
	      return (EdrReport::END_OF_FILE);
	    }
	  else
	    continue;
	}

      //
      // Get the tail number
      //
      ret = getBufrVar(tailnum, (char *)"0-01-008",   bv);
      
      if (ret)
	{
	  if (_params.debug == Params::DEBUG_VERBOSE)
	    cerr << "EdrIngest::decodeBufrMsg: Unable to get the tail number.\n";
	  if ( ret == BUFR_EOM || ret == BUFR_EOF)
	    {
	      cerr << "EdrIngest::decodeEdrMsg(): Looking for tail number, got EOF, destroying BUFR, exiting\n";
	      BUFR_Destroy(1);  
	      cerr << "EdrIngest::decodeEdrMsg(): End of processing " << buffer << ".\n";
	      return (EdrReport::END_OF_FILE);
	    }
	  else
	    continue;
	}
            
      tailMap = new UalTailMap();
      sprintf(aircraft_registry_num, "%s", tailMap->lookup(tailnum));
      delete tailMap;

      // If we don't get a valid tailnumber, then this is not a
      // United EDR record

      if (strcmp(aircraft_registry_num,"") != 0)
	{

	// United message
	_report = new UnitedEdrReport (&_params, 
				      msgTime, 
				      flight_id,
				      aircraft_registry_num);
	  
          
	}
      else
	{

	  // Check Delta and Southwest tailmaps in future
	  // when we get them; If none, then continue, 
	  // because invalid tailnum

	  continue;
	}

      //
      // Initialize/clear data members
      //
      _report->clearAll();
      
      EdrReport::status_t ret_val;
  
      ret_val =_report->decodeBufr(bv);

      if (ret_val == EdrReport::END_OF_FILE)
	{
	  cerr << "EdrIngest::decodeBufrMsg(): End of processing " << buffer << ".\n";
	  delete _report;
	  return ret_val;
	}
      else if ((ret_val == EdrReport::BAD_DATA) || (ret_val == EdrReport::ALL_OK) || (ret_val == EdrReport::INCORRECT_SEQUENCE))
	{
	  delete _report;
	  continue;
	}

    } // end while


  // Don't think we ever get to this code !!!

  BUFR_Destroy(1);  

  cerr << "EdrIngest::decodeBufrMsg(): End of processing " << buffer << ".\n";

  return (EdrReport::ALL_OK);

}

int EdrIngest::gotFlightNumber(char* flight_id,  BUFR_Val_t &bv )
{
  //
  // Bufr variable is checked and recorded if found
  //
  int n;

  if ( (( n = BUFR_Get_Value(&bv, 1))  != BUFR_EOM ) && ( n != BUFR_EOF ))
    {
      char *c_fxy = FXY_String( bv.FXY_Val );  
      if (strcmp( c_fxy, "0-01-006" ))
	{
	  if ( _params.debug == Params::DEBUG_VERBOSE )
	    cerr << "EdrIngest::gotFlightNumber():  FXY_String is not flight number. fxy: "  << c_fxy << endl;
	  return 1;
	}
      else
	sprintf( flight_id, "%s", bv.Val.string );
    }

  return n;

}

int EdrIngest::getBufrVar(char *stringVar, char* fxy,  BUFR_Val_t &bv)
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
