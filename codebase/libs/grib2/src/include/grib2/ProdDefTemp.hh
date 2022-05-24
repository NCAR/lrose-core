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
 * @file ProdDefTemp.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_PROD_DEF_TEMPLATE
#define _GRIB2_PROD_DEF_TEMPLATE

#include <dataport/port_types.h>
#include <grib2/Grib2Record.hh>
#include <grib2/GribSection.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

class Grib2Record;

/** 
 * @class ProdDefTemp
 *
 * ProdDefTemp is an abstract class for grib Product Definition Templates.
 * As an abstract class no object can be declared as type ProDefTemp, it
 * can only be used with as a reference or pointer.
 *
 * Known derived classes include:  Template4_pt_0, Template4_pt_1, Template4_pt_2,
 * Template4_pt_5, Template4_pt_6, Template4_pt_7 and Template4_pt_8
 *
 * Product Definition Template is part of the Product Definition Section (PDS).
 *
 * @note The pack and unpack routines are static methods in GribSection.hh
 *
 */
class ProdDefTemp {

public:

  /** 
   * @brief Defualt Constructor usually called only from derived classes.
   */
  ProdDefTemp();

  /** 
   * @brief Internal constructor used during grib2 decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  ProdDefTemp(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~ProdDefTemp();

  /** @brief Unpack a Product Definition Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack( ui08 *projPtr ) = 0;

  /** @brief Pack up this Product Definition Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack( ui08 *projPtr ) = 0;

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const = 0;


  /** Get Functions, used when reading a grib file */

  /** @brief Requests that the internal name and units be set based on the parameter
   * category and number. These strings can then be requested through the
   * getRecSummary() function. */
  void setParamStrings();

  /** @brief Get the record summary for this grib record 
   *  @param[out] summary Record summary struct */
  virtual void getRecSummary (Grib2Record::rec_summary_t *summary ) = 0;

  /** @brief Get the forecast time for this grib record 
   *  @return Forecast lead time in seconds */
  virtual long int getForecastTime() const = 0;

  /** @brief Get the size of the packed derived template class. 
   *  @return Size of packed template in bytes */ 
  virtual si32 getTemplateSize() = 0;

  /** Set Functions, used when creating a grib file */

  /** @brief Set the Parameters category and number 
   *  @param[in] category The grib2 Parameter Category, 0-255
   *  @param[in] number The grib2 Parameter number, 0-255 */
  inline void setParamNumbers(si32 category, si32 number) {_parameterCategory = category; _paramNumber = number; };

  /** @brief Used internally to set pointers to other sections of the grib file.
   *  @param[in] sectionsPtr Sections pointer struct */
  void setSectionsPtr(Grib2Record::Grib2Sections_t sectionsPtr);

  /** @brief Set the Generating process ID number
   *  See http://www.nco.ncep.noaa.gov/pmb/docs/on388/tablea.html
   *  @param[in] generatingProcessID The process ID number, 0-255 */
  inline void setProcessID(si32 generatingProcessID) { _processID = generatingProcessID; };

  /** @brief Returns the Generating Process Name 
   *  @return A string with name of the generating process */
  string getGeneratingProcess() const;

  /** @details Struct used for local parameters.
   *
   * Local table is defined as category or paramNumber >= 192.
   */
  typedef struct {
    /** Product Discipline 
     * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table0-0.shtml */
    si32 prodDiscipline;
    /** Product Category 
     * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-1.shtml */
    si32 category;
    /** Parameter Number within the category */
    si32 paramNumber;
    /** Official Abbreviation for the parameter */
    string name;
    /** Official Name for the parameter */
    string comment;
    /** Units of the parameter */
    string unit;
  } _GRIB2LocalTable;

  /** @details Struct used for official parameters */
  typedef struct {
    /** Official Abbreviation for the parameter */
    string name;
    /** Official Name for the parameter */
    string comment;
    /** Units of the parameter */
    string unit;
  } _GRIB2ParmTable;

