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
// TitanStormFile.cc
//
// TitanStormFile class
//
// This class handles the file IO for TITAN storms.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////


#include <fcntl.h>
#include <cerrno>
#include <dataport/bigend.h>
#include <titan/TitanStormFile.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanStormFile::TitanStormFile()

{

  MEM_zero(_header);
  MEM_zero(_scan);
  _gprops = NULL;
  _lprops = NULL;
  _hist = NULL;
  _runs = NULL;
  _proj_runs = NULL;
  _scan_offsets = NULL;
  _storm_num = 0;

  _header_file = NULL;
  _data_file = NULL;

  _header_file_label = STORM_HEADER_FILE_TYPE;
  _data_file_label = STORM_DATA_FILE_TYPE;

  _max_scans = 0;
  _max_storms = 0;
  _max_layers = 0;
  _max_dbz_intervals = 0;
  _max_runs = 0;
  _max_proj_runs = 0;

}

////////////////////////////////////////////////////////////
// destructor

TitanStormFile::~TitanStormFile()

{
  FreeAll();
  CloseFiles();
}

////////////////////
// clear error string

void TitanStormFile::_clearErrStr()
{
  _errStr = "";
  TaStr::AddStr(_errStr, "ERROR at time: ", DateTime::str());
}

//////////////////////////////////////////////////////////////
//
// allocate space for the layer props
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocLayers(int n_layers)
     
{
  if (n_layers > _max_layers) {
    _max_layers = n_layers;
    _lprops = (storm_file_layer_props_t *)
      urealloc(_lprops, n_layers * sizeof(storm_file_layer_props_t));
  }
}

void TitanStormFile::FreeLayers()
     
{
  if (_lprops) {
    ufree(_lprops);
    _lprops = NULL;
    _max_layers = 0;
  }
}

//////////////////////////////////////////////////////////////
//
// allocate space for dbz hist
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocHist(int n_dbz_intervals)
     
