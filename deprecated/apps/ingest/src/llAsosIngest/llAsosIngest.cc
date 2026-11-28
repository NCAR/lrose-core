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


#include <cstdio>
#include <toolsa/umisc.h>
#include <rapformats/station_reports.h>
#include <toolsa/pmu.h>
#include <physics/physics.h>
#include <ctype.h>

#include "llAsosIngest.hh"
using namespace std;


//
// Constructor.
//
llAsosIngest::llAsosIngest(Params *P){

  _socketTimeoutSecs = P->socketTimeoutSecs;;
  
  if (_loadLocations(P->st_location_path, P->altInFeet)) {
    fprintf(stderr,"Failed to load station locations from %s\n",
	    P->st_location_path);
    exit(-1);
  }

  _numInBuffer = 0;

  return;
}

//
// Destructor.
//

llAsosIngest::~llAsosIngest(){
  return;
}

void llAsosIngest::ProcFile(char *FilePath, Params *P){

  //
  // Open the read source.
  //
  if (_openReadSource(P, FilePath)){
    fprintf(stderr,"Data source - file or stream - unopenable.\n");
    return;
  }

  const int bufferSize = 256;
  unsigned char buffer[bufferSize];
  memset(buffer, 0, bufferSize);

  //
  // Go looking for valid callsigns in the data stream.
  //

  int count = 0;
  do {

    PMU_auto_register( "Waiting for data key");

    //
    // Shuffle the existing bytes and read another one, looking for
    // a valid 4-byte station ID and time in the first 11 bytes.
    //
     for (int i=0; i < 10; i++){
       buffer[i] = buffer[i+1];
     }
     if ( 0== _readFromSource(&buffer[10], 1)) break;
     count++;

     //
     // See if we have four valid ASCII characters that could be a station ID.
     //
     int callSignOk = 1;
     for (int k=0; k < 4; k++){
       if ((!(isdigit(buffer[k]))) && (!(isupper(buffer[k])))){
	 callSignOk = 0;
	 break;
       }
     }

     if ((P->KstationsOnly) && ((char)buffer[0] != 'K')) callSignOk = 0;

     if (callSignOk){
       //
       // We had the characters, try the time fields.
       //
       
       // buffer[6] should be year modulo 100
       if (buffer[6] > 99) callSignOk = 0;

       // buffer[7] should be month, 1-12
       if ((buffer[7] < 1) || (buffer[7] > 12)) callSignOk = 0;

       // buffer[8] - day 
       if ((buffer[8] < 1) || (buffer[8] > 31)) callSignOk = 0;

       // buffer[9] - hour 
       if (buffer[9] > 23) callSignOk = 0;

       // buffer[10] - minute 
       if (buffer[10] > 59) callSignOk = 0;

     }

     //
     // There is one more check
     // we can do, and it's really a good one - the data time
     // should be about now in realtime modes. Probably even worth
     // doing in archive mode.
     //
     if ((P->realtimeCheck.compareDataToCurrentTime) && (callSignOk)){
       time_t now = time(NULL);
       date_time_t dTime;
       dTime.sec = 0;
       dTime.min = buffer[10];
       dTime.hour = buffer[9];
       dTime.day = buffer[8];
       dTime.month = buffer[7];
       dTime.year = buffer[6] + 2000;
       uconvert_to_utime( &dTime );
       if (fabs((double)now - (double)dTime.unix_time) > P->realtimeCheck.maxDiffDays * 86400) 
	 callSignOk = 0;
     }

     if (callSignOk){
       if (P->debug){
	 fprintf(stderr, "\nStation key found after %d bytes : ", count);
	 for (int l=0; l<4; l++){
	   fprintf(stderr,"%c", (char)buffer[l]);
	 }
	 fprintf(stderr, " at %d/%02d/%02d %02d:%02d\n",
		 buffer[6] + 2000, buffer[7], buffer[8],
		 buffer[9], buffer[10]);
	 
       }
       count = 0;

       //
       // Have key, also have first 11 bytes in the buffer. Read the remaining
       // 47 bytes in.
       //
       int numRead = _readFromSource(buffer + 11, 47);
       if (numRead != 47) break;

       //
       // Decode and print.
       //
       if (P->debug) _printData(buffer);
       
       //
       // Decode to SPDB.
       //
       _writeSpdb(buffer, P);

     }

  } while(1);


  _closeReadSource();

  //
  // If we have anything left in the buffer, write it out.
  //
  if (_numInBuffer){
    _numInBuffer = 0;
    if ( _outSpdb.put( P->OutUrl, SPDB_STATION_REPORT_ID, SPDB_STATION_REPORT_LABEL)){
      fprintf(stderr, "Put error flushing to %s\n",
	      P->OutUrl);
    }
    _outSpdb.clearPutChunks(); // Probably not necessary
  }

  return;


}