  /** @details Struct used for official surface types */
  typedef struct {
    /** Official Abbreviation for the parameter */
     string name;
    /** Official Name for the parameter */
     string comment;
    /** Units of the parameter */
     string unit;
  } _GRIB2SurfTable;

  /* OFFICIAL TABLES */

  /** @brief Fixed surface types and units. GRIB2 Code Table 4.5 */
  static const _GRIB2SurfTable _surface[106];

  /** @brief Meteorological products Temperature category. GRIB2 Code table 4.2 : 0.0 */
  static const _GRIB2ParmTable _meteoTemp[30];

  /** @brief Meteorological products Moisture category. GRIB2 Code table 4.2 : 0.1 */
  static const _GRIB2ParmTable _meteoMoist[122];

  /** @brief Meteorological products Momentum category. GRIB2 Code table 4.2 : 0.2 */
  static const _GRIB2ParmTable _meteoMoment[47];

  /** @brief Meteorological products Mass category. GRIB2 Code table 4.2 : 0.3 */
  static const _GRIB2ParmTable _meteoMass[32];

  /** @brief Meteorological products Short wave radiation category. GRIB2 Code table 4.2 : 0.4 */
  static const _GRIB2ParmTable _meteoShortRadiate[54];

  /** @brief Meteorological products Long wave radiation category. GRIB2 Code table 4.2 : 0.5 */
  static const _GRIB2ParmTable _meteoLongRadiate[9];

  /** @brief Meteorological products Cloud category. GRIB2 Code table 4.2 : 0.6 */
  static const _GRIB2ParmTable _meteoCloud[50];

  /** @brief Meteorological products Thermodynamic stability indices category. GRIB2 Code table 4.2 : 0.7 */
  static const _GRIB2ParmTable _meteoStability[20];

  /** @brief Meteorological products Aerosols category. GRIB2 Code table 4.2 : 0.13 */
  static const _GRIB2ParmTable _meteoAerosols[1];

  /** @brief Meteorological products Trace gases category. GRIB2 Code table 4.2 : 0.14 */
  static const _GRIB2ParmTable _meteoGases[3];

  /** @brief Meteorological products Radar category. GRIB2 Code table 4.2 : 0.15 */
  static const _GRIB2ParmTable _meteoRadar[17];

  /** @brief Meteorological products Forecast Radar Imagery category. GRIB2 Code table 4.2 : 0.16 */
  static const _GRIB2ParmTable _meteoRadarForecast[6];

  /** @brief Meteorological products Electrodynamics category. GRIB2 Code table 4.2 : 0.17 */
  static const _GRIB2ParmTable _meteoElectro[5];

  /** @brief Meteorological products Nuclear/radiology category. GRIB2 Code table 4.2 : 0.18 */
  static const _GRIB2ParmTable _meteoNuclear[19];

  /** @brief Meteorological products Physical atmospheric properties category. GRIB2 Code table 4.2 : 0.19 */
  static const _GRIB2ParmTable _meteoAtmos[37];

  /** @brief  Meteorological products Atmospheric Chemical Constituents category. GRIB2 Code table 4.2 : 0.20  */
  static const _GRIB2ParmTable _meteoChem[113];

  /** @brief Meteorological products CCITT IA5 string category. GRIB2 Code table 4.2 : 0.190 */
  static const _GRIB2ParmTable _meteoText[1];

  /** @brief Meteorological products Miscellaneous category. GRIB2 Code table 4.2 : 0.191 */
  static const _GRIB2ParmTable _meteoMisc[4];

  /** @brief Hydrologic products basic products category. GRIB2 Code table 4.2 : 1.0 */
  static const _GRIB2ParmTable _hydroBasic[17];
  
  /** @brief Hydrologic products probabilities category. GRIB2 Code table 4.2 : 1.1 */
  static const _GRIB2ParmTable _hydroProb[3];

  /** @brief Hydrologic products probabilities category. GRIB2 Code table 4.2 : 1.2 */
  static const _GRIB2ParmTable _hydroWaterSediment[14];
  
