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
// <titan/TitanStormFile.hh>
//
// TITAN C++ storm file io
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanStormFile_HH
#define TitanStormFile_HH


#include <titan/storm.h>
#include <string>
using namespace std;

class TitanStormFile
{

public:
  
  // constructor
  
  TitanStormFile();
  
  // destructor
  
  virtual ~TitanStormFile();

  // data access

  const storm_file_header_t &header() const { return _header; }
  const storm_file_params_t &params() const { return _header.params; }
  const storm_file_scan_header_t &scan() const { return _scan; }
  const storm_file_global_props_t *gprops() const { return _gprops; }
  const storm_file_layer_props_t *lprops() const { return _lprops; }
  const storm_file_dbz_hist_t *hist() const { return _hist; }
  const storm_file_run_t *runs() const { return _runs; }
  const storm_file_run_t *proj_runs() const { return _proj_runs; }
  const int *scan_offsets() const { return _scan_offsets; }
  int storm_num() const { return _storm_num; }
  
  const string &header_file_path() { return _header_file_path; }
  const string &header_file_label() { return _header_file_label; }
  const string &data_file_path() { return _data_file_path; }
  const string &data_file_label() { return _data_file_label; }

  ///////////////////////////////////////////////////////////////////
  // error string
  
  const string &getErrStr() { return (_errStr); }

  // public functions

  // memory allocation and freeing

  void AllocLayers(int n_layers);
  void FreeLayers();

  void AllocHist(int n_dbz_intervals);
  void FreeHist();

  void AllocRuns(int n_runs);
  void FreeRuns();

  void AllocProjRuns(int n_proj_runs);
  void FreeProjRuns();

  void AllocGprops(int nstorms);
  void FreeGprops();

  void AllocScanOffsets(int n_scans_needed);
  void FreeScanOffsets();

  void FreeAll();
    
  // Open the storm header and data files
  
  int OpenFiles(const char *mode,
		const char *header_file_path,
		const char *data_file_ext = NULL);
  
  // Close the storm header and data files

  void CloseFiles();
     
  // Flush the storm header and data files

  void FlushFiles();
  
  // Put an advisory lock on the header file
  // Mode is "w" - write lock, or "r" - read lock.
  // returns 0 on success, -1 on failure

  int LockHeaderFile(const char *mode);

  // Remove advisory lock from the header file
  // returns 0 on success, -1 on failure

  int UnlockHeaderFile();
  
  // read the storm file header

  int ReadHeader(bool clear_error_str = true);
     
  // read in the storm projected area runs
  // Space for the array is allocated.
  // returns 0 on success, -1 on failure

  int ReadProjRuns(int storm_num);
     
  // Read in the scan info and global props for a particular scan
  // in a storm properties file.
  // If storm num is set, only the gprops for that storm is swapped
  // returns 0 on success, -1 on failure

  int ReadScan(int scan_num, int storm_num = -1);
     
  // read in the seconday storm property data (lprops, hist, runs)
  // for a given storm in a scan.
  // Space for the arrays of structures is allocated as required.
  // returns 0 on success, -1 on failure

  int ReadProps(int storm_num);
     
  // seek to the end of the storm data in data file
  // returns 0 on success, -1 on failure

  int SeekEndData();

  // seek to the start of the storm data in data file
  // returns 0 on success, -1 on failure

  int SeekStartData();
  
  // write the storm_file_header_t structure to a storm file.
  // returns 0 on success, -1 on failure
  
  int WriteHeader();
     
  // write the storm layer property and histogram data for a storm,
  // at the end of the file.
  // returns 0 on success, -1 on failure

  int WriteProps(int storm_num);

  // write scan header and global properties for a particular scan
  // in a storm properties file.
  // Performs the writes from the end of the file.
  // returns 0 on success, -1 on failure

  int WriteScan(int scan_num);
     
  // Convert the ellipse data (orientation, major_radius and minor_radius)
  // for a a gprops struct to local (km) values.
  // This applies to structs which were derived from lat-lon grids, for
  // which some of the fields are in deg instead of km.
  // It is a no-op for other projections.
  //
  // See Note 3 in storms.h

  void GpropsEllipses2Km(const storm_file_scan_header_t &scan,
			 storm_file_global_props_t &gprops);
  
     
  // Convert the (x,y) km locations in a gprops struct to lat-lon.
  // This applies to structs which were computed for non-latlon 
  // grids. It is a no-op for lat-lon grids.
  //
  // See Note 3 in storms.h
  
  void GpropsXY2LatLon(const storm_file_scan_header_t &scan,
		       storm_file_global_props_t &gprops);
  
  // Truncate header file
  // Returns 0 on success, -1 on failure.

  int TruncateHeaderFile(int length);

  // Truncate data file
  // Returns 0 on success, -1 on failure.

  int TruncateDataFile(int length);

protected:

  // file details
  
  string _header_file_path;
  string _header_file_label;
  string _data_file_path;
  string _data_file_label;

  FILE *_header_file;
  FILE *_data_file;

  // data

  storm_file_header_t _header;
  storm_file_scan_header_t _scan;
  storm_file_global_props_t *_gprops;
  storm_file_layer_props_t *_lprops;
  storm_file_dbz_hist_t *_hist;
  storm_file_run_t *_runs;
  storm_file_run_t *_proj_runs;
  si32 *_scan_offsets;
  int _storm_num;

  // memory allocation

  int _max_scans;
  int _max_storms;
  int _max_layers;
  int _max_dbz_intervals;
  int _max_runs;
  int _max_proj_runs;

  // errors

  string _errStr;

  // functions

  void _clearErrStr();

  void _convert_ellipse_2km(const titan_grid_t &tgrid,
			    double centroid_x,
			    double centroid_y,
			    fl32 &orientation,
			    fl32 &minor_radius,
			    fl32 &major_radius);

  int _truncate(FILE *&fd, const string &path, int length);

public:

  // friends for Titan program which writes the storm and track files
  
  friend class StormFile;
  friend class Area;
  friend class Props;
  friend class Identify;
  friend class StormIdent;

private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  TitanStormFile(const TitanStormFile & orig);
  TitanStormFile & operator = (const TitanStormFile & other);
  
};

#endif


