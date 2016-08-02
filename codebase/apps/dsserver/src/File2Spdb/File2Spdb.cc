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


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Spdb_typedefs.hh>



/*****

g++ -D_BSD_TYPES -DF_UNDERSCORE2 -g -c  -I/home/steves/tech/taiwan/sigwx2symprod/rapinstall/include -I/home/steves/tech/taiwan/sigwx2symprod/rapbase/include   file2Spdb.cc


g++ -g -o file2Spdb file2Spdb.o -L/home/steves/tech/taiwan/sigwx2symprod/rapinstall/lib -L/home/steves/tech/taiwan/sigwx2symprod/rapbase/lib   -lSpdb -ldsserver -ldidss -lrapformats -ltoolsa -ldataport -ltdrp -lpthread -lm


seq 1 10 > tempa
./file2Spdb -start '2008 04 01' -end '2008 04 02' -type 777 -putMode over -compress n -in tempa -out tempdir

SpdbQuery -url tempdir -start '2008 04 01 00 00 00' -end '2008 04 02 00 00 00'
*****/


//=========================================================================

class ProdSpec {
  public:
  char * name;
  int spdb_id;
  char * spdb_label;

  ProdSpec( char * name, int spdb_id, char * spdb_label) {
    this->name = name;
    this->spdb_id = spdb_id;
    this->spdb_label = spdb_label;
  }
}; // end class ProdSpec


//=========================================================================


ProdSpec * getProdSpec( char * name);

time_t convertToTime( const char * stg);

char * readFile(
  const char * fname,
  bool addNullFlag,
  int * pbuflen);

long long getSecondNum( int year, int month, int day,
  int hour, int minute, int second);

int parseInt( const char * stg, int offset, int limit);


//=========================================================================


void badparms( const char * msg, const char * parm) {
  if (parm != NULL) printf("\nError: %s  \"%s\"\n", msg, parm);
  else printf("\nError: %s\n", msg);

  printf("File2Spdb:\n");
  printf("Parms:\n");
  printf("  -start     <timeSpec>   UTC start time (see below)\n");
  printf("  -end       <timeSpec>   UTC end time (see below)\n");
  printf("  -product   <string>     spec SPDB product id (see below)\n");
  printf("  -dataType  <val>        spec SPDB dataType\n");
  printf("  -dataType2 vali>        spec SPDB dataType2\n");
  printf("  -addNull   y/n          if y, append \\0 to the file in SPDB\n");
  printf("  -compress  y/n          if y, data chunk will be compressed\n");
  printf("  -hash      y/n          if y, datatype and datatype2 will be hashed\n");
  printf("  -in        <string>     input file\n");
  printf("  -out       <string>     output url\n");
  printf("\n");
  printf("The product may be one of:\n");
  printf("  ACARS  AC_DATA  AC_POSN  AC_POSN_WMOD\n");
  printf("  AC_ROUTE  AC_VECTOR  ALERT_META  ASCII\n");
  printf("  AUDIO  BDRY  COMBO_POINT  CONTOUR\n");
  printf("  DS_RADAR_POWER  DS_RADAR_SWEEP  EDR_POINT  FLT_PATH\n");
  printf("  FLT_ROUTE  GENERIC_POINT  GENERIC_POLYLINE  HYDRO_STATION\n");
  printf("  IRSA_FORECAST  KAV_LTG  LTG  MAD\n");
  printf("  NWS_WWA  PIREP  POSN_RPT  RAW_METAR\n");
  printf("  SIGAIRMET  SIGMET  SNDG  SNDG_PLUS\n");
  printf("  STATION_REPORT  STATION_REPORT_ARRAY  SYMPROD  TREC_GAUGE\n");
  printf("  TREC_PT_FORECAST  TSTORMS  TWN_LTG  USGS\n");
  printf("  VERGRID_REGION\n");
  printf("  WAFS_SIGWX\n");
  printf("  WAFS_SIGWX_CLOUD\n");
  printf("  WAFS_SIGWX_JETSTREAM\n");
  printf("  WAFS_SIGWX_TURBULENCE\n");
  printf("  WAFS_SIGWX_VOLCANO\n");
  printf("  WX_HAZARDS  XML  ZRPF  ZR_PARAMS\n");
  printf("  ZVIS_CAL  ZVIS_FCAST  ZVPF\n");
  printf("\n");
  printf("If dataType and DataType2 are to be hashed the character length is 4\n");
  printf("\n");
  printf("For more info on product type codes, see:\n");
  printf("   cvs/libs/Spdb/src/include/Spdb/Product_defines.hh\n");
  printf("\n");
  printf("Times may be specified in any of the formats:\n");
  printf("  \"yyyy mm dd\"             (sets HH=0, MM=0, SS=0)\n");
  printf("  \"yyyy mm dd HH MM\"       (sets SS=0)\n");
  printf("  \"yyyy mm dd HH MM SS\"\n");
  exit(1);
}