  /** @brief Land Surface products Vegetation/Biomass category. GRIB2 Code table 4.2 : 2.0 */
  static const _GRIB2ParmTable _landVeg[39];
  
  /** @brief Land Surface products Soil products category. GRIB2 Code table 4.2 : 2.3 */
  static const _GRIB2ParmTable _landSoil[28];

  /** @brief Land Surface products Fire Weather products category. GRIB2 Code table 4.2 : 2.4 */
  static const _GRIB2ParmTable _landFire[12];

  /** @brief Land Surface products, Glaciers and Inland Ice category. GRIB2 Code table 4.2 : 2.5 */
  static const _GRIB2ParmTable _landIce[2];
  
  /** @brief Space products Image format products category. GRIB2 Code table 4.2 : 3.0 */
  static const _GRIB2ParmTable _spaceImage[10];
  
  /** @brief Space products Quantitative products category. GRIB2 Code table 4.2 : 3.1 */
  static const _GRIB2ParmTable _spaceQuantitative[30];

  /** @brief Oceanographic products Waves category. GRIB2 Code table 4.2 : 10.0 */
  static const _GRIB2ParmTable _oceanWaves[46];
  
  /** @brief Oceanographic products Currents category. GRIB2 Code table 4.2 : 10.1 */
  static const _GRIB2ParmTable _oceanCurrents[4];
  
  /** @brief Oceanographic products Ice category. GRIB2 Code table 4.2 : 10.2 */
  static const _GRIB2ParmTable _oceanIce[13];
  
  /** @brief Oceanographic products Surface properties category. GRIB2 Code table 4.2 : 10.3 */
  static const _GRIB2ParmTable _oceanSurface[3];
  
  /** @brief Oceanographic products Sub-surface properties category. GRIB2 Code table 4.2 : 10.4 */
  static const _GRIB2ParmTable _oceanSubSurface[16];

  /** @brief Oceanographic products Miscellaneous category. GRIB2 Code table 4.2 : 10.191 */
  static const _GRIB2ParmTable _oceanMisc[4];

  /* LOCAL TABLES */
  /** @brief Size of table _NCAR_RALlocalTable */
  static const ui32 _NCAR_RALlocalTable_numElements = 8;
  /** @brief NCAR / RAL local use table 
   *
   * Parameters can be added to this table as needed */
  static const _GRIB2LocalTable _NCAR_RALlocalTable[ _NCAR_RALlocalTable_numElements ];

  /** @brief Size of table _NCEPlocalTable */
  static const ui32 _NCEPlocalTable_numElements = 388;
  /** @brief NCEP's Local Use table */
  static const _GRIB2LocalTable _NCEPlocalTable[ _NCEPlocalTable_numElements ];

  /** @brief Size of table _SPClocalTable */
  static const ui32 _SPClocalTable_numElements = 27;
  /** @brief NCEP's Local Use table */
  static const _GRIB2LocalTable _SPClocalTable[ _SPClocalTable_numElements ];

  /** @brief Size of table _MSC_MONTREAL_localTable */
  static const ui32 _MSC_MONTREAL_localTable_numElements = 4;
  /** @brief Meteorological Service of Canada (MSC) - Montreal - local use table */
  static const _GRIB2LocalTable _MSC_MONTREAL_localTable[ _MSC_MONTREAL_localTable_numElements ];

  /** @brief Size of table _NOAA_FSLlocalTable */
  static const ui32 _NOAA_FSLlocalTable_numElements = 45;
  /** @brief NOAA Forecast Systems Lab local use table */
  static const _GRIB2LocalTable _NOAA_FSLlocalTable[ _NOAA_FSLlocalTable_numElements ];

  /** @brief Size of table _MRMSlocalTable */
  static const ui32 _MRMSlocalTable_numElements = 110;
  /** @brief NSSL MRMS local use table */
  static const _GRIB2LocalTable _MRMSlocalTable[ _MRMSlocalTable_numElements ];

