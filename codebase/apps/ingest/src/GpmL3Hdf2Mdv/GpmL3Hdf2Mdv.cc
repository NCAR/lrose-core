/**
 *
 * @file GpmL3Hdf2Mdv.cc
 *
 * @class GpmL3Hdf2Mdv
 *
 * GpmL3Hdf2Mdv program object.
 *  
 * @date 10/30/2014
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dataport/port_types.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/os_config.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "GpmL3Hdf2Mdv.hh"

using namespace std;

// Global variables

GpmL3Hdf2Mdv *GpmL3Hdf2Mdv::_instance =
     (GpmL3Hdf2Mdv *)NULL;


GpmL3Hdf2Mdv::GpmL3Hdf2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string methodName = "GpmL3Hdf2Mdv::GpmL3Hdf2Mdv(): ";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (GpmL3Hdf2Mdv *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display copyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << methodName << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }

}

GpmL3Hdf2Mdv::~GpmL3Hdf2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}

GpmL3Hdf2Mdv *GpmL3Hdf2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (GpmL3Hdf2Mdv *)NULL)
    new GpmL3Hdf2Mdv(argc, argv);
  
  return(_instance);
}

GpmL3Hdf2Mdv *GpmL3Hdf2Mdv::Inst()
{
  assert(_instance != (GpmL3Hdf2Mdv *)NULL);
  
  return(_instance);
}


bool GpmL3Hdf2Mdv::init()
{
  static const string methodName = "GpmL3Hdf2Mdv::init(): ";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
 
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}

void GpmL3Hdf2Mdv::run()
{
  static const string methodName = "GpmL3Hdf2Mdv::run(): ";
  

  if ( _params->trigger_mode == Params::FILE_LIST )
  {
    vector <string> files =  _args->getFileList();
    for (int i = 0; i < (int) files.size(); i++)
    {
       _processFile(files[i]);
    }
  }
  else 
  {
    //  Real=time trigger
    while (!_dataTrigger->endOfData())
    {
      TriggerInfo trigger_info;
      
      if (_dataTrigger->next(trigger_info) != 0)
      {
	cerr << "ERROR: " << methodName << endl;
	cerr << "Error getting next trigger information" << endl;
	cerr << "Trying again...." << endl;
	
	continue;
      }
      _processFile(trigger_info.getFilePath());
      
    } /* endwhile - !_dataTrigger->endOfData() */
  }
}


bool GpmL3Hdf2Mdv::_processFile(const string &inputFileName)
{
  static const string methodName = "GpmL3Hdf2Mdv::_processFile(): ";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << methodName << "Processing file: " << inputFileName << endl;
  
  // Create the HDF file object

  if (_params->verbose)
    cerr << methodName << "Creating HDF file object" << endl;
  
  HdfFile hdfFile(inputFileName, _params->debug, _params->verbose);

  hdfFile.init();

  DateTime start, end;

  hdfFile.getTimes( start, end);

  DsMdvx mdvx;

  _createMdvHeader(start, end, inputFileName, mdvx);
  
  for (int i = 0; i < _params->datasets_n; i++)
  {
    void *mdvData;

    fl32 missingDataVal;
    
    string unitsStr("");

    HdfFile::HdfDataType_t dataType;
   
    hdfFile.getDataset(_params->_datasets[i], &mdvData,
		       missingDataVal, unitsStr, dataType);
   
    //
    // Get field name from input dataset name
    // 
    string inputField =_params->_datasets[i];

    size_t pos  = inputField.find_last_of("/");  

    string fieldName = inputField.substr(pos+1);


    if (_params->verbose)
    {
      cerr << methodName << "fieldName: " << fieldName.c_str() << endl;
    }

    _addMdvField( fieldName, unitsStr, 
		  mdvData, missingDataVal, dataType, mdvx);    
  }

  _writeMdv(mdvx);
 
  return true;
}

void GpmL3Hdf2Mdv::_createMdvHeader(const DateTime &begin_time,
				  const DateTime &end_time,
				  const string &input_path,
				  DsMdvx &mdvx)
{ 
   static const string methodName = "GpmL3Hdf2Mdv::_createMdvHeader(): ";

  //
  // Create master header
  //
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = begin_time.utime();
  master_hdr.time_end = end_time.utime();
  //
  // Assign time_centroid begin time to be in sync with GPM filenaming convention
  //
  master_hdr.time_centroid = begin_time.utime();
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.data_dimension = 2;
  STRncopy(master_hdr.data_set_info,
           "GPM satellite data ingested by GpmL3Hdf2Mdv", MDV_INFO_LEN);
  STRncopy(master_hdr.data_set_name, "GpmL3Hdf2Mdv", MDV_NAME_LEN);
  STRncopy(master_hdr.data_set_source, input_path.c_str(), MDV_NAME_LEN);

  mdvx.setMasterHeader(master_hdr);
}

