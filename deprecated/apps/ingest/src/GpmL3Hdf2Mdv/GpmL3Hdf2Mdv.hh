/**
 *
 * @file GpmL3Hdf2Mdv.hh
 *
 * @class GpmL3Hdf2Mdv
 *
 * GpmL3Hdf2Mdv program object.
 *  
 * @date 10/30/2008
 *
 */

#ifndef GpmL3Hdf2Mdv_HH
#define GpmL3Hdf2Mdv_HH

#include <string>
#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"
#include "HdfFile.hh"

using namespace std;

/** 
 * @class GpmL3Hdf2Mdv
 */

class GpmL3Hdf2Mdv
{
 public:

  /**
   * Flag indicating whether the program status is currently okay.
   */
  bool okay;

  /**
   *  Destructor
   */

  ~GpmL3Hdf2Mdv(void);
  
  /**
   * Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the GpmL3Hdf2Mdv instance.
   */

  static GpmL3Hdf2Mdv *Inst(int argc, char **argv);

  /**
   * Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the GpmL3Hdf2Mdv instance.
   */

  static GpmL3Hdf2Mdv *Inst();
  
  /**
   * Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init();
  
  /**
   * @brief Run the program.
   */

  void run();
  
 private:

  /**
   * @brief Singleton instance pointer.
   */

  static GpmL3Hdf2Mdv *_instance;
  
  /**
   * Program name.
   */

  char *_progName;

  /**
   * Command line arguments.
   */

  Args *_args;

  /**
   * Parameter file parameters.
   */

  Params *_params;
  
  /**
   * Data triggering object.
   */

  DsTrigger *_dataTrigger;
  
 
  /**
   * Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  GpmL3Hdf2Mdv(int argc, char **argv);
  

 
  /**
   * Initialize the data trigger.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

 
  /**
   * Process the given file: get time information, requested datasets
   * and store in Mdv file
   *
   * @param[in] file_path Path for the input file to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processFile(const string &file_path);

  /**
   * Create Mdv master header struct 
   * @param[in] begin_time  StartGranuleDateTime as listed in Hdf5 file header
   * @param[in] end_time  StopGranuleDateTime as listed in Hdf5 file header
   * @param[in] input_path  Input path of file to be processed
   * @param[in] mdvx  DsMdvx object for creating and writing Mdv file.
   */
  void _createMdvHeader(const DateTime &begin_time,
			const DateTime &end_time,
			const string &input_path,
			DsMdvx &mdvx);
  /**
   * Create Mdv field from hdf5 gpm level3 dataset.
   * @param[in] fieldName  fieldName of hdf5 dataset
   * @param[in] units  Units string (attribute of hdf5 dataset if available)
   * @param[in] missingData  Missing data value (attribute of hdf5 dataset)
   * @param[in] dataType  data type of hdf5 dataset is 8 bit int, 16 bit int or 
   *            32 bit floating point 
   *                      
   * @param[in]  mdvx  DsMdvx object for creating and writing Mdv file.
   */
  void _addMdvField(const string &fieldName,
		    const string &units,
		    void *data, const fl32 missingData,
		    const HdfFile::HdfDataType_t dataType,
		    DsMdvx &mdvx) const;

  /**
   * Write Mdv field created from hdf5 gpm level3 dataset to user specified
   * directory.
   * @param[in]  mdvx  DsMdvx object for creating and writing Mdv file.
   */
  void _writeMdv(DsMdvx &mdvx) const;

};


#endif
