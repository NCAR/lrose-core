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
// MetarSelect.cc
//
// MetarSelect object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <cmath>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <rapmath/math_macros.h>

#include "MetarSelect.hh"
#include "StationLocate.hh"

// FMQ Support Classes
#include <Fmq/RemoteUIQueue.hh>

using namespace std;

// Constructor

MetarSelect::MetarSelect(int argc, char **argv)
  
{
  
  isOK = true;
  _coordShmem = NULL;
  strcpy(_output_filename, "test.html");
  
  // set programe name

  _progName = "MetarSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  key_t myKey;
  switch (_params.output_mode) {
    case Params::MODE_SHMEM:
      // Attach to Cidd's shared memory segment
     
	  myKey = _params.coord_shmem_key;
      if ((_coordShmem =
           (coord_export_t *) ushm_create(myKey,
                                          sizeof(coord_export_t),
                                          0666)) == NULL) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  Cannot create/attach Cidd's coord export shmem segment";
        cerr << endl;
        isOK = false;
      }
      break;
      
    case Params::MODE_FMQ:
      bool compression = false;
      _remote_ui = new RemoteUIQueue();
      
      if(_remote_ui->initReadWrite( _params.fmq_url,(char *) "MetarSelect",
                                    false, DsFmq::END, compression,
                                    4096, 4096*256 ) != 0 ) {
        cerr << "Problems initialising Remote Command Fmq:" << _params.fmq_url;
        cerr << endl;
      }
      
      
      break;
      
  }
  
  return;

}

// destructor

MetarSelect::~MetarSelect()
  
