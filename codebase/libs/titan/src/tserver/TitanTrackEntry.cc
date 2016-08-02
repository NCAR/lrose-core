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
////////////////////////////////////////////////////////////////
// TitanTrackEntry.cc
//
// Track entry object for TitanServer
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////


#include <titan/TitanTrackEntry.hh>
#include <dataport/bigend.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanTrackEntry::TitanTrackEntry()

{

  MEM_zero(_entry);
  MEM_zero(_scan);
  MEM_zero(_gprops);

}

////////////////////////////////////////////////////////////
// destructor

TitanTrackEntry::~TitanTrackEntry()

{

}

////////////////////////////////////////////////////////////
// clear the object

void TitanTrackEntry::clear()

{
  
  MEM_zero(_entry);
  MEM_zero(_scan);
  MEM_zero(_gprops);

  _lprops.clear();
  _hist.clear();
  _runs.clear();
  _proj_runs.clear();

}

////////////////////////////////////////////////////////////
// assemble into buffer

void TitanTrackEntry::assemble(MemBuf &buf,
			       bool clear_buffer /* = false*/ ) const

{

  if (clear_buffer) {
    buf.free();
  }

  // make copy of entry, swap and add

  track_file_entry_t entry = _entry;
  BE_from_array_32(&entry, sizeof(entry));
  buf.add(&entry, sizeof(entry));

  // make copy of scan, swap and add

  storm_file_scan_header_t scan = _scan;
  BE_from_array_32(&scan, sizeof(scan) - _scan.nbytes_char);
  buf.add(&scan, sizeof(scan));

  // make copy of gprops, swap and add
  // Ensure that gprops has correct number of layers, hist and runs

  storm_file_global_props_t gprops = _gprops;
  gprops.n_layers = _lprops.size();
  gprops.n_dbz_intervals = _hist.size();
  gprops.n_runs = _runs.size();
  gprops.n_proj_runs = _proj_runs.size();
  BE_from_array_32(&gprops, sizeof(gprops));
  buf.add(&gprops, sizeof(gprops));

  // make copy of lprops and add

  for (size_t ii = 0; ii < _lprops.size(); ii++) {
    storm_file_layer_props_t lprops = _lprops[ii];
    BE_from_array_32(&lprops, sizeof(lprops));
    buf.add(&lprops, sizeof(lprops));
  }
  
  // make copy of hist and add

  for (size_t ii = 0; ii < _hist.size(); ii++) {
    storm_file_dbz_hist_t hist = _hist[ii];
    BE_from_array_32(&hist, sizeof(hist));
    buf.add(&hist, sizeof(hist));
  }
  
  // make copy of runs and add

  for (size_t ii = 0; ii < _runs.size(); ii++) {
    storm_file_run_t run = _runs[ii];
    BE_from_array_32(&run, sizeof(run));
    buf.add(&run, sizeof(run));
  }
  
  // make copy of proj_runs and add

  for (size_t ii = 0; ii < _proj_runs.size(); ii++) {
    storm_file_run_t proj_run = _proj_runs[ii];
    BE_from_array_32(&proj_run, sizeof(proj_run));
    buf.add(&proj_run, sizeof(proj_run));
  }

}

////////////////////////////////////////////////////////////
// disassemble from buffer
//
// Sets len_used to the length of the buffer used while disassembling.
//
// Returns 0 on success, -1 on failure

int TitanTrackEntry::disassemble(const void *buf, int buf_len,
				 int &len_used)

