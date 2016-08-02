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
// TitanTrackFile.cc
//
// TitanTrackFile class
//
// This class handles the file IO for TITAN tracks.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////


#include <cerrno>
#include <dataport/bigend.h>
#include <titan/TitanTrackFile.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanTrackFile::TitanTrackFile()

{

  MEM_zero(_header);
  MEM_zero(_simple_params);
  MEM_zero(_complex_params);
  MEM_zero(_entry);

  _scan_index = NULL;
  _scan_entries = NULL;
  _track_utime = NULL;
  
  _complex_track_nums = NULL;
  _complex_track_offsets = NULL;
  _simple_track_offsets = NULL;
  _nsimples_per_complex = NULL;
  _simples_per_complex_offsets = NULL;
  _simples_per_complex = NULL;

  _header_file_label = TRACK_HEADER_FILE_TYPE;
  _data_file_label = TRACK_DATA_FILE_TYPE;

  _header_file = NULL;
  _data_file = NULL;

  _first_entry = true;

  _n_scan_entries = 0;
  _lowest_avail_complex_slot = 0;

  _n_simple_allocated = 0;
  _n_complex_allocated = 0;
  _n_simples_per_complex_allocated = 0;
  _n_scan_entries_allocated = 0;
  _n_scan_index_allocated = 0;
  _n_utime_allocated = 0;

}

////////////////////////////////////////////////////////////
// destructor

TitanTrackFile::~TitanTrackFile()

{
  FreeAll();
  CloseFiles();
}

////////////////////////////////////////////////////////////
// data access

const simple_track_params_t &TitanTrackFile::simple_params() const { 
  return _simple_params;
}

const complex_track_params_t &TitanTrackFile::complex_params() const {
  return _complex_params;
}

////////////////////
// clear error string

void TitanTrackFile::_clearErrStr()
{
  _errStr = "";
  TaStr::AddStr(_errStr, "ERROR at time: ", DateTime::str());
}

#define N_ALLOC 20

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocSimpleArrays()
//
// allocate space for the simple track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::AllocSimpleArrays(int n_simple_needed)
     