{

  // detach from shared memory
  
  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MetarSelect::Run ()
{
  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  switch (_params.output_mode) {

    case Params::MODE_SHMEM:
      _runShmemMode();
      break;

    case Params::MODE_FMQ:
      _runFmqMode();
      break;

    default:
      return -1;

  }

  return 0;

}

//////////////////////////////////////////////////
// Run in shmem mode

void MetarSelect::_runShmemMode ()

{

  int last_no =  0;
  time_t last_display_time = 0;
  time_t data_time;
  string  msg;

  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
      
  // Store the latest click number at startup
  
  // Now, operate forever
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    time_t curr_time = time(NULL);
    
    if (_coordShmem->pointer_seq_num != last_no ||
        (_params.auto_click_interval > 0 &&
         abs(curr_time - last_display_time) > _params.auto_click_interval)) {
      
      if (_coordShmem->pointer_seq_num != last_no) {
        
        if (_params.mouse_button == 0 ||
            _coordShmem->button == _params.mouse_button) {
          
          if (_params.debug) {
            fprintf(stderr,
                    "Click - lat = %g, lon = %g, mouse button = %d\n",
                    _coordShmem->pointer_lat,
                    _coordShmem->pointer_lon,
                    (int) _coordShmem->button);
          }
          
          
          if(_params.use_cidd_time) {
            data_time = _coordShmem->time_cent;
          } else {
            data_time = curr_time;
          }
          
          _doPrint(data_time,
                   _coordShmem->pointer_lat,
                   _coordShmem->pointer_lon);
          
          last_display_time = curr_time;
          
        } else {
          
          if (_params.debug) {
            fprintf(stderr, "   Not processing clicks for mouse button %ld\n",
                    _coordShmem->button);
          }
          
        }
        
        last_no = _coordShmem->pointer_seq_num;
        
      } else {
        
        time_t data_time;
        
        if(_params.use_cidd_time) {
          data_time = (time_t) _coordShmem->time_cent;
        } else {
          data_time = curr_time;
        }
        
        _doPrint(data_time,
                 _params.startup_location.lat,
                 _params.startup_location.lon);
        
        last_display_time = curr_time;
        
      }
      
    } // if (_coordShmem->pointer_seq_num != last_no ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(true)

}

//////////////////////////////////////////////////
// Run in FMQ mode

void MetarSelect::_runFmqMode()
{

  int status;
  time_t data_time;
  string  msg;

  while (true) {
    // Register with the process mapper
    PMU_auto_register("Waiting for user input");
    
    msg = _remote_ui->readNextContents(status);
    if(status == 0) {
      if (_params.debug) {
        fprintf(stderr,
                "Received Remote UI FMQ MESSAGE: %s\n",
                msg.c_str());
      }
      
      double lat,lon;
      
      long ltime;
      sscanf(msg.c_str(),
             "%lf %lf %ld %s",&lat,&lon, &ltime, _output_filename);
      data_time = ltime;
      
      if (_params.debug) {
        fprintf(stderr, " LAT,LON, NAME: %lf %lf %d %s\n",
                lat,lon,(int)data_time,_output_filename);
      }
      
      _doPrint(data_time, lat, lon);
      
    } else {
      usleep(100000);  //  Don't hammer the CPU - wait 100 msec.
    }
  } // while(forever)
  
}

void MetarSelect::_doPrint(time_t data_time,
			   double click_lat,
			   double click_lon)
  
{
   static time_t last_request_time = 0;
   static time_t last_data_time = 0;
   time_t now = time(0);

   if(now > (_params.max_data_age + last_request_time) ||
	  abs(data_time - last_data_time) > _params.max_data_age ) {

     // should we check write time?

     if (_params.use_cidd_time && _coordShmem &&
         _coordShmem->checkWriteTimeOnRead) {
       _spdb.setCheckWriteTimeOnGet(_coordShmem->latestValidWriteTime);
     } else {
       _spdb.clearCheckWriteTimeOnGet();
     }
  
    // Request the data from the spdb database

    if (_spdb.getInterval(_params.input_url,
			data_time - _params.retrieval_period,
			data_time, 0)) {
      if (_params.log_spdb_errors) {
        MsgLog msgLog;
        msgLog.setApplication(_progName, _params.instance);
        msgLog.setSuffix("log");
        msgLog.setAppendMode();
        msgLog.setDayMode();
        msgLog.setOutputDir(_params.errors_log_dir);
        msgLog.postMsg("ERROR - MetarSelect::_doPrint");
        msgLog.postMsg("  Retrieving data.");
        msgLog.postMsg("%s", _spdb.getErrStr().c_str());
      } else {
        cerr << "ERROR - MetarSelect::_doPrint" << endl;
        cerr << "  Retrieving data." << endl;
        cerr << "  " << _spdb.getErrStr() << endl;
        }
      }
  
      if (_params.debug) {
        cerr << "Retrieved " << _spdb.getNChunks() << " chunks from "
	     << _params.input_url << endl;
        cerr << "start_time: "
	     << DateTime::str(data_time - _params.retrieval_period) << endl;
        cerr << "end_time: "
	     << DateTime::str(data_time) << endl;
      }

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        Spdb::chunk_ref_t *refs = _spdb.getChunkRefs();
        for (int i = 0; i < _spdb.getNChunks(); i++) {
          cerr << "data_type: " << refs[i].data_type << endl;
          cerr << "  dehashed: "
	       << Spdb::dehashInt32To4Chars(refs[i].data_type) << endl;
        }
      }

	  last_request_time = now;
	  last_data_time = data_time;
   }
  
  // chunks are returned in time order
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();

  bool sort_by_location = _params.sort_by_location;
  StationLocate loc;
  if (_params.sort_by_location && _params.use_location_file) {
    if (loc.load(_params.station_location_file)) {
      sort_by_location = false;
    }
  }

  // load up a map of indices in reverse time order, using the
  // distance as a sorting measure if available
  
  double largeDist = 1e5;
  distmap_t distmap;
  
  for (int ii = (int) chunks.size() - 1; ii >= 0; ii--){
    
    if (sort_by_location) {

      double Lat, Lon, Elev;
      string name(Spdb::dehashInt32To4Chars(chunks[ii].data_type));
      if (_params.use_location_file) {
	if (loc.getPos(name, Lat, Lon, Elev)) {
	  Lat = 90.0;
	  Lon = 0.0;
	}
      } else {
        string stationId;
        double lat, lon, elevM;
        if (WxObs::disassembleStationDetails(chunks[ii].data,
                                             chunks[ii].len,
                                             stationId, lat, lon, elevM)) {
	  Lat = 90.0;
	  Lon = 0.0;
        } else {
	  Lat = lat;
	  Lon = lon;
        }
      }
      double dist, dirn;
      PJGLatLon2RTheta(Lat,Lon,click_lat,click_lon,
		       &dist, &dirn);
      distpair_t pp(dist, ii);
      distmap.insert(pp);

    } else {

      distpair_t pp(largeDist, ii);
      distmap.insert(pp);

    }
   
  } // i

  if (_params.output_style == Params::STYLE_WSDDM) {
    _printWsddmHeader();
  } else if (_params.output_style == Params::STYLE_AOAWS){
    _printAoawsHeader();
  } else if (_params.output_style == Params::STYLE_WSDDM_ENGLISH){
    _printWsddmHeaderEng();
  } else if (_params.output_style == Params::STYLE_WSDDM_E_HTML){
      if((_ofile = fopen(_output_filename,"w")) == NULL) {
          fprintf(stderr,"Problem opening %s\n",_output_filename);
          perror("_printWsddmMetarEngHtml:");
  }
    _printWsddmHeaderEngHtml();
  } 
  
  // print out the number required

  distmap_t::iterator ii;
  int nprint = 0;
  for (ii = distmap.begin(); ii != distmap.end(); ii++) {

    // check number of items printed

    nprint++;
    if (nprint > _params.nmax_items_print) {
      break;
    }

    // get chunk

    int chunkIndex = (*ii).second;
    const Spdb::chunk_t &chunk = chunks[chunkIndex];
    WxObs metar;
    metar.disassemble(chunk.data, chunk.len);
    
    if (_params.output_style == Params::STYLE_WSDDM) {
      _printWsddmMetar(metar);
    } else if (_params.output_style == Params::STYLE_AOAWS){
      _printAoawsMetar(metar);
    } else if (_params.output_style == Params::STYLE_WSDDM_ENGLISH){
      _printWsddmMetarEng(metar);
    } else if (_params.output_style == Params::STYLE_WSDDM_E_HTML){
      _printWsddmMetarEngHtml(metar);
    }
    
  }

  // Close out the table in HTML mode.
  if (_params.output_style == Params::STYLE_WSDDM_E_HTML){
      fprintf(_ofile, "</TABLE>\n");
    
      fclose(_ofile);
      _ofile = NULL;
  }

  // Print no data message, if appropriate.

  if (_spdb.getNChunks() == 0){
    cout << _params.no_data_message << endl;
  }

}