void GpmL3Hdf2Mdv::_addMdvField( const string &fieldName,
			       const string &units,
			       void *data, 
			       const fl32 missingData,
			       const HdfFile::HdfDataType_t dataType, 
			       DsMdvx &mdvx) const
{
  static const string methodName = "GpmL3Hdf2Mdv::_addMdvField(): ";

  //
  // Create fieldheader
  //
  Mdvx::field_header_t fhdr;
  memset(&fhdr, 0, sizeof(fhdr));
  // 
  //  Set projection
  // 
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.nx =  _params->output_proj.nx;
  fhdr.ny =  _params->output_proj.ny;
  fhdr.nz =  1;
  fhdr.grid_minx = _params->output_proj.minx;
  fhdr.grid_miny = _params->output_proj.miny;
  fhdr.grid_minz = 0;
  fhdr.grid_dx = _params->output_proj.dx;
  fhdr.grid_dy = _params->output_proj.dy;
  fhdr.grid_dz = 0.0;
    
  //
  // Set data type
  //
  if(dataType == HdfFile::HDF_DATA_INT8)
  {
    fhdr.encoding_type = Mdvx::ENCODING_INT8;
    fhdr.data_element_nbytes =  sizeof(ui08);
    fhdr.volume_size = fhdr.nx * fhdr.ny *  fhdr.nz * sizeof(ui08);
    fhdr.scale =1;
    fhdr.bias = 0;
  }
  else if (dataType == HdfFile::HDF_DATA_INT16)
  {
    fhdr.encoding_type = Mdvx::ENCODING_INT16;
    fhdr.data_element_nbytes =  sizeof(si16);
    fhdr.volume_size = fhdr.nx * fhdr.ny *  fhdr.nz * sizeof(si16);
  }
  else
  {
    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes =  sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny *  fhdr.nz * sizeof(fl32);
  }

   fhdr.bad_data_value = missingData;
   fhdr.missing_data_value = missingData;

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  sprintf(fhdr.field_name, "%s", fieldName.c_str());
  sprintf(fhdr.field_name_long,"%s", fieldName.c_str());
  sprintf(fhdr.units, "%s", units.c_str());
  sprintf(fhdr.transform,"%s", "none");

  //
  // Set up vlevel information
  //
  Mdvx::vlevel_header_t vhdr;
  memset(&vhdr, 0, sizeof(Mdvx::vlevel_header_t));
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;

  //
  // Create MdvxField
  //
  MdvxField *field = new MdvxField(fhdr, vhdr, data);
 
  field->convertType(Mdvx::ENCODING_INT8,
                     Mdvx::COMPRESSION_GZIP,
                     Mdvx::SCALING_DYNAMIC);

 
  //
  // Add field to Mdvx object
  //
  mdvx.addField(field);

  if(dataType == HdfFile::HDF_DATA_INT8)
  {
    delete[] (ui08*)data;
  }
  else  if (dataType == HdfFile::HDF_DATA_INT16)
  {
    delete[] (si16*)data;
  }
  else
  {
    delete[] (fl32*)data;
  }
}

void GpmL3Hdf2Mdv::_writeMdv(DsMdvx &mdvx) const
{
  static const string methodName = "GpmL3Hdf2Mdv::_writeMdv(): ";

  //
  // Write data
  //
  if (_params->debug)
  {
    cerr << methodName << "Writing mdv volume to " << _params->output_url << endl;
  }

  if (mdvx.writeToDir(_params->output_url)) 
  {
    cerr << "ERROR: " << methodName << "Cannot write to url: " 
	 << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
  }
}

bool GpmL3Hdf2Mdv::_initTrigger()
{
  static const string methodName = "TrmmHdf2Mdv;:_initTrigger()";

  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    
    _dataTrigger = NULL;

    break;
  }

  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
                      _params->input_substring,
                      false, PMU_auto_register, false,
                      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << methodName 
	   << "Error initializing INPUT_DIR trigger" << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }
 
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
                      _params->max_valid_secs,
                      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
   
      return false;
    }
   
    _dataTrigger = trigger;
   
    break;
  }
 
  } /* endswitch - _params->trigger_mode */

return true;
}
