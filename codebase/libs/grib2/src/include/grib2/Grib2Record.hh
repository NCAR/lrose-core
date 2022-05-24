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
 * @file Grib2Record.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef Grib2_Grib2Record_HH
#define Grib2_Grib2Record_HH

#include <grib2/IndicatorSec.hh>
#include <grib2/IdSec.hh>
#include <grib2/ES.hh>
#include <vector>
#include <list>

using namespace std;

namespace Grib2 {

class ProdDefTemp;
class IndicatorSec;
class IdSec;
class LocalUseSec;
class DRS;
class DataRepTemp;
class GDS;
class GribProj;
class BMS;
class DS;
class ES;
class PDS;

/** 
 * @class Grib2Record
 *
 * Container for a Grib2 Record.  A grib2 file can contain multiple records and 
 *  each record can contain multiple fields.
 */
class Grib2Record
{

public:

  /** @details Struct used for requesting printing */
  typedef struct {
    /** Request Indicator Section(s) */
    bool is;
    /** Request Identification Section(s) */
    bool ids;
    /** Request Local Use Section(s) */
    bool lus;
    /** Request rid Definition Section(s) */
    bool gds;
    /** Request Product Definition Section(s) */
    bool pds;
    /** Request Data Representation Section(s) */
    bool drs;
    /** Request Bit Map Section(s) */
    bool bms;
    /** Request Data Section(s) */
    bool ds;
  } print_sections_t;

  /** @details Struct used for summary of a field */  
  typedef struct {
    /** Discipline number of field */
    si32    discipline;
    /** Parameter Category number of field */
    si32    category;
    /** Parameter number of field */
    si32    paramNumber;
    /** Forecast time of field, with units */
    string forecastTime;
    /** Short or Abbreviated Name of field */
    string  name;
    /** Full name of field */
    string  longName;
    /** Units of field */
    string  units;
    /** Short or Abbreviated Level type of field */
    string  levelType;
    /** Full level type name of field */
    string  levelTypeLong;
    /** Level Units of field */
    string  levelUnits;
    /** Horizontal Level, or Top of level if level has a range */
    fl32    levelVal;
    /** Bottom of level */
    fl32    levelVal2;
    /** Additional Summary Information */
    string  additional;
  } rec_summary_t;
  
  /** @details Struct used for accessing a single field */
  typedef struct {
    /** Section 0, Indicator Section */
    IndicatorSec *is;
    /** Section 1, Identification Section */
    IdSec *ids;
    /** Section 2, Local Use Section */
    LocalUseSec *lus;
    /** Section 3, Grid Definition Section  */
    GDS  *gds;
    /** Section 4, Product Definition Section */
    PDS  *pds;
    /** Section 5, Data Representation Section */
    DRS  *drs;
    /** Section 6, Bit-map Section */
    BMS  *bms;
    /** Section 7, Data Section */
    DS   *ds;
    /** Summary of field */
    rec_summary_t *summary;
    /** Section 8, End Section */
    ES   *es;
  } Grib2Sections_t;

  /** @brief Empty constructor */
  Grib2Record();

