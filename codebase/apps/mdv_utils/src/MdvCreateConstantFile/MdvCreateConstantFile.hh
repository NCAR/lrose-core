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
 * @file MdvCreateConstantFile.hh
 *
 * @class MdvCreateConstantFile
 *
 * MdvCreateConstantFile is the top level application class.
 *  
 * @date 12/12/2011
 *
 */

#ifndef MdvCreateConstantFile_HH
#define MdvCreateConstantFile_HH

#include <cstdio>
#include <string>

#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class MdvCreateConstantFile
 */

class MdvCreateConstantFile
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

  ~MdvCreateConstantFile(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static MdvCreateConstantFile *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static MdvCreateConstantFile *Inst();
  

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

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static MdvCreateConstantFile *_instance;
  
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

  MdvCreateConstantFile(int argc, char **argv);
  
  MdvxField *_createField();
  
  inline Mdvx::compression_type_t _compressionType2Mdv(const Params::compression_type_t compression_type)
  {
    switch (compression_type)
    {
    case Params::COMPRESSION_NONE :
      return Mdvx::COMPRESSION_NONE;
    case Params::COMPRESSION_RLE :
      return Mdvx::COMPRESSION_RLE;
    case Params::COMPRESSION_LZO :
      return Mdvx::COMPRESSION_LZO;
    case Params::COMPRESSION_ZLIB :
      return Mdvx::COMPRESSION_ZLIB;
    case Params::COMPRESSION_BZIP :
      return Mdvx::COMPRESSION_BZIP;
    case Params::COMPRESSION_GZIP :
      return Mdvx::COMPRESSION_GZIP;
    case Params::COMPRESSION_GZIP_VOL :
      return Mdvx::COMPRESSION_GZIP_VOL;
    }

    // Should never get here

    return Mdvx::COMPRESSION_NONE;
  }
  
  inline Mdvx::encoding_type_t _encodingType2Mdv(const Params::encoding_type_t encoding_type)
  {
    switch (encoding_type)
    {
    case Params::ENCODING_INT8 :
      return Mdvx::ENCODING_INT8;
    case Params::ENCODING_INT16 :
      return Mdvx::ENCODING_INT16;
    case Params::ENCODING_FLOAT32 :
      return Mdvx::ENCODING_FLOAT32;
    }

    // Should never get here

    return Mdvx::ENCODING_FLOAT32;
  }
  
  inline Mdvx::scaling_type_t _scalingType2Mdv(const Params::scaling_type_t scaling_type)
  {
    switch (scaling_type)
    {
    case Params::SCALING_NONE :
      return Mdvx::SCALING_NONE;
    case Params::SCALING_ROUNDED :
      return Mdvx::SCALING_ROUNDED;
    case Params::SCALING_INTEGRAL :
      return Mdvx::SCALING_INTEGRAL;
    case Params::SCALING_DYNAMIC :
      return Mdvx::SCALING_DYNAMIC;
    case Params::SCALING_SPECIFIED :
      return Mdvx::SCALING_SPECIFIED;
    }

    // Should never get here

    return Mdvx::SCALING_NONE;
  }
  
  inline Mdvx::vlevel_type_t _vertType2Mdv(const Params::vert_type_t vert_type)
  {
    switch (vert_type)
    {
    case Params::VERT_TYPE_UNKNOWN :
      return Mdvx::VERT_TYPE_UNKNOWN;
    case Params::VERT_TYPE_SURFACE :
      return Mdvx::VERT_TYPE_SURFACE;
    case Params::VERT_TYPE_SIGMA_P :
      return Mdvx::VERT_TYPE_SIGMA_P;
    case Params::VERT_TYPE_PRESSURE :
      return Mdvx::VERT_TYPE_PRESSURE;
    case Params::VERT_TYPE_Z :
      return Mdvx::VERT_TYPE_Z;
    case Params::VERT_TYPE_SIGMA_Z :
      return Mdvx::VERT_TYPE_SIGMA_Z;
    case Params::VERT_TYPE_ETA :
      return Mdvx::VERT_TYPE_ETA;
    case Params::VERT_TYPE_THETA :
      return Mdvx::VERT_TYPE_THETA;
    case Params::VERT_TYPE_MIXED :
      return Mdvx::VERT_TYPE_MIXED;
    case Params::VERT_TYPE_ELEV :
      return Mdvx::VERT_TYPE_ELEV;
    case Params::VERT_TYPE_COMPOSITE :
      return Mdvx::VERT_TYPE_COMPOSITE;
    case Params::VERT_TYPE_CROSS_SEC :
      return Mdvx::VERT_TYPE_CROSS_SEC;
    case Params::VERT_TYPE_SATELLITE_IMAGE :
      return Mdvx::VERT_SATELLITE_IMAGE;
    case Params::VERT_TYPE_VARIABLE_ELEV :
      return Mdvx::VERT_VARIABLE_ELEV;
    case Params::VERT_TYPE_FIELDS_VAR_ELEV :
      return Mdvx::VERT_FIELDS_VAR_ELEV;
    case Params::VERT_TYPE_FLIGHT_LEVEL :
      return Mdvx::VERT_FLIGHT_LEVEL;
    case Params::VERT_TYPE_EARTH_IMAGE :
      return Mdvx::VERT_EARTH_IMAGE;
    case Params::VERT_TYPE_AZ :
      return Mdvx::VERT_TYPE_AZ;
    case Params::VERT_TYPE_TOPS :
      return Mdvx::VERT_TYPE_TOPS;
    case Params::VERT_TYPE_ZAGL_FT :
      return Mdvx::VERT_TYPE_ZAGL_FT;
    case Params::VERT_TYPE_SOIL :
      return Mdvx::VERT_SOIL;
    case Params::VERT_TYPE_WRF_ETA :
      return Mdvx::VERT_TYPE_WRF_ETA;
    }

    // Should never get here

    return Mdvx::VERT_TYPE_UNKNOWN;
  }
  
  void _updateMasterHeader(DsMdvx &mdvx);
  

};


#endif