//=========================================================================



int main( int argc, char *argv[]) {
  int irc;

  char * startStg = NULL;
  char * endStg = NULL;
  char * product = NULL;
  char * dataTypeStg = NULL;
  char * dataType2Stg = NULL;
  char * putMode = NULL;
  char * compressStg = NULL;
  char * hashStg = NULL;
  char * inFile = NULL;
  char * outUrl = NULL;

  if (argc % 2 != 1) badparms("parms must be pairs:  -key value", NULL);
  for (int ii = 1; ii < argc; ii += 2) {    // skip command name
    char * key = argv[ii];
    char * val = argv[ii+1];
    if (0 == strcmp( key, "-start")) startStg = val;
    else if (0 == strcmp( key, "-end")) endStg = val;
    else if (0 == strcmp( key, "-product")) product = val;
    else if (0 == strcmp( key, "-dataType")) dataTypeStg = val;
    else if (0 == strcmp( key, "-dataType2")) dataType2Stg = val;
    else if (0 == strcmp( key, "-putMode")) putMode = val;
    else if (0 == strcmp( key, "-compress")) compressStg = val;
    else if (0 == strcmp( key, "-hash")) hashStg = val;
    else if (0 == strcmp( key, "-in")) inFile = val;
    else if (0 == strcmp( key, "-out")) outUrl = val;
    else badparms("unknown keyword:", key);
  }
  if (startStg == NULL) badparms("-start not specified", NULL);
  if (endStg == NULL) badparms("-end not specified", NULL);
  if (product == NULL) badparms("-product not specified", NULL);
  if (dataTypeStg == NULL) badparms("-dataType not specified", NULL);
  if (putMode == NULL) badparms("-putMode not specified", NULL);
  if (compressStg == NULL) badparms("-compress not specified", NULL);
  if (hashStg == NULL) badparms("-hash not specified", NULL);
  if (inFile == NULL) badparms("-in not specified", NULL);
  if (outUrl == NULL) badparms("-out not specified", NULL);

  printf("File2Spdb:\n");
  printf("  start:     \"%s\"\n", startStg);
  printf("  end:       \"%s\"\n", endStg);
  printf("  product:   \"%s\"\n", product);
  printf("  dataType:  \"%s\"\n", dataTypeStg);
  printf("  dataType2: \"%s\"\n", dataType2Stg);
  printf("  putMode:   \"%s\"\n", putMode);
  printf("  compress:  \"%s\"\n", compressStg);
  printf("  hash:      \"%s\"\n", hashStg);
  printf("  in:        \"%s\"\n", inFile);
  printf("  outUrl:    \"%s\"\n", outUrl);

  time_t startTime = convertToTime( startStg);  
  time_t endTime = convertToTime( endStg);

  ProdSpec * prodSpec = getProdSpec( product);
  printf("File2Spdb:\n");
  printf("  prodSpec: name:       %s\n", prodSpec->name);
  printf("  prodSpec: spdb_id:    %d\n", prodSpec->spdb_id);
  printf("  prodSpec: spdb_label: %s\n", prodSpec->spdb_label);

  bool compressFlag;
  if (0 == strcmp( compressStg, "n")) compressFlag = false;
  else if (0 == strcmp( compressStg, "y")) compressFlag = true;
  else badparms("invalid -compress:", compressStg);

  int dataType = 0;
  int dataType2 = 0;
  if (0 == strcmp(hashStg, "y")) {
    dataType = Spdb::hash4CharsToInt32(dataTypeStg);
    if (0 != dataType2Stg) { 
      dataType2 = Spdb::hash4CharsToInt32(dataType2Stg);
    }
  }
  else {
    dataType = parseInt( dataTypeStg, 0, strlen( dataTypeStg));
    if (0 != dataType2Stg) { 
      dataType2 = parseInt( dataType2Stg, 0, strlen( dataType2Stg));
    }
  }

  int chunkLen;
  char * fileContent = readFile( inFile, false, &chunkLen);
  printf("File2Spdb:\n");
  printf("  chunkLen: %d\n", chunkLen);
  printf("  product id: %d\n", prodSpec->spdb_id);
  printf("  product label: \"%s\"\n", prodSpec->spdb_label);

  // See:
  //   libs/Spdb/src/include/Spdb/Spdb.hh
  //   libs/Spdb/src/Spdb/Spdb.cc

  DsSpdb spdb;
  spdb.setDebug( true);

  // See:
  //   libs/Spdb/src/include/Spdb/Spdb_typedefs.hh
  if (0 == strcmp(putMode, "over")) {}
  else if (0 == strcmp(putMode, "add"))
    spdb.setPutMode( Spdb::putModeAdd);
  else badparms("invalid putMode", putMode);



  spdb.clearPutChunks();

  spdb.addPutChunk(
    dataType,
    startTime,
    endTime,
    chunkLen,
    fileContent,
    dataType2);
  
  irc = spdb.put(
    outUrl,
    prodSpec->spdb_id,        // e.g., SPDB_WAFS_SIGWX_ID
    prodSpec->spdb_label);    // e.g., SPDB_WAFS_SIGWX_LABEL
  if (irc != 0)
    badparms("spdb put error:", spdb.getErrStr().c_str());

  free( fileContent);
} // end main



