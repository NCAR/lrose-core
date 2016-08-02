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
 *        and output of NIDS data in Mdv or Spdb format. 
 */

#ifndef NIDSINGEST_H
#define NIDSINGEST_H

#include "Params.hh"
#include <dataport/port_types.h>
#include <string> 
#include <map>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include "NidsFile.hh"

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
   * Directory of output text data. The format is:
   * _outTextUrl/radarName/productName/date/time.txt
   */ 
  string _outTextUrl;

  /**
   * radar name
   */
  string _radarName;


  /**
   * Number of last volume processed. Initialized to -1. A change in this
   * number will trigger output of data volume and initialization of new
   * volume
   */
  int _prevVolNum;
  
  /**
   * Current volume number
   */
  int _currVolNum;
  

  /**
   * Flag to indicate  starting a new volume (or adding to the previous one)
   */
  bool _newVolume;
  
  /**
   * Flag to indicate the end of a volume for a product. 
   */
  bool _endOfVol;

  /**
   * Flag to indicate the end the start of a volume for a product. 
   */
  bool _startOfVol;

  /**
   * Flag to indicate that the first tilt is needed in volume.
   * Eliminates partial volumes at startup with proper lookback setting
   */
  bool _needFirstVolFirstTilt;

  /**
   * Flag to indicate end of volume decision based on integer tilt number. 
   */
  bool _outputLastTiltInVol;

  /**
   * Flag to indicate outputting point data in text format. This would
   * include 
   */
  bool _outputTextPointData;

  /**
   * This tilt number will be considered the last tilt in the volume. 
   */
  int _lastTiltNum;

  /**
   * Previous tilt number -- we keep track of order of tilts since they can
   * arrive in realtime in the wrong order. Processing needs to be in order
   * to properly build an Mdv file
   */
  int _prevTiltNum;

  /**
   * Previous file (and current file suffix) are used to determine tilt order
   */
  string _prevFileSuffix;
  
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


  vector < pair <NidsFile*, string> > _outOfOrderTilts; 

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
   * Array of nids product gridded field data
   */
  fl32 *_data;

  /**
   * Recursively process all files under directory dirName (in archive mode).
   * Files which start with "." or "_" will be skipped. Other files will 
   * be inserted into a map container by file time. Files are processed in
   * time order starting with oldest files.
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
   * @param[in] filePath  Fully qualified file path of NIDS data file
   */
  void _convertFile(  const char *filePath);

  /**
   * Extract data from NidsFile object and put in Mdv structures.
   * Start a new Mdv volume if appropriate or add on to an existing one.
   */
  void _createMdvVolume ( NidsFile &H, const string filePath, bool newVolume);

  /** 
   * A plane or tilt of data NIDS radar data is copied to the array holding
   * the 3D volume of data. If the dimensions of the existing volume and new
   * plane are the same, then the data is just copied directly to the volume
   * array. If the dimensions are different, the volume data is first set 
   * to missing. Then the x and y dimensions of the plane data and volume 
   * data are compared. The smaller of each becomes the upperbound in each 
   * dimension for data that gets copied from the plane to the volume.
   * @param[out] volPtr  Array to which planePtr data will be copied
   * @param[in] volNx  East-West dimension of volume grid
   * @param[in] volNy  North-South dimension of volume grid
   * @param[in] planePtr Array of one tilt of radar data
   * @param[in] planeNx East-West dimension of the tilt of data
   * @param[in] planeNy North-South dimension of the tilt of data
   */
  void _copyDataToVolume( fl32** volPtr,  int volNx,  int volNy,  
			  fl32** planePtr, int planeNx, int planeNy);
  /**
   * Declare DsMdvx object. Set Mdv fieldheader, create and add mdv field, and
   * then write the field to the specified output directory member, _outUrl.
   */
  void _outputVolume();

  /**
   * Create GenPoly objects for MeltingLayer contours and write to 
   * Spdb database.
   * @param[in] NidsFile object
   */
  void _createSpdbContours( NidsFile &nidsFile);

  /**
   * Determine type of Special Graphic Symbol data ( Hail, Tornado 
   * Vortex Signature, Mesocylone, and storm Ids) and call relevant
   * method to write the data to an Spdb data base
   * @param[in] NidsFile object
   */
  void _createSpdbGenPt( NidsFile &nidsFile);

  /**
   * Create a GenPt object for each hail report and write to 
   * Spdb database
   * @param[in] NidsFile object
   * @param[in] txtFile  FILE ptr for optional text output
   */
  void _writeSpdbHail( NidsFile &nidsFile, FILE *txtFile);

  /**
   * Create a GenPt object for each Tornado Vortex Signature
   * report and write to Spdb database
   * @param[in] NidsFile object
   * @param[in] txtFile  FILE ptr for optional text output
   */
  void _writeSpdbTvs( NidsFile &nidsFile, FILE *txtFile);
  
  /**
   * Create a GenPt object for each Mesocyclone report and 
   * write to Spdb database
   * @param[in] NidsFile object
   * @param[in] txtFile  FILE ptr for optional text output
   */
  void _writeSpdbMesocyclone( NidsFile &nidsFile, FILE *txtFile);
 
  /**
   * Create a GenPt object for eac storm Id report and 
   * write to Spdb database
   * @param[in] NidsFile object
   */
  void _writeSpdbStormIDs( NidsFile &nidsFile);

  /**
   * If using threads in radar data processing, free thread and decrement
   * the number of threads in use.
   */
  void _freeThread();
  
  /**
   * Compare part of filepath tail to startOfVolStr and set private member 
   * _startOfVol to true or false accordingly.
   * @param[in] filepath  Filepath string, NIDS product identifier  is in 
   *                      the tail of the path
   * @param[in] startOfVolStr  Start of volume indicator for this product
   */
  void _setStartOfVolFlag(const string filepath, const string startOfVolStr);
  
  /**
   * Compare part of filepath tail to endOfVolStr and set private member 
   * _endOfVol to true or false accordingly.
   * @param[in] filepath  Filepath string, NIDS product identifier  is in 
   *                      the tail of the path
   * @param[in] startOfVolStr  Start of volume indicator for this product
   * @param[in] tiltNum  Current tilt number of product. Used of end of 
   *                     volume trigger is a tilt number.
   */  
  void _setEndOfVolFlag(const string filepath, const string endOfVolStr,
			const int tiltNum);
  
  void _handleGriddedData(NidsFile *nidsFile, string filePath);

  void _saveOutOfOrderTilt(NidsFile *nidsFile, string &filePath, const int tiltNum);

  void _writeOutOfOrderTiltsCurrVol( const string prevSuffix );

  
  void _dumpOldData( const int currVolNum);

  void _clearData();
};

#endif





