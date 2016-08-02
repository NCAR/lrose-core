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
 * @file Grib2File.hh
 * @brief Main class for manipulating GRIB2 files.
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef Grib2_Grib2File_HH
#define Grib2_Grib2File_HH

#include <string>
#include <vector>
#include <list>

#include <grib2/Grib2Record.hh>

using namespace std;

namespace Grib2 {

class GribProj;
class ProdDefTemp;
class DataRepTemp;

/** 
 * @class Grib2File
 * @brief Main class for manipulating GRIB2 files.
 */
class Grib2File
{

public:

  /** 
   * @brief Defualt and only constructor
   */
  Grib2File();

  ~Grib2File();

  /** @brief Clear out any file in memory */
  void clearInventory();

  // Functions for reading a Grib2 file  

  /** @brief Open and read (unpack) a grib2 file, including all records in the file.
   *  @param[in] file_path Full path to file to open
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int read(const string &file_path = "");

  /** @brief Print to stream/file all Grib2 sections */
  void print(FILE *stream);

  /** @brief Print to stream/file specific Grib2 sections
   *  @param[out] stream File pointer to print to  
   *  @param[in] printSec Struct containing list of sections to print */
  void printContents (FILE *stream, Grib2Record::print_sections_t printSec) const;
  
  /** @brief Print a summary of the fields in the file */
  void printSummary (FILE *stream, int debug = 0) const;
  
  /** @brief Get a list of strings containing all fields in the file */
  list <string> getFieldList ();

  /** @brief Given a field get the list of strings of levels of that field */
  list <string> getFieldLevels (const string &fieldName);

  /** @brief Get a list of ints containing all forecast lead times in the file */
  list <long int> getForecastList ();

  /** @brief Get the records matching a given field name and level 
   *
   * After using read to open a given grib2 file, using getFieldList with getFieldLevels
   * user can get a full listing of all field/level combinations in the file which can
   * then be requested through getRecrods
   *
   * @param[in] fieldName Requested field name as listed from getFieldList
   * @param[in] level     Requested field level as listed from getFieldLevels */
  vector <Grib2Record::Grib2Sections_t> getRecords (const string &fieldName, const string &level,
						    const long int &leadTime = -99);



  /** @brief Begins a new Grib2Record
   *
   * @Note Functions for creating/writing a Grib2 file:
   *
   *
   * To start a new GRIB2 record, call function create.  
   * This sets Sections 0 and 1 at the beginning of the message.
   * 
   * Function addLocalUse can be used to add a Local Use Section ( Section 2 ).
   * Note that this section is optional and need not be called. addLocalUse can
   * only be called after function create.
   * 
   * Function addGrid is used to set a grid definition into Section 3.
   * This grid definition defines the geometry of the the data values in the
   * fields that follow it.  addGrid can be called again to change the grid 
   * definition describing subsequent data fields. addGrid can only be called
   * after create, addlocalUse or addField functions.
   * 
   * Each data field is added to the GRIB2 record using routine addField,
   * which adds Sections 4, 5, 6, and 7 to the message. addField can only
   * be called after addGrid or addField.
   * 
   * To finish creating a record call you may either start a new record with
   * create or write out the record(s) with write.  The write function may
   * only be called after a addField or to after reading a complete file with
   * read.
   * 
   * There is no way to "add" into a complete GRIB2 record. The best way to add
   * a field is to read the Grib file and then create a new record onto the 
   * file and add your grid and fields and write out the multi record file.
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
   * - 0 = Analysis products  (Default value)
   * - 1 = Forecast products
   * - 2 = Analysis and Forecast products
   * - 3 = Control Forecast Products
   * - 4 = Perturbed Forecast Products
   * - 5 = Control and Perturbed Forecast Products
   * - 6 = Processed satellite observations
   * - 7 = Processed radar observations
   * @param[in] generatingSubCentreID Optional code to identify a creating process or group within generatingCentreID
   * @param[in] generatingCentreID Code to identify greating centre
   *       (see http://www.nco.ncep.noaa.gov/pmb/docs/on388/table0.html for complete list)
   * - 7 = NWS NCEP
   * - 59 = NOAA Forecast Systems Lab
   * - 60 = National Center for Atmospheric Research (NCAR)  (Default value)
   * @param[in] productionStatus Optional production status of data contained within
   * - 0 = Operational products
   * - 1 = Operational test products
   * - 2 = Research products  (Default value)
   * - 3 = Re-analysis products
   * @param[in] localTablesVersion Optional flag for generatingCentre local tables versioning system.
   * - 0 = Local Tables not used  (Default value)
   * - 1-254 = Number of local tables version used
   * @param[in] masterTablesVersion Optional override internal default master table (not recommended)
   * - 0 = Experimental
   * - 1 = Initial operational version number
   * - 2 = Current operational version number  (Default value)
   */
  int create(si32 disciplineNumber, time_t referenceTime, si32 referenceTimeType, 
	      si32 typeOfData = 0, si32 generatingSubCentreID = 0, si32 generatingCentreID = 60, 
	      si32 productionStatus = 2, si32 localTablesVersion = 0, si32 masterTablesVersion = 2);

