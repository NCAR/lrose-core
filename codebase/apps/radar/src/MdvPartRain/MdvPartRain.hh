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
/////////////////////////////////////////////////////////////
// MdvPartRain.hh
//
// MdvPartRain object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////

/**
 * @file MdvPartRain.hh
 * @brief Compute derived fields from dual pol data
 * @class MdvPartRain
 *
 * MdvPartRain reads dual pol data from an MDV file, computes
 * derived fields and writes the fields to an output file.
 *
 */

#ifndef MdvPartRain_H
#define MdvPartRain_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/DsMdvx.hh>
#include <radar/PrecipRate.hh>
#include <radar/KdpBringi.hh>
#include <radar/NcarParticleId.hh>
#include <toolsa/TaArray.hh>
using namespace std;

class MdvPartRain {
  
public:


  /**
   * Constructor
   * @param[in] argc Number of command-line arguments
   * @param[in] argv Array of command-line arguments
   */
  MdvPartRain (int argc, char **argv);

  /**
   * Destructor
   */
  ~MdvPartRain();

  /**
   * Run the application
   * @return 0 on success, -1 on failure
   */
  int Run();

  bool isOK; /**< Flag to indicate whether the application initialized correctly */

  const static double missingDouble;   /**< Flag for missing type double data */
  const static fl32 missingFloat;      /**< Flag for missing type float data */
  const static fl32 pseudoEarthDiamKm; /**< Diameter of the earth, if it was a perfect sphere */

protected:
  
private:

  string _progName;   /**< Name of the application (for debugging messages) */
  char *_paramsPath;  /**< Path to the application parameter file */
  Args _args;         /**< Command-line arguments parsed into an Args object */
  Params _params;     /**< Application parameters parsed into a Params object */
  DsMdvxInput _input; /**< Object used to provide input control for MDV data */
  DsMdvx _inMdvx;     /**< Object used to read input MDV data */

  const Mdvx::master_header_t *_mhdr;  /**< Pointer to data master header */
  const Mdvx::field_header_t *_fhdr0;  /**< Pointer to a field header */
  const Mdvx::vlevel_header_t *_vhdr0; /**< Pointer to field vertical level header */
  
  PrecipRate _rate;        /**< Object to calculate precip rates for a radar beam */
  NcarParticleId _partId;  /**< Object to calculate particle id for a radar beam*/
  KdpBringi _kdpBringi;    /**< Object to calculate Kdp for SBand, based on Bringi code */

  /////////////////////////////////////////
  // input data

  int _nGates;   
  int _nBeams;   
  int _nTilts;   
  int _nPoints;  
  double _startRange;
  double _gateSpacing;
  double _startAz;
  double _deltaAz;
  vector<double> _elevs;
  bool _isPolarRadar;
  bool _isRhiRadar;
  bool _isPpiCart;
  bool _vlevelInKm;
  double _rhiMinEl;
  double _rhiDeltaEl;
  double _wavelengthCm;
  double _radarHtKm;
  
  // These are pointers into the input Mdvx object.
  // This memory is managed by the Mdvx class and should not be freed
  // by the calling class.

  fl32 *_dbz;
  fl32 *_snr;
  fl32 *_zdr;
  fl32 *_ldr;
  fl32 *_phidp;
  fl32 *_rhohv;

  fl32 _dbzMiss;
  fl32 _snrMiss;
  fl32 _zdrMiss;
  fl32 _ldrMiss;
  fl32 _phidpMiss;
  fl32 _rhohvMiss;
  fl32 _kdpMiss;

  //////////////////////////////
  // output fields for MDV volume

  TaArray<fl32> _kdp_;
  TaArray<fl32> _dbzForKdp_;
  TaArray<fl32> _snrForKdp_;
  TaArray<fl32> _zdrForKdp_;
  TaArray<fl32> _rhohvForKdp_;
  TaArray<fl32> _phidpForKdp_;
  TaArray<fl32> _sdphidpForKdp_;

  TaArray<fl32> _dbzForRate_;
  TaArray<fl32> _zdrForRate_;
  TaArray<fl32> _kdpForRate_;
  TaArray<fl32> _rateZh_;
  TaArray<fl32> _rateZZdr_;
  TaArray<fl32> _rateKdp_;
  TaArray<fl32> _rateKdpZdr_;
  TaArray<fl32> _rateHybrid_;

  TaArray<fl32> _dbzForPid_;
  TaArray<fl32> _zdrForPid_;
  TaArray<fl32> _ldrForPid_;
  TaArray<fl32> _phidpForPid_;
  TaArray<fl32> _rhohvForPid_;
  TaArray<fl32> _kdpForPid_;
  TaArray<fl32> _sdzdrForPid_;
  TaArray<fl32> _sdphidpForPid_;
  TaArray<fl32> _tempForPid_;
  TaArray<fl32> _pidInterest_;
  TaArray<fl32> _pidInterest2_;
  TaArray<fl32> _pidConfidence_;
  TaArray<ui16> _pid_;
  TaArray<ui16> _pid2_;