////////////////////////////////////////////////////////////
//
// Write out SBDB data given the input stream of bytes.
//
void llAsosIngest::_writeSpdb(unsigned char *buffer, Params *P){

  //
  // The first four bytes are the ASCII station identifier - something
  // like KFTG - see if we can locate the lat/lon/alt. If not, return, there
  // is no point in decoding these.
  //
  char ID[5];
  ID[4] = char(0);
  memcpy(ID, buffer, 4);

  string stationName(ID);
  map< string, StationLoc, less<string> >::iterator iloc;
  iloc = _locations.find(stationName);
  if (iloc == _locations.end()) {
    if (P->printUnlistedStations) fprintf(stderr, "Unable to locate station %s\n", ID);
    return;
  }
  StationLoc &stationLoc = iloc->second;

    // should we check acceptedStations?

  if (P->useAcceptedStationsList) {
    bool accept = false;
    for (int ii = 0; ii < P->acceptedStations_n; ii++) {
      if (stationName == P->_acceptedStations[ii]) {
	accept = true;
	break;
      }
    }
    if (!accept) {
      if (P->debug) {
	cerr << endl;
	cerr << "Rejecting station: " << stationName << endl;
	cerr << "  Not in acceptedStations list" << endl;
      }
      return;
    }
  }

  // should we check rejectedStations?

  if (P->useRejectedStationsList) {
    bool reject = false;
    for (int ii = 0; ii < P->rejectedStations_n; ii++) {
      if (stationName == P->_rejectedStations[ii]) {
	reject = true;
	break;
      }
    }
    if (reject) {
      if (P->debug) {
	cerr << endl;
	cerr << "Rejecting station: " << stationName << endl;
	cerr << "  Station name is in rejectedStations list" << endl;
      }
      return;
    }
  }

  // should we check bounding box?

  if (P->checkBoundingBox) {
    if (stationLoc.lat < P->boundingBox.min_lat ||
	stationLoc.lat > P->boundingBox.max_lat ||
	stationLoc.lon < P->boundingBox.min_lon ||
	stationLoc.lon > P->boundingBox.max_lon) {
      if (P->debug) {
	cerr << endl;
	cerr << "Rejecting station: " << stationName << endl;
	cerr << "  Station position no within bounding box" << endl;
      }
      return;
    }
  }


  //
  // Declare a station_report_t, zero it out, and set fields to STATION_NAN
  // where appropriate.
  //
  station_report_t R;
  memset(&R, 0, sizeof(station_report_t));

  R.lat = stationLoc.lat;
  R.lon = stationLoc.lon;
  R.alt = stationLoc.alt;

  sprintf(R.station_label, "%s", ID);

  R.msg_id = STATION_REPORT;
  R.temp = STATION_NAN;
  R.dew_point = STATION_NAN;
  R.relhum = STATION_NAN;
  R.windspd = STATION_NAN;
  R.winddir = STATION_NAN;
  R.windgust = STATION_NAN;
  R.pres = STATION_NAN;
  R.liquid_accum = STATION_NAN;
  R.precip_rate = STATION_NAN;
  R.visibility = STATION_NAN;
  R.rvr = STATION_NAN;
  R.ceiling = STATION_NAN;
  R.shared.station.Spare1 = STATION_NAN;
  R.shared.station.Spare2 = STATION_NAN;
  R.shared.station.liquid_accum2 = STATION_NAN;

  //
  // Decode the data time.
  //
  date_time_t dataTime;
  dataTime.year = 2000 + buffer[6];
  dataTime.month =  buffer[7];
  dataTime.day =  buffer[8];
  dataTime.hour =  buffer[9];
  dataTime.min =  buffer[10];
  dataTime.sec = 0;

  uconvert_to_utime( &dataTime );

  R.time = dataTime.unix_time;

  //
  // Temp
  //
  bool tempsInC = false;
  if (_bitSet(buffer,5,1)) tempsInC = true;

  if (buffer[31] < 254){
    double temp = buffer[31] - 100.0;
    if (!(tempsInC)){
      temp = (temp - 32.0)*5.0/9.0;
    }
    R.temp = temp;
  }

  //
  // Dew point
  //
  if (buffer[32] < 254){
    double dpTemp = buffer[32] - 100.0;
    if (!(tempsInC)){
      dpTemp = (dpTemp - 32.0)*5.0/9.0;
    }
    R.dew_point = dpTemp;
  }

  //
  // If we have temp and DP, fill in RH.
  //
  if ((R.dew_point != STATION_NAN) && (R.temp != STATION_NAN)){
    R.relhum = PHYrelh(R.temp, R.dew_point);
  }


  //
  // Wind.
  //
  double windDir = 10.0*buffer[33];
  double windSpeed = buffer[35] * 0.5145; // Knots to m/s
  double windGust = buffer[36];
  
  if (windDir < 361.0){
    R.windspd = windSpeed;
    R.winddir = windDir;
    R.windgust = windGust;
  }

  //
  // Pressure.
  //
  if (buffer[40] < 254){
    double pres = (buffer[40]*256 + buffer[41])/10.0;
    pres *= 1013.0/29.92;
    R.pres = pres;
  }

  //
  // Runway Visual Range, Km
  //
  if (buffer[43] < 254){
    R.rvr =  100.0 * double(buffer[43]) * 0.0003; // Convert feet to Km
  }

  //
  // Ceiling. Take this off cloud height one, if set.
  //
  if ((buffer[15] > 0) && (buffer[15] < 254)){
    R.ceiling = 100.0 * double(buffer[15]) * 0.0003; // Convert feet to Km
  }

  //
  // liquid accum. Resets at top of hour.
  //
  if (buffer[25] < 254){
    double accum = 25.4*(buffer[25]*256 + buffer[26])/100.0; // 25.4 converts inches to mm
    date_time_t accumTime;
    accumTime.year = 2000 + buffer[6];
    accumTime.month =  buffer[7];
    accumTime.day =  buffer[8];
    accumTime.hour =  buffer[9];
    accumTime.min = 0;
    accumTime.sec = 0;
    uconvert_to_utime( &accumTime );
    R.liquid_accum = accum;
    R.accum_start_time = accumTime.unix_time;
  }

  //
  // Filter based on accumulation, if desired.
  //
  if (
      (P->saveAccumOption == Params::SAVE_VALID_ACCUM) &&
      (R.liquid_accum == STATION_NAN)
      ){
    return;
  }

  if (P->saveAccumOption == Params::SAVE_NONZERO_ACCUM){
    if (
	(R.liquid_accum == STATION_NAN) ||
	(R.liquid_accum == 0)
	){
      return;
    }
  }


  if (P->saveAccumOption == Params::SAVE_CHANGING_ACCUM){

    //
    // If we don't have nonzero accum, return.
    //
    if (
	(R.liquid_accum == STATION_NAN) ||
	(R.liquid_accum == 0)
	){
      return;
    }

    //
    // If we don't have any previous record from this station,
    // then save this record, and return.
    //
    bool havePrior = false;
    double lastAccum = 0.0;
    unsigned index = 0;

    for (unsigned ik=0; ik < _pastAccums.size(); ik++){
      if (0==strcmp(ID, _pastAccums[ik].ID)){
	havePrior = true;
	lastAccum = _pastAccums[ik].accum;
	index = ik;
	break;
      }
    }
    
    if (havePrior){
      //
      // We have a prior accum, if it is the same, return.
      // Otherwise we can proceed and save it out.
      //
      if (R.liquid_accum == lastAccum){
	if (P->debugAccum)
	  fprintf(stderr,"Station %s remains constant with %g - skipping.\n",	
		  ID, lastAccum);
	return;
      } else {
	//
	// We have a value and it has changed!
	// Delete the current entry in the _pastAccums vector, and update
	// it, then go on to save this value out (ie no 'return' statement).
	//
	_pastAccums.erase( _pastAccums.begin() + index );
	accumSave_t Ac;
	sprintf(Ac.ID, "%s", ID);
	Ac.accum = R.liquid_accum;
	_pastAccums.push_back( Ac );
	if (P->debugAccum)
	  fprintf(stderr,"Updating station %s to accum list with %g - will save data.\n",	
		  Ac.ID, Ac.accum);
      }
    } else {
      //
      // We have no prior record of this report.
      // Add this one, then return.
      //
      accumSave_t Ac;
      sprintf(Ac.ID, "%s", ID);
      Ac.accum = R.liquid_accum;
      _pastAccums.push_back( Ac );
      if (P->debugAccum)
	fprintf(stderr,"Adding station %s to accum list with %g - first time entry\n",
		Ac.ID, Ac.accum);
      return;
    }

  }


  // 
  // Visibility
  //
  if (buffer[21] < 254){
    double vis = 1.609 * (buffer[21]*256 + buffer[22])/100.0; // Convert statute miles to Km
    R.visibility = vis;
  }


  //
  // Weather type bitwise settings. This gets a little interpretive.
  //
  
  // Fog WT_FG
  if ( 
      (_bitSet(buffer, 24, 1)) ||
      (_bitSet(buffer, 24, 2)) ||
      (_bitSet(buffer, 24, 3)) ||
      (_bitSet(buffer, 25, 7))
      ){
    R.weather_type = R.weather_type | WT_FG;
  }

  if ((_bitSet(buffer,15,6)) || (_bitSet(buffer,15,7))) R.weather_type = R.weather_type | WT_FG;

  // Hail
  if ((_bitSet(buffer,13,0)) || (_bitSet(buffer,13,1)))  R.weather_type = R.weather_type | WT_GR;


  // Haze WT_HZ
  if (_bitSet(buffer, 24, 4)) R.weather_type = R.weather_type | WT_HZ;

  // Mist WT_BR
  if (_bitSet(buffer, 25, 3)) R.weather_type = R.weather_type | WT_BR;

  // Volcanic ash
  if (_bitSet(buffer, 25, 4)) R.weather_type = R.weather_type | WT_VA;

  // Freezing fog
  if (_bitSet(buffer, 47, 3)) R.weather_type = R.weather_type | WT_FZFG;

  // Light snow
  if (_bitSet(buffer, 46, 0)) R.weather_type = R.weather_type | WT_MSN;

  // Heavy snow
  if (_bitSet(buffer, 46, 1)) R.weather_type = R.weather_type | WT_PSN;

  // Non-specific precip
  if (_getPrecip(buffer[27], 0)) R.weather_type = R.weather_type | WT_UP;

  // Rain
  switch (_getPrecip(buffer[27], 1)){

  case 0 :
    break;
    
  case 2 : // light
    R.weather_type = R.weather_type | WT_MRA;
    break;

  case 4 : // heavy
    R.weather_type = R.weather_type | WT_PRA;
    break;

  default :
    R.weather_type = R.weather_type | WT_RA;
    break;
  }

  // Drizzle
  if (_getPrecip(buffer[28], 0)) R.weather_type = R.weather_type | WT_DZ;

  // Freezing rain 
  switch (_getPrecip(buffer[28], 1)){

  case 0 :
    break;

  case 2 : // light
    R.weather_type = R.weather_type | WT_MFZRA;
    break;
    
  case 4 : // heavy
    R.weather_type = R.weather_type | WT_PFZRA;
    break;

  default :
    R.weather_type = R.weather_type | WT_FZRA;
    break;
  }

  // Freezing drizzle 
  switch (_getPrecip(buffer[29], 0)){

  case 0 :
    break;
    
  case 2 : // light
    R.weather_type = R.weather_type | WT_MFZDZ;
    break;
    
  case 4 : // heavy
    R.weather_type = R.weather_type | WT_PFZDZ;
    break;

  default :
    R.weather_type = R.weather_type | WT_FZDZ;
    break;
  }
	  
  // Ice pellets
  if(_getPrecip(buffer[29], 1)) R.weather_type = R.weather_type | WT_PE;

  // Snow
  switch (_getPrecip(buffer[30], 0)){

  case 0 :
    break;
    
  case 2 : // light
    R.weather_type = R.weather_type | WT_MSN;
    break;
    
  case 4 : // heavy
    R.weather_type = R.weather_type | WT_PSN;
    break;

  default :
    R.weather_type = R.weather_type | WT_SN;
    break;
  }

  if ((_bitSet(buffer,15,0)) || (_bitSet(buffer,15,1))) R.weather_type = R.weather_type | WT_SN;


  //
  // OK, we're all set, add this to the buffer to write out to the URL.
  //
  station_report_to_be( &R );

  int dataType = Spdb::hash4CharsToInt32( ID );

  _outSpdb.addPutChunk(dataType, dataTime.unix_time,
	    dataTime.unix_time + P->expiry, sizeof(R), &R );

  //
  // See if we have enough in the buffer to do a write, if so, write.
  //
  _numInBuffer++;
  if (_numInBuffer >= P->numToBuffer){
    _numInBuffer = 0;
    if ( _outSpdb.put( P->OutUrl, SPDB_STATION_REPORT_ID, SPDB_STATION_REPORT_LABEL)){
      fprintf(stderr, "Put error writing to %s\n",
	      P->OutUrl);
    }
    _outSpdb.clearPutChunks(); // Probably not necessary
  }

  return;

}