  /** @brief This constructor sets the Is and Ids sections 
   *
   * @param[in] disciplineNumber Discipline type of data contained within 
   * - 0 = Meteorological
   * - 1 = Hydrological products
   * - 2 = Land surface products
   * - 3 = Space products
   * - 10 = Oceanographic products
   * @param[in] referenceTime Time of data according to referenceTimeType
   * @param[in] referenceTimeType Significane of referenceTime
   * - 0 = Analysis
   * - 1 = Start of forecast
   * - 2 = Verifying time of forecast
   * - 3 = Observation time
   * @param[in] typeOfData Type of data contained within 
   * - 0 = Analysis products
   * - 1 = Forecast products
   * - 2 = Analysis and Forecast products
   * - 3 = Control Forecast Products
   * - 4 = Perturbed Forecast Products
   * - 5 = Control and Perturbed Forecast Products
   * - 6 = Processed satellite observations
   * - 7 = Processed radar observations
   * @param[in] generatingSubCentreID Code to identify a creating process or group within generatingCentreID
   * @param[in] generatingCentreID Code to identify greating centre
   *       (see http://www.nco.ncep.noaa.gov/pmb/docs/on388/table0.html for complete list)
   * - 7 = NWS NCEP
   * - 59 = NOAA Forecast Systems Lab
   * - 60 = National Center for Atmospheric Research (NCAR)  (Default value)
   * @param[in] productionStatus Production status of data contained within
   * - 0 = Operational products
   * - 1 = Operational test products
   * - 2 = Research products
   * - 3 = Re-analysis products
   * @param[in] localTablesVersion Flag for generatingCentre local tables versioning system.
   * - 0 = Local Tables not used 
   * - 1-254 = Number of local tables version used
   * @param[in] masterTablesVersion Internal default master table versioning number
   * - 0 = Experimental
   * - 1 = Initial operational version number
   * - 2 = Current operational version number 
   */
  Grib2Record(si32 disciplineNumber, time_t referenceTime, si32 referenceTimeType, 
	      si32 typeOfData, si32 generatingSubCentreID, si32 generatingCentreID, 
	      si32 productionStatus, si32 localTablesVersion, si32 masterTablesVersion);

  ~Grib2Record();

  /** @brief Unpack a grib2 record pointed to by filePtr
   *  @param[in] filePtr Pointer to start of record
   *  @param[in] file_size Size of filePtr
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack(ui08 **filePtr, ui64 file_size);

  /** @brief Packs all the data of this record into a byte array.
   *  @return A ui08 array with the data of this record into it.
   *   The caller is responsible for delete[] ing the returned array. */
  ui08 *pack();
  
  /** @brief Print this records Indicator Section */
  void printIs (FILE *);

  /** @brief Print this records Identification Section */
  void printIds (FILE *);

  /** @brief Print this records Local Use Section(s) */
  void printLus (FILE *);

  /** @brief Print this records Grid Definition Section(s) */
  void printGds (FILE *);

  /** @brief Print this records Product Definition Section(s) */
  void printPds (FILE *);

  /** @brief Print this records Data Representation Section(s) */
  void printDrs (FILE *);

  /** @brief Print this records Bit Map Section(s) */
  void printBms (FILE *);

  /** @brief Print this records Data Section(s) */
  void printDs (FILE *);

  /** @brief Print a summary of the fields in this record */
  int printSummary (FILE *, int debug = 0);

  /** @brief Print all sections in this record */
  void print (FILE *stream);

  /** @brief Print to stream/file specific Grib2 sections from this record
   *  @param[out] stream File pointer to print to  
   *  @param[in] printSec Struct containing list of sections to print */
  int print (FILE *stream, print_sections_t printSec);

  /** @brief Get a list of strings containing all fields in this record */
  list <string> getFieldList();

  /** @brief Given a field get the list of strings of levels of that field that are in this record */
  list <string> getFieldLevels(const string &fieldName);

  /** @brief Get a list of ints containing all forecast lead times in this record */
  list <long int> getForecastList();

  /** @brief Get the records matching a given field name and level 
   *
   * @param[in] fieldName Requested field name as listed from getFieldList
   * @param[in] level     Requested field level as listed from getFieldLevels 
   * @param[in] leadTime  Requested forecast time as listed from getForecastList */
  vector <Grib2Sections_t> getRecords(const string &fieldName, const string &level, 
				      const long int &leadTime = -99);

  /** @brief Determine if there are any fields matching fieldName and level
   *
   * @param[in] fieldName Requested field name
   * @param[in] level     Requested field level  */
  bool recordMatches(const string &fieldName, const string &level);

  /** @brief Get the total size of the packed grib record */
  inline int getRecordSize() { return _is.getTotalSize(); }

  /** @brief Store a char array in the local use section.
   *
   * @param[in] dataSize Size of the char array
   * @param[in] localUseData pointer to the char array
   */
  void setLocalUse(si32 dataSize, ui08 *localUseData);

