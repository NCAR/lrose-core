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
// PrintSpdbMetar.cc
//
// PrintSpdbMetar object
//
// Modified from SpdbQuery by Mike Dixon
// RAP, NCAR P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// PrintSpdbMetar queries an SPDB data base, and prints to stdout.
//
///////////////////////////////////////////////////////////////

#include "PrintSpdbMetar.hh"
#include "Args.hh"

#include <Spdb/DsSpdb.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <rapformats/metar_decode.h>


using namespace std;

// Constructor

PrintSpdbMetar::PrintSpdbMetar(int argc, char **argv)

{

  // initialize

  OK = true;
  
  // set programe name
  
  _progName = "PrintSpdbMetar";

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

PrintSpdbMetar::~PrintSpdbMetar()

{

}

//////////////////////////////////////////////////
// Run

int PrintSpdbMetar::Run()
{

  // The Product ID is the same for METARs. The Data Type varies
  // with the station name. Get the Data Type, based on the input 
  // station name

  bool headerPrinted = false;
  si32 dataType;
  
  if (_args.debug) {
    cout << "Station string: " << _args.stationStr << endl;
  }

  // find the stations - it can be a comma-delimited list

  string thisStation;
  size_t startPos = 0;
  size_t commaPos = _args.stationStr.find(',', startPos);
  if (commaPos == string::npos) {
    thisStation = _args.stationStr;
  } else {
    thisStation.assign(_args.stationStr, startPos, commaPos - startPos);
  }
  
  while (thisStation.size() != 0) {
    
    if (_args.debug) {
      cerr << "thisStation: " << thisStation << endl;
    }
      
    if (thisStation == "all") {
      dataType = 0;
    } else {
      dataType = Spdb::hash4CharsToInt32(thisStation.c_str());
    }
    
    if (_args.debug) {
      cout << "Data type: " << dataType << endl;
    }

    if (_doDataType(dataType, headerPrinted)) {
      return -1;
    }

    if (commaPos == string::npos) {
      thisStation = "";
    } else {
      startPos = commaPos + 1;
      commaPos = _args.stationStr.find(',', startPos);
      thisStation.assign(_args.stationStr, startPos, commaPos - startPos);
    }

  }

  if (_args.xml && headerPrinted) {
    fprintf(stdout,"</metars>\n");
  }
 

  return 0;

}

//////////////////////////////////////////////////
// print a data type

int PrintSpdbMetar::_doDataType(si32 dataType, bool &headerPrinted)
{

  DsSpdb spdb;

  switch (_args.mode) {
    
  case Args::exactMode:
    if (spdb.getExact(_args.urlStr, _args.requestTime, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getExact for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::closestMode:
    if (spdb.getClosest(_args.urlStr,
			_args.requestTime, _args.timeMargin, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getClosest for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::intervalMode:
    if (spdb.getInterval(_args.urlStr,
			 _args.startTime, _args.endTime, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getInterval for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::validMode:
    if (spdb.getValid(_args.urlStr, _args.requestTime, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getValid for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::latestMode:
    if (spdb.getLatest(_args.urlStr, _args.timeMargin, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getLatest for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::firstBeforeMode:
    if (spdb.getFirstBefore(_args.urlStr,
			    _args.requestTime, _args.timeMargin, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstBefore for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  case Args::firstAfterMode:
    if (spdb.getFirstAfter(_args.urlStr,
			   _args.requestTime, _args.timeMargin, dataType)) {
      cerr << "ERROR - COMM - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstAfter for url: " << _args.urlStr << endl;
      return (-1);
    }
    break;
    
  default:
    break;

  }

  // print table header if required

  if ((_args.table || _args.xml) && !headerPrinted && spdb.getNChunks() > 0) {
    if (_args.xml) {
      fprintf(stdout,"<metars>\n");
    }
    else if (_args.format == Args::AOAWS) {
      _printAoawsHeader();
    } else if (_args.format == Args::AOAWS_WIDE) {
      _printAoawsWideHeader();
    } else if (_args.format == Args::WSDDM) {
      _printWsddmHeader();
    }
    headerPrinted = true;
  }
  
  // Print out station data

  int prod_id=0;
  int nSoFar = 0;

  for (int ii = 0; ii < spdb.getNChunks(); ii++) {
    
    int jj;
    if (_args.reverse) {
      jj = spdb.getNChunks() - 1 - ii;
    } else {
      jj = ii;
    }

    nSoFar++;

    // Set the product id
    
    prod_id = spdb.getProdId();

    // Get the rest of the chunk and print it out based on product ID

    const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
    const void *chunk_data = chunks[jj].data;
    int chunk_len = chunks[jj].len;
    time_t valid_time = chunks[jj].valid_time;

    string stationId;
    double lat, lon, elevM;
    if (WxObs::disassembleStationDetails(chunk_data,
					 chunk_len,
					 stationId, lat, lon, elevM)) {
      lat = 90.0;
      lon = 0.0;
    }

    switch (prod_id) {
      
    case SPDB_ASCII_ID:
    case SPDB_RAW_METAR_ID:

      if (_args.table || _args.xml) {

	// table format with remarks
	// copy chunk data to str, remove line feeds

	TaArray<char> str_;
	char *str = str_.alloc(chunks[jj].len + 1);
	strncpy(str, (char *) chunk_data, chunk_len);
	str[chunk_len] = '\0';
	for (int i = 0; i < chunk_len; i++) {
	  if (str[i] == '\n' || str[i] == '\r' || str[i] == '=') {
	    str[i] = ' ';
	  }
	}

	// find the remarks if there

	char *remarks = NULL;
	if ((remarks = strstr(str, "RMK")) == NULL) {
	  if ((remarks = strstr(str, "RMRK")) == NULL) {
	    remarks = strstr(str, "REMARK");
	  }
	}
	
	// decode the metar
	TaArray<char> tmp_;
	char *tmp = tmp_.alloc(chunk_len + 1);
	strcpy(tmp, str);
	Decoded_METAR dcdMetar;

	// DcdMETAR clobbers tmp, so make another copy w/o extra white space
	string metarStr;
	for (unsigned int ix = 0; ix < strlen(str); ix++)
	  if (ix > 0 && str[ix] == ' ' && str[ix-1] == ' ') {}
	  else
	    metarStr += str[ix];
	
	if (DcdMETAR(tmp, &dcdMetar, true) == 0) {

	  WxObs metar;
	  DateTime vtime(valid_time);

 	  if (metar.setFromDecodedMetar(metarStr.c_str(), stationId, dcdMetar, 
					vtime.utime(), 0, 0, 0) == 0) {
	    
	    metar.assembleAsReport(REPORT_PLUS_METAR_XML);
	    if (_args.xml)
	      {
		string xml;
		metar.loadXml(xml, true);
		cout << xml;
	      }
	    else{
	      if (_args.format == Args::AOAWS ||
		  _args.format == Args::AOAWS_WIDE) {
		_printAoawsMetar(metar, false);
		if (remarks && _args.remarks) {
		  cout << " " << remarks << endl;
		} else {
		  cout << endl;
		}
	      } else if (_args.format == Args::WSDDM) {
		_printWsddmMetar(metar);
	      } 
	    }
	  }
	}		
	
      } else {

	// use the ascii data

	if (_args.timeLabel) {
	  cout << DateTime::str(valid_time) << endl;
	}
	const char *str = (const char *) chunk_data;
	fprintf(stdout, "%s\n", str);
	if (str[strlen(str)-1] != '\n') {
	  fprintf(stdout, "\n");
	}
      }
      break;
      
    case SPDB_STATION_REPORT_ID:

      WxObs metar;
      metar.disassemble(chunk_data, chunk_len);

      if (_args.table) {
	if (_args.format == Args::AOAWS ||
            _args.format == Args::AOAWS_WIDE) {
	  _printAoawsMetar(metar);
	} else  if (_args.format == Args::WSDDM){
	  _printWsddmMetar(metar);
	}
      } else if (!_args.xml) {
	cout << "============================================" << endl;
	//	print_station_report(stdout, "", &metar);
	metar.print(cout);
      }

      break;
      
    } /* endswitch - ProductId */ 

    if (_args.maxItems > 0 && nSoFar >= _args.maxItems) {
      return 0;
    }
    
  } /* endfor */
  

  return 0;

}

void PrintSpdbMetar::_printWsddmHeader()

{

  fprintf(stdout, "STN GMT  TMP DEW DIR SPD GST VSBY  CEIL WEATHER\n");
  fprintf(stdout, "           C   C deg  kt  kt   mi    ft\n");
  fprintf(stdout, "=== ==== === === === === === ====  ==== =======\n");

}
  
void PrintSpdbMetar::_printAoawsHeader()

{

  fprintf(stdout, "NAME TIME W/D W/S GST  VIS   WEATHER CEIL TMP DEW  QNH\n");
  fprintf(stdout, "          deg  kt  kt    m             FL   C   C  hPa\n");
  fprintf(stdout, "==== ==== === === === ====   ======= ==== === === ====\n");

}
  
void PrintSpdbMetar::_printAoawsWideHeader()

{

  fprintf(stdout, "NAME TIME W/D W/S GST  VIS     WEATHER CEIL TMP DEW  QNH\n");
  fprintf(stdout, "          deg  kt  kt    m               FL   C   C  hPa\n");
  fprintf(stdout, "==== ==== === === === ====     ======= ==== === === ====\n");

}
  
void PrintSpdbMetar::_printWsddmMetar(WxObs &metar,
				      bool add_carriage_return /* = true*/ )

{

  // station name (don't print the first character)

  fprintf(stdout, "%3s", &metar.getStationId().c_str()[1]);
    
  // report time
  
  date_time_t *time_struct = udate_time(metar.getObservationTime());
  fprintf(stdout, " %02d%02d",
	  time_struct->hour, time_struct->min);
    
  // temperature
  
  if (metar.getTempC() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getTempC());
  }
  
  // dew point
  
  if (metar.getDewpointC() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getDewpointC());
  }
  
  // wind direction  - ouput to nearest 10 degrees
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else if (signbit(metar.getWindDirnDegt())) {
    // variable winds - dir is set to -0.0
    // we need to use signbit() to detect this
    fprintf(stdout, " VRB");
  } else {
    fprintf(stdout, " %3.0f", metar.getWindDirnDegt());
  }
  
  // wind speed

  if (metar.getWindSpeedMps() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getWindSpeedMps() * NMH_PER_MS);
  }
    
  // wind gust
  
  double windgust = metar.getWindGustMps() * NMH_PER_MS;
    
  if (metar.getWindGustMps() != WxObs::missing && windgust > 5.0) {
    fprintf(stdout, "G");
    fprintf(stdout, "%3.0f", windgust);
  } else {
    fprintf(stdout, "    ");
  }
    
  // visibility
  
  double vis_miles = metar.getVisibilityKm() / KM_PER_MI;
  double integral_val;
  double frac_val = modf(vis_miles, &integral_val);

  if (metar.getVisibilityKm() == WxObs::missing) {
    fprintf(stdout, "     ");
  } else if (vis_miles > 4 || frac_val < 0.001 || frac_val > 0.999) {
    fprintf(stdout, " %4.0f", vis_miles);
  } else {
    fprintf(stdout, " %4.2f", vis_miles);
  }
    
  // ceiling

  if (metar.getCeilingKm() == WxObs::missing) {
    fprintf(stdout, "      ");
  } else {
    int printed_int_value =
      (int)(_nearest(((metar.getCeilingKm() / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
    fprintf(stdout, " %5d", printed_int_value);
  }
    
  // weather string
  fprintf(stdout, " %s",
	  _truncateWxStr(metar.getMetarWx(), 2).c_str());
  
  if (add_carriage_return) {
    fprintf(stdout, "\n");
  }
    
}

void PrintSpdbMetar::_printAoawsMetar(WxObs &metar,
				      bool add_carriage_return /* = true*/ )

{

  // station name
  
  fprintf(stdout, "%4s", metar.getStationId().c_str());
  
  // report time
  
  date_time_t *time_struct = udate_time(metar.getObservationTime());
  fprintf(stdout, " %02d%02d", time_struct->hour, time_struct->min);

  // wind direction  - ouput to nearest degree
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else if (signbit(metar.getWindDirnDegt())) {
    // variable winds - dir is set to -0.0
    // we need to use signbit() to detect this
    fprintf(stdout, " VRB");
  } else {
    fprintf(stdout, " %03d", (int)metar.getWindDirnDegt());
  }
    
  // wind speed

  if (metar.getWindSpeedMps() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getWindSpeedMps() * NMH_PER_MS);
  }
    
  // wind gust
  
  double windgust = metar.getWindGustMps() * NMH_PER_MS;
    
  if (metar.getWindGustMps() != WxObs::missing && windgust > 5.0) {
    fprintf(stdout, " %3.0f", windgust);
  } else {
    fprintf(stdout, "    ");
  }
    
  // visibility

  double vis_m = metar.getVisibilityKm() * 1000.0;

  if (metar.getVisibilityKm() == WxObs::missing){
    fprintf(stdout, "     ");
  } else {
    if (metar.getVisibilityIsMinimum()) {
      fprintf(stdout, " 10k+");
    } else if (vis_m > 9998.0) {
      fprintf(stdout, " 10k+");
    } else {
      fprintf(stdout, " %-4d", (int)(vis_m + 0.5) );
    }
  }

  // weather string

  char weather_str[12];
  STRncopy(weather_str, metar.getMetarWx().c_str(), 11);

  if (_args.format == Args::AOAWS) {
    // max length 9 chars
    // if longer than 9, put * in 9th position to indicate this
    if (strlen(weather_str) > 9) {
      weather_str[8] = '*';
      weather_str[9] = '\0';
    }
    fprintf(stdout, " %9s", weather_str);
  } else {
    fprintf(stdout, " %11s", weather_str);
  }

  // ceiling
  
  if (metar.getCeilingKm() == WxObs::missing){
    fprintf(stdout, "     ");
  } else {
    double ceiling_km = metar.getCeilingKm();
    double ceiling_ft = (ceiling_km / KM_PER_MI) * FT_PER_MI;
    int ceiling_fl = (int) ((ceiling_ft / 100.0) + 0.5);
    if (metar.getCeilingIsMinimum()) {
      fprintf(stdout, "     ");
    } else if (ceiling_fl > 500) {
      fprintf(stdout, "     ");
    } else {
      fprintf(stdout, "  %.3d", ceiling_fl);
    }
  }
  
  // temperature

  if (metar.getTempC() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getTempC());
  }
    
  // dew point

  if (metar.getDewpointC() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getDewpointC());
  }
  
  // pressure
  
  if (metar.getSeaLevelPressureMb() == WxObs::missing) {
    fprintf(stdout, "     ");
  } else {
    fprintf(stdout, " %4.0f", metar.getSeaLevelPressureMb());
  }

  if (add_carriage_return) {
    fprintf(stdout, "\n");
  }
    
}

double PrintSpdbMetar::_nearest(double target, double delta)
{

  double answer;
  double rem;                                                                 
  
  delta = fabs(delta);                   
  rem = remainder(target,delta);
  
  if(target >= 0.0) {
    if(rem > (delta / 2.0)) {
      answer = target + (delta - rem);
    } else {
      answer = target -  rem;
    }
  } else {
    if(fabs(rem) > (delta / 2.0)) {
      answer = target - (delta + rem);
    } else {
      answer = target -  rem;
    }
  }
  
  return answer;

}

string PrintSpdbMetar::_truncateWxStr(const string &wxStr,
                                      int nToks)
  
{

  // tokenize the wx string

  vector<string> toks;
  _tokenize(wxStr, " \n\r\t", toks);
  
  int nn = nToks;
  if (nn > (int) toks.size()) {
    nn = (int) toks.size();
  }

  string concat;
  for (int ii = 0; ii < nn; ii++) {
    if (ii != 0) {
      concat += " ";
    }
    concat += toks[ii];
  }
  
  return concat;

}

void PrintSpdbMetar::_tokenize(const string &str,
                               const string &spacer,
                               vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}