void MetarSelect::_printWsddmHeader()

{

  // Start by printing 5 blank lines to scroll the old data off
  // the display.
 
  for (int i = 0; i < 5; i++) {
    fprintf(stdout, "\n");
  }

  fprintf(stdout, "STN UTC  TMP DEW DIR SPD GST VSBY  CEIL WEATHER\n");
  fprintf(stdout, "           C   C deg  kt  kt   mi    ft\n");
  fprintf(stdout, "=== ==== === === === === === ====  ==== =======\n");

}

void MetarSelect::_printWsddmHeaderEng()

{

  // Start by printing 5 blank lines to scroll the old data off
  // the display.
 
  for (int i = 0; i < 5; i++) {
    fprintf(stdout, "\n");
  }

  fprintf(stdout, "STN UTC  TMP DEW DIR SPD GST VSBY  CEIL WEATHER\n");
  fprintf(stdout, "           F   F deg mph mph   mi    ft\n");
  fprintf(stdout, "=== ==== === === === === === ====  ==== =======\n");

}

  
void MetarSelect::_printAoawsHeader()

{

  // Start by printing 5 blank lines to scroll the old data off
  // the display.
 
  for (int i = 0; i < 5; i++) {
    fprintf(stdout, "\n");
  }

  fprintf(stdout, "NAME TIME W/D W/S GST  VIS   WEATHER CEIL TMP DEW  QNH\n");
  fprintf(stdout, "          deg  kt  kt    m             FL   C   C  hPa\n");
  fprintf(stdout, "==== ==== === === === ====   ======= ==== === === ====\n");

}
  
void MetarSelect::_printWsddmHeaderEngHtml()

