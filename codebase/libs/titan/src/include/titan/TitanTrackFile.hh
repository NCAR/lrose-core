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
////////////////////////////////////////////////////////////////////
// <titan/TitanTrackFile.hh>
//
// TITAN C++ track file io
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanTrackFile_HH
#define TitanTrackFile_HH


#include <titan/storm.h>
#include <titan/track.h>
#include <string>
using namespace std;

class TitanTrackFile
{

public:
  
  // constructor
  
  TitanTrackFile();
  
  // destructor
  
  virtual ~TitanTrackFile();

  // data access

  const track_file_header_t &header() const { return _header; }
  const track_file_params_t &params() const { return _header.params; }
  const simple_track_params_t &simple_params() const;
  const complex_track_params_t &complex_params() const;
  const track_file_entry_t &entry() const { return _entry; }
  const track_file_entry_t *scan_entries() const { return _scan_entries; }
  const track_file_scan_index_t *scan_index() const { return _scan_index; }
  const track_utime_t *track_utime() const { return _track_utime; }
  int n_scan_entries() { return _n_scan_entries; }
  
  const string &header_file_path() { return _header_file_path; }
  const string &header_file_label() { return _header_file_label; }
  const string &data_file_path() { return _data_file_path; }
  const string &data_file_label() { return _data_file_label; }

  const si32 *complex_track_nums() { return _complex_track_nums; }
  const si32 *complex_track_offsets() { return _complex_track_offsets; }
  const si32 *simple_track_offsets() { return _simple_track_offsets; }
  const si32 *nsimples_per_complex() { return _nsimples_per_complex; }
  const si32 *simples_per_complex_offsets() { return _simples_per_complex_offsets; }
  si32 **simples_per_complex() { return _simples_per_complex; }

  // memory allocation and freeing

  void AllocSimpleArrays(int n_simple_needed);
  void FreeSimpleArrays();
  void AllocComplexArrays(int n_complex_needed);
  void FreeComplexArrays();
  void AllocSimplesPerComplex(int n_simple_needed);
  void FreeSimplesPerComplex();
  void AllocScanEntries(int n_entries_needed);
  void FreeScanEntries();
  void AllocScanIndex(int n_scans_needed);
  void FreeScanIndex();
  void AllocUtime();
  void FreeUtime();
  void FreeAll();
     
  // Open the track header and data files
  // Returns 0 on success, -1 on error

  int OpenFiles(const char *mode,
		const char *header_file_path,
		const char *data_file_ext = NULL);
  
  // Close the storm header and data files

  void CloseFiles();
     
  // Flush the storm header and data files

  void FlushFiles();
  
  // Put an advisory lock on the header file.
  // Mode is "w" - write lock, or "r" - read lock.
  // returns 0 on success, -1 on failure

  int LockHeaderFile(const char *mode);

  // Remove advisory lock from the header file
  // returns 0 on success, -1 on failure

  int UnlockHeaderFile();
  
  // read in the track_file_header_t structure from a track file.
  // Read in associated arrays (complex_track_nums, complex_track_offsets,
  //   simple_track_offsets, scan_index, nsimples_per_complex,
  //   simples_per_complex_offsets)
  // returns 0 on success, -1 on failure

  int ReadHeader(bool clear_error_str = true);
     
  // Read in the track_file_header_t and scan_index array.
  // returns 0 on success, -1 on failure

  int ReadScanIndex(bool clear_error_str = true);
     
  // reads in the parameters for a complex track
  // For normal reads, read_simples_per_complex should be set true. This
  // is only set FALSE in Titan, which creates the track files.
  // returns 0 on success, -1 on failure
  
  int ReadComplexParams(int track_num, bool read_simples_per_complex,
			bool clear_error_str = true);
     
  // read in the parameters for a simple track
  // returns 0 on success, -1 on failure

  int ReadSimpleParams(int track_num,
		       bool clear_error_str = true);
     
  // read in an entry for a track
  // If first_entry is set to TRUE, then the first entry is read in. If not
  // the next entry is read in.
  // returns 0 on success, -1 on failure
  
  int ReadEntry();
  
  // read in the array of simple tracks for each complex track
  // returns 0 on success, -1 on failure
  
  int ReadSimplesPerComplex();
  
  // read in entries for a scan
  // returns 0 on success, -1 on failure

  int ReadScanEntries(int scan_num);
     
  // read in track_utime_t array
  // Returns 0 on success or -1 on error

  int ReadUtime();
     
  // Reinitialize headers and arrays

  void Reinit();

  // Set a complex params slot in the file available for
  // reuse, by setting the offset to its negative value.
  // returns 0 on success, -1 on failure

  int ReuseComplexSlot(int track_num);
     
  // prepare a simple track for reading by reading in the simple track
  // params and setting the first_entry flag
  // returns 0 on success, -1 on failure

  int RewindSimple(int track_num);
     
  // rewrite an entry for a track in the track data file
  // The entry is written at the file offset of the original entry
  // returns 0 on success, -1 on failure
  
  int RewriteEntry();
     
  // seek to the end of the track file data

  int SeekEndData();

  // seek to the start of data in track data file

  int SeekStartData();
     
  // write the track_file_header_t structure to a track data file
  // returns 0 on success, -1 on failure

  int WriteHeader();

  // write simple track params at the end of the file
  // returns 0 on success, -1 on failure
  
  int WriteSimpleParams(int track_num);
     
  // write complex track params
  // returns 0 on success, -1 on failure
  
  int WriteComplexParams(int track_num);
     
  // write an entry for a track in the track data file
  // The entry is written at the end of the file
  // returns offset of last entry written on success, -1 on failure
  
  long WriteEntry(int prev_in_track_offset,
		  int prev_in_scan_offset);
     
  ///////////////////////////////////////////////////////////////////
  // error string
  
  const string &getErrStr() { return (_errStr); }

protected:

  // track file
  
  string _header_file_path;
  string _header_file_label;
  string _data_file_path;
  string _data_file_label;
  
  FILE *_header_file;
  FILE *_data_file;

  bool _first_entry;  // set to TRUE if first entry of a track
  
  // track data

  track_file_header_t _header;
  simple_track_params_t _simple_params;
  complex_track_params_t _complex_params;
  track_file_entry_t _entry;

  track_file_scan_index_t *_scan_index;
  track_file_entry_t *_scan_entries;
  track_utime_t *_track_utime;
  
  si32 *_complex_track_nums;
  si32 *_complex_track_offsets;
  si32 *_simple_track_offsets;
  si32 *_nsimples_per_complex;
  si32 *_simples_per_complex_offsets;
  si32 **_simples_per_complex;
  int _n_scan_entries;
  
  // memory allocation control

  int _n_simple_allocated;
  int _n_complex_allocated;
  int _n_simples_per_complex_allocated;
  int _n_scan_entries_allocated;
  int _n_scan_index_allocated;
  int _n_utime_allocated;

  int _lowest_avail_complex_slot;

  // errors

  string _errStr;

  // functions

  void _clearErrStr();

public:

  // friends for Titan program which writes the storm and track files
  
  friend class StormTrack;
  friend class TrConsolidate;
  friend class TrTrack;

private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  TitanTrackFile(const TitanTrackFile & orig);
  TitanTrackFile & operator = (const TitanTrackFile & other);
  
};

#endif