////////////////////////////////////////////////////////////
//
//  Returns a 4-bit precip rate.
//
int llAsosIngest::_getPrecip(unsigned char byte, int higherBits){

  //
  // Pick the 4 bits we want out of this byte.
  //
  int fourBitVal;
  if (higherBits){
    fourBitVal = byte >> 4;
  } else {
    fourBitVal = byte & 0xf;
  }

  // Return this value - see _printPrecip for significance.

  if (fourBitVal == 14) fourBitVal = 0; // No sensor.
  if (fourBitVal == 15) fourBitVal = 0; // Missing data.

  return fourBitVal;

}




////////////////////////////////////////////////////////////
//
// Small routine to determine is a given bit of
// a given byte is set. You pass in :
// a pointer to the byte stream, 
// the byte number (starting at 1) and
// the bit number (range 0..7)
//
// returns 1 if set, else 0.
//
int llAsosIngest::_bitSet(unsigned char *bf, int byteNum, int bitNum){

  unsigned char andValue = 1 << bitNum;
  return bf[byteNum-1] & andValue;

}


////////////////////////////////////////////////////////////
//
//
// Small routine to print precip intensity information.
//
void llAsosIngest::_printPrecip(unsigned char byte, int higherBits){

  //
  // Pick the 4 bits we want out of this byte.
  //
  int fourBitVal;
  if (higherBits){
    fourBitVal = byte >> 4;
  } else {
    fourBitVal = byte & 0xf;
  }

  switch ( fourBitVal ) {

  case 0 :
    fprintf(stderr, "No precip\n");
    break;

  case 1 :
    fprintf(stderr, "No precip intensity information\n");
    break;

  case 2 :
    fprintf(stderr, "Light\n");
    break;

  case 3 :
    fprintf(stderr, "Moderate\n");
    break;

  case 4 :
    fprintf(stderr, "Heavy\n");
    break;

  case 5 :
    fprintf(stderr, "Showers - light\n");
    break;

  case 6 :
    fprintf(stderr, "Showers - moderate\n");
    break;

  case 7 :
    fprintf(stderr, "Showers - heavy\n");
    break;

  case 8 :
    fprintf(stderr, "Showers in vicinity\n");
    break;

  case 9 :
    fprintf(stderr, "Low drifting\n");
    break;

  case 10 :
    fprintf(stderr, "Blowing\n");
    break;

  case 11 :
    fprintf(stderr, "Blowing in vicinity\n");
    break;

  case 14 :
    fprintf(stderr, "Sensor not installed\n");
    break;

  case 15 :
    fprintf(stderr, "Missing data\n");
    break;

  default :
    fprintf(stderr,"Unknown code (%d).\n", fourBitVal);
    break;

  }
  return;
}