{

  if (n_dbz_intervals > _max_dbz_intervals) {
    _max_dbz_intervals = n_dbz_intervals;
    _hist = (storm_file_dbz_hist_t *)
      urealloc(_hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  }

}

void TitanStormFile::FreeHist()
     
{

  if (_hist) {
    ufree (_hist);
    _hist = NULL;
    _max_dbz_intervals = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the runs
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocRuns(int n_runs)
     
{

  if (n_runs > _max_runs) {
    _max_runs = n_runs;
    _runs = (storm_file_run_t *)
      urealloc(_runs, n_runs * sizeof(storm_file_run_t));
  }

}

void TitanStormFile::FreeRuns()
     
{

  if (_runs) {
    ufree ((char *) _runs);
    _runs = NULL;
    _max_runs = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the proj runs
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocProjRuns(int n_proj_runs)
     
{

  if (n_proj_runs > _max_proj_runs) {
    _max_proj_runs = n_proj_runs;
    _proj_runs = (storm_file_run_t *)
      urealloc((char *) _proj_runs,
	       n_proj_runs * sizeof(storm_file_run_t));
  }

}

void TitanStormFile::FreeProjRuns()
     
{

  if (_proj_runs) {
    ufree ((char *) _proj_runs);
    _proj_runs = NULL;
    _max_proj_runs = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate gprops array
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocGprops(int nstorms)
     
{
  
  if (nstorms > _max_storms) {
    _max_storms = nstorms;
    _gprops = (storm_file_global_props_t *)
      urealloc(_gprops, nstorms * sizeof(storm_file_global_props_t));
  }

}

void TitanStormFile::FreeGprops()
     
{

  if (_gprops) {
    ufree ((char *) _gprops);
    _gprops = NULL;
    _max_storms = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the scan offset array.
//
//////////////////////////////////////////////////////////////

void TitanStormFile::AllocScanOffsets(int n_scans_needed)
     
{

  // allocate the required space plus a buffer so that 
  // we do not do too many reallocs
  
  if (n_scans_needed > _max_scans) {
    _max_scans = n_scans_needed + 100;
    _scan_offsets = (si32 *) urealloc
      (_scan_offsets, (_max_scans * sizeof(si32)));
  }

}

void TitanStormFile::FreeScanOffsets()
     
{

  if (_scan_offsets) {
    ufree(_scan_offsets);
    _scan_offsets = NULL;
    _max_scans = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// Free all arrays
//
//////////////////////////////////////////////////////////////

void TitanStormFile::FreeAll()
     
{

  FreeLayers();
  FreeHist();
  FreeRuns();
  FreeProjRuns();
  FreeGprops();
  FreeScanOffsets();

}


//////////////////////////////////////////////////////////////
//
// Opens the storm header and data files
//
// The storm header file path must have been set
//
//////////////////////////////////////////////////////////////

int TitanStormFile::OpenFiles(const char *mode,
			      const char *header_file_path,
			      const char *data_file_ext /* = NULL*/ )
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::OpenFiles\n";

  // close files
  
  CloseFiles();
  
  // open the header file - file path may change if it is compressed
  
  char hdr_file_path[MAX_PATH_LEN];
  STRncopy(hdr_file_path, header_file_path, MAX_PATH_LEN);
  if ((_header_file = ta_fopen_uncompress(hdr_file_path, mode)) == NULL) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open header file: ",
		  header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _header_file_path = hdr_file_path;
  
  // compute the data file name
   
  if (*mode == 'r') {

    // read the header if the file is opened for reading
    
    if (ReadHeader(false)) {
      return -1;
    }

    // compute the file path from the header file path and
    // the data file name
    
    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, _header_file_path.c_str(), MAX_PATH_LEN);
    
    // if dir path has slash, get pointer to that and end the sting
    // immediately after
    
    char *chptr;
    if ((chptr = strrchr(tmp_path, '/')) != NULL) {
      *(chptr + 1) = '\0';
      _data_file_path = tmp_path;
      _data_file_path += _header.data_file_name;
    } else {
      _data_file_path = _header.data_file_name;
    }
    
  } else {
    
    // file opened for writing, use ext to compute file name
    
    if (data_file_ext == NULL) {
      _errStr += "Must provide data file extension for file creation\n";
      return -1;
    }

    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, _header_file_path.c_str(), MAX_PATH_LEN);

    char *chptr;
    if ((chptr = strrchr(tmp_path, '.')) == NULL) {
      TaStr::AddStr(_errStr, "  Header file must have extension : ",
		    _header_file_path);
      return -1;
    }
    
    *(chptr + 1) = '\0';
    _data_file_path = tmp_path;
    _data_file_path += data_file_ext;

  } // if (*mode == 'r') 
    
  // open the data file - file path may change if it is compressed
  
  char dat_file_path[MAX_PATH_LEN];
  STRncopy(dat_file_path, _data_file_path.c_str(), MAX_PATH_LEN);
    
  if ((_data_file = ta_fopen_uncompress(dat_file_path, mode)) == NULL) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open storm data file: ",
		  _data_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _data_file_path = dat_file_path;

  // In write mode, write file labels
  
  if (*mode == 'w') {
    
    // header file
    
    char header_file_label[R_FILE_LABEL_LEN];
    MEM_zero(header_file_label);
    strcpy(header_file_label, _header_file_label.c_str());
    
    if (ufwrite(header_file_label, 1, R_FILE_LABEL_LEN,
		_header_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing header file label to: ",
		    _header_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

    // data file
    
    char data_file_label[R_FILE_LABEL_LEN];
    MEM_zero(data_file_label);
    strcpy(data_file_label, STORM_DATA_FILE_TYPE);
    _data_file_label = data_file_label;
    
    if (ufwrite(data_file_label, 1, R_FILE_LABEL_LEN,
		_data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing data file label to: ",
		    _data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } else {
    
    // read mode - read in data file label
    
    char data_file_label[R_FILE_LABEL_LEN];
    if (ufread(data_file_label, sizeof(char), R_FILE_LABEL_LEN,
	       _data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Reading data file label from: ",
		    _data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // check label
    
    if (_data_file_label != data_file_label) {
      _errStr +=
	"  Data file does not have the correct label\n";
      TaStr::AddStr(_errStr, "  File label is: ", data_file_label);
      TaStr::AddStr(_errStr, "  Should be: ", _data_file_label);
      return -1;
    }
    
  } // if (*mode == 'w') 

  return 0;

}

//////////////////////////////////////////////////////////////
//
// Closes the storm header and data files
//
//////////////////////////////////////////////////////////////

void TitanStormFile::CloseFiles()
     
{

  // unlock the header file

  UnlockHeaderFile();

  // close the header file
  
  if (_header_file != NULL) {
    fclose(_header_file);
    _header_file = (FILE *) NULL;
  }

  // close the data file
  
  if (_data_file != NULL) {
    fclose(_data_file);
    _data_file = (FILE *) NULL;
  }
  
}

//////////////////////////////////////////////////////////////
//
// Flush the storm header and data files
//
//////////////////////////////////////////////////////////////

void TitanStormFile::FlushFiles()
  
{
  
  fflush(_header_file);
  fflush(_data_file);

}

//////////////////////////////////////////////////////////////
//
// Put an advisory lock on the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::LockHeaderFile(const char *mode)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::LockHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _header_file_path);
  
  if (ta_lock_file_procmap(_header_file_path.c_str(),
			   _header_file, mode)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot lock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// Remove advisory lock from the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::UnlockHeaderFile()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::UnlockHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _header_file_path);
  
  if (ta_unlock_file(_header_file_path.c_str(),
		     _header_file)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot unlock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// reads in the storm_file_header_t structure from a storm
// properties file.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::ReadHeader(bool clear_error_str /* = true*/ )
     
{

  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanStormFile::ReadHeader\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _header_file_path);

  // rewind file
  
  fseek(_header_file, 0L, SEEK_SET);
  
  // read in header file label
  
  char header_file_label[R_FILE_LABEL_LEN];
  if (ufread(header_file_label, sizeof(char), R_FILE_LABEL_LEN,
	     _header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading header file label from: ",
		  _header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // check label
  
  if (_header_file_label != header_file_label) {
    _errStr +=
      "  Header file does not contain correct label.\n";
    TaStr::AddStr(_errStr, "  File label is: ", header_file_label);
    TaStr::AddStr(_errStr, "  Should be: ", _header_file_label);
    return -1;
  }
    
  // read in header
  
  if (ufread(&_header, sizeof(storm_file_header_t),
	     1, _header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading storm file header structure");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_header, (sizeof(storm_file_header_t) - nbytes_char));
  
  // allocate space for scan offsets array
  
  int n_scans = _header.n_scans;
  
  AllocScanOffsets(n_scans);
  
  // read in scan offsets
  
  if (ufread(_scan_offsets, sizeof(si32), n_scans,
	     _header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading storm file scan offsets");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the offset array from network byte order into host byte order
  
  BE_to_array_32(_scan_offsets, n_scans * sizeof(si32));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// read in the storm projected area runs
// Space for the array is allocated.
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::ReadProjRuns(int storm_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::ReadProjRuns\n";

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }
  
  // store storm number
  
  _storm_num = storm_num;
  
  // allocate mem
  
  int n_proj_runs = _gprops[storm_num].n_proj_runs;

  AllocProjRuns(n_proj_runs);
  
  // move to proj_run data position in file
  
  fseek(_data_file, _gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  // read in proj_runs
  
  if (ufread(_proj_runs, sizeof(storm_file_run_t), n_proj_runs,
	     _data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading proj runs, file: ", _data_file_path);
    TaStr::AddInt(_errStr, "  N runs: ", n_proj_runs);
    TaStr::AddInt(_errStr, "  Storm number: ", storm_num);
    TaStr::AddInt(_errStr, "  Scan number: ", _scan.scan_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode proj_runs from network byte order into host byte order
  
  BE_to_array_16(_proj_runs, n_proj_runs * sizeof(storm_file_run_t));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// read in the storm property data for a given storm in a scan
// Space for the arrays of structures is allocated as required.
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::ReadProps(int storm_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::ReadProps\n";
  TaStr::AddStr(_errStr, "  Reading storm props from file: ", _data_file_path);
  TaStr::AddInt(_errStr, "  Storm number: ", storm_num);
  TaStr::AddInt(_errStr, "  Scan number: ", _scan.scan_num);
  
  // store storm number
  
  _storm_num = storm_num;
  
  // allocate or realloc mem
  
  int n_layers = _gprops[storm_num].n_layers;
  int n_dbz_intervals = _gprops[storm_num].n_dbz_intervals;
  int n_runs = _gprops[storm_num].n_runs;
  int n_proj_runs = _gprops[storm_num].n_proj_runs;

  AllocLayers(n_layers);
  AllocHist(n_dbz_intervals);
  AllocRuns(n_runs);
  AllocProjRuns(n_proj_runs);

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }
  
  // move to layer data position in file
  
  fseek(_data_file, _gprops[storm_num].layer_props_offset, SEEK_SET);
  
  // read in layer props
  
  if (ufread(_lprops, sizeof(storm_file_layer_props_t),
	     n_layers, _data_file) != n_layers) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading layer props");
    TaStr::AddInt(_errStr, "  N layers: ", n_layers);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode layer props from network byte order into host byte order
  
  BE_to_array_32(_lprops, n_layers * sizeof(storm_file_layer_props_t));
  
  // move to hist data position in file
  
  fseek(_data_file, _gprops[storm_num].dbz_hist_offset, SEEK_SET);
  
  // read in histogram data
  
  if (ufread(_hist, sizeof(storm_file_dbz_hist_t),
	     n_dbz_intervals, _data_file) != n_dbz_intervals) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading dbz histogram");
    TaStr::AddInt(_errStr, "  N intervals: ", n_dbz_intervals);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode histogram data from network byte order into host byte order
  
  BE_to_array_32(_hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // move to run data position in file
  
  fseek(_data_file, _gprops[storm_num].runs_offset, SEEK_SET);
  
  // read in runs
  
  if (ufread(_runs, sizeof(storm_file_run_t),
	     n_runs, _data_file) != n_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading runs");
    TaStr::AddInt(_errStr, "  N runs: ", n_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode runs from network byte order into host byte order
  
  BE_to_array_16(_runs, n_runs * sizeof(storm_file_run_t));
  
  // move to proj_run data position in file
  
  fseek(_data_file, _gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  // read in proj_runs
  
  if (ufread(_proj_runs, sizeof(storm_file_run_t),
	     n_proj_runs, _data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading proj runs");
    TaStr::AddInt(_errStr, "  N proj runs: ", n_proj_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // decode proj_runs from network byte order into host byte order
  
  BE_to_array_16(_proj_runs, n_proj_runs * sizeof(storm_file_run_t));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// Read in the scan info for a particular scan in a storm properties
// file.
//
// If storm num is set, only the gprops for that storm is swapped
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::ReadScan(int scan_num, int storm_num /* = -1*/ )
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::ReadScan\n";
  TaStr::AddStr(_errStr, "  Reading scan from file: ", _data_file_path);
  TaStr::AddInt(_errStr, "  Scan number: ", scan_num);

  // move to scan position in file
  
  if (_scan_offsets && scan_num < _max_scans) {
    fseek(_data_file, _scan_offsets[scan_num], SEEK_SET);
  } else {
    return -1;
  }
  
  // read in scan struct
  
  storm_file_scan_header_t scan;
  if (ufread(&scan, sizeof(storm_file_scan_header_t),
	     1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the scan struct from network byte order into host byte order
  
  si32 nbytes_char = scan.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&scan, (sizeof(storm_file_scan_header_t) - nbytes_char));
  
  // allocate or reallocate
  
  int nstorms = scan.nstorms;
  AllocGprops(nstorms);
  
  // copy scan header into storm file index

  _scan = scan;
  
  // return early if nstorms is zero
  
  if (nstorms == 0) {
    return 0;
  }
  
  // move to gprops position in file
  
  fseek(_data_file, _scan.gprops_offset, SEEK_SET);
  
  // read in global props
  
  if (ufread(_gprops, sizeof(storm_file_global_props_t),
	     nstorms, _data_file) != nstorms) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading gprops");
    TaStr::AddInt(_errStr, "  nstorms: ", nstorms);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode global props from network byte order into host byte order

  if (storm_num >= 0) {
    BE_to_array_32(_gprops + storm_num, sizeof(storm_file_global_props_t));
  } else {
    BE_to_array_32(_gprops, nstorms * sizeof(storm_file_global_props_t));
  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// seek to the end of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanStormFile::SeekEndData()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::SeekEndData\n";
  TaStr::AddStr(_errStr, "  File: ", _data_file_path);

  if (fseek(_data_file, 0L, SEEK_END)) {

    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;

  } else {

    return 0;

  }

}

//////////////////////////////////////////////////////////////
//
// seek to the start of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanStormFile::SeekStartData()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::SeekStartData\n";
  TaStr::AddStr(_errStr, "  File: ", _data_file_path);

  if (fseek(_data_file, R_FILE_LABEL_LEN, SEEK_SET) != 0) {
    
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;

  } else {

    return 0;

  }

}

//////////////////////////////////////////////////////////////
//
// write the storm_file_header_t structure to a storm file.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::WriteHeader()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::WriteHeader\n";
  TaStr::AddStr(_errStr, "  File: ", _header_file_path);

  // get data file size

  fflush(_data_file);
  struct stat data_stat;
  ta_stat(_data_file_path.c_str(), &data_stat);
  _header.data_file_size = data_stat.st_size;
  
  // copy file label
  
  char file_label[R_FILE_LABEL_LEN];
  MEM_zero(file_label);
  strcpy(file_label, STORM_HEADER_FILE_TYPE);

  _header.major_rev = STORM_FILE_MAJOR_REV;
  _header.minor_rev = STORM_FILE_MINOR_REV;
  
  // set file time to gmt
  
  _header.file_time = time(NULL);
  
  // copy in the file names, checking whether the path has a
  // delimiter or not, and only copying after the delimiter
  
  const char *hptr = strrchr(_header_file_path.c_str(), '/');

  if (hptr != NULL) {
    strncpy(_header.header_file_name, (hptr + 1), R_LABEL_LEN);
  } else {
    strncpy(_header.header_file_name, _header_file_path.c_str(), R_LABEL_LEN);
  }
  
  const char *dptr = strrchr(_data_file_path.c_str(), '/');
  if (dptr != NULL) {
    strncpy(_header.data_file_name, (dptr + 1), R_LABEL_LEN);
  } else {
    strncpy(_header.data_file_name, _data_file_path.c_str(), R_LABEL_LEN);
  }
      
  // make local copies of the global file header and scan offsets
  
  storm_file_header_t header = _header;
  int n_scans = _header.n_scans;

  TaArray<si32> offsetArray;
  si32 *scan_offsets = offsetArray.alloc(n_scans);
  memcpy (scan_offsets, _scan_offsets, n_scans * sizeof(si32));
  
  // encode the header and scan offset array into network byte order
  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  header.nbytes_char = STORM_FILE_HEADER_NBYTES_CHAR;
  BE_from_array_32(&header,
		   (sizeof(storm_file_header_t) - header.nbytes_char));
  BE_from_array_32(scan_offsets, n_scans * sizeof(si32));
  
  // write label to file
  
  fseek(_header_file, 0, SEEK_SET);
  ustr_clear_to_end(file_label, R_FILE_LABEL_LEN);

  if (ufwrite(file_label, sizeof(char), R_FILE_LABEL_LEN,
	      _header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing label");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write header to file
  
  if (ufwrite(&header, sizeof(storm_file_header_t),
	      1, _header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write scan offsets to file
  
  if (ufwrite(scan_offsets, sizeof(si32),
	      n_scans, _header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing scan offsets");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  FlushFiles();

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// write the storm layer property and histogram data for a storm,
// at the end of the file.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::WriteProps(int storm_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::WriteProps\n";
  TaStr::AddStr(_errStr, "  File: ", _data_file_path);

  int n_layers = _gprops[storm_num].n_layers;
  int n_dbz_intervals = _gprops[storm_num].n_dbz_intervals;
  int n_runs = _gprops[storm_num].n_runs;
  int n_proj_runs = _gprops[storm_num].n_proj_runs;
  
  // set layer props offset
  
  fseek(_data_file, 0, SEEK_END);
  int offset = ftell(_data_file);
  _gprops[storm_num].layer_props_offset = offset;
  
  // if this is the first storm, store the first_offset value
  // in the scan header
  
  if (storm_num == 0) {
    _scan.first_offset = offset;
  }
  
  // copy layer props to local array
  
  TaArray<storm_file_layer_props_t> lpropsArray;
  storm_file_layer_props_t *lprops = lpropsArray.alloc(n_layers);
  memcpy (lprops, _lprops,
          n_layers * sizeof(storm_file_layer_props_t));
  
  // code layer props into network byte order from host byte order
  
  BE_from_array_32(lprops, n_layers * sizeof(storm_file_layer_props_t));
  
  // write layer props
  
  if (ufwrite(lprops, sizeof(storm_file_layer_props_t),
	      n_layers, _data_file) != n_layers) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing layers");
    TaStr::AddInt(_errStr, "  n_layers: ", n_layers);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set dbz hist offset
  
  _gprops[storm_num].dbz_hist_offset = ftell(_data_file);
  
  // copy histogram data to local variable

  TaArray<storm_file_dbz_hist_t> histArray;
  storm_file_dbz_hist_t *hist = histArray.alloc(n_dbz_intervals);
  memcpy (hist, _hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // encode histogram data to network byte order from host byte order
  
  BE_from_array_32(hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // write in histogram data
  
  if (ufwrite(hist, sizeof(storm_file_dbz_hist_t),
	      n_dbz_intervals, _data_file) != n_dbz_intervals) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing hist");
    TaStr::AddInt(_errStr, "  n_dbz_intervals: ", n_dbz_intervals);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set runs offset
  
  _gprops[storm_num].runs_offset = ftell(_data_file);
  
  // copy runs to local array

  TaArray<storm_file_run_t> runsArray;
  storm_file_run_t *runs = runsArray.alloc(n_runs);
  memcpy (runs, _runs, n_runs * sizeof(storm_file_run_t));
  
  // code run props into network byte order from host byte order
  
  BE_from_array_16(runs, n_runs * sizeof(storm_file_run_t));
  
  // write runs
  
  if (ufwrite(runs, sizeof(storm_file_run_t),
	      n_runs, _data_file) != n_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing runs");
    TaStr::AddInt(_errStr, "  n_runs: ", n_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set proj_runs offset
  
  _gprops[storm_num].proj_runs_offset = ftell(_data_file);
  
  // copy proj_runs to local array

  TaArray<storm_file_run_t> projRunsArray;
  storm_file_run_t *proj_runs = projRunsArray.alloc(n_proj_runs);
  memcpy (proj_runs, _proj_runs,
          n_proj_runs * sizeof(storm_file_run_t));
  
  // code run props into network byte order from host byte order
  
  BE_from_array_16(proj_runs,
		   n_proj_runs * sizeof(storm_file_run_t));
  
  // write proj_runs
  
  if (ufwrite(proj_runs, sizeof(storm_file_run_t),
	      n_proj_runs, _data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing proj_runs");
    TaStr::AddInt(_errStr, "  n_proj_runs: ", n_proj_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// write scan header and global properties for a particular scan
// in a storm properties file.
// Performs the writes from the end of the file.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanStormFile::WriteScan(int scan_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::WriteScan\n";
  TaStr::AddStr(_errStr, "  File: ", _data_file_path);
  
  // Move to the end of the file before beginning the write.

  fseek(_data_file, 0, SEEK_END);

  // if nstorms is greater than zero, write global props to file
  
  int nstorms = _scan.nstorms;
  
  if (nstorms > 0) {
    
    // get gprops position in file
    
    _scan.gprops_offset = ftell(_data_file);
    
    // make local copy of gprops and encode into network byte order

    TaArray<storm_file_global_props_t> gpropsArray;
    storm_file_global_props_t *gprops = gpropsArray.alloc(nstorms);
    memcpy (gprops, _gprops, nstorms * sizeof(storm_file_global_props_t));
    BE_from_array_32(gprops,
		     nstorms * sizeof(storm_file_global_props_t));
    
    // write in global props
    
    if (ufwrite(gprops, sizeof(storm_file_global_props_t),
		nstorms, _data_file) != nstorms) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Writing gprops");
      TaStr::AddInt(_errStr, "  nstorms: ", nstorms);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

  } // if (nstorms > 0) 
  
  // get scan position in file
  
  AllocScanOffsets(scan_num + 1);
  long offset = ftell(_data_file);
  _scan_offsets[scan_num] = offset;
  
  // set last scan offset
  
  _scan.last_offset = offset + sizeof(storm_file_scan_header_t) - 1;
  
  // copy scan header to local variable, and encode. Note that the 
  // character data at the end of the struct is not encoded
  
  storm_file_scan_header_t scan = _scan;
  scan.grid.nbytes_char =  TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  scan.nbytes_char = scan.grid.nbytes_char;
  
  ustr_clear_to_end(scan.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32(&scan,
		   (sizeof(storm_file_scan_header_t) - scan.nbytes_char));
  
  // write scan struct
  
  if (ufwrite(&scan, sizeof(storm_file_scan_header_t), 1,
	      _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing scan header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// convert ellipse parameters from deg to km,
// for those which were computed from latlon grids.
//
//////////////////////////////////////////////////////////////

void TitanStormFile::_convert_ellipse_2km(const titan_grid_t &tgrid,
					  double centroid_x,
					  double centroid_y,
					  fl32 &orientation,
					  fl32 &minor_radius,
					  fl32 &major_radius)
  
{

  // only convert for latlon projection
  
  if (tgrid.proj_type == TITAN_PROJ_LATLON) {
    
    double centroid_lon, centroid_lat;
    double major_orient_rad, major_lon, major_lat;
    double minor_orient_rad, minor_lon, minor_lat;
    double dist, theta;
    double orientation_km, major_radius_km, minor_radius_km;
    double sin_major, cos_major;
    double sin_minor, cos_minor;

    centroid_lon = centroid_x;
    centroid_lat = centroid_y;
    
    major_orient_rad = orientation * DEG_TO_RAD;
    ta_sincos(major_orient_rad, &sin_major, &cos_major);
    major_lon = centroid_lon + major_radius * sin_major;
    major_lat = centroid_lat + major_radius * cos_major;
    
    minor_orient_rad = (orientation + 270.0) * DEG_TO_RAD;
    ta_sincos(minor_orient_rad, &sin_minor, &cos_minor);
    minor_lon = centroid_lon + minor_radius * sin_minor;
    minor_lat = centroid_lat + minor_radius * cos_minor;

    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     major_lat, major_lon,
		     &dist, &theta);

    orientation_km = theta;
    major_radius_km = dist;
    
    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     minor_lat, minor_lon,
		     &dist, &theta);
    
    minor_radius_km = dist;
    
    orientation = orientation_km;
    major_radius = major_radius_km;
    minor_radius = minor_radius_km;

  }

}
		     
//////////////////////////////////////////////////////////////
//
// Convert the ellipse data (orientation, major_radius and minor_radius)
// for a a gprops struct to local (km) values.
// This applies to structs which were derived from lat-lon grids, for
// which some of the fields are in deg instead of km.
// It is a no-op for other projections.
//
// See Note 3 in storms.h
//
//////////////////////////////////////////////////////////////

void TitanStormFile::GpropsEllipses2Km(const storm_file_scan_header_t &scan,
				       storm_file_global_props_t &gprops)
     
{
  
  // convert the ellipses as appropriate

  _convert_ellipse_2km(scan.grid,
		       gprops.precip_area_centroid_x,
		       gprops.precip_area_centroid_y,
		       gprops.precip_area_orientation,
		       gprops.precip_area_minor_radius,
		       gprops.precip_area_major_radius);
  
  _convert_ellipse_2km(scan.grid,
		       gprops.proj_area_centroid_x,
		       gprops.proj_area_centroid_y,
		       gprops.proj_area_orientation,
		       gprops.proj_area_minor_radius,
		       gprops.proj_area_major_radius);

}

//////////////////////////////////////////////////////////////
//
// Convert the (x,y) km locations in a gprops struct to lat-lon.
// This applies to structs which were computed for non-latlon 
// grids. It is a no-op for lat-lon grids.
//
// See Note 3 in storms.h
//
//////////////////////////////////////////////////////////////

void TitanStormFile::GpropsXY2LatLon(const storm_file_scan_header_t &scan,
				     storm_file_global_props_t &gprops)
  
{
  
  const titan_grid_t  &tgrid = scan.grid;

  switch (tgrid.proj_type) {
    
  case TITAN_PROJ_LATLON:
    break;
    
  case TITAN_PROJ_FLAT:
    {
      double lat, lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.vol_centroid_x,
			gprops.vol_centroid_y,
			&lat, &lon);
      gprops.vol_centroid_y = lat;
      gprops.vol_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.refl_centroid_x,
			gprops.refl_centroid_y,
			&lat, &lon);
      gprops.refl_centroid_y = lat;
      gprops.refl_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.precip_area_centroid_x,
			gprops.precip_area_centroid_y,
			&lat, &lon);
      gprops.precip_area_centroid_y = lat;
      gprops.precip_area_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.proj_area_centroid_x,
			gprops.proj_area_centroid_y,
			&lat, &lon);
      gprops.proj_area_centroid_y = lat;
      gprops.proj_area_centroid_x = lon;
      break;
    }
    
  case TITAN_PROJ_LAMBERT_CONF:
    {
      double lat, lon;
      PJGstruct *ps = PJGs_lc2_init(tgrid.proj_origin_lat,
				    tgrid.proj_origin_lon,
				    tgrid.proj_params.lc2.lat1,
				    tgrid.proj_params.lc2.lat2);
      if (ps != NULL) {
	PJGs_lc2_xy2latlon(ps,
			   gprops.vol_centroid_x,
			   gprops.vol_centroid_y,
			   &lat, &lon);
	gprops.vol_centroid_y = lat;
	gprops.vol_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.refl_centroid_x,
			   gprops.refl_centroid_y,
			   &lat, &lon);
	gprops.refl_centroid_y = lat;
	gprops.refl_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.precip_area_centroid_x,
			   gprops.precip_area_centroid_y,
			   &lat, &lon);
	gprops.precip_area_centroid_y = lat;
	gprops.precip_area_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.proj_area_centroid_x,
			   gprops.proj_area_centroid_y,
			   &lat, &lon);
	gprops.proj_area_centroid_y = lat;
	gprops.proj_area_centroid_x = lon;
	free(ps);
      }
      break;
    }
  
  default:
    break;
    
  } // switch 
  
}

///////////////////////////////////////////////////////////////
// Truncate header file
//
// Returns 0 on success, -1 on failure.

int TitanStormFile::TruncateHeaderFile(int length)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::TruncateHeaderFile\n";
  return (_truncate(_header_file, _header_file_path, length));

}

///////////////////////////////////////////////////////////////
// Truncate data file
//
// Returns 0 on success, -1 on failure.

int TitanStormFile::TruncateDataFile(int length)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanStormFile::TruncateDataFile\n";
  return (_truncate(_data_file, _data_file_path, length));

}

///////////////////////////////////////////////////
//
// Truncate open file - close, truncate and reopen
//
// Returns 0 on success, -1 on failure
//

int TitanStormFile::_truncate(FILE *&fd, const string &path, int length)
     
{
  
  TaStr::AddStr(_errStr, "ERROR - ", "TitanStormFile::_truncate");
  TaStr::AddStr(_errStr, "  File: ", path);
  
  // close the buffered file
  
  fclose(fd);
  
  // open for low-level io
  
  int low_fd;
  if ((low_fd = open(path.c_str(), O_WRONLY)) < 0) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ",
		  "Cannot open file - low level - for truncation.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // truncate the file
  
  if (ftruncate(low_fd, length) != 0) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot truncate file.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return(-1);
  }
  
  // close low-level io
  
  close(low_fd);
  
  // re-open the file for buffered i/o
  
  if ((fd = fopen(path.c_str(), "r+")) == NULL) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot reopen file.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return(-1);
  }

  return (0);
  
}