  /** @brief set the Grid Definition
   *
   * @param[in] numberDataPoints total number of data points in each field
   * @param[in] gridDefNum Template number of the projectionTemplate
   * - 0 = LatLonProj = Latitude/longitude projection
   * - 10 = MercatorProj = Mercator projection
   * - 20 = PolarStereoProj = Polar Stereographic projection
   * - 30 = LambertConfProj = Lambert conformal projection
   * @param[in] projectionTemplate Pointer to projection class (this class will now be owned by the library)
   */
  void setGrid(si32 numberDataPoints, si32 gridDefNum, GribProj *projectionTemplate);
  
  /** @brief Add a data field to the record
   *
   * @param[in] prodDefNum Template number of the productTemplate pointer
   * -   0 = Template4_pt_0 = Analysis or forecast at horizontal level
   * -   1 = Template4_pt_1 = Individual ensemble forecast at horizontal level
   * -   2 = Template4_pt_2 = Derived forecasts based on all ensemble members at a horizontal level
   * -   5 = Template4_pt_5 = Probability forecasts at a horizontal level
   * -   6 = Template4_pt_6 = Percentile forecasts at a horizontal level
   * -   7 = Template4_pt_7 = Analysis or forecast error at a horizontal level
   * -   8 = Template4_pt_8 = Average, accumulation or extreme values at horizontal level in a time interval
   * -   9 = Template4_pt_9 = Probability forecast at a horizontal level in a time interval
   * -   10 = Template4_pt_10 = Percentile forecast at a horizontal level in a time interval
   * -   11 = Template4_pt_11 = Individual ensemble forecast at horizontal level in a time interval
   * -   12 = Template4_pt_12 = Derived forecasts based on all ensemble members at a horizontal level in a time interval
   * -   15 = Template4_pt_15 = Average, accumulation or extreme values over a spatial area in a time interval
   * @param[in] productTemplate Pointer to the Product Definition class (this class will now be owned by the library)
   * @param[in] dataRepNum Template number of the dataRepTemplate
   * -   0 = Template5_pt_0 = Grid point data simple packing
   * -   2 = Template5_pt_2 = Grid point data complex packing
   * -   3 = Template5_pt_3 = Grid point data complex pack with spatial differencing
   * -  41 = Template5_pt_41 = Grid point data PNG Code Stream 
   * -  40 = Template5_pt_4000 = Grid point data Jpeg 2000 packing
   * @param[in] dataRepTemplate Pointer to the Data Representation class (this class will now be owned by the library)
   * @param[in] data Pointer to the data array
   * @param[in] bitMapType Type of Bit Map to use, or 255 for no Bit Map
   * -   0     = Bitmap applies and is included as bitMap
   * -   1-253 = Predefined bitmap applies
   * -   254   = Last defined bitmap applies to this field
   * -   255   = Bit map does not apply to this product
   * @param[in] bitMap Pointer to the bitmap if bitMapType = 0, NULL otherwise
   */
  int addField(si32 prodDefNum, ProdDefTemp *productTemplate, 
		si32 dataRepNum, DataRepTemp *dataRepTemplate,
		fl32 *data, si32 bitMapType, si32 *bitMap);

private:
  
  /** @brief Section 0, Indicator Section */
  IndicatorSec _is;

  /** @brief Section 1, Identification Section */
  IdSec _ids;

  // Vectors are necessary to handle the fact that sections 2 to 7, sections 
  // 3 to 7 or sections 4 to 7 may be repeated within a single GRIB message.     
  // All sections within such repeated sequences must be present and shall 
  // appear in the numerical order. Unrepeated sections remain in effect 
  // until redefined.
  typedef struct {
    /** Section 2, Local Use Section */
    LocalUseSec *lus;
    /** Section 3, Grid Definition Section  */
    GDS  *gds;
    /** Section 4, Product Definition Section */
    PDS  *pds;
    /** Section 5, Data Representation Section */
    DRS  *drs;
    /** Section 6, Bit-map Section */
    BMS  *bms;
    /** Section 7, Data Section */
    DS   *ds;
    /** Summary contains record specific product information */
    rec_summary_t summary;
  } repeatSections_t;

  /** @brief Vector of repeating sections making up the record */
  vector< repeatSections_t > _repeatSec; 

  /** @brief Section 8, End Section */
  ES    _es;

   
};

} // namespace Grib2

#endif