////////////////////////////////////////////////////////////
//
//
// Routine to print to stderr. You pass in the binary surface station
// message, and it decodes the stream and prints to stderr. Does not
// write to SPDB - but does demonstrate a very complete decoding of
// the data, decoding some fields that cannot go into the SPDB structure.
//

void llAsosIngest::_printData(unsigned char *bf){

  //
  // The byte stream is described in the documentation
  // as a set of octets. Roughly, octets are bytes, and
  // Octet 1== bf[0], Octet 2 == bf[1] etc.
  //

  //
  // Station ID. Looks like you have to look up the lat/lon from this.
  // These are octets 1-4, so for us, i=0 to 3
  //  
  fprintf(stderr,"Station ID : ");
  for (int i=0; i < 4; i++){
    fprintf(stderr,"%c", (char)bf[i]);
  }
  fprintf(stderr,"\n");
  

  //
  // Determine if temps are in C or F. Octet 5 bit 1 set implies C, else F.
  //
  bool tempsInC = false;
  if (_bitSet(bf,5,1)) tempsInC = true;

  if (tempsInC){
    fprintf(stderr,"temps were recorded in C\n");
  } else {
    fprintf(stderr,"temps were recorded in F\n");
  }


  //
  // Data time, octets 7-11
  //
  fprintf(stderr,"Time : ");
  fprintf(stderr,"%d ", (int)bf[6] + 2000);
  fprintf(stderr,"%d ", (int)bf[7]);
  fprintf(stderr,"%d ", (int)bf[8]);
  fprintf(stderr,"%d ", (int)bf[9]);
  fprintf(stderr,"%d ", (int)bf[10]);

  fprintf(stderr,"\n");

  //
  // Weather conditions.
  //

  // Octet 13 - bit 0 => large hail "GR"  bit 1 => small hail "GS"
  if (_bitSet(bf,13,0)) fprintf(stderr, "GR - Large hail\n");
  if (_bitSet(bf,13,1)) fprintf(stderr, "GS - Large hail\n");


  // Octet 15 Bit 0 or bit 1 => snow (documentation alittle unclear to me)
  if ((_bitSet(bf,15,0)) || (_bitSet(bf,15,1))) fprintf(stderr, "SG - Snow\n");

  // Octet 15 bit 6 or 7 => Fog - again, a little unclear.
  if ((_bitSet(bf,15,6)) || (_bitSet(bf,15,7))) fprintf(stderr, "FG - Fog\n");


  // Octets 16, 18 and 20 are byte value cloud heights in
  // hundreds of feet. Missing values are indicated by setting both bytes to 254 or 255.
  if (bf[15] > 253){
    fprintf(stderr, "Cloud height one   : Missing\n");
  } else {
    fprintf(stderr, "Cloud height one   : %d feet\n", 100 * bf[15]);
  }

  if (bf[17] > 253){
    fprintf(stderr, "Cloud height two   : Missing\n");
  } else {
    fprintf(stderr, "Cloud height two   : %d feet\n", 100 * bf[17]);
  }

  if (bf[19] > 253){
    fprintf(stderr, "Cloud height three : Missing\n");
  } else {
    fprintf(stderr, "Cloud height three : %d feet\n", 100 * bf[19]);
  }

  // Octets 17, 19 and 21 are cloud amounts.
  // bit 0 = scattered   bit 1 = broken  bit 2 = overcast  bit 3 = obscured
  // bit 5 = indefinite ceiling  bit 6 = no cloud detected  bit 7 = few

  fprintf(stderr, "Cloud amount one   : ");
  if (bf[16] == 255){
    fprintf(stderr,"Missing");
  } else {
    if (_bitSet(bf, 17, 0)) fprintf(stderr,"scattered ");
    if (_bitSet(bf, 17, 1)) fprintf(stderr,"broken ");
    if (_bitSet(bf, 17, 2)) fprintf(stderr,"overcast ");
    if (_bitSet(bf, 17, 3)) fprintf(stderr,"obscured ");
    if (_bitSet(bf, 17, 5)) fprintf(stderr,"indefinite ceiling ");
    if (_bitSet(bf, 17, 6)) fprintf(stderr,"no clouds detected ");
    if (_bitSet(bf, 17, 7)) fprintf(stderr,"few ");
  }
  fprintf(stderr,"\n");

  fprintf(stderr, "Cloud amount two   : ");
  if (bf[18] == 255){
    fprintf(stderr,"Missing");
  } else {
    if (_bitSet(bf, 19, 0)) fprintf(stderr,"scattered ");
    if (_bitSet(bf, 19, 1)) fprintf(stderr,"broken ");
    if (_bitSet(bf, 19, 2)) fprintf(stderr,"overcast ");
    if (_bitSet(bf, 19, 3)) fprintf(stderr,"obscured ");
    if (_bitSet(bf, 19, 5)) fprintf(stderr,"indefinite ceiling ");
    if (_bitSet(bf, 19, 6)) fprintf(stderr,"no clouds detected ");
    if (_bitSet(bf, 19, 7)) fprintf(stderr,"few ");
  }
  fprintf(stderr,"\n");
  fprintf(stderr, "Cloud amount three : ");
  if (bf[20] == 255){
    fprintf(stderr,"Missing");
  } else {
    if (_bitSet(bf, 21, 0)) fprintf(stderr,"scattered ");
    if (_bitSet(bf, 21, 1)) fprintf(stderr,"broken ");
    if (_bitSet(bf, 21, 2)) fprintf(stderr,"overcast ");
    if (_bitSet(bf, 21, 3)) fprintf(stderr,"obscured ");
    if (_bitSet(bf, 21, 5)) fprintf(stderr,"indefinite ceiling ");
    if (_bitSet(bf, 21, 6)) fprintf(stderr,"no clouds detected ");
    if (_bitSet(bf, 21, 7)) fprintf(stderr,"few ");
  }
  fprintf(stderr,"\n");

  //
  // Octets 22 and 23 - horizontal visibility in hundredth of statute miles.
  //
  if (bf[21] < 254){
    double vis = (bf[21]*256 + bf[22])/100.0;
    fprintf(stderr, "Horizontal visibility : %g statute miles\n", vis);
  } else {
    fprintf(stderr, "Horizontal visibility : Missing\n");
  } 


  //
  // Octets 24 and 25 - Obscurations.
  //
  if (_bitSet(bf, 24, 0)) fprintf(stderr,"Non-specific obscuration\n");
  if (_bitSet(bf, 24, 1)) fprintf(stderr,"FG - Fog - obscuration\n");
  if (_bitSet(bf, 24, 2)) fprintf(stderr,"MDFG - moderate fog- obscuration\n");
  if (_bitSet(bf, 24, 3)) fprintf(stderr,"PRFG - partial fog - obscuration\n");
  if (_bitSet(bf, 24, 4)) fprintf(stderr,"HZ - haze - obscuration\n");
  if (_bitSet(bf, 24, 5)) fprintf(stderr,"FU - smoke - obscuration\n");
  if (_bitSet(bf, 24, 6)) fprintf(stderr,"DRDU - drifiting dust - obscuration\n");
  if (_bitSet(bf, 24, 7)) fprintf(stderr,"DRSA - drifitng sand - obscuration\n");

  if (_bitSet(bf, 25, 0)) fprintf(stderr,"BLSA - blowing sand - obscuration\n");
  if (_bitSet(bf, 25, 1)) fprintf(stderr,"BLDU - blowing dust - obscuration\n");
  if (_bitSet(bf, 25, 2)) fprintf(stderr,"BLDY - blowing spray - obscuration\n");
  if (_bitSet(bf, 25, 3)) fprintf(stderr,"BR - mist - obscuration\n");
  if (_bitSet(bf, 25, 4)) fprintf(stderr,"VA - volcanic ash - obscuration\n");
  if (_bitSet(bf, 25, 5)) fprintf(stderr,"PO - sand/dust whirls - obscuration\n");
  if (_bitSet(bf, 25, 6)) fprintf(stderr,"VCPO - sand/dust whirls - obscuration\n");
  if (_bitSet(bf, 25, 7)) fprintf(stderr,"BCFG - patchy fog - obscuration\n");

  //
  // Jump around a little bit here - more obcurations in octets 46 and 47
  //
  if (_bitSet(bf, 46, 0)) fprintf(stderr,"Light snow grains - obscuration\n");
  if (_bitSet(bf, 46, 1)) fprintf(stderr,"Heavy snow grains - obscuration\n");
  if (_bitSet(bf, 46, 2)) fprintf(stderr,"Sandstorm - obscuration\n");
  if (_bitSet(bf, 46, 3)) fprintf(stderr,"Heavy sandstorm - obscuration\n");
  if (_bitSet(bf, 46, 4)) fprintf(stderr,"Sandstorm vicinity - obscuration\n");
  if (_bitSet(bf, 46, 5)) fprintf(stderr,"Duststorm - obscuration\n");
  if (_bitSet(bf, 46, 6)) fprintf(stderr,"Heavy duststorm - obscuration\n");
  if (_bitSet(bf, 46, 7)) fprintf(stderr,"Duststorm vicinity - obscuration\n");


  if (_bitSet(bf, 47, 0)) fprintf(stderr,"Blowing sand - obscuration\n");
  if (_bitSet(bf, 47, 1)) fprintf(stderr,"Blowing dust - obscuration\n");
  if (_bitSet(bf, 47, 2)) fprintf(stderr,"Fog vicinity - obscuration\n");
  if (_bitSet(bf, 47, 3)) fprintf(stderr,"Freezing fog - obscuration\n");
  if (_bitSet(bf, 47, 4)) fprintf(stderr,"Squall - obscuration\n");



  //
  // Octets 26 and 27 - accumulated precip, binary, hundreths of an inch.
  // For ASOS these reset at the start of the hour.
  //
  if (bf[25] < 254){
    double accum = (bf[25]*256 + bf[26])/100.0;
    fprintf(stderr, "Accumulated precip : %g inches\n", accum);
  } else {
    fprintf(stderr, "Accumulated precip : Missing\n");
  } 

  //
  // Octets 28 through 31 are precip information,
  // packed as 4 bit numbers.
  //
  fprintf(stderr,"Non-specific precip : ");
  _printPrecip(bf[27], 0);

  fprintf(stderr,"Rain : ");
  _printPrecip(bf[27], 1);

  fprintf(stderr,"Drizzle : ");
  _printPrecip(bf[28], 0);

  fprintf(stderr,"Freezing rain : ");
  _printPrecip(bf[28], 1);

  fprintf(stderr,"Freezing drizzle : ");
  _printPrecip(bf[29], 0);

  fprintf(stderr,"Ice pellets : ");
  _printPrecip(bf[29], 1);

  fprintf(stderr,"Snow : ");
  _printPrecip(bf[30], 0);

  // Octet 32 - ambient temperature plus 100. We know by now if
  // this is Celcuis or Fahrenheit

  if (bf[31] < 254){
    double temp = bf[31] - 100.0;
    if (!(tempsInC)){
      temp = (temp - 32.0)*5.0/9.0;
    }
    fprintf(stderr,"Temperature : %g C\n", temp);
  } else {
    fprintf(stderr,"Temperature : Missing\n");
  }

  // Similarly, Octet 33 - dew point temperature.



  if (bf[32] < 254){
    double dpTemp = bf[32] - 100.0;
    if (!(tempsInC)){
      dpTemp = (dpTemp - 32.0)*5.0/9.0;
    }
    fprintf(stderr,"Dew point : %g C\n", dpTemp);
  } else {
    fprintf(stderr,"Dew point : Missing\n");
  }
  
  //
  // Wind dir, speed, gust, knots.
  //
  double windDir = 10.0*bf[33];
  double windSpeed = bf[35];
  double windGust = bf[36];
  
  if (windDir < 361.0){
    fprintf(stderr,"Wind Speed, Gust, Dir : %g Knots %g Knots %g Degrees\n",
            windSpeed, windGust, windDir);
  } else {
    fprintf(stderr, "Missing wind values.\n");
  }
  

  // Octets 38 and 39 - altimeter setting, hundredths of inches of Hg

  if (bf[37] > 253){
    fprintf(stderr, "Altimeter setting : Missing\n");
  } else {
    double altSet = (bf[37]*256 + bf[38])/100.0;
    altSet *= 1013.0/29.92;
    fprintf(stderr, "Altimeter setting : %g\n", altSet);
  }

  // Octet 40 - density altitude, hundreds of feet

  if (bf[39] > 253){
    fprintf(stderr, "Density altitude : Missing\n");
  } else {
    double Dalt = bf[39]*100.0;
    fprintf(stderr, "Density altitude : %g feet\n", Dalt);
  }


  // Octets 41 and 42 - Sea level pressure. Documentation says this is in hundreths of mb.
  // That gives silly values. I'm guessing it is in tenths of inches of Hg.

  if (bf[40] > 253){
    fprintf(stderr, "Pressure : Missing\n");
  } else {
    double pres = (bf[40]*256 + bf[41])/10.0;
    pres *= 1013.0/29.92;
    fprintf(stderr, "Pressure : %g\n", pres);
  }

  // Octet 43 - runway ID, tens of degrees.
  if (bf[42] < 254){
    fprintf(stderr,"Runway ID : %d degrees\n", 10*bf[42]);
  } else {
    fprintf(stderr,"Runway ID : Missing\n");
  }
  
  // Octet 44 - runway visual range, hundreds of feet.
  if (bf[43] < 254){
    fprintf(stderr,"Runway visual range : %d feet ", 100*bf[43]);
    //
    // If we have RVR then Octet 45 has some info about it in the high
    // order bits.
    //
    int fourBitVal = bf[44] >> 4;
    switch ( fourBitVal ){

    case 0 :
      fprintf(stderr,"between lowest and highest reportable value.\n");
      break;

    case 1 :
      fprintf(stderr,"lowest reportable value.\n");
      break;

    case 2 :
      fprintf(stderr,"highest reportable value.\n");
      break;

    default :
      fprintf(stderr, "Error in RVR significance code!\n"); // This does happen!
      break;
    }
  } else {
    fprintf(stderr,"Runway visual range : Missing\n");
  }
  

  // Octet 57 bit 1 - if this bit is 1 then ltg information is not avalable.

  if (_bitSet(bf,57,1)){
    fprintf(stderr,"Ltg information : not avalable.\n");
  } else {
    fprintf(stderr,"Ltg information : avalable.\n");

    if (_bitSet(bf,57,6)) fprintf(stderr,"Ltg at airport!\n");
    if (_bitSet(bf,57,7)) fprintf(stderr,"Ltg in vicinity!\n");

    //
    // Sometimes one of the sector messages (below) will appear
    // without either "Ltg in vicinity" or "Ltg at airport"
    // being set. I'm not sure how much credibility that carries.
    //
    if (_bitSet(bf,58,0)) fprintf(stderr,"Ltg in sector : N\n");
    if (_bitSet(bf,58,1)) fprintf(stderr,"Ltg in sector : NE\n");
    if (_bitSet(bf,58,2)) fprintf(stderr,"Ltg in sector : E\n");
    if (_bitSet(bf,58,3)) fprintf(stderr,"Ltg in sector : SE\n");
    if (_bitSet(bf,58,4)) fprintf(stderr,"Ltg in sector : S\n");
    if (_bitSet(bf,58,5)) fprintf(stderr,"Ltg in sector : SW\n");
    if (_bitSet(bf,58,6)) fprintf(stderr,"Ltg in sector : W\n");
    if (_bitSet(bf,58,7)) fprintf(stderr,"Ltg in sector : NW\n");

  }

  return;

}