{

  // clear object

  clear();

  // check min length

  int minLen = sizeof(_entry) + sizeof(_scan) + sizeof(_gprops);
  if (buf_len < minLen) {
    cerr << "ERROR - TitanTrackEntry::disassemble" << endl;
    cerr << "  Buffer passed in too short" << endl;
    cerr << "  Min buffer length: " << minLen << endl;
    cerr << "  Actual buffer length: " << buf_len << endl;
    return -1;
  }

  ui08 *bptr = (ui08 *) buf;
  
  // copy in entry and swap

  _entry = *((track_file_entry_t *) bptr);
  BE_to_array_32(&_entry, sizeof(_entry));
  bptr += sizeof(_entry);
 
  // copy in scan and swap

  _scan = *((storm_file_scan_header_t *) bptr);
  si32 nbytes_char = _scan.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_scan, sizeof(_scan) - nbytes_char);
  bptr += sizeof(_scan);
 
  // copy in gprops and swap

  _gprops = *((storm_file_global_props_t *) bptr);
  BE_to_array_32(&_gprops, sizeof(_gprops));
  bptr += sizeof(_gprops);
 
  // check full length

  int fullLen = (minLen +
		 _gprops.n_layers * sizeof(storm_file_layer_props_t) +
		 _gprops.n_dbz_intervals * sizeof(storm_file_dbz_hist_t) +
		 _gprops.n_runs * sizeof(storm_file_run_t) +
		 _gprops.n_proj_runs * sizeof(storm_file_run_t));

  if (buf_len < fullLen) {
    cerr << "ERROR - TitanTrackEntry::disassemble" << endl;
    cerr << "  Buffer passed in too short" << endl;
    cerr << "  Min buffer length: " << fullLen << endl;
    cerr << "  Actual buffer length: " << buf_len << endl;
    return -1;
  }

  // copy in lprops, swap and add to vector
  
  for (int i = 0; i < _gprops.n_layers; i++) {
    storm_file_layer_props_t lprops = *((storm_file_layer_props_t *) bptr);
    BE_to_array_32(&lprops, sizeof(lprops));
    _lprops.push_back(lprops);
    bptr += sizeof(lprops);
  }

  // copy in hist, swap and add to vector
  
  for (int i = 0; i < _gprops.n_dbz_intervals; i++) {
    storm_file_dbz_hist_t hist = *((storm_file_dbz_hist_t *) bptr);
    BE_to_array_32(&hist, sizeof(hist));
    _hist.push_back(hist);
    bptr += sizeof(hist);
  }

  // copy in runs, swap and add to vector
  
  for (int i = 0; i < _gprops.n_runs; i++) {
    storm_file_run_t run = *((storm_file_run_t *) bptr);
    BE_to_array_32(&run, sizeof(run));
    _runs.push_back(run);
    bptr += sizeof(run);
  }

  // copy in proj_runs, swap and add to vector
  
  for (int i = 0; i < _gprops.n_proj_runs; i++) {
    storm_file_run_t proj_run = *((storm_file_run_t *) bptr);
    BE_to_array_32(&proj_run, sizeof(proj_run));
    _proj_runs.push_back(proj_run);
    bptr += sizeof(proj_run);
  }

  // set len used

  len_used = (int) (bptr - (ui08 *) buf);

  return 0;

}

////////////////////////////////////////////////////////////
// Print

void TitanTrackEntry::print(FILE *out,
			    int entry_num,
			    const storm_file_params_t &sparams,
			    const track_file_params_t &tparams)

{

  fprintf(out, "\n");
  fprintf(out, "    Track entry num: %d\n", entry_num);
  fprintf(out, "    ===============\n\n");
  
  RfPrintTrackEntry(out, "      ", entry_num, &tparams, &_entry);
  RfPrintStormScan(out, "      ", &sparams, &_scan);
  RfPrintStormProps(out, "      ", &sparams, &_scan, &_gprops);
  
  RfPrintStormLayer(out, "        ",
		    &sparams, &_scan, &_gprops, &_lprops[0]);

  RfPrintStormHist(out, "        ",
		   &sparams, &_gprops, &_hist[0]);

  RfPrintStormRuns(out, "        ", &_gprops, &_runs[0]);

  RfPrintStormProjRuns(out, "        ", &_gprops, &_proj_runs[0]);
  
}

////////////////////////////////////////////////////////////
// PrintXML

void TitanTrackEntry::printXML(FILE *out,
			    int entry_num,
			    const storm_file_params_t &sparams,
			    const track_file_params_t &tparams)

{

  fprintf(out, "\n");
  fprintf(out, "    <track_entry entry_number=\"%d\">\n", entry_num);
  
  RfPrintTrackEntryXML(out, "      ", entry_num, &tparams, &_entry);
  RfPrintStormScanXML(out, "      ", &sparams, &_scan);
  RfPrintStormPropsXML(out, "      ", &sparams, &_scan, &_gprops);
  
  RfPrintStormLayerXML(out, "        ",
		    &sparams, &_scan, &_gprops, &_lprops[0]);

  RfPrintStormHistXML(out, "        ",
		   &sparams, &_gprops, &_hist[0]);

  RfPrintStormRunsXML(out, "        ", &_gprops, &_runs[0]);

  RfPrintStormProjRunsXML(out, "        ", &_gprops, &_proj_runs[0]);
  fprintf(out, "    </track_entry>\n");
  
}

