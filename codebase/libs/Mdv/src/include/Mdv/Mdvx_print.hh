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
// Mdvx_print.hh
//
// Print functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

// print state of current, read and write formats

void printFormats(ostream &out, bool force = false) const;

// print all headers in object

void printAllHeaders(ostream &out) const;

// print all file headers in object

void printAllFileHeaders(ostream &out) const;

// print master header in full

void printMasterHeader(ostream &out) const;

static void printMasterHeader(const master_header_t &mhdr,
                              ostream &out,
                              const string dataSetInfo = "");

static void printMasterHeader(const master_header_32_t &mhdr32,
                              ostream &out,
                              const string dataSetInfo = "");

// print master header in summary

static void printMasterHeaderSummary(const master_header_t &mhdr,
                                     ostream &out);

// print field header in full

static void printFieldHeader(const field_header_t &fhdr,
                             ostream &out);
static void printFieldHeader(const field_header_32_t &fhdr32,
                             ostream &out);

// print field header summary

static void printFieldHeaderSummary(const field_header_t &fhdr,
                                    ostream &out);

// print vlevel header

static void printVlevelHeader(const vlevel_header_t &vhdr,
                              const int nz,
                              const char *field_name,
                              ostream &out);
static void printVlevelHeader(const vlevel_header_32_t &vhdr32,
                              const int nz,
                              const char *field_name,
                              ostream &out);

// print chunk header

static void printChunkHeader(const chunk_header_t &chdr,
                             ostream &out);
static void printChunkHeader(const chunk_header_32_t &chdr32,
                             ostream &out);

// print the chunks which Mdvx recognizesw

void printChunks(ostream &out) const;

/////////////////////////////////////////////////////////////
// static print methods

// print entire volume

static void printVol(ostream &out,
                     const Mdvx *mdvx,
                     bool printFieldFileHeadersAlso = false,
                     bool printData = false,
                     bool transformToLinear = false,
                     bool printNative = false,
                     bool printCanonical = false,
                     int printNlinesData = -1);

// print volume summary

static void printVolSummary(ostream &out,
                            const Mdvx *mdvx);

// print volume in GIS format

static void printVolGis(ostream &out,
                        const Mdvx *mdvx,
                        bool startAtTop = true);

// print volume in tabular format

static void printVolTable(ostream &out,
                          const Mdvx *mdvx);

// print all headers

static void printAllHeaders(ostream &out,
                            const Mdvx *mdvx);

// print time list

static void printTimeList(ostream &out,
                          const Mdvx *mdvx,
                          MdvxTimeList::time_list_mode_t mode,
                          const string &url,
                          time_t startTime,
                          time_t endTime,
                          time_t searchTime,
                          time_t genTime,
                          int marginSecs);

// print the time height profile

static void printTimeHeight(ostream &out,
                            const Mdvx *mdvx, 
                            bool printData = false,
                            bool transformToLinear = false,
                            bool printNative = false);

/////////////////////////////////////////////////////////////
// strings from enums and vice versa

// return string representation of data format

static string format2Str(const int format);
static mdv_format_t str2Format(const string &format);

// return string representation of proj type

static const char *projType2Str(const int proj_type);

// return string representation of vertical type

static const char *vertType2Str(const int vert_type);

// return string for units for x coordinate
// based on projection

static const char *projType2XUnits(const int proj_type);

// return string for units for y coordinate
// based on projection

static const char *projType2YUnits(const int proj_type);

// return string for units for z coordinate
// based on vlevel type

static const char *vertTypeZUnits(const int vert_type);

// return string representation of encoding type

static const char *encodingType2Str(const int encoding_type);

// return string representation of collection type

static const char *collectionType2Str(const int collection_type);

// return string representation of orientation type

static const char *orientType2Str(const int orient_type);

// return string representation of data order

static const char *orderType2Str(const int order_type);

// return string representation of data compression

static const char *compressionType2Str(const int compression_type);

// return string representation of data transform

static const char *transformType2Str(const int transform_type);

// return string representation of data scaling

static const char *scalingType2Str(const int scaling_type);

// return string representation of chunk ID

static const char *chunkId2Str(const int chunk_id);

// return string representation of climatology type

static const char *climoType2Str(const int climo_type);

#endif

    