////////////////////////////////////////////////////////////////
//
// Open read source, be it file or stream.
//
int llAsosIngest::_openReadSource(Params *P, char *FileName){
  //
  // Decide if we are reading from a file.
  //
  if (P->mode == Params::REALTIME_STREAM){
    _readingFromFile = false;
  } else {
    _readingFromFile = true;
  }
  //
  // And open the source appropriately.
  //
  int retVal = 0;
  if ( _readingFromFile ){
    //
    // Set up for file reads.
    //
    _fp = fopen(FileName,"rb");
    if (_fp == NULL){
      retVal = -1;
    }
  } else {
    retVal = _S.open(P->hostname, P->port);
  }

  if (retVal != 0){
    if ( _readingFromFile ){
      fprintf(stderr,"Failed to read from file %s\n",
	      FileName);
    } else {
      fprintf(stderr,"Attempt to open port %d at %s returned %d\n",
	      P->port, P->hostname, retVal);

      fprintf(stderr,"Error string : %s\n",_S.getErrString().c_str());

      switch(_S.getErrNum()){

      case Socket::UNKNOWN_HOST :
	fprintf(stderr,"Unknown host.\n");
	break;

      case Socket::SOCKET_FAILED :
	fprintf(stderr,"Could not set up socked (maxed out decriptors?).\n");
	break;
      
      case Socket::CONNECT_FAILED :
	fprintf(stderr,"Connect failed.\n");
	break;
 
      case Socket::TIMED_OUT :
	fprintf(stderr,"Timed out..\n");
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Unexpected error.\n");
	break;

      default :
	fprintf(stderr,"Unknown error.\n");
	break;

      }
    }
  }
  return retVal;
}

