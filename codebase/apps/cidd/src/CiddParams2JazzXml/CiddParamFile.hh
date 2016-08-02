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
 * @file CiddParamFile.hh
 *
 * @class CiddParamFile
 *
 * Class representing a CIDD parameter file
 *  
 * @date 9/24/2010
 *
 */

#ifndef CiddParamFile_HH
#define CiddParamFile_HH

#include "GuiConfigParams.hh"
#include "GridsParams.hh"
#include "MainParams.hh"
#include "MapsParams.hh"
#include "SymprodsParams.hh"
#include "TerrainParams.hh"
#include "WindsParams.hh"

using namespace std;


class CiddParamFile
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  CiddParamFile();
  
  /**
   * @brief Destructor
   */

  virtual ~CiddParamFile(void);
  
  /**
   * @brief Initialize the parameters.
   *
   * @param[in] param_file_path CIDD parameter file path.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const string &param_file_path);
  

  ////////////////////
  // Access methods //
  ////////////////////

  const MainParams &getMainParams() const
  {
    return _mainParams;
  }
  
  const GuiConfigParams &getGuiConfigParams() const
  {
    return _guiConfig;
  }
  
  const vector< GridField > &getGridFields() const
  {
    return _grids.getGridFields();
  }

  const vector< WindField > &getWindFields() const
  {
    return _winds.getWindFields();
  }

  const vector< MapField > &getMapFields() const
  {
    return _maps.getMapFields();
  }

  const vector< SymprodField > &getSymprodFields() const
  {
    return _symprods.getSymprodFields();
  }

  const TerrainParams &getTerrainParams() const
  {
    return _terrain;
  }

  
 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The MAIN_PARAMS section.
   */

  MainParams _mainParams;
  
  /**
   * @brief The GUI_CONFIG section.
   */

  GuiConfigParams _guiConfig;
  
  /**
   * @brief The GRIDS section.
   */

  GridsParams _grids;
  
  /**
   * @brief The WINDS section.
   */

  WindsParams _winds;
  
  /**
   * @brief The MAPS section.
   */

  MapsParams _maps;
  
  /**
   * @brief The SYMPRODS section.
   */

  SymprodsParams _symprods;
  
  /**
   * @brief The TERRAIN section.
   */

  TerrainParams _terrain;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Read the indicated CIDD parameter file into a buffer in memory.
   *
   * @param[in] fname The full file path for the CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readFile(const string &fname);
  

};


#endif