  /** @brief Optional call to store a char array in the local use section.
   *
   * @note Can only be called after function create.
   *
   * @param[in] dataSize Size of the char array
   * @param[in] localUseData pointer to the char array
   */
  int addLocalUse(si32 dataSize, ui08 *localUseData);

  /** @brief Defines the geometry of the the data values in the fields that follow it
   *
   * @note Can only be called after create, addlocalUse or addField functions.
   *
   * @param[in] numberDataPoints total number of data points in each field
   * @param[in] gridDefNum Template number of the projectionTemplate
   * - 0 = LatLonProj = Latitude/longitude projection
   * - 20 = PolarStereoProj = Polar Stereographic projection
   * - 30 = LambertConfProj = Lambert conformal projection
   * @param[in] projectionTemplate Pointer to projection class (this class will now be owned by the library)
   */
  int addGrid(si32 numberDataPoints, si32 gridDefNum, GribProj *projectionTemplate);


  /** @brief Add a data field to the grib2 record
   *
   * @note Can only be called after addGrid or addField.
   *
   * @param[in] prodDefNum Template number of the productTemplate pointer
   * -   0 = Template4_pt_0 = Analysis or forecast at horizontal level
   * -   1 = Template4_pt_1 = Individual ensemble forecast at horizontal level
   * -   2 = Template4_pt_2 = Derived forecasts based on all ensemble members at a horizontal level
   * -   5 = Template4_pt_5 = Probability forecasts at a horizontal level
   * -   6 = Template4_pt_6 = Percentile forecasts at a horizontal level
   * -   7 = Template4_pt_7 = Analysis or forecast error at a horizontal level
   * -   8 = Template4_pt_8 = Average, accumulation or extreme values at horizontal level
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
   *
   * @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE
   */
  int addField(si32 prodDefNum, ProdDefTemp *productTemplate, 
		si32 dataRepNum, DataRepTemp *dataRepTemplate,
		fl32 *data, si32 bitMapType, si32 *bitMap = NULL);

  /** @brief Write out all grib2 records stored within this class
   *  @param[in] file_path Full path to location to write grib2 file */
  int write(const string &file_path = "");

private:

  /** @brief Internally set the file we are reading */
  void _setFilePath (const string &new_file_path);
  
  typedef struct {

    Grib2Record *record;

  } file_inventory_t;
  
  /** @breif Vector of records making up this file */
  vector< file_inventory_t > _inventory;

  /** @brief Current read file path */  
  string _filePath;

  /** @brief Current read file pointer */
  FILE *_filePtr;

  /** @brief Curret file pointer read state */
  bool _fileContentsRead;

  typedef enum {
    CONSTRUCT,
    CLEAR,
    READ,
    CREATE,
    ADDLOCALUSE,
    ADDGRID,
    ADDFIELD,
    WRITE,
  } last_action_t;

  /** @brief Flag indicating last user function call */
  last_action_t _last_file_action;

};

} // namespace Grib2

#endif