////////////////////////////////////////////////////////////////
//
// Close read source, be it file or stream.
//
void llAsosIngest::_closeReadSource(){
  if (_readingFromFile){
    fclose(_fp);
  } else {
    _S.close();
    _S.freeData();
  }
}

///////////////////////////////////////////////////
//
// Read N bytes from the input source into buffer.
// Size of buffer to read to not checked.
// Returns number of bytes read.
//
int llAsosIngest::_readFromSource(unsigned char *buffer, int numbytes){

  int retVal;
  if (_readingFromFile){
    retVal = fread(buffer, sizeof(unsigned char), numbytes, _fp);
  } else {

    if (_S.readSelectPmu()){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read select timed out.\n");
	exit(-1);
	break;

      case Socket::SELECT_FAILED :
	fprintf(stderr,"Read select failed.\n");
	exit(-1);
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Read select - unexpected error.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read select.\n");
	exit(-1);
	break;

      }
    }

    if (_S.readBufferHb(buffer,
			numbytes,
			numbytes,
			(Socket::heartbeat_t)PMU_auto_register,
			_socketTimeoutSecs*1000) != 0 ){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read buffer timed out.\n");
	exit(-1);
	break;

      case Socket::BAD_BYTE_COUNT :
	fprintf(stderr,"Read buffer gave bad byte count.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read buffer.\n");
	exit(-1);
	break;
      }

      return -1;
    }

    retVal = _S.getNumBytes();
  }

  return retVal;
}