  fl32 *_kdp;
  fl32 *_dbzForKdp;
  fl32 *_snrForKdp;
  fl32 *_zdrForKdp;
  fl32 *_rhohvForKdp;
  fl32 *_phidpForKdp;
  fl32 *_sdphidpForKdp;

  fl32 *_dbzForRate;
  fl32 *_zdrForRate;
  fl32 *_kdpForRate;
  fl32 *_rateZh;
  fl32 *_rateZZdr;
  fl32 *_rateKdp;
  fl32 *_rateKdpZdr;
  fl32 *_rateHybrid;

  fl32 *_dbzForPid;
  fl32 *_zdrForPid;
  fl32 *_ldrForPid;
  fl32 *_phidpForPid;
  fl32 *_rhohvForPid;
  fl32 *_kdpForPid;
  fl32 *_sdzdrForPid;
  fl32 *_sdphidpForPid;
  fl32 *_tempForPid;
  fl32 *_pidInterest;
  fl32 *_pidInterest2;
  fl32 *_pidConfidence;
  ui16 *_pid;
  ui16 *_pid2;

  vector<TaArray<fl32> > _pIntArray_;
  vector<fl32 *> _pIntArray;

  // methods

  /**
   * Set up the read for the next MDV file
   */
  void _setupRead();
 
  /**
   * Read in next radar volume
   */ 
  int _readNextVolume();
  
  /**
   * Check that the input fields are uniform in size
   * @return True on success, False on error
   */
  bool _checkFields();
  
  /**
   * Set up pointers to the input data
   * @return 0 on success, -1 on error
   */
  int _setInputPointers();

  /**
   * Compute Kdp
   */
  void _computeKdp();

  /**
   * Compute precip rates
   */
  void _computeRates();
  
  /**
   * Compute particle id
   * @return 0 on success, -1 on error
   */
  int _computePid();
  
  /**
   * Fill temperature array for polar data, computing height
   * from elevation angle and range
   * @param[in] elev The radar elevation angle to process
   * @param[out] temp The grid to populate with temperature data
   */
  void _fillTempArrayPolar(double elev, double *temp);

  /**
   * Fill temperature array for cart PPI
   * computing ht from elevation angle and range from radar
   * @param[in] iy The beam index
   * @param[in] elev The radar elevation angle to process
   * @param[out] temp The grid to populate with temperature data
   */
  void _fillTempArrayPpi(int iy, double elev, double *temp);

  /**
   * Fill temperature array, for Cartesian volume
   * @param[in] htKm The height to process (in km)
   * @param[out] temp The grid to populate with temperature data
   */
  void _fillTempArrayCart(double htKm, double *temp);

  /**
   * Fill output MDV object by defining fields and adding
   * data
   * @param[out] outMdvx The data object to fill
   */
  void _fillOutput(DsMdvx &outMdvx);

  /**
   * Add a float output field to working MDV object
   * @param[out] outMdvx The mdvx object to add the field to
   * @param[in] field_name The short name of the field to add
   * @param[in] long_field_name The descriptive name of the field to add
   * @param[in] units The unit of the field
   * @param[in] encoding_type How the data will be encoded on disk
   * @param[in] data The float data with which to populate the field
   */
  void _addField(DsMdvx &outMdvx,
		 const string &field_name,
		 const string &long_field_name,
		 const string &units,
		 int encoding_type,
		 const fl32 *data);

  /**
   * Add an integer output field to working MDV object
   * @param[out] outMdvx The mdvx object to add the field to
   * @param[in] field_name The short name of the field to add
   * @param[in] long_field_name The descriptive name of the field to add
   * @param[in] units The unit of the field
   * @param[in] encoding_type How the data will be encoded on disk
   * @param[in] data The integer data with which to populate the field
   */
  void _addField(DsMdvx &outMdvx,
		 const string &field_name,
		 const string &long_field_name,
		 const string &units,
		 int encoding_type,
		 const ui16 *data);

  /**
   * Add input fields that will be passed through unchanged
   * to the output MDV object.
   * @param[out] The mdvx object to add fields to
   */
  int _echoInputFields(DsMdvx &outMdvx);

  /**
   * Write interest fields for individual particle types
   * @param[out] The mdvx object to add fields to
   */
  void _writeParticleInterestFields(DsMdvx &outMdvx);

  /**
   * Write populated output MDV object to disk
   * @param[in] The mdvx object to write out.
   */
  int _writeOutput(DsMdvx &outMdvx);

  /**
   * Override temperature profile from parameter file
   * using sounding from SPDB
   * @param[in] dataTime The time for which a current sounding
   *            is needed to determine the temperature profile
   * @return 0 on success, -1 on error
   */
  int _overrideTempProfile(time_t dataTime);

  /**
   * Compute the SNR field from the DBZ field
   */
  void _computeSnrFromDbz();

  /**
   * Free the SNR field memory, if it has been allocated locally
   */
  void _freeSnr();

};

#endif

