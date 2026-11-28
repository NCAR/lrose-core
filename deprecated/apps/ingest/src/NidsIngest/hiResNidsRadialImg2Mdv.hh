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


/**
 * @file NidsIngest.hh
 * @brief NidsIngest class manages search, decoding, conversion, 
 *        and output of Mdv files. 
 */

#ifndef NIDSINGEST_H
#define NIDSINGEST_H

#include "Params.hh"
#include <dataport/port_types.h>
#include <string> 
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include "hiResRadialFile.hh"

extern int nt; // The number of threads working
extern pthread_mutex_t ntMutex; // Mutex to lock

using namespace std;

class NidsIngest {
  
public:

  /**
   * Constructor
   * Copy pointer to the parameters. Initialize various data flags (radar 
   * volume number, existence of converted data). Set the data input and 
   * output directories members based on the radar and product names. 
   * Instantiate file search objects for real-time or archive modes and 
   * begin processing.
   * 
   * @param[in] params  Pointer to object containing all user defined 
   *                    parameters
   * @param[in] iradar  Integer indicator of radar for which data will be
   *                    processed
   * @param[in] ifield  Integer indicator of product or field to be processed
   */
  NidsIngest (Params *params, int iradar, int ifield);

  /**
   * Destructor
   * If there is a volume of data stored in data members, output the data.
   * Clean up memory, reset flag indicating that data volume exists to false.
   */
   ~NidsIngest ();

protected:
  
private:
  /**
   *  User defined parameters
   */
  Params *_params;
  
  /**
   * Integer indicator or field or product to be processed
   */
  int _ifield;

  /**
   * Directory of input data. The format is:
   * topLevelInputDir/radarName/productName
   */ 
  string _inDir;

  /**
   * Directory of output data. The format is:
   * topLevelOutputDir/productName/radarName/
   */ 
  string _outUrl;
 
  /**
   * Directory of output spdb data. The format is:
   * topLevelOutputDir/radarName/productName
   */ 
  string _outSpdbUrl;

  /**
   * Number of last volume processed. Initialized to -1. A change in this
   * number will trigger output of data volume and initialization of new
   * volume
   */
  int _lastVolNum;
  
  /**
   * Flag to indicate that radar field data is currently stored in data
   * members.
   */
  bool _haveData;

  /**
   * Flag to indicate that data is to be output in radial format. If this is
   * false data will be output in a cartesian grid format with user defined 
   * grid parameters
   */
  bool _saveAsRadial;

  /**
   * Mdv master header
   */
  Mdvx::master_header_t _mhdr;

  /**
   * Mdv field header
   */
  Mdvx::field_header_t _fhdr;
  
  /**
   * Mdv vertical level header
   */
  Mdvx::vlevel_header_t _vhdr;

  /**
   * Array of nids product field data
   */
  fl32 *_data;

  /**
   * Recursively process all files under directory dirName (in archive mode).
   * Files which start with "." or "_" will be skipped. Other files will 
   * be inserted into a map container by file time. Files are processed in
   * time order starting with oldest files.
   *
   * @param[in] dirName  Name of top level input directory 
   */
  void _procDir(string dirName);

  /** 
   * Convert file to Mdv format:
   * If using threads, wait for a thread to become available, or if available,
   * increment increment the threads in use. Instantiate a hiResRadialFile 
   * object to decode the input file. If decoding is successful, start a new 
   * Mdv volume or add new data to existing data volume. Free thread if 
   * necessary. Output data volume as appropriate.
   *
   * @param[in] filePath  Fully qualified file path of NIDS data file
   */
  void _convertFile(  const char *filePath);

  /** 
   * A plane or tilt of data NIDS radar data is copied to the array holding
   * the 3D volume of data. If the dimensions of the existing volume and new
   * plane are the same, then the data is just copied directly to the volume
   * array. If the dimensions are different, the volume data is first set 
   * to missing. Then the x and y dimensions of the plane data and volume 
   * data are compared. The smaller of each becomes the upperbound in each 
   * dimension for data that gets copied from the plane to the volume.
   */
  void _copyDataToVolume( fl32** volPtr,  int volNx,  int volNy ,  
			  fl32** planePtr, int planeNx, int planeNy);
  /**
   * Declare DsMdvx object. Set Mdv fieldheader, create and add mdv field, and
   * then write the field to the specified output directory member, _outUrl.
   */
  void _outputVolume();

  /**
   * If using threads in radar data processing, free thread and decrement
   * the number of threads in use.
   */
  void _freeThread();

  void _createSpdbContours( hiResRadialFile &H);

  void _createMdvVolume ( hiResRadialFile &H);

  void _createSpdbGenPt( hiResRadialFile &H);

  void _writeSpdbHail( hiResRadialFile &H);

  void _writeSpdbTvs( hiResRadialFile &H);

  void _writeSpdbMesocyclone( hiResRadialFile &H);
  
  void _writeSpdbStormIDs( hiResRadialFile &H);

};

#endif