////////////////////////////////
// load up the station locations. Shameless
// cut-and-paste from Metar2Spdb.

int llAsosIngest::_loadLocations(const char* station_location_path, 
				 int altInFeet)
{

  FILE *fp;
  char line[BUFSIZ];
  char station[128];
  double lat, lon, alt;
  
  string stationId;
  
  if((fp = fopen(station_location_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Metar2Spdb::loadLocations" << endl;
    cerr << "  Cannot open station location file: "
	 << station_location_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  int count = 0;

  while( fgets(line, BUFSIZ, fp) != NULL ) {

    // If the line is not a comment, process it
    
    if(
       (line[0] == '#' ) ||
       (line[0] == '!' ) ||
       (strlen(line) < 6)
       ){
      continue;
    }

    // Read in the line - try different formats
    
    if( sscanf(line, "%4s, %lg, %lg, %lg", 
	       station, &lat, &lon, &alt) != 4 ) {
      if( sscanf(line, "%4s,%lg,%lg,%lg", 
		 station, &lat, &lon, &alt) != 4 ) {
	if( sscanf(line, "%3s,%lg,%lg,%lg", 
		   station, &lat, &lon, &alt) != 4 ) {
	  fprintf(stderr, "Cannot read line from %s : %s\n",
		  station_location_path, line);
	  continue;
	}
      }
    }
    
    count++;
      
    // Convert station to a string
    
    stationId = station;
    
    // Convert altitude to meters

    if( alt == -999.0 ||
	alt == 999.0) {
      alt = STATION_NAN;
    } else {
      if (altInFeet) alt = alt * 0.3;
    }
    
    // Create new metar location and add it to the map

    pair<string, StationLoc> pr;
    pr.first = stationId;
    pr.second.set(stationId, lat, lon, alt);
    _locations.insert(_locations.begin(), pr);

  }

  fclose(fp);

  if (count == 0) {
    cerr << "ERROR - Metar2Spdb::loadLocations" << endl;
    cerr << "  No suitable locations in file: : "
	 << station_location_path << endl;
    return -1;
  }

  return 0;

}