  /** @brief Size of table _COSMO_localTable */
  static const ui32 _COSMO_localTable_numElements = 7;
  /** @brief NCAR / RAL local use table 
   *
   * Parameters can be added to this table as needed */
  static const _GRIB2LocalTable _COSMO_localTable[ _COSMO_localTable_numElements ];

  /** @details Struct for optional repeating section */
  typedef struct
  {
    /** Statistical process used to calculate the processed field from
     * the field at each time increment during the time range (see Code Table 4.10) */
    si32 _processId;
    /** Type of time increment between successive fields used in the statistical
     * processing (See Code Table 4.11) */
    si32 _timeIncrementType;
    /** Indicator of unit of time for time range over which statistical processing
     * is done (See Code Table 4.3) */
    si32 _timeRangeUnit;
    /** Length of the time range over which statistical processing is done, in units
     * defined by the _timeRangeUnit */
    si32 _timeRangeLen;
    /** Indicator of unit of time for the increment between the successive 
     * fields used (See Code Table 4.3) */
    si32 _timeIncrUnit;
    /** Time increment between successive fields, in units defined by the
     *_timeIncrUnit */
    si32 _timeIncrement;
  } interval_t;

protected:
  /** @brief translates a surface number into a index to the _surface table 
   *  @param[in] value a fixed surface type number 
   *  @return The index into the _surface table */
  int _getSurfaceIndex(int value) const;

  /** @breif Prints the long name of the time increment type */
  void _printTimeIncrementType(FILE *stream, int timeIncrement) const;

  /** @brief Prints the Generating Process type */
  void _printGeneratingProcessType(FILE *stream, int processType) const;

  /** @brief Prints the Spatial interpolation process type */
  void _printSpatialProcess(FILE *stream, int processId) const;

  /** @brief Prints the Ensemble Forecast Type */
  void _printEnsembleForecastType(FILE *stream, int ensembleId) const;

  /** @brief Prints the Derived Forecast Type */
  void _printDerivedForecastType(FILE *stream, int derivedId) const;

  /** @brief Returns the Derived Forecast Type
    *  @return A string with name of the derived forecast type */
  string _getDerivedForecastType(int derivedId) const;

  /** @brief Prints the Probability type */
  void _printProbabilityType(FILE *stream, int probId) const;

  /** @brief Prints the Statistical process type */
  void _printStatisticalProcess(FILE *stream, int processId) const;

  /** @brief Returns the Statistical process type 
   *  @return A string with abbreviation of the statistical process type */
  string _getStatisticalProcess(int processId) const;

  /** @brief Print to stream/file a string representing the time units */
  void _printTimeUnits(FILE *stream, int timeUnits) const;

  /** @brief Get the time multiplication factor based for this timeUnit 
   *  @param[in] timeUnits A time unit, (See Code Table 4.3) 
   *  @return Factor to multiply the time by to convert it to seconds */
  long int _getTimeUnits( long int timeUnits) const;

  /** @brief Get a string representing the time
   *  @param[in] time The time
   *  @param[in] timeUnits A time unit, (See Code Table 4.3) 
   *  @return A string with units represeing the time */
  string _getTimeUnitName(int time, int timeUnits) const;

  /** @brief Struct containing pointers to other parts of this grib file 
   *  @note Only sections appearing before the PDS will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief The parameter category number */
  si32 _parameterCategory;
  /** @brief The parameter number */
  si32 _paramNumber;

  /** @brief The Generating processes identifier */
  si32 _processID; 
  /** @brief The ceter id number from the IDS section */
  si32 _centerId;
  /** @brief The sub center id number for the IDS section */
  si32 _subCenterId;

  /** @brief The discipline number from the IS section */
  si32 _disciplineNum;

  /** @brief the Paramenter name */
  string *_parameterLongName;
  /** @brief the Paramenter abbreviation */
  string *_parameterName;
  /** @brief the Paramenter units */
  string *_parameterUnits;

private: 


};

} // namespace Grib2

#endif

