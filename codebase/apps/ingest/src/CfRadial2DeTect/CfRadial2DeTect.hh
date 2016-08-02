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
 *
 * @file CfRadial2DeTect.hh
 *
 * @class CfRadial2DeTect
 *
 * CfRadial2DeTect is the top level application class.
 *  
 * @date 7/5/2011
 *
 */

#ifndef CfRadial2DeTect_HH
#define CfRadial2DeTect_HH

#include <cstdio>
#include <string>

#include <DeTect/ArchiveDirectory.hh>
#include <dsdata/DsTrigger.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class CfRadial2DeTect
 */

class CfRadial2DeTect
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  ~CfRadial2DeTect(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static CfRadial2DeTect *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static CfRadial2DeTect *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief the size of each sector to include in a DataObject in degrees.
   */

  static const double SECTOR_SIZE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static CfRadial2DeTect *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  
  /**
   * @brief The data trigger object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief The output file.
   */

  TaFile _outputFile;
  
  /**
   * @brief Flag indicating whether we've opened the output file yet.  We open
   *        the output file and write the archive label when processing the 
   *        first input file.
   */

  bool _outputFileCreated;
  
  /**
   * @brief Flag indicating whether this is the first sector being processed.
   */

  bool _firstSector;
  
  /**
   * @brief The archive label information written at the beginning of the
   *        DeTect file.
   */

  ArchiveLabel _archiveLabel;

  /**
   * @brief The archive directory information written at the end of the DeTect
   *        file.
   */

  ArchiveDirectory _archiveDirectory;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  CfRadial2DeTect(int argc, char **argv);
  

  /**
   * @brief Initialize the data trigger object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process the given input file.
   *
   * @param[in] file_path The full path of the input file to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFile(const string &file_path);
  

  /**
   * @brief Process the data for the given sector.  This is where the data
   *        is actually written to the DeTect file.
   *
   * @param[in] volume               The full radar volume.
   * @param[in] sweep                The current sweep information.
   * @param[in] rays                 Pointers to the rays in the volume.
   * @param[in] start_sector_index   The index of the first ray in the sector.
   * @param[in] end_sector_index     The index of the last ray inthe sector.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processSector(const RadxVol &volume,
		      const RadxSweep *sweep,
		      const vector< RadxRay* > &rays,
		      const size_t start_sector_index,
		      const size_t end_sector_index);
  
  /**
   * @brief Process the given sweep.
   *
   * @param[in] volume The full radar volume.
   * @param[in] sweep  Pointer to the sweep information.
   * @param[in] rays   List of rays in the volume.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processSweep(const RadxVol &volume,
		     const RadxSweep *sweep,
		     const vector< RadxRay* > &rays);
  
};


#endif