{

  if (_n_simple_allocated < n_simple_needed) {
    
    int n_start = _n_simple_allocated;

    int n_realloc = n_simple_needed + N_ALLOC;
    _n_simple_allocated = n_realloc;
    
    _simple_track_offsets = (si32 *) urealloc
      (_simple_track_offsets, n_realloc * sizeof(si32));
      
    _nsimples_per_complex = (si32 *) urealloc
      (_nsimples_per_complex, n_realloc * sizeof(si32));
    
    _simples_per_complex_offsets = (si32 *) urealloc
      (_simples_per_complex_offsets, n_realloc * sizeof(si32));
    
    _complex_track_offsets = (si32 *) urealloc
      (_complex_track_offsets, n_realloc * sizeof(si32));
    
    // initialize new elements to zero
  
    int n_new = _n_simple_allocated - n_start;

    memset (_simple_track_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_nsimples_per_complex + n_start, 0, n_new * sizeof(si32));
    memset (_simples_per_complex_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_complex_track_offsets + n_start, 0, n_new * sizeof(si32));
  
  } // if (_n_simple_allocated < n_simple_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeSimpleArrays()
//
// free space for the simple track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeSimpleArrays()
     
{
  
  if (_simples_per_complex) {
    for (int i = 0; i < _n_simples_per_complex_allocated; i++) {
      if(_simples_per_complex[i] != NULL) {
	ufree(_simples_per_complex[i]);
      }
    }
    ufree(_simples_per_complex);
    _simples_per_complex = NULL;
  }
  
  if (_simple_track_offsets) {
    ufree(_simple_track_offsets);
    _simple_track_offsets = NULL;
  }

  if (_nsimples_per_complex) {
    ufree(_nsimples_per_complex);
    _nsimples_per_complex = NULL;
  }

  if (_simples_per_complex_offsets) {
    ufree(_simples_per_complex_offsets);
    _simples_per_complex_offsets = NULL;
  }

  if (_complex_track_offsets) {
    ufree(_complex_track_offsets);
    _complex_track_offsets = NULL;
  }

  _n_simple_allocated = 0;

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocComplexArrays()
//
// allocate space for the complex track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::AllocComplexArrays(int n_complex_needed)
     
{

  if (_n_complex_allocated < n_complex_needed) {

    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
    
    int n_start = _n_complex_allocated;
    int n_realloc = n_complex_needed + N_ALLOC;
    _n_complex_allocated = n_realloc;
    
    _complex_track_nums = (si32 *) urealloc
      (_complex_track_nums, n_realloc * sizeof(si32));

    // initialize new elements to zero
  
    int n_new = n_realloc - n_start;
    memset (_complex_track_nums + n_start, 0, n_new * sizeof(si32));
    
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeComplexArrays()
//
// Free space for the complex track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeComplexArrays()
     
{

  if (_complex_track_nums) {
    ufree(_complex_track_nums);
    _complex_track_nums = NULL;
    _n_complex_allocated = 0;
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocSimplesPerComplex()
//
// allocate space for the array of pointers to simples_per_complex
//
///////////////////////////////////////////////////////////////////////////


void TitanTrackFile::AllocSimplesPerComplex(int n_simple_needed)
     
{

  if (_n_simples_per_complex_allocated < n_simple_needed) {
    
    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
    
    int n_start = _n_simples_per_complex_allocated;
    int n_realloc = n_simple_needed + N_ALLOC;
    _n_simples_per_complex_allocated = n_realloc;
    
    _simples_per_complex = (si32 **) urealloc
      (_simples_per_complex, n_realloc * sizeof(si32 *));
    
    // initialize new elements to zero
  
    int n_new = n_realloc - n_start;

    memset (_simples_per_complex + n_start,
	    0, n_new * sizeof(si32 *));
  
  } // if (_n_simples_per_complex_allocated < n_simple_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeSimplesPerComplex()
//
///////////////////////////////////////////////////////////////////////////


void TitanTrackFile::FreeSimplesPerComplex()
     
{
  if (_simples_per_complex) {
    for (int i = 0; i < _n_simples_per_complex_allocated; i++) {
      if (_simples_per_complex[i] != NULL) {
	ufree(_simples_per_complex[i]);
	_simples_per_complex[i] = NULL;
      }
    }
    ufree(_simples_per_complex);
    _simples_per_complex = NULL;
    _n_simples_per_complex_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocScanEntries()
//
// allocate mem for scan entries array
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::AllocScanEntries(int n_entries_needed)
  
{
  
  if (n_entries_needed > _n_scan_entries_allocated) {
    
    _scan_entries = (track_file_entry_t *) urealloc
      (_scan_entries, n_entries_needed * sizeof(track_file_entry_t));
    
    _n_scan_entries_allocated = n_entries_needed;
    
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeScanEntries()
//
// free scan entries
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeScanEntries()
     
{
  
  if (_scan_entries) {
    ufree(_scan_entries);
    _scan_entries = NULL;
    _n_scan_entries_allocated = 0;
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocScanIndex()
//
// allocate space for the scan index
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::AllocScanIndex(int n_scans_needed)
     
{

  if (_n_scan_index_allocated < n_scans_needed) {

    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
      
    int n_start = _n_scan_index_allocated;
    int n_realloc = n_scans_needed + N_ALLOC;
    _n_scan_index_allocated = n_realloc;

    _scan_index = (track_file_scan_index_t *) urealloc
      (_scan_index, n_realloc * sizeof(track_file_scan_index_t));
      
    // initialize new elements to zero
  
    int n_new = _n_scan_index_allocated - n_start;

    memset (_scan_index + n_start, 0,
	    n_new * sizeof(track_file_scan_index_t));
  
  } // if (_n_scan_index_allocated < n_scans_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeScanIndex()
//
// free space for the scan index
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeScanIndex()
     
{
  if (_scan_index) {
    ufree(_scan_index);
    _scan_index = NULL;
    _n_scan_index_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::AllocUtime()
//
// allocate array of track_utime_t structs
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::AllocUtime()
     
{

  if (_n_utime_allocated < _header.max_simple_track_num + 1) {
      
    _n_utime_allocated = _header.max_simple_track_num + 1;
    
    _track_utime = (track_utime_t *) urealloc
      (_track_utime, _n_utime_allocated * sizeof(track_utime_t));
      
    memset (_track_utime, 0,
	    _n_utime_allocated * sizeof(track_utime_t));
      
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeUtime()
//
// free space for the utime array
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeUtime()
     
{
  if (_track_utime) {
    ufree(_track_utime);
    _track_utime = NULL;
    _n_utime_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanTrackFile::FreeAll()
//
// free all arrays
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::FreeAll()
  
{

  FreeSimpleArrays();
  FreeComplexArrays();
  FreeSimplesPerComplex();
  FreeScanEntries();
  FreeScanIndex();
  FreeUtime();

}

///////////////////////////////////////////////////////////////////////////
//
// Open the track header and data files
//
// Returns 0 on success, -1 on error
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::OpenFiles(const char *mode,
			      const char *header_file_path,
			      const char *data_file_ext /* = NULL*/ )
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::OpenFiles\n";

  // close files

  CloseFiles();

  // open the header file - file path may change if it is compressed
  
  char hdr_file_path[MAX_PATH_LEN];
  STRncopy(hdr_file_path, header_file_path, MAX_PATH_LEN);
  if ((_header_file = ta_fopen_uncompress(hdr_file_path, mode)) == NULL) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open header file: ", header_file_path);
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
    strncpy(tmp_path, header_file_path, MAX_PATH_LEN);

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
    
  // open the data file
  
  char dat_file_path[MAX_PATH_LEN];
  STRncopy(dat_file_path, _data_file_path.c_str(), MAX_PATH_LEN);
    
  if ((_data_file = ta_fopen_uncompress(dat_file_path, mode)) == NULL) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open data file: ",
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
    strcpy(header_file_label, TRACK_HEADER_FILE_TYPE);
    _header_file_label = header_file_label;
    
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
    strcpy(data_file_label, TRACK_DATA_FILE_TYPE);
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
// Closes the track header and data files
//
//////////////////////////////////////////////////////////////

void TitanTrackFile::CloseFiles()
     
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
// Flush the track header and data files
//
//////////////////////////////////////////////////////////////

void TitanTrackFile::FlushFiles()
  
{
  
  fflush(_header_file);
  fflush(_data_file);

}

//////////////////////////////////////////////////////////////
//
// TitanTrackFile::LockHeaderFile()
//
// Put an advisory lock on the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanTrackFile::LockHeaderFile(const char *mode)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::LockHeaderFile\n";
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
// TitanTrackFile::UnlockHeaderFile()
//
// Remove advisory lock from the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanTrackFile::UnlockHeaderFile()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::UnlockHeaderFile\n";
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

///////////////////////////////////////////////////////////////////////////
//
// Read in the track_file_header_t structure from a track file.
// Read in associated arrays.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadHeader(bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanTrackFile::ReadHeader\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _header_file_path);

  // rewind file
  
  fseek(_header_file, 0L, SEEK_SET);
  
  // read in file label
  
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
  
  if (ufread(&_header, sizeof(track_file_header_t),
	     1, _header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading file header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_header, (sizeof(track_file_header_t) - nbytes_char));

  int n_complex_tracks = _header.n_complex_tracks;
  int n_simple_tracks = _header.n_simple_tracks;
  int n_scans = _header.n_scans;
  
  // check that the constants in use when the file was written are
  // less than or the same as those in use now
  
  if (_header.max_parents != MAX_PARENTS) {
    TaStr::AddStr(_errStr, "  ", "MAX_PARENTS has changed");
    TaStr::AddInt(_errStr, "  _header.max_parents: ", _header.max_parents);
    TaStr::AddInt(_errStr, "  MAX_PARENTS: ", MAX_PARENTS);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  if (_header.max_children != MAX_CHILDREN) {
    TaStr::AddStr(_errStr, "  ", "MAX_CHILDREN has changed");
    TaStr::AddInt(_errStr, "  _header.max_children: ", _header.max_children);
    TaStr::AddInt(_errStr, "  MAX_CHILDREN: ", MAX_CHILDREN);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  if (_header.max_nweights_forecast != MAX_NWEIGHTS_FORECAST) {
    TaStr::AddStr(_errStr, "  ", "MAX_NWEIGHTS_FORECAST has changed");
    TaStr::AddInt(_errStr, "  _header.max_nweights_forecast: ",
		  _header.max_nweights_forecast);
    TaStr::AddInt(_errStr, "  MAX_NWEIGHTS_FORECAST: ",
		  MAX_NWEIGHTS_FORECAST);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  // alloc arrays

  AllocComplexArrays(n_complex_tracks);
  AllocSimpleArrays(n_simple_tracks);
  AllocScanIndex(n_scans);
  
  // read in complex track num array

  if (ufread(_complex_track_nums, sizeof(si32),
	     n_complex_tracks, _header_file) != n_complex_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex track nums");
    TaStr::AddInt(_errStr, "  n_complex_tracks", n_complex_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_complex_track_nums, n_complex_tracks * sizeof(si32));
  
  // read in complex track offsets
  
  if (ufread(_complex_track_offsets, sizeof(si32),
	     n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex track offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_complex_track_offsets, n_simple_tracks * sizeof(si32));
  
  // read in simple track offsets
  
  if (ufread(_simple_track_offsets, sizeof(si32),
	     n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simple track offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_simple_track_offsets, n_simple_tracks * sizeof(si32));
  
  // read in scan index array
  
  if (ufread(_scan_index, sizeof(track_file_scan_index_t),
	     n_scans, _header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading scan index array");
    TaStr::AddInt(_errStr, "  n_scans", n_scans);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_scan_index, n_scans * sizeof(track_file_scan_index_t));
  
  // read in nsimples_per_complex
  
  if (ufread(_nsimples_per_complex, sizeof(si32),
	     n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading nsimples_per_complex");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_nsimples_per_complex, n_simple_tracks * sizeof(si32));
  
  // read in simples_per_complex_offsets
  
  if (ufread(_simples_per_complex_offsets, sizeof(si32),
	     n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simples_per_complex_offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_simples_per_complex_offsets,
		 n_simple_tracks * sizeof(si32));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// Read in the track_file_header_t and scan_index array.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadScanIndex(bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanTrackFile::ReadScanIndex\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _header_file_path);

  // rewind file
  
  fseek(_header_file, 0L, SEEK_SET);
  
  // read in file label
  
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
  
  if (ufread(&_header, sizeof(track_file_header_t),
	     1, _header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading file header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_header, (sizeof(track_file_header_t) - nbytes_char));

  int n_complex_tracks = _header.n_complex_tracks;
  int n_simple_tracks = _header.n_simple_tracks;
  int n_scans = _header.n_scans;
  
  // seek ahead

  long nbytesSkip = (n_complex_tracks * sizeof(si32) +
		     2 * n_simple_tracks * sizeof(si32));
		    
  if (fseek(_header_file, nbytesSkip, SEEK_CUR)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Seeking over  arrays");
    TaStr::AddInt(_errStr, "  Cannot seek ahead by nbytes: ", nbytesSkip);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // alloc array

  AllocScanIndex(n_scans);

  // read in scan index array
  
  if (ufread(_scan_index, sizeof(track_file_scan_index_t),
	     n_scans, _header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading scan index array");
    TaStr::AddInt(_errStr, "  n_scans", n_scans);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_scan_index, n_scans * sizeof(track_file_scan_index_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// reads in the parameters for a complex track
//
// For normal reads, read_simples_per_complex should be set true. This
// is only set FALSE in Titan, which creates the track files.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadComplexParams(int track_num,
				      bool read_simples_per_complex,
				      bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanTrackFile::ReadComplexParams\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);
  TaStr::AddInt(_errStr, "  track_num", track_num);

  // move to offset in file
  
  if (_complex_track_offsets[track_num] == 0) {
    return -1;
  }
  
  fseek(_data_file, _complex_track_offsets[track_num], SEEK_SET);
  
  // read in params
  
  if (ufread(&_complex_params, sizeof(complex_track_params_t),
	     1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex_track_params");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(&_complex_params, sizeof(complex_track_params_t));
  
  // If read_simples_per_complex is set,
  // read in simples_per_complex array, which indicates which
  // simple tracks are part of this complex track.

  if (read_simples_per_complex) {
    
    int nsimples = _nsimples_per_complex[track_num];
    
    AllocSimplesPerComplex(track_num + 1);

    if (_simples_per_complex[track_num] == NULL) {
      _simples_per_complex[track_num] = (si32 *) umalloc
	(nsimples * sizeof(si32));
    } else {
      _simples_per_complex[track_num] = (si32 *) urealloc
	(_simples_per_complex[track_num],
	 nsimples * sizeof(si32));
    }
    
    fseek(_header_file, _simples_per_complex_offsets[track_num], SEEK_SET);
  
    if (ufread(_simples_per_complex[track_num],
	       sizeof(si32), nsimples, _header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading simples per complex for");
      TaStr::AddStr(_errStr, "  ", "  complex track params.");
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    BE_to_array_32(_simples_per_complex[track_num], nsimples * sizeof(si32));

  } //   if (read_simples_per_complex) 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the parameters for a simple track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadSimpleParams(int track_num,
				     bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanTrackFile::ReadSimpleParams\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);
  TaStr::AddInt(_errStr, "  track_num", track_num);

  // move to offset in file
  
  fseek(_data_file, _simple_track_offsets[track_num], SEEK_SET);
  
  // read in params
  
  if (ufread(&_simple_params, sizeof(simple_track_params_t),
	     1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simple_track_params");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(&_simple_params, sizeof(simple_track_params_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in an entry for a track
//
// If first_entry is set to TRUE, then the first entry is read in. If not
// the next entry is read in.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadEntry()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::ReadEntry\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);

  // move to the entry offset in the file
  
  long offset;
  if (_first_entry) {
    offset = _simple_params.first_entry_offset;
    _first_entry = false;
  } else {
    offset = _entry.next_entry_offset;
  }
  
  fseek(_data_file, offset, SEEK_SET);
  
  // read in entry
  
  if (ufread(&_entry, sizeof(track_file_entry_t),
	     1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading track entry");
    TaStr::AddInt(_errStr, "  Simple track num: ",
		  _simple_params.simple_track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  BE_to_array_32(&_entry, sizeof(track_file_entry_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the array of simple tracks for each complex track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadSimplesPerComplex()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::ReadSimplesPerComplex\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);

  for (int itrack = 0; itrack < _header.n_complex_tracks; itrack++) {

    int complex_num = _complex_track_nums[itrack];
    int nsimples = _nsimples_per_complex[complex_num];

    AllocSimplesPerComplex(complex_num + 1);

    _simples_per_complex[complex_num] = (si32 *) urealloc
      (_simples_per_complex[complex_num],
       (nsimples * sizeof(si32)));

    fseek(_header_file,
	  _simples_per_complex_offsets[complex_num], SEEK_SET);
    
    if (ufread(_simples_per_complex[complex_num], sizeof(si32), nsimples,
	       _header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading simples_per_complex");
      TaStr::AddInt(_errStr, "  track_num: ", complex_num);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    BE_to_array_32(_simples_per_complex[complex_num],
		   nsimples * sizeof(si32));

  } // itrack 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in entries for a scan
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadScanEntries(int scan_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::ReadScanEntries\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);

  // allocate as necessary
  
  _n_scan_entries = _scan_index[scan_num].n_entries;
  AllocScanEntries(_n_scan_entries);

  track_file_entry_t *entry = _scan_entries;
  int next_entry_offset = _scan_index[scan_num].first_entry_offset;
  
  for (int ientry = 0; ientry < _n_scan_entries; ientry++, entry++) {
    
    // move to the next entry offset in the file
    
    fseek(_data_file, next_entry_offset, SEEK_SET);
  
    // read in entry
  
    if (ufread(entry, sizeof(track_file_entry_t),
	       1, _data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading track entry");
      TaStr::AddInt(_errStr, "  ientry: ", ientry);
      TaStr::AddInt(_errStr, "  scan_num: ", scan_num);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
  
    BE_to_array_32(entry, sizeof(track_file_entry_t));
    next_entry_offset = entry->next_scan_entry_offset;

  } // ientry 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in track_utime_t array
//
// Returns 0 on success or -1 on error
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReadUtime()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::ReadUtime\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _data_file_path);

  AllocUtime();
  
  // read the complex and simple track params and load up
  // the start and end julian time arrays - these are used to
  // determine if a track is a valid candidate for display
  
  for (int itrack = 0; itrack < _header.n_complex_tracks; itrack++) {
    
    int complex_track_num = _complex_track_nums[itrack];
    
    if (ReadComplexParams(complex_track_num, true, false)) {
      return -1;
    }
    
    time_t start_time = _complex_params.start_time;
    time_t end_time = _complex_params.end_time;
    
    _track_utime[complex_track_num].start_complex = start_time;
    _track_utime[complex_track_num].end_complex = end_time;

  } // itrack 
  
  for (int itrack = 0; itrack < _header.n_simple_tracks; itrack++) {
    
    int simple_track_num = itrack;
    
    if (ReadSimpleParams(simple_track_num, false)) {
      return -1;
    }
    
    time_t start_time = _simple_params.start_time;
    time_t end_time = _simple_params.end_time;
    
    _track_utime[simple_track_num].start_simple = start_time;
    _track_utime[simple_track_num].end_simple = end_time;

  } // itrack 

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// Reinitialize headers and arrays
//
///////////////////////////////////////////////////////////////////////////

void TitanTrackFile::Reinit()
     
{
  
  MEM_zero(_header);
  MEM_zero(_complex_params);
  MEM_zero(_simple_params);
  MEM_zero(_entry);
  
  if (_n_complex_allocated > 0) {
    memset(_complex_track_nums, 0, _n_complex_allocated * sizeof(si32));
  }

  if (_n_scan_entries_allocated > 0) {
    memset(_scan_entries, 0, _n_scan_entries * sizeof(track_file_entry_t));
  }
  
  if (_n_scan_index_allocated > 0) {
    memset(_scan_index, 0,
	   _n_scan_index_allocated * sizeof(track_file_scan_index_t));
  }
  
  if (_n_simple_allocated > 0) {
    memset (_simple_track_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_nsimples_per_complex, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_simples_per_complex_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_complex_track_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
  }
    
  if (_n_utime_allocated > 0) {
    memset(_track_utime, 0,
	   _n_utime_allocated * sizeof(track_utime_t));
  }

}

///////////////////////////////////////////////////////////////////////////
//
// Set a complex params slot in the file available for
// reuse, by setting the offset to its negative value.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::ReuseComplexSlot(int track_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::ReuseComplexSlot\n";

  si32 *offset = _complex_track_offsets + track_num;

  if (*offset <= 0) {
    TaStr::AddStr(_errStr, "  ", "Slot for track not available");
    TaStr::AddInt(_errStr, "  track_num: ", track_num);
    return -1;
  }
  
  *offset *= -1;
  
  if (track_num < _lowest_avail_complex_slot)
    _lowest_avail_complex_slot = track_num;

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// prepare a simple track for reading by reading in the simple track
// params and setting the first_entry flag
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::RewindSimple(int track_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::RewindSimpleTrack\n";
  
  // read in simple track params
  
  if (ReadSimpleParams(track_num, false)) {
    return -1;
  }

  // set first_entry flag
  
  _first_entry = TRUE;

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// rewrite an entry for a track in the track data file
//
// The entry is written at the file offset of the original entry
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::RewriteEntry()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::RewriteEntry\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _data_file_path);

  // code copy into network byte order
  
  track_file_entry_t entry = _entry;
  BE_to_array_32(&entry, sizeof(track_file_entry_t));
  
  // move to entry offset
  
  fseek(_data_file, _entry.this_entry_offset, SEEK_SET);
  
  // write entry
  
  if (ufwrite(&entry, sizeof(track_file_entry_t),
	      1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing track entry");
    TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// seek to the end of the track file data
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::SeekEndData()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::SeekEndData\n";
  TaStr::AddStr(_errStr, "  File: ", _data_file_path);

  if (fseek(_data_file, 0L, SEEK_END) != 0) {
    
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
    
  } else {

    return 0;

  }

}

///////////////////////////////////////////////////////////////////////////
//
// seek to the start of data in track data file
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::SeekStartData()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::SeekStartData\n";
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

///////////////////////////////////////////////////////////////////////////
//
// write the track_file_header_t structure to a track data file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::WriteHeader()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::WriteHeader\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _header_file_path);
  
  // get data file size

  fflush(_data_file);
  struct stat data_stat;
  ta_stat (_data_file_path.c_str(), &data_stat);
  _header.data_file_size = data_stat.st_size;
  
  // set file time to gmt
  
  _header.file_time = time(NULL);
  
  // copy file label
  
  char label[R_FILE_LABEL_LEN];
  MEM_zero(label);
  strcpy(label, _header_file_label.c_str());
  
  // move to start of file and write label
  
  fseek(_header_file, (si32) 0, SEEK_SET);
  
  if (ufwrite(label, sizeof(char), R_FILE_LABEL_LEN,
	      _header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing file label.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // create local arrays

  int n_complex_tracks = _header.n_complex_tracks;
  int n_simple_tracks = _header.n_simple_tracks;
  int n_scans = _header.n_scans;

  TaArray<si32> nums, coffsets, soffsets, nsimples, simples; 
  si32 *complex_track_nums = nums.alloc(n_complex_tracks);
  si32 *complex_track_offsets = coffsets.alloc(n_simple_tracks);
  si32 *simple_track_offsets = soffsets.alloc(n_simple_tracks);
  si32 *nsimples_per_complex = nsimples.alloc(n_simple_tracks);
  si32 *simples_per_complex_offsets = simples.alloc(n_simple_tracks);

  TaArray<track_file_scan_index_t> sindexArray;
  track_file_scan_index_t *scan_index = sindexArray.alloc(n_scans);

  // copy the header and arrays to local variables
  
  track_file_header_t header = _header;
  
  memcpy (complex_track_nums,_complex_track_nums,
          n_complex_tracks *  sizeof(si32));
  
  memcpy (complex_track_offsets, _complex_track_offsets,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (simple_track_offsets, _simple_track_offsets,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (nsimples_per_complex, _nsimples_per_complex,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (scan_index, _scan_index,
          n_scans *  sizeof(track_file_scan_index_t));
  
  // code into network byte order
  
  header.nbytes_char = (TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN +
                        TRACK_FILE_HEADER_NBYTES_CHAR);
  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.storm_header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.verify.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32(&header,
		   sizeof(track_file_header_t) - header.nbytes_char);
  
  BE_from_array_32(complex_track_nums,
		   n_complex_tracks *  sizeof(si32));
  
  BE_from_array_32(complex_track_offsets,
		   n_simple_tracks *  sizeof(si32));
    
  BE_from_array_32(simple_track_offsets,
		   n_simple_tracks *  sizeof(si32));
  
  BE_from_array_32(nsimples_per_complex,
		   n_simple_tracks *  sizeof(si32));
  
  BE_from_array_32(scan_index,
		   n_scans *  sizeof(track_file_scan_index_t));
  
  // write header
  
  if (ufwrite(&header, sizeof(track_file_header_t),
	      1, _header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing header.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in complex track num array
  
  if (ufwrite(complex_track_nums, sizeof(si32),
	      n_complex_tracks, _header_file) != n_complex_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex_track_nums.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in complex track offset
  
  if (ufwrite(complex_track_offsets, sizeof(si32),
	      n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex_track_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in simple track offset
  
  if (ufwrite(simple_track_offsets, sizeof(si32), n_simple_tracks,
	      _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simple_track_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in scan index
  
  if (ufwrite(scan_index, sizeof(track_file_scan_index_t),
	      n_scans, _header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing scan_index.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // write in nsimples_per_complex

  if (ufwrite(nsimples_per_complex, sizeof(si32),
	      n_simple_tracks, _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing nsimples_per_complex.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  long offsets_pos = ftell(_header_file);
  
  // seek ahead of the simples_per_complex_offsets array
  
  fseek(_header_file, n_simple_tracks * sizeof(si32), SEEK_CUR);

  // clear offsets array

  memset(simples_per_complex_offsets, 0, n_simple_tracks * sizeof(si32));
  
  // loop through complex tracks

  for (int icomplex = 0; icomplex < n_complex_tracks; icomplex++) {

    int complex_num = _complex_track_nums[icomplex];
    int nsimples = _nsimples_per_complex[complex_num];
    simples_per_complex_offsets[complex_num] = ftell(_header_file);

    TaArray<si32> simpleArray;
    si32 *simples_per_complex = simpleArray.alloc(nsimples);
    memcpy(simples_per_complex, _simples_per_complex[complex_num],
	   nsimples * sizeof(si32));
    BE_from_array_32(simples_per_complex, nsimples * sizeof(si32));
		     
    // write out simple tracks array
    
    if (ufwrite(simples_per_complex, sizeof(si32), nsimples,
		_header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Writing simples_per_complex.");
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // icomplex 

  // write out simples_per_complex_offsets
  
  fseek(_header_file, offsets_pos, SEEK_SET);
  BE_from_array_32(simples_per_complex_offsets,
		   n_simple_tracks * sizeof(si32));
  
  if (ufwrite(simples_per_complex_offsets, sizeof(si32), n_simple_tracks,
	      _header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simples_per_complex_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  FlushFiles();

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write simple track params at the end of the file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::WriteSimpleParams(int track_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::WriteSimpleParams\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _data_file_path);
  
  // Go to the end of the file.

  fseek(_data_file, 0, SEEK_END);
  long file_mark = ftell(_data_file);
  
  // if params have been written before, go to the stored offset.
  // If not, store offset as current file location
  
  bool rewrite = true;
  if (_simple_track_offsets[track_num] == 0) {
    _simple_track_offsets[track_num] = file_mark;
    rewrite = false;
  }
  
  // copy track params, encode and write to file
  
  simple_track_params_t simple_params = _simple_params;
  BE_from_array_32(&simple_params,
		   sizeof(simple_track_params_t));
  
  // for rewrite, move to stored offset
  
  if (rewrite) {
    fseek(_data_file, _simple_track_offsets[track_num], SEEK_SET);
  }
  
  if (ufwrite(&simple_params, sizeof(simple_track_params_t),
	      1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simple track params.");
    TaStr::AddInt(_errStr, "  track_num", track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  fflush(_data_file);
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write complex track params
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanTrackFile::WriteComplexParams(int track_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::WriteSimpleParams\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _data_file_path);
  
  if (_complex_track_offsets[track_num] == 0) {
    
    // params have not been written before.
    //
    // Two steps: 1) look for a slot which has been freed
    //               up when a complex track was consolidated.
    //            2) If no available slot, use end of file
    
    int lowest_avail_slot = _lowest_avail_complex_slot;
    si32 *offset_p = _complex_track_offsets + lowest_avail_slot;
    si32 avail_offset;
    bool slot_found = false;
      
    for (int i = lowest_avail_slot; i < track_num; i++) {
      
      if (*offset_p < 0) {
	
	// avail slot found
	
	avail_offset = -(*offset_p);
	*offset_p = 0;
	_lowest_avail_complex_slot = i + 1;
	slot_found = true;
	break;
	
      } // if (*offset_p < 0) 
      
      offset_p++;
      
    } // i 
    
    if (slot_found) {
      
      _complex_track_offsets[track_num] = avail_offset;
      fseek(_data_file, avail_offset, SEEK_SET);
      
    } else {
      
      fseek(_data_file, 0, SEEK_END);
      _complex_track_offsets[track_num] = ftell(_data_file);
      _lowest_avail_complex_slot = track_num + 1;
      
    }
    
  } else {
    
    // params have been stored before, so go to the stored offset
    
    fseek(_data_file, _complex_track_offsets[track_num], SEEK_SET);
    
  }
  
  // copy track params, encode and write to file
  
  complex_track_params_t complex_params = _complex_params;
  
  BE_from_array_32(&complex_params,
		   sizeof(complex_track_params_t));
  
  if (ufwrite(&complex_params, sizeof(complex_track_params_t),
	      1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex track params.");
    TaStr::AddInt(_errStr, "  track_num", track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  fflush(_data_file);
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write an entry for a track in the track data file
//
// The entry is written at the end of the file
//
// returns offset of last entry written on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

long TitanTrackFile::WriteEntry(int prev_in_track_offset,
				int prev_in_scan_offset)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanTrackFile::WriteEntry\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _data_file_path);

  long file_mark;

  // Go to the end of the file and save the file position.

  fseek(_data_file, 0, SEEK_END);
  file_mark = ftell(_data_file);
  
  // If prev_in_track_offset is non-zero (which indicates that this is not
  // the first entry in a track) read in the entry at that location,
  // update the next_entry prev_in_track_offset with the current file
  // location and write back to file
  
  if (prev_in_track_offset != 0) {
    
    // move to the entry prev_in_track_offset in the file
    
    fseek(_data_file, prev_in_track_offset, SEEK_SET);
    
    // read in entry
    
    track_file_entry_t entry;
    if (ufread(&entry, sizeof(track_file_entry_t),
	       1, _data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Reading track entry to update in_track_offset");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // store next_entry_offset, swap
    
    entry.next_entry_offset = file_mark;
    BE_from_array_32(&entry.next_entry_offset,
		     sizeof(entry.next_entry_offset));
    
    // move back to offset
    
    fseek(_data_file, prev_in_track_offset, SEEK_SET);
    
    // rewrite entry
    
    if (ufwrite(&entry, sizeof(track_file_entry_t),
		1, _data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Re-writing track entry.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // if (prev_in_track_offset == 0) 
  
  // If prev_in_scan_offset is non-zero (which indicates that this is not
  // the first entry in a track) read in the entry at that location,
  // update the next_scan_entry_offset with the current file
  // location and write back to file
  
  if (prev_in_scan_offset != 0) {
    
    // move to the entry prev_in_scan_offset in the file
    
    fseek(_data_file, prev_in_scan_offset, SEEK_SET);
    
    // read in entry
    
    track_file_entry_t entry;
    if (ufread(&entry, sizeof(track_file_entry_t),
	       1, _data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Reading track entry to update in_track_offset.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // store next_entry_offset, swap
    
    entry.next_scan_entry_offset = file_mark;
    BE_from_array_32(&entry.next_scan_entry_offset,
		     sizeof(entry.next_scan_entry_offset));
    
    // move back to offset
    
    fseek(_data_file, prev_in_scan_offset, SEEK_SET);
    
    // rewrite entry
    
    if (ufwrite(&entry, sizeof(track_file_entry_t),
		1, _data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Re-writing track entry.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // if (prev_in_scan_offset == 0) 
  
  // go to end of file to write entry structure
  
  fseek(_data_file, 0, SEEK_END);
  
  // copy entry to local variable
  
  track_file_entry_t entry = _entry;
  
  // set entry offsets
  
  entry.prev_entry_offset = prev_in_track_offset;
  entry.this_entry_offset = file_mark;
  entry.next_entry_offset = 0;
  
  // swap
  
  BE_from_array_32(&entry, sizeof(track_file_entry_t));
  
  // write entry
  
  if (ufwrite(&entry, sizeof(track_file_entry_t),
	      1, _data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ",
		  "Writing track entry.");
    TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return (file_mark);
  
}
