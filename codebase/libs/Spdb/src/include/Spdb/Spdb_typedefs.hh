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
////////////////////////////////////////////////
//
// Spdb_typedefs.hh
//
// Typedefs for Spdb class
//
////////////////////////////////////////////////

// This header file can only be included from within Spdb.hh

#ifdef _in_Spdb_hh

// defines

#define SPDB_LABEL_MAX 64
#define SPDB_PATH_MAX 1024
#define SECS_IN_DAY 86400
#define SECS_IN_MIN 60
#define MINS_IN_DAY 1440

// enum for indicating lead time storage
//
// If you are dealing with forecast data, you may wish to store
// the lead time in each chunk header as data_type or data_type2
// If you do so, and you set up the storage type using this routine,
// the latest_data_info file will show the gen time and forecast time
// instead of the valid time. Also, the header will show this
// decision.
//
// This enum specifies that decision.

typedef enum {
  LEAD_TIME_NOT_APPLICABLE = 0,
  LEAD_TIME_IN_DATA_TYPE = 1,
  LEAD_TIME_IN_DATA_TYPE2 = 2
} lead_time_storage_t;

// enum for compression
//
// Compression can take place in 2 ways:
//   (a) inndividual chunks can be compresseed
//   (b) DsSpdbServer messages can be compressed for transmission

typedef enum {
  COMPRESSION_NONE = 0,
  COMPRESSION_GZIP = 1,
  COMPRESSION_BZIP2 = 2
} compression_t;

// header struct - occurs once at the top of the
// index file

typedef struct {
  
  char prod_label[SPDB_LABEL_MAX];

  // There must be 80 si32's following, including spares
  
  si32 major_version;
  si32 minor_version;
  
  si32 prod_id;
  si32 n_chunks;
  
  si32 nbytes_frag;    // number of fragmented bytes - these
                       // are 'lost' because during overwrite
                       // the new chunk did not occupy exactly
                       // the same space as the old chunk.

  si32 nbytes_data;    // number of bytes of usable data -
                       // file size = nbytes_data + nbytes_frag
    
  si32 max_duration;   // max number of secs over which a product
                       // is valid
    
  ti32 start_of_day;   // time of start of day for this file */
  ti32 end_of_day;     // time of end of day for this file */
    
  ti32 start_valid;    // start valid time of data in this file */
  ti32 end_valid;      // end valid time of data in this file */
    
  ti32 earliest_valid; // the earliest time for which there are valid
                       // products during this day. If no products from
                       // previous files overlap into this day, this
                       // value will be the same as start_valid.
                       // If products from previous days are valid 
                       // during this day, this time be set to the
                       // earliest valid time for those products */
    
  ti32 latest_expire;  // latest expire time for data in this file */

  si32 lead_time_storage; // see lead_time_storage_t above

  si32 spares[66];
    
  // Minute_posn stores the first chunk position for each minute
  // of the day. If no chunk corresponds to this minute the value is -1
  
  si32 minute_posn[MINS_IN_DAY];
  
} header_t;

// chunk ref struct - there is an array of n_chunks of
// these following the header. They are written in 
// order sorted on the valid_time field.  The data_type
// field can be used to filter data returned to clients.
// To be used, the data_type field must be set to a non-zero
// value.  Data requests for data type 0 return all data
// in the calls in this library.

typedef struct {
  
  ti32 valid_time;
  ti32 expire_time;
  si32 data_type;
  si32 data_type2;
  ui32 offset;       // offset of data chunk in .data file
  ui32 len;          // length of data chunk in data file
  
} chunk_ref_t;

// auxiliary reference, for storing information which does not fit
// into the chunk_refs

#define TAG_LEN 24

typedef struct {
  
  ti32 write_time; // time entry written to the data base
  ui32 compression;
  ui32 spares[4];
  char tag[TAG_LEN];
  
} aux_ref_t;

// chunk class

class chunk_t {

public:

  time_t valid_time;
  time_t expire_time;
  time_t write_time;
  int data_type;
  int data_type2;
  int len;
  compression_t current_compression;
  compression_t stored_compression;
  void *data;
  string tag;

};

// enums

typedef enum {
  putModeOver,
  putModeAdd,
  putModeOnce,
  putModeErase,
  putModeAddUnique
} put_mode_t;

typedef enum {
  ReadMode, WriteMode
} open_mode_t;

typedef enum {
  UniqueOff,
  UniqueLatest,
  UniqueEarliest
} get_unique_t;

#endif
