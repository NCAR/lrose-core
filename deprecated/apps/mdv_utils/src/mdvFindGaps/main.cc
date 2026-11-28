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


#include "Params.hh"

#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include <iostream>

using namespace std;

typedef struct {
  time_t start;
  time_t end;
} gap_t;

int main(int argc, char *argv[]){

  //
  // Get start, end times. Default to yesterday.
  //
  time_t start, end, now;
  now = time(NULL);

  end = (time_t) 86400*(now/86400) - 1;
  start = end - 86399;

  bool startSpec = false;
  bool endSpec = false;
  bool todaySpec = false;

  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
        !strcmp(argv[i], "-h") ||
        !strcmp(argv[i], "-help") ||
        !strcmp(argv[i], "-man")) {

      cerr << "USAGE : mdvFindGaps -params paramFile -start \"YYYY MM DD hh mm ss\" -end \"YYYY MM DD hh mm ss\"" << endl;
      cerr << "Default is to process yesterday's data if -start and -end not specified. The -today arg" << endl;
      cerr << "will process today. Times are UTC. Niles Oien." << endl;
      exit (0);

    } else if (!strcmp(argv[i], "-start")) {

      startSpec = true;

      if (todaySpec){
	cerr << "Time specification cannot be used with -today" << endl;
	return -1;
      }

      if (i < argc - 1) {
        start = DateTime::parseDateTime(argv[++i]);
        if (start == DateTime::NEVER){
	  cerr << "Cannot read start time." << endl;
	  exit(-1);
	}
      } else {
	cerr << "Start time not specified." << endl;
	exit(-1);
      }
    } else if (!strcmp(argv[i], "-end")) {

      endSpec = true;

      if (todaySpec){
	cerr << "Time specification cannot be used with -today" << endl;
	return -1;
      }

      if (i < argc - 1) {
        end = DateTime::parseDateTime(argv[++i]);
        if (end == DateTime::NEVER){
	  cerr << "Cannot read end time." << endl;
	  exit(-1);
	}
      } else {
	cerr << "End time not specified." << endl;
	exit(-1);
      }
    } else if (!strcmp(argv[i], "-today")) {
      //
      // -today specified, check no -start and -end on cli, add one day to default which are for yesterday.
      //
      todaySpec = true;

      if (startSpec || endSpec){
	cerr <<  "Time specification cannot be used with -today" << endl;
	return -1;
      }

      start += 86400; end = time(NULL);
    } 

  } // End of arg loop


  //
  // Load TDRP parameters.
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Print report header
  //
  cout << "Looking for temporal data gaps greater than " << P.gap.maxInterval << " seconds at MDV url " << P.gap.url << endl;
  cout << "Considering period between " << utimstr(start) << " and " << utimstr( end ) << endl;
  if (P.gap.testAfield) cout << "Requiring field " << P.gap.fieldName << " to be less than " << P.gap.percentMissingMax << " percent missing." << endl;
  cout << endl;



  DsMdvxTimes inTimes;

  if (inTimes.setArchive(P.gap.url, start, end)){
    cerr << "Failed to set URL " << P.gap.url << endl;
    return -1;
  }

  vector<time_t> dataFileTimes =  inTimes.getArchiveList();

  cout << dataFileTimes.size() << " volumes found." << endl;

  // If we are testing fields, do that. Also use this as
  // an opportunity to prepend the start time and append the end
  // time to a new list of time we want to use (this is implied
  // for what we want to do).

  vector<time_t> usableDataFileTimes;
  unsigned numRejectedVols = 0;

  usableDataFileTimes.push_back( start );

  if ( P.gap.testAfield ) {

    for(unsigned i=0; i < dataFileTimes.size(); i++){

      DsMdvx New;
      
      New.setReadTime(Mdvx::READ_FIRST_BEFORE, P.gap.url, 0, dataFileTimes[i]);
      New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
      New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
      New.addReadField( P.gap.fieldName );

      if (!(New.readVolume())){
	MdvxField *field= New.getFieldByNum( 0 );
	if (field != NULL){
	  Mdvx::field_header_t fhdr = field->getFieldHeader();
	  fl32 *data = (fl32 *) field->getVol();
	  unsigned long num = fhdr.nx * fhdr.ny * fhdr.nz;
	  unsigned long numBad = 0L;
	  for (unsigned long k=0; k < num; k++){
	    if ((data[k] == fhdr.bad_data_value) || (data[k] == fhdr.missing_data_value)) numBad++;
	  }
	  double percentBad = 100.0*double(numBad)/double(num);
	  if (percentBad < P.gap.percentMissingMax){
	    usableDataFileTimes.push_back( dataFileTimes[i] );
	  } else {
	    numRejectedVols++;
	  }
	}
      }

    }

    cout << numRejectedVols << " volumes rejected due to field " << P.gap.fieldName << " being more than " << P.gap.percentMissingMax << " percent missing." << endl << endl;

  } else {
    //
    // Not testing a field, add all data times we have.
    //
    for(unsigned i=0; i < dataFileTimes.size(); i++){
      usableDataFileTimes.push_back( dataFileTimes[i] );
    }
  }

  usableDataFileTimes.push_back( end );

  //
  // OK - now have list of times we can use. Compile list of gaps.
  //
  vector <gap_t> gaps;
  for(unsigned i=0; i <  usableDataFileTimes.size()-1; i++){

    if ( usableDataFileTimes[i+1]- usableDataFileTimes[i] > P.gap.maxInterval ){
      gap_t gap;
      gap.start = usableDataFileTimes[i];
      gap.end = usableDataFileTimes[i+1];
      gaps.push_back( gap );
    }

  }

  //
  // Print report trailer
  //
  if (gaps.size() == 1)
    cout << gaps.size() << " gap found." << endl << endl;
  else
    cout << gaps.size() << " gaps found." << endl << endl;

  time_t totalGapTime = 0L;
  for (unsigned i=0; i < gaps.size(); i++){
    cout << "Gap " << i+1 << " from " << utimstr(gaps[i].start) << " to " << utimstr(gaps[i].end) << " duration " << (gaps[i].end - gaps[i].start)/60 << " minutes" << endl;
    totalGapTime += gaps[i].end - gaps[i].start;
  }

  cout << endl;

  cout << "Sum of gap times : " << totalGapTime/60 << " minutes. Data were missing for " << (int)rint( 100.0*totalGapTime/(end-start)) << " percent of this period." << endl;

  if (usableDataFileTimes.size() == 2) // Only the start and end times are in there
    cout << endl << "NOTE : The entire period is missing according to the criteria set." << endl << endl;

  cout << endl << "Report compiled " << utimstr(time(NULL)) << endl;

  return 0;

}
