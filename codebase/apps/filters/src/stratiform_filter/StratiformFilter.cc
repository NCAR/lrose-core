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
/*********************************************************************
 * StratiformFilter.cc: stratiform_filter program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <assert.h>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/LinearPtFunc.hh>
#include <rapmath/StepPtFunc.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "ConvPartition.hh"
#include "Params.hh"
#include "StratiformFilter.hh"

// Global variables

StratiformFilter *StratiformFilter::_instance = (StratiformFilter *)NULL;


/*********************************************************************
 * Constructor
 */

StratiformFilter::StratiformFilter(int argc, char **argv)

{
  const string routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (StratiformFilter *)NULL);
  
  // Set the singleton instance pointer

  _instance = this;

  // Initialize the okay flag.

  okay = true;
  
  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name.c_str());
    fprintf(stderr,
	    "Problem with command line arguments.\n");
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name.c_str());
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Make sure start and end times were specified if in archive mode

  if (_params->mode == Params::ARCHIVE_MODE)
  {
    if (_args->getStartTime() <= 0 ||
	_args->getEndTime() <= 0)
    {
      fprintf(stderr,
	      "ERROR: %s::%s\n",
	      _className(), routine_name.c_str());
      fprintf(stderr,
	      "Must specify -starttime and -endtime on command line in ARCHIVE mode\n");
      
      okay = false;
      
      return;
    }
  }
  
  // Initialize the input data object

  switch (_params->mode)
  {
  case Params::REALTIME_MODE :
    if (_fileRetriever.setRealtime(_params->input_url, _params->max_valid_age,
				   PMU_auto_register) != 0)
    {
      okay = false;
      return;
    }
    break;
    
  case Params::ARCHIVE_MODE :
    if (_fileRetriever.setArchive(_params->input_url,
				  _args->getStartTime(),
				  _args->getEndTime()) != 0)
    {
      okay = false;
      return;
    }
    break;

  } /* endswitch - _params->mode */

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Create the partitioning object

  if (!_createPartitioner())
  {
    fprintf(stderr,
	    "ERROR:  %s::%s\n", _className(), routine_name.c_str());
    fprintf(stderr,
	    "Error creating partitioner object.\n");
    
    okay = false;
    
    return;
  }
  
}


/*********************************************************************
 * Destructor
 */

