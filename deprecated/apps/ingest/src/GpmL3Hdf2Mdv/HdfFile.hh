/**
 *
 * @file HdfFile.hh
 *
 * @class HdfFile
 *
 * Class controlling access to a HDF5 GPM Level 3 file.
 *
 */

#ifndef HdfFile_HH
#define HdfFile_HH

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
#include <dataport/port_types.h>
#include <Ncxx/H5x.hh>
using namespace H5x;
using namespace std;


/** 
 * @class HdfFile
 */

class HdfFile
{
public:

  typedef enum {

    HDF_DATA_INT8,
    HDF_DATA_INT16,
    HDF_DATA_FLOAT32,
  } HdfDataType_t;
   
  /**
   * Constructor
   *
   * @param[in] file_path Path for GPM Level 3 HDF5 file to process.
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose flag.
   */
  HdfFile(const string &file_path,
	  const bool debug_flag = false,
	  const bool verbose_flag = false);

  /**
   * Destructor
   */
  virtual ~HdfFile(void);
   
  /**
   * Open the HDF5 file.
   *
   * @return Returns true on success, false on failure.
   */
  bool init();

  /**
   * Get start and end times of data in hdf5 file
   * @param[out] beginTime start time of data in hdf5 file       
   * @param[out] endTime  Stop time of data in hdf5 file
   */
  void getTimes(DateTime &beginTime, DateTime &endTime);

  /**
   * Get dataset from hdf5 file and store in array mdvData.
   * Get missing data and units attributes of dataset if available. 
   * @param[in] datasetName  Full hdf5 datapath and dataset name       
   * @param[out] mdvData  Pointer to array containing data
   * @param[out] missingDataVal  Missing data value specified in dataset attribute
   * @param[out] units  String containing dataset units as specified 
   *                    in dataset attribute
   * @param[out] dataType  Enumerated type to indicate data storage type
   *                       (we assume 8bit int, 16bit int, or 32bit float)
   */
  bool getDataset(const string datasetName, void** mdvData, 
		  fl32 &missingDataVal, string &units, 
		  HdfDataType_t &dataType);
  
private:

  /**
   * Debug flag.
   */
  bool _debug;
  
  /**
   * Verbose debug flag.
   */
  bool _verbose;
  
  /**
   * The path to the HDF5 file.
   */
  string _filePath;

  /**
   *  HDF5 file object.
   */
  H5File _file;
};


#endif