//=========================================================================



ProdSpec * getProdSpec( char * name) {

  ProdSpec * prodSpecs[] = {
    new ProdSpec( const_cast<char*>(string("ASCII").c_str()), SPDB_ASCII_ID, const_cast<char*>(string(SPDB_ASCII_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AUDIO").c_str()), SPDB_AUDIO_ID, const_cast<char*>(string(SPDB_AUDIO_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("XML").c_str()), SPDB_XML_ID, const_cast<char*>(string(SPDB_XML_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("GENERIC_POINT").c_str()), SPDB_GENERIC_POINT_ID, const_cast<char*>(string(SPDB_GENERIC_POINT_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("COMBO_POINT").c_str()), SPDB_COMBO_POINT_ID, const_cast<char*>(string(SPDB_COMBO_POINT_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("FLT_PATH").c_str()), SPDB_FLT_PATH_ID, const_cast<char*>(string(SPDB_FLT_PATH_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("GENERIC_POLYLINE").c_str()), SPDB_GENERIC_POLYLINE_ID, const_cast<char*>(string(SPDB_GENERIC_POLYLINE_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("TREC_PT_FORECAST").c_str()), SPDB_TREC_PT_FORECAST_ID, const_cast<char*>(string(SPDB_TREC_PT_FORECAST_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("STATION_REPORT").c_str()), SPDB_STATION_REPORT_ID, const_cast<char*>(string(SPDB_STATION_REPORT_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("IRSA_FORECAST").c_str()), SPDB_IRSA_FORECAST_ID, const_cast<char*>(string(SPDB_IRSA_FORECAST_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("STATION_REPORT_ARRAY").c_str()), SPDB_STATION_REPORT_ARRAY_ID, const_cast<char*>(string(SPDB_STATION_REPORT_ARRAY_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("RAW_METAR").c_str()), SPDB_RAW_METAR_ID, const_cast<char*>(string(SPDB_RAW_METAR_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WAFS_SIGWX").c_str()), SPDB_WAFS_SIGWX_ID, const_cast<char*>(string(SPDB_WAFS_SIGWX_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WAFS_SIGWX_CLOUD").c_str()), SPDB_WAFS_SIGWX_CLOUD_ID, const_cast<char*>(string(SPDB_WAFS_SIGWX_CLOUD_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WAFS_SIGWX_JETSTREAM").c_str()), SPDB_WAFS_SIGWX_JETSTREAM_ID, const_cast<char*>(string(SPDB_WAFS_SIGWX_JETSTREAM_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WAFS_SIGWX_TURBULENCE").c_str()), SPDB_WAFS_SIGWX_TURBULENCE_ID, const_cast<char*>(string(SPDB_WAFS_SIGWX_TURBULENCE_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WAFS_SIGWX_VOLCANO").c_str()), SPDB_WAFS_SIGWX_VOLCANO_ID, const_cast<char*>(string(SPDB_WAFS_SIGWX_VOLCANO_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("KAV_LTG").c_str()), SPDB_KAV_LTG_ID, const_cast<char*>(string(SPDB_KAV_LTG_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("LTG").c_str()), SPDB_LTG_ID, const_cast<char*>(string(SPDB_LTG_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("TWN_LTG").c_str()), SPDB_TWN_LTG_ID, const_cast<char*>(string(SPDB_TWN_LTG_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("SIGMET").c_str()), SPDB_SIGMET_ID, const_cast<char*>(string(SPDB_SIGMET_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("CONTOUR").c_str()), SPDB_CONTOUR_ID, const_cast<char*>(string(SPDB_CONTOUR_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("SIGAIRMET").c_str()), SPDB_SIGAIRMET_ID, const_cast<char*>(string(SPDB_SIGAIRMET_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("BDRY").c_str()), SPDB_BDRY_ID, const_cast<char*>(string(SPDB_BDRY_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("SNDG").c_str()), SPDB_SNDG_ID, const_cast<char*>(string(SPDB_SNDG_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("SNDG_PLUS").c_str()), SPDB_SNDG_PLUS_ID, const_cast<char*>(string(SPDB_SNDG_PLUS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ALERT_META").c_str()), SPDB_ALERT_META_ID, const_cast<char*>(string(SPDB_ALERT_META_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("HYDRO_STATION").c_str()), SPDB_HYDRO_STATION_ID, const_cast<char*>(string(SPDB_HYDRO_STATION_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("EDR_POINT").c_str()), SPDB_EDR_POINT_ID, const_cast<char*>(string(SPDB_EDR_POINT_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("SYMPROD").c_str()), SPDB_SYMPROD_ID, const_cast<char*>(string(SPDB_SYMPROD_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("WX_HAZARDS").c_str()), SPDB_WX_HAZARDS_ID, const_cast<char*>(string(SPDB_WX_HAZARDS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("FLT_ROUTE").c_str()), SPDB_FLT_ROUTE_ID, const_cast<char*>(string(SPDB_FLT_ROUTE_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("POSN_RPT").c_str()), SPDB_POSN_RPT_ID, const_cast<char*>(string(SPDB_POSN_RPT_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AC_POSN").c_str()), SPDB_AC_POSN_ID, const_cast<char*>(string(SPDB_AC_POSN_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AC_DATA").c_str()), SPDB_AC_DATA_ID, const_cast<char*>(string(SPDB_AC_DATA_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AC_POSN_WMOD").c_str()), SPDB_AC_POSN_WMOD_ID, const_cast<char*>(string(SPDB_AC_POSN_WMOD_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AC_ROUTE").c_str()), SPDB_AC_ROUTE_ID, const_cast<char*>(string(SPDB_AC_ROUTE_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("PIREP").c_str()), SPDB_PIREP_ID, const_cast<char*>(string(SPDB_PIREP_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ACARS").c_str()), SPDB_ACARS_ID, const_cast<char*>(string(SPDB_ACARS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("TSTORMS").c_str()), SPDB_TSTORMS_ID, const_cast<char*>(string(SPDB_TSTORMS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("USGS").c_str()), SPDB_USGS_ID, const_cast<char*>(string(SPDB_USGS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("MAD").c_str()), SPDB_MAD_ID, const_cast<char*>(string(SPDB_MAD_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("TREC_GAUGE").c_str()), SPDB_TREC_GAUGE_ID, const_cast<char*>(string(SPDB_TREC_GAUGE_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ZR_PARAMS").c_str()), SPDB_ZR_PARAMS_ID, const_cast<char*>(string(SPDB_ZR_PARAMS_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ZRPF").c_str()), SPDB_ZRPF_ID, const_cast<char*>(string(SPDB_ZRPF_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ZVPF").c_str()), SPDB_ZVPF_ID, const_cast<char*>(string(SPDB_ZVPF_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ZVIS_CAL").c_str()), SPDB_ZVIS_CAL_ID, const_cast<char*>(string(SPDB_ZVIS_CAL_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("ZVIS_FCAST").c_str()), SPDB_ZVIS_FCAST_ID, const_cast<char*>(string(SPDB_ZVIS_FCAST_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("VERGRID_REGION").c_str()), SPDB_VERGRID_REGION_ID, const_cast<char*>(string(SPDB_VERGRID_REGION_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("AC_VECTOR").c_str()), SPDB_AC_VECTOR_ID, const_cast<char*>(string(SPDB_AC_VECTOR_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("DS_RADAR_SWEEP").c_str()), SPDB_DS_RADAR_SWEEP_ID, const_cast<char*>(string(SPDB_DS_RADAR_SWEEP_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("DS_RADAR_POWER").c_str()), SPDB_DS_RADAR_POWER_ID, const_cast<char*>(string(SPDB_DS_RADAR_POWER_LABEL).c_str())),
    new ProdSpec( const_cast<char*>(string("NWS_WWA").c_str()), SPDB_NWS_WWA_ID, const_cast<char*>(string(SPDB_NWS_WWA_LABEL).c_str())),
    new ProdSpec( (char*)NULL, 0, (char*)NULL)     // denote table end
  };

  ProdSpec * pspec = NULL;
  for (int ii = 0; prodSpecs[ii]->name != NULL; ii++) {
    if (0 == strcmp( prodSpecs[ii]->name, name)) {
      pspec = prodSpecs[ii];
      break;
    }
  }
  return pspec;
}



//=========================================================================


time_t convertToTime( const char * stg) {

  int slen = strlen( stg);
  if (slen < 10) badparms("invalid time format:", stg);
  int year = parseInt( stg, 0, 4);
  if (stg[4] != ' ') badparms("invalid time format:", stg);

  int month = parseInt( stg, 5, 7);
  if (stg[7] != ' ') badparms("invalid time format:", stg);

  int day = parseInt( stg, 8, 10);

  int hour = 0;
  int minute = 0;
  int second = 0;
  if (slen > 10) {
    if (slen < 16) badparms("invalid time format:", stg);
    if (stg[10] != ' ') badparms("invalid time format:", stg);
    hour = parseInt( stg, 11, 13);
    if (stg[13] != ' ') badparms("invalid time format:", stg);
    minute = parseInt( stg, 14, 16);

    if (slen > 16) {
      if (slen != 19) badparms("invalid time format:", stg);
      if (stg[16] != ' ') badparms("invalid time format:", stg);
      second = parseInt( stg, 17, 19);
    }
  }

  long long second1970 = getSecondNum( 1970, 1, 1, 0, 0, 0);   // Jan 1, 1970
  long long secondNum = getSecondNum( year, month, day, hour, minute, second);
  secondNum -= second1970;

  return secondNum;
} // end convertToTime



//=========================================================================



long long getSecondNum( int year, int month, int day,
  int hour, int minute, int second)
{
  if (year < 1900 || year > 2100) badparms("getSecondNum: invalid year", NULL);
  if (month < 1 || month > 12) badparms("getSecondNum: invalid month", NULL);
  if (day < 1 || day > 31) badparms("getSecondNum: invalid day", NULL);
  if (hour < 0 || hour >= 24) badparms("getSecondNum: invalid hour", NULL);
  if (minute < 0 || minute >= 60) badparms("getSecondNum: invalid minute", NULL);
  if (second < 0 || second >= 60) badparms("getSecondNum: invalid second", NULL);

  // Counting includes year 0.
  // Get num days in all previous years
  long cy = year - 1;     // num complete years
  long leapYears = cy / 4 - cy / 100 + cy / 400 + 1;     // +1 for year 0
  long stdYears = cy - leapYears;
  long long dayNum = leapYears * 366 + stdYears * 365;

  // Get num days in all previous months, this year
  bool isLeap = false;
  if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0) isLeap = true;

  //                     jan feb mar apr may jun jul aug sep oct nov dec
  int monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (isLeap) monthDays[2] = 29;

  for (int ii = 1; ii < month; ii++) {
    dayNum += monthDays[ii];
  }
  dayNum += day - 1;

  long long secondNum = dayNum * 24 * 3600 + hour * 3600 + minute * 60 + second;
  return secondNum;
} // end getSecondNum



//=========================================================================


int parseInt( const char * stg, int offset, int limit) {
  int len = limit - offset;
  char * cbuf = (char *) calloc( len + 1, sizeof( char));     // +1 for '/0'
  memcpy( cbuf, stg + offset, len);
  cbuf[len] = '\0';
  char * endPtr;
  int ires = strtol( cbuf, &endPtr, 10);
  if (endPtr != cbuf + len)
    badparms("invalid integer:", cbuf);
  free( cbuf);
  return ires;
}



//=========================================================================



// Reads a file and returns the length and contents of the file.
// If addNullFlag, appends a \0 to the file.

char * readFile(
  const char * fname,
  bool addNullFlag,
  int * pbuflen)
{

  FILE * fin = fopen( fname, "r");
  if (fin == NULL) badparms("cannot open input file:", fname);

  int buflen = 0;       // allocated len of bufa
  int curlen = 0;       // current len of data in bufa
  char * bufa = NULL;
  while (true) {                   // while not eof
    if (curlen >= buflen) {        // reallocate a bigger buffer
      int newlen = buflen * 2 + 1000;
      bufa = (char *) realloc( bufa, newlen);
      buflen = newlen;
    }
    int gotlen = fread( bufa + curlen, sizeof(char), buflen - curlen, fin);
    curlen += gotlen;
    if (curlen < buflen) break;
  }

  if (ferror( fin)) badparms("error reading input file:", fname);
  if ( ! feof( fin)) badparms("error reading input file:", fname);
  fclose( fin);

  if (addNullFlag) bufa[curlen++] = '\0';    // terminate the string

  // Reallocate to curlen
  bufa = (char *) realloc( bufa, curlen);
  *pbuflen = curlen;
  return bufa;
}


//=========================================================================