StratiformFilter::~StratiformFilter()
{
  // Free contained objects

  delete _params;
  delete _args;
  delete _convPartition;
  
  // Unregister

  PMU_auto_unregister();
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

StratiformFilter *StratiformFilter::Inst(int argc, char **argv)
{
  if (_instance == (StratiformFilter *)NULL)
    new StratiformFilter(argc, argv);
  
  return(_instance);
}

StratiformFilter *StratiformFilter::Inst()
{
  assert(_instance != (StratiformFilter *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void StratiformFilter::run()
{
  char pmu_message[BUFSIZ];
  
  // Process each file

  while (true)
  {
    DsMdvx input_file;
    
    if (!_readNextFile(input_file))
    {
      if (_params->mode == Params::REALTIME_MODE)
	continue;
      else
      {
	if (_params->debug)
	  cerr << "No more files to process -- exiting" << endl;

	break;
      }
    }
      
    sprintf(pmu_message, "Processing data for time %s",
	    utimstr(input_file.getMasterHeader().time_centroid));
    PMU_force_register(pmu_message);
      
    if (_params->debug)
    {
      fprintf(stderr, "*** %s\n", pmu_message);
      fprintf(stderr, "    current time is %s\n", utimstr(time(NULL)));
    }
      
    // Partition the data into stratiform and convection

    MdvxField *field = input_file.getFieldByNum(0);
      
    _convPartition->run(*field);

    PMU_auto_register("Partition calculated");
      
    if (_params->debug)
      cerr << "    Partition calculated" << endl;

    ui08 *partition = _convPartition->getPartition();
      
    SimpleGrid<fl32> *partitioned_data =
      _partitionData(*field,
		     partition,
		     ConvPartition::CONVECTIVE);
      
    PMU_auto_register("Data partitioned");
      
    if (_params->debug)
      cerr << "    Data partitioned" << endl;
    
    // Construct and write the output file
      
    _generateOutput(input_file,
		    partition,
		    partitioned_data->getGrid(),
		    _convPartition->getMeanData());
      
    PMU_auto_register("Output generated");

    if (_params->debug)
      fprintf(stderr, "    Output generated\n");

    delete partitioned_data;
    
  } /* endwhile - FOREVER */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createPartitioner() - Create the partitioning object.  Note that
 *                        this must be done AFTER reading in the TDRP
 *                        parameters since it depends on these parameters.
 *
 * Returns true on success, false if there was any problem creating the
 * object.
 */

bool StratiformFilter::_createPartitioner(void)
{
  map< double, double, less<double> > value_diff_func_pts;
  map< double, double, less<double> > conv_radius_func_pts;
  
  // Create the value difference function in the correct format

  for (int i = 0; i < _params->value_diff_func_n; i++)
  {
    value_diff_func_pts[_params->_value_diff_func[i].mean_bkgnd_value] =
      _params->_value_diff_func[i].value_difference;
  }
  
  // Create the convective area radius function in the correct format

  for (int i = 0; i < _params->conv_area_radius_func_n; i++)
  {
    conv_radius_func_pts[_params->_conv_area_radius_func[i].mean_bkgnd_value] =
      _params->_conv_area_radius_func[i].conv_radius;
  }
  

  LinearPtFunc *value_diff_func =
    new LinearPtFunc(value_diff_func_pts);
  StepPtFunc *conv_radius_func =
    new StepPtFunc(conv_radius_func_pts);
  
  _convPartition = new ConvPartition(value_diff_func,
				     conv_radius_func,
				     _params->conv_center_min,
				     _params->background_min_dbz,
				     _params->background_radius,
				     _params->data_increasing,
				     _params->debug);
  
  return true;
}


/*********************************************************************
 * _generateOutput() - Construct the output file and write it to the
 *                     appropriate directory.
 */

void StratiformFilter::_generateOutput(DsMdvx &input_file,
				       ui08 *partition,
				       fl32 *partitioned_data,
				       fl32 *mean_data)
{
  // Retrieve the needed headers from the input file

  Mdvx::master_header_t master_hdr_in = input_file.getMasterHeader();

  MdvxField *field_in = input_file.getFieldByNum(0);
  Mdvx::field_header_t field_hdr_in = field_in->getFieldHeader();
  
  /*
   * Construct the output file.
   */

  DsMdvx *output_file = new DsMdvx();
  
  // Construct the master header from the one used for the input data

  Mdvx::master_header_t master_hdr_out;
  
  memset(&master_hdr_out, 0, sizeof(Mdvx::master_header_t));
  
  master_hdr_out.time_gen = time((time_t *)NULL);
  master_hdr_out.user_time = master_hdr_in.user_time;
  master_hdr_out.time_begin = master_hdr_in.time_begin;
  master_hdr_out.time_end = master_hdr_in.time_end;
  master_hdr_out.time_centroid = master_hdr_in.time_centroid;
  master_hdr_out.time_expire = master_hdr_in.time_expire;
  master_hdr_out.data_dimension = 2;
  master_hdr_out.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr_out.user_data = master_hdr_in.user_data;
  master_hdr_out.native_vlevel_type = master_hdr_in.native_vlevel_type;
  master_hdr_out.vlevel_type = master_hdr_in.vlevel_type;
  master_hdr_out.vlevel_included = 0;
  master_hdr_out.grid_orientation = master_hdr_in.grid_orientation;
  master_hdr_out.data_ordering = master_hdr_in.data_ordering;
  master_hdr_out.n_fields = 0;
  master_hdr_out.max_nx = master_hdr_in.max_nx;
  master_hdr_out.max_ny = master_hdr_in.max_ny;
  master_hdr_out.max_nz = 1;
  master_hdr_out.n_chunks = 0;
  master_hdr_out.field_grids_differ = 0;
  master_hdr_out.sensor_lon = master_hdr_in.sensor_lon;
  master_hdr_out.sensor_lat = master_hdr_in.sensor_lat;
  master_hdr_out.sensor_alt = master_hdr_in.sensor_alt;
  
  STRcopy(master_hdr_out.data_set_info,
	  "Output from stratiform_filter",
	  MDV_INFO_LEN);
  STRcopy(master_hdr_out.data_set_name,
	  "stratiform_filter",
	  MDV_NAME_LEN);
  STRcopy(master_hdr_out.data_set_source,
	  input_file.getReadPath().c_str(),
	  MDV_NAME_LEN);
  
  output_file->setMasterHeader(master_hdr_out);
  
  // Add the original grid to the output file, if requested

  if (_params->include_input_field)
  {
    MdvxField *input_field = input_file.getFieldByNum(0);
    MdvxField *input_field_copy = new MdvxField(*input_field);
    
    input_field_copy->convertType(Mdvx::ENCODING_INT16,
				  Mdvx::COMPRESSION_GZIP,
				  Mdvx::SCALING_SPECIFIED);
  
    output_file->addField(input_field_copy);
  }
  
  // field header template

  Mdvx::field_header_t template_field_hdr = field_hdr_in;
  template_field_hdr.field_code = 0;
  template_field_hdr.nz = 1;
  template_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  template_field_hdr.data_element_nbytes = 4;
  template_field_hdr.volume_size =
    template_field_hdr.nx * template_field_hdr.ny * template_field_hdr.nz *
    template_field_hdr.data_element_nbytes;
  template_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  template_field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  template_field_hdr.scaling_type = Mdvx::SCALING_NONE;
  template_field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  template_field_hdr.dz_constant = true;
  template_field_hdr.scale = 1.0;
  template_field_hdr.bias = 0.0;
  template_field_hdr.bad_data_value = field_hdr_in.bad_data_value;
  template_field_hdr.missing_data_value = field_hdr_in.missing_data_value;
  template_field_hdr.min_value = 0.0;
  template_field_hdr.max_value = 0.0;

  // Add the partitioned data field to the output file

  Mdvx::field_header_t part_data_field_hdr = template_field_hdr;

  STRcopy(part_data_field_hdr.field_name_long,
	  "convective/stratiform partitioned data",
	  MDV_LONG_FIELD_LEN);
  if (strlen(_params->field_name_for_convective_input_data) > 0) {
    STRcopy(part_data_field_hdr.field_name,
            _params->field_name_for_convective_input_data,
            MDV_SHORT_FIELD_LEN);
  } else {
    STRcopy(part_data_field_hdr.field_name,
            "conv partitioned data",
            MDV_SHORT_FIELD_LEN);
  }
  STRcopy(part_data_field_hdr.units, field_hdr_in.units, MDV_UNITS_LEN);
  STRcopy(part_data_field_hdr.transform, "", MDV_TRANSFORM_LEN);
  
  MdvxField *part_data_field = new MdvxField(part_data_field_hdr,
					     field_in->getVlevelHeader(),
					     (void *)partitioned_data);

  part_data_field->convertType(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_GZIP);
  
  output_file->addField(part_data_field);
  
  // Add the partition field to the output file

  Mdvx::field_header_t part_field_hdr = template_field_hdr;
  part_field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  part_field_hdr.data_element_nbytes = 1;
  part_field_hdr.volume_size =
    part_field_hdr.nx * part_field_hdr.ny * part_field_hdr.nz *
    part_field_hdr.data_element_nbytes;
  part_field_hdr.scale = 1.0;
  part_field_hdr.bias = 0.0;
  part_field_hdr.bad_data_value = ConvPartition::MISSING_DATA;
  part_field_hdr.missing_data_value = ConvPartition::MISSING_DATA;

  if (strlen(_params->field_name_for_partition_flag) > 0) {
    STRcopy(part_field_hdr.field_name,
            _params->field_name_for_partition_flag,
            MDV_SHORT_FIELD_LEN);
  } else {
    STRcopy(part_field_hdr.field_name,
            "conv partition",
            MDV_SHORT_FIELD_LEN);
  }

  STRcopy(part_field_hdr.field_name_long,
	  "convective/stratiform partition",
	  MDV_LONG_FIELD_LEN);
  part_field_hdr.units[0] = '\0';
  part_field_hdr.transform[0] = '\0';
  
  MdvxField *part_field = new MdvxField(part_field_hdr,
					field_in->getVlevelHeader(),
					(void *)partition);
  
  part_field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_NONE);
  
  output_file->addField(part_field);
  
  
  // Add the calculated means data field to the output file

  Mdvx::field_header_t means_field_hdr = template_field_hdr;
  
  STRcopy(means_field_hdr.field_name_long,
	  "convective/stratiform means",
	  MDV_LONG_FIELD_LEN);
  if (strlen(_params->field_name_for_background_mean) > 0) {
    STRcopy(means_field_hdr.field_name,
            _params->field_name_for_background_mean,
            MDV_SHORT_FIELD_LEN);
  } else {
    STRcopy(means_field_hdr.field_name, "conv means",
            MDV_SHORT_FIELD_LEN);
  }

  STRcopy(means_field_hdr.units, field_hdr_in.units, MDV_UNITS_LEN);
  STRcopy(means_field_hdr.transform, "", MDV_TRANSFORM_LEN);
  
  MdvxField *means_field = new MdvxField(means_field_hdr,
					 field_in->getVlevelHeader(),
					 (void *)mean_data);
  
  means_field->convertType(Mdvx::ENCODING_INT16,
			   Mdvx::COMPRESSION_GZIP);
  
  output_file->addField(means_field);

  // if requested add debug output fields
  
  Mdvx::field_header_t excess_field_hdr = template_field_hdr;
  STRcopy(excess_field_hdr.field_name_long,
	  "input_dbz_minus_background_dbz",
	  MDV_LONG_FIELD_LEN);
  STRcopy(excess_field_hdr.field_name,
          "excess",
          MDV_SHORT_FIELD_LEN);
  STRcopy(excess_field_hdr.units, field_hdr_in.units, MDV_UNITS_LEN);
  STRcopy(excess_field_hdr.transform, "", MDV_TRANSFORM_LEN);
  MdvxField *excess_field = new MdvxField(excess_field_hdr,
                                          field_in->getVlevelHeader(),
                                          _convPartition->getExcess());
  excess_field->convertType(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_GZIP);
  output_file->addField(excess_field);

  Mdvx::field_header_t threshold_field_hdr = template_field_hdr;
  STRcopy(threshold_field_hdr.field_name_long,
	  "value_diff_func_of_input_dbz",
	  MDV_LONG_FIELD_LEN);
  STRcopy(threshold_field_hdr.field_name,
          "threshold",
          MDV_SHORT_FIELD_LEN);
  STRcopy(threshold_field_hdr.units, field_hdr_in.units, MDV_UNITS_LEN);
  STRcopy(threshold_field_hdr.transform, "", MDV_TRANSFORM_LEN);
  MdvxField *threshold_field = new MdvxField(threshold_field_hdr,
                                             field_in->getVlevelHeader(),
                                          _convPartition->getThreshold());
  threshold_field->convertType(Mdvx::ENCODING_INT16,
                               Mdvx::COMPRESSION_GZIP);
  output_file->addField(threshold_field);

  
  /*
   * Write the output file
   */

  output_file->clearWrite();
  output_file->setWriteLdataInfo();
  output_file->writeToDir(_params->output_url);

  if (_params->debug) {
    cerr << "Wrote file: " << output_file->getPathInUse() << endl;
  }

  delete output_file;
  
  return;
}


/*********************************************************************
 * _partitionData() - Apply the given partition to the data.
 */

SimpleGrid<fl32> *StratiformFilter::_partitionData(const MdvxField &field,
						   ui08 *partition,
						   const ui08 partition_value)
{
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  fl32 *data = (fl32 *)field.getVol();
  
  SimpleGrid<fl32> *partitioned_data =
    new SimpleGrid<fl32>(field_hdr.nx, field_hdr.ny);
  
  // Partition the data

  for (int i = 0; i < field_hdr.nx * field_hdr.ny; i++)
  {
    if (partition[i] == partition_value)
      partitioned_data->set(i, data[i]);
    else
      partitioned_data->set(i, (fl32)field_hdr.missing_data_value);
    
  } /* endfor - i */
  
  return partitioned_data;
}


/**********************************************************************
 * _readNextFile() - Read the next file to be processed.  In realtime
 *                   mode, blocks until a new file is available.
 *
 * Returns true if successful, false otherwise.
 */

bool StratiformFilter::_readNextFile(DsMdvx &mdv_file)
{
  const string routine_name = "_readNextFile()";
  
  // Set up the read request

  mdv_file.clearRead();
  mdv_file.clearReadFields();
  mdv_file.addReadField(_params->field_num);
  mdv_file.clearReadVertLimits();
  mdv_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv_file.setReadScalingType(Mdvx::SCALING_NONE);

  mdv_file.setReadFieldFileHeaders();
  
  if (_params->do_composite)
  {
    mdv_file.setReadVlevelLimits(_params->lower_comp_vlevel,
				 _params->upper_comp_vlevel);
    mdv_file.setReadComposite();
  }
  else
  {
    mdv_file.setReadPlaneNumLimits(_params->level_num,
				   _params->level_num);
  }

  if (_params->debug)
    mdv_file.printReadRequest(cerr);

  // Read the next file -- blocks in realtime.

  PMU_force_register("Reading volume...");

  if (_fileRetriever.readVolumeNext(mdv_file) != 0)
  {
    if (_params->mode == Params::REALTIME_MODE) {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error reading in new input file: " <<
        _fileRetriever.getErrStr() << endl;
    }
    return false;
  }
  
  if (_params->debug)
    cerr << "Read in input file: " << mdv_file.getPathInUse() << endl;
  
  return true;
}