{

  fprintf(_ofile, "<TABLE>\n <TR>");
    fprintf(_ofile, "<TH> STN </TH> <TH> UTC </TH> <TH> TMP </TH> <TH> DEW </TH>");
    fprintf(_ofile, "<TH> DIR </TH> <TH> SPD </TH> <TH> GST </TH> <TH> VSBY </TH>");
    fprintf(_ofile, "<TH> CEIL </TH> <TH> WEATHER </TH> \n");
    fprintf(_ofile, "</TR>\n <TR>");
    fprintf(_ofile, "<TH>&nbsp;</TH> <TH>&nbsp;</TH> <TH> F </TH> <TH> F </TH>");
    fprintf(_ofile, "<TH> deg </TH> <TH> mph </TH> <TH> mph </TH> <TH> mi </TH>");
    fprintf(_ofile, "<TH> ft </TH> <TH>&nbsp;</TH> \n");
  fprintf(_ofile, "</TR>\n");

}

void MetarSelect::_printWsddmMetarEngHtml(WxObs &metar)

{

  fprintf(_ofile, "<TR>");

  // station name (don't print the first character)

  fprintf(_ofile, "<TD>%3s</TD>", &metar.getStationId().c_str()[1]);
    
  // report time
  
  date_time_t *time_struct = udate_time(metar.getObservationTime());
  fprintf(_ofile, "<TD>%02d%02d</TD>",
	  time_struct->hour, time_struct->min);
    
  // temperature
  
  if (metar.getTempC() == WxObs::missing) {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  } else {
	double val = metar.getTempC() * 9.0 / 5.0 + 32.0;
    fprintf(_ofile, "<TD>%3.0f</TD>", val);
  }
  
  // dew point
  
  if (metar.getDewpointC() == WxObs::missing) {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  } else {
	double val = metar.getDewpointC() * 9.0 / 5.0 + 32.0;
    fprintf(_ofile, "<TD>%3.0f</TD>", val);
  }
  
  // wind direction  - output to nearest 10 degrees
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  } else if (signbit(metar.getWindDirnDegt())) { // variable
    fprintf(_ofile, "<TD>VRB</TD>");
  } else {
    fprintf(_ofile, "<TD>%3.0f</TD>", metar.getWindDirnDegt());
  }
  
  // wind speed

  if (metar.getWindSpeedMps() == WxObs::missing) {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  } else {
    fprintf(_ofile, "<TD>%3.0f</TD>", metar.getWindSpeedMps() / MPH_TO_MS);
  }
    
  // wind gust
  
  double windgust = metar.getWindGustMps()  / MPH_TO_MS;
    
  if (metar.getWindGustMps() != WxObs::missing && windgust > 5.0) {
    fprintf(_ofile, "<TD>G%3.0f</TD>", windgust);
  } else {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  }
    
  // visibility
  
  double vis_miles = metar.getVisibilityKm() / KM_PER_MI;
  double integral_val;
  double frac_val = modf(vis_miles, &integral_val);

  if (metar.getVisibilityKm() == WxObs::missing) {
    fprintf(_ofile, "<TD>&nbsp;</TD>");
  } else if (vis_miles > 4 || frac_val < 0.001 || frac_val > 0.999) {
    fprintf(_ofile, "<TD>%4.0f</TD>", vis_miles);
  } else {
    fprintf(_ofile, "<TD> %4.2f</TD>", vis_miles);
  }
    
  // ceiling

  if (metar.getCeilingKm() == WxObs::missing) {
    fprintf(_ofile, "<TD> &nbsp;</TD>");
  } else {
	if(metar.getCeilingKm() == 30.0)  { // Indeterminate
      fprintf(_ofile, "<TD> - </TD>");
	} else {
      int printed_int_value =
        (int)(_nearest(((metar.getCeilingKm() / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
      fprintf(_ofile, "<TD>%5d</TD>", printed_int_value);
	}
  }
    
  // weather string
  
  fprintf(_ofile, "<TD>%s</TD>",
          _truncateWxStr(metar.getMetarWx(), 2).c_str());

  fprintf(_ofile, "</TR>\n");

}

void MetarSelect::_printWsddmMetarEng(WxObs &metar)

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
	double val = metar.getTempC() * 9.0 / 5.0 + 32.0;
    fprintf(stdout, " %3.0f", val);
  }
  
  // dew point
  
  if (metar.getDewpointC() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
	double val = metar.getDewpointC() * 9.0 / 5.0 + 32.0;
    fprintf(stdout, " %3.0f", val);
  }
  
  // wind direction  - output to nearest 10 degrees
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else if (signbit(metar.getWindDirnDegt())) { // variable
    fprintf(stdout, " VRB");
  } else {
    fprintf(stdout, " %3.0f", metar.getWindDirnDegt());
  }
  
  // wind speed

  if (metar.getWindSpeedMps() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else {
    fprintf(stdout, " %3.0f", metar.getWindSpeedMps() / MPH_TO_MS);
  }
    
  // wind gust
  
  double windgust = metar.getWindGustMps()  / MPH_TO_MS;
    
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
	if(metar.getCeilingKm() == 30.0)  { // Indeterminate
      fprintf(stdout, "   -  ");
	} else {
      int printed_int_value =
        (int)(_nearest(((metar.getCeilingKm() / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
      fprintf(stdout, " %5d", printed_int_value);
	}
  }
    
  // weather string

  fprintf(stdout, " %s\n", _truncateWxStr(metar.getMetarWx(), 2).c_str());

}
  
void MetarSelect::_printWsddmMetar(WxObs &metar)

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
  
  // wind direction  - output to nearest 10 degrees
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else if (signbit(metar.getWindDirnDegt())) { // variable
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
	if(metar.getCeilingKm() == 30.0)  { // Indeterminate
      fprintf(stdout, "   -  ");
	} else {
      int printed_int_value =
        (int)(_nearest(((metar.getCeilingKm() / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
      fprintf(stdout, " %5d", printed_int_value);
	}
  }
    
  // weather string

  fprintf(stdout, " %s\n", _truncateWxStr(metar.getMetarWx(), 2).c_str());
    
}

void MetarSelect::_printAoawsMetar(WxObs &metar)

{

  // station name
  
  fprintf(stdout, "%4s", metar.getStationId().c_str());
  
  // report time
  
  date_time_t *time_struct = udate_time(metar.getObservationTime());
  fprintf(stdout, " %02d%02d", time_struct->hour, time_struct->min);

  // wind direction  - output to nearest degree
  
  if (metar.getWindDirnDegt() == WxObs::missing) {
    fprintf(stdout, "    ");
  } else if (signbit(metar.getWindDirnDegt())) { // variable
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

  if (metar.getVisibilityKm() == WxObs::missing) {
    fprintf(stdout, "     ");
  } else {
    if (metar.getVisibilityIsMinimum()) {
      fprintf(stdout, _params.min_visibility_text);
    } else if (vis_m > 9998.0) {
      fprintf(stdout, _params.min_visibility_text);
    } else {
      fprintf(stdout, " %-4d", (int)(vis_m + 0.5) );
    }
  }

  // weather string
  // max length 9 chars
  // if longer than 9, put * in 9th position to indicate this

  char weather_str[16];
  STRncopy(weather_str, metar.getMetarWx().c_str(), 15);
  if (strlen(weather_str) > 9) {
    weather_str[8] = '*';
    weather_str[9] = '\0';
  }
  
  fprintf(stdout, " %9s", weather_str);

  // ceiling
  
  if (metar.getCeilingKm() == WxObs::missing) {
    fprintf(stdout, "     ");
  } else {
    double ceiling_km = metar.getCeilingKm();
    double ceiling_ft = (ceiling_km / KM_PER_MI) * FT_PER_MI;
    int ceiling_fl = (int) ((ceiling_ft / 100.0) + 0.5);
    if (metar.getCeilingIsMinimum()) {
      fprintf(stdout, _params.min_ceiling_text);
    } else if (ceiling_fl > 500) {
      fprintf(stdout, _params.min_ceiling_text);
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

  fprintf(stdout,"\n");
    
}

double MetarSelect::_nearest(double target, double delta)
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

//////////////////////////////////////////////
// truncate weather string

string MetarSelect::_truncateWxStr(const string &wxStr,
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

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void MetarSelect::_tokenize(const string &str,
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
