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
// PrintAcTracks.cc
//
// PrintAcTracks object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////
//
// PrintAcTracks queries an SPDB data base, and prints to stdout.
//
///////////////////////////////////////////////////////////////

#include "PrintAcTracks.hh"
#include "Args.hh"
#include "Print.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/file_io.h>
using namespace std;

// Constructor

PrintAcTracks::PrintAcTracks(int argc, char **argv)

{

  // initialize

  OK = true;
  
  // set programe name
  
  _progName = "PrintAcTracks";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = false;
    return;
  }

  return;

}

// destructor

PrintAcTracks::~PrintAcTracks()

{

}

//////////////////////////////////////////////////
// Run

int PrintAcTracks::Run()
{

  for (size_t ii = 0; ii < _args.dataTypes.size(); ii++) {

    DsSpdb *spdb;

    if (_args.threaded) {
      if (_args.debug) {
	cerr << "** THREADING ON **" << endl;
      }
      spdb = new DsSpdbThreaded;
    } else {
      spdb = new DsSpdb;
    }

    if (_args.debug) {
      spdb->setDebug(true);
    }
    
    if (_doDataType(_args.dataTypes[ii], spdb)) {
      return -1;
      delete spdb;
    }
    
    delete spdb;

  }

  return 0;

}

//////////////////////////////////////////////////
// process a given data type

int PrintAcTracks::_doDataType(int dataType,
			   DsSpdb *spdb)

{

  if (_args.horizLimitsSet) {
    spdb->setHorizLimits(_args.minLat, _args.minLon,
			_args.maxLat, _args.maxLon);
  }
  if (_args.vertLimitsSet) {
    spdb->setVertLimits(_args.minHt, _args.maxHt);
  }
  if (_args.auxXmlPath.size() > 0) {
    _setAuxXml(spdb);
  }

  // get times is a special case

  if (_args.mode == Args::timesMode) {

    time_t firstTime;
    time_t lastTime;
    time_t lastValidTime;
    
    if (spdb->getTimes(_args.urlStr, firstTime, lastTime, lastValidTime)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getTimes for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    
    cout << "==================================================" << endl;
    cout << "url: " << _args.urlStr << endl;
    cout << "Prod label: " << spdb->getProdLabel() << endl;
    cout << "Prod id:    " << spdb->getProdId() << endl;
    cout << "DataType: " << dataType << endl;
    
    cout << "  First time:      " << DateTime::str(firstTime, false) << endl;
    cout << "  Last time:       " << DateTime::str(lastTime, false) << endl;
    cout << "  Last valid time: "
	 << DateTime::str(lastValidTime, false) << endl;

    return (0);

  }

  if (_args.unique == Args::uniqueLatest) {
    spdb->setUniqueLatest();
  } else if (_args.unique == Args::uniqueEarliest) {
    spdb->setUniqueEarliest();
  }

  if (_args.checkWriteTimeOnGet) {
    spdb->setCheckWriteTimeOnGet(_args.latestWriteTime);
  }

  switch (_args.mode) {

  case Args::exactMode:
    if (spdb->getExact(_args.urlStr, _args.requestTime,
		      dataType, _args.dataType2,
		      _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getExact for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::closestMode:
    if (spdb->getClosest(_args.urlStr,
			_args.requestTime, _args.timeMargin,
			dataType, _args.dataType2,
			_args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getClosest for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::intervalMode:
    if (spdb->getInterval(_args.urlStr,
			 _args.startTime, _args.endTime,
			 dataType, _args.dataType2,
			 _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getInterval for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::validMode:
    if (spdb->getValid(_args.urlStr, _args.requestTime,
		      dataType, _args.dataType2,
		      _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getValid for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::latestMode:
    if (spdb->getLatest(_args.urlStr, _args.timeMargin,
		       dataType, _args.dataType2,
		       _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getLatest for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::firstBeforeMode:
    if (spdb->getFirstBefore(_args.urlStr,
			    _args.requestTime, _args.timeMargin,
			    dataType, _args.dataType2,
			    _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstBefore for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::firstAfterMode:
    if (spdb->getFirstAfter(_args.urlStr,
			   _args.requestTime, _args.timeMargin,
			   dataType, _args.dataType2,
			   _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstAfter for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::headerMode:
    if (spdb->printHeader(_args.urlStr,
			 _args.requestTime, cout)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling printHeader for url: " << _args.urlStr << endl;
      cerr << "  Request time: "
	   << DateTime::str(_args.requestTime, false) << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    return 0;
    break;
    
  case Args::timeListMode:
    if (spdb->compileTimeList(_args.urlStr,
			     _args.startTime, _args.endTime,
			     _args.timeListMinInterval)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling compileTimeList for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  default:
    break;

  }

  if (_args.threaded) {
    DsSpdbThreaded *tspdb = (DsSpdbThreaded *) spdb;
    while (!tspdb->getThreadDone()) {
      if (_args.debug) {
	cerr << " ... waiting for thread to complete, % done: "
	     << tspdb->getPercentComplete() << endl;
	cerr << "     NbytesExpected: " << tspdb->getNbytesExpected() << endl;
	cerr << "     NbytesDone: " << tspdb->getNbytesDone() << endl;
      }
      umsleep(500);
    }
    if (tspdb->getThreadRetVal()) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Thread ret val: " << tspdb->getThreadRetVal() << endl;
      cerr << tspdb->getErrStr() << endl;
      return (-1);
    }
  }

  Print print(stdout, cout, _args.debug);

  if (_args.mode == Args::timeListMode) {
    cout << "==================================================" << endl;
    cout << "url: " << _args.urlStr << endl;
    cout << "Prod label: " << spdb->getProdLabel() << endl;
    cout << "Prod id:    " << spdb->getProdId() << endl;
    cout << "Time list:" << endl;
    const vector<time_t> &times = spdb->getTimeList();
    for (size_t ii = 0; ii < times.size(); ii++) {
      cout << "  " << DateTime::str(times[ii]) << endl;
    }
    return 0;
  }

  if (!_args.noHeader && !_args.timeLabel) {

    cout << "==================================================" << endl;
    cout << "url: " << _args.urlStr << endl;
    cout << "Prod label: " << spdb->getProdLabel() << endl;
    cout << "Prod id:    " << spdb->getProdId() << endl;
    cout << "DataType: " << dataType << endl;
    cout << "N Chunks:   " << spdb->getNChunks() << endl;

  }
   
  // How many items to print?
  
  const vector<Spdb::chunk_t> &chunks = spdb->getChunks();
  int nChunks = (int) chunks.size();
  int startPos = 0;
  if ((_args.doLastN > 0) && (_args.doLastN < nChunks)) {
    startPos = nChunks - _args.doLastN;
  }

  // load up print vector

  vector<Spdb::chunk_t> printChunks;
  for (int ii = startPos; ii < nChunks; ii++) {
    printChunks.push_back(chunks[ii]);
  }
  int nPrint = (int) printChunks.size();

  // Print out the results. Loop through either in forward or reverse order
  
  for (int jj = 0; jj < nPrint; jj++) {

    int ii = jj;
    if (_args.doReverse) {
      ii = nPrint - 1 - jj;
    }
    
    const Spdb::chunk_t &chunk = printChunks[ii];

    if (!_args.summaryMode) {
      if (_args.timeLabel) {
	cout << DateTime::str(chunk.valid_time) << endl;
      } else if (!_args.noHeader) {
	fprintf(stdout, "\n");
	fprintf(stdout, "Header for chunk ---> %d <---\n", ii);
	print.chunk_hdr(chunk);
      }
    }
    
    if (_args.blankLine) {
      fprintf(stdout, "\n");
    }

    if (_args.refsOnly) {
      continue;
    }

    int data_len = chunk.len;
    void *chunk_data = chunk.data;

    switch (spdb->getProdId()) {
      
    case SPDB_ASCII_ID :
    case SPDB_XML_ID :
    case SPDB_RAW_METAR_ID :
    case SPDB_WAFS_SIGWX_CLOUD_ID:
    case SPDB_WAFS_SIGWX_JETSTREAM_ID:
    case SPDB_WAFS_SIGWX_TURBULENCE_ID:
      print.ascii(data_len, chunk_data);
      break;
      
    case SPDB_AC_VECTOR_ID :
      print.ac_vector(data_len, chunk_data);
      break;

    case SPDB_AC_POSN_ID :
      if (_args.summaryMode) {
	if (chunk.data_type2 ==  SPDB_AC_POSN_WMOD_ID) {
	  print.ac_posn_wmod_summary(chunk.valid_time, data_len, chunk_data);
	} else {
	  print.ac_posn_summary(chunk.valid_time, data_len, chunk_data);
	}
      } else {
	if (chunk.data_type2 ==  SPDB_AC_POSN_WMOD_ID) {
	  print.ac_posn_wmod(data_len, chunk_data);
	} else {
	  print.ac_posn(data_len, chunk_data);
	}
      }
      break;
      
    case SPDB_AC_POSN_WMOD_ID :
      print.ac_posn_wmod(data_len, chunk_data);
      break;
    
    case SPDB_AC_ROUTE_ID :
      print.ac_route(chunk_data);
      break;

    case SPDB_AC_DATA_ID :
      print.ac_data(chunk_data);
      break;
      
    } /* endswitch - ProductId */ 

  } /*end while */
  
  return (0);

}

//////////////////////////////////////////////////////
// set the auxiliary XML by reading in XML from a file

int PrintAcTracks::_setAuxXml(DsSpdb *spdb)
  
{

  // Stat the file to get length
  
  struct stat fileStat;
  if (ta_stat(_args.auxXmlPath.c_str(), &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - PrintAcTracks::_setAuxXml" << endl;
    cerr << "  Cannot stat file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  int fileLen = fileStat.st_size;
  
  // open file
  
  FILE *xmlFile;
  if ((xmlFile = fopen(_args.auxXmlPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - PrintAcTracks::_setAuxXml" << endl;
    cerr << "  Cannot open file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // create buffer
  
  TaArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(fileLen + 1);
  memset(xmlBuf, 0, fileLen + 1);
  
  // read in buffer, close file
  
  if (ta_fread(xmlBuf, 1, fileLen, xmlFile) != fileLen) {
    int errNum = errno;
    cerr << "ERROR - PrintAcTracks::_setAuxXml" << endl;
    cerr << "  Cannot read file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(xmlFile);
    return -1;
  }
  fclose(xmlFile);
  
  // set XML

  spdb->setAuxXml(xmlBuf);
  return 0;

}
