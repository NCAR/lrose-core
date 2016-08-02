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
 * @file CfRadial2DeTect.cc
 *
 * @class CfRadial2DeTect
 *
 * CfRadial2DeTect is the top level application class.
 *  
 * @date 7/5/2011
 *
 */

#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <DeTect/ArchiveLabel.hh>
#include <DeTect/DataObject.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxFile.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CfRadial2DeTect.hh"
#include "Params.hh"

using namespace std;


// Global variables

const double CfRadial2DeTect::SECTOR_SIZE = 90.0;

CfRadial2DeTect *CfRadial2DeTect::_instance = (CfRadial2DeTect *)NULL;

/*********************************************************************
 * Constructor
 */

CfRadial2DeTect::CfRadial2DeTect(int argc, char **argv) :
  _outputFileCreated(false),
  _firstSector(true)
{
  static const string method_name = "CfRadial2DeTect::CfRadial2DeTect()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CfRadial2DeTect *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

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
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

CfRadial2DeTect::~CfRadial2DeTect()
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


/*********************************************************************
 * Inst()
 */

CfRadial2DeTect *CfRadial2DeTect::Inst(int argc, char **argv)
{
  if (_instance == (CfRadial2DeTect *)NULL)
    new CfRadial2DeTect(argc, argv);
  
  return(_instance);
}

CfRadial2DeTect *CfRadial2DeTect::Inst()
{
  assert(_instance != (CfRadial2DeTect *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool CfRadial2DeTect::init()
{
  static const string method_name = "CfRadial2DeTect::init()";
  
  // Initialize the input trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run()
 */

void CfRadial2DeTect::run()
{
  static const string method_name = "CfRadial2DeTect::run()";
  
  // Process all of the input data

  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processFile(trigger_info.getFilePath()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << trigger_info.getFilePath() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */

  
  // Write the directory format information at the end of the file.

  _archiveLabel.setDirPosition(_outputFile.ftell());
  _archiveDirectory.setArchiveLabel(_archiveLabel);
  _archiveDirectory.write(_outputFile);

  // Rewrite the archive label at the beginning of the file now that we have
  // the correct position of the archive directory.

  _outputFile.fseek(0, SEEK_SET);
  _archiveLabel.write(_outputFile, false);
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initTrigger()
 */

bool CfRadial2DeTect::_initTrigger(void)
{
  static const string method_name = "CfRadial2DeTect::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    const vector< string > file_list = _args->getFileList();
    
    if (file_list.size() == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify file paths on command line" << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing FILE_LIST trigger: " << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
    }
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(file_list) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger:" << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processFile()
 */

bool CfRadial2DeTect::_processFile(const string &file_path)
{
  static const string method_name = "CfRadial2DeTect::_processFile()";
  
  PMU_auto_register("Processing input file");

  if (_params->debug)
    cerr << endl << endl
	 << "*** Processing file: " << file_path << endl << endl;
  
  // Read in the input file

  RadxFile input_file;
  input_file.setReadIgnoreIdleMode(true);
  
  RadxVol volume;
  if (input_file.readFromPath(file_path, volume) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file: " << file_path << endl;
    
    return false;
  }
  
  if (_params->debug)
  {
    cerr << "   Volume has " << volume.getNSweeps() << " sweeps" << endl;
    cerr << "   Volume has " << volume.getNRays() << " rays" << endl;
  }
  
  // If we haven't started the output file yet, start it here.  There is only
  // one output file, no matter how many input files we process.  It begins
  // with an archive label.  If we can't open the output file, we go ahead and
  // exit since we won't be able to do anything after that.

  if (!_outputFileCreated)
  {
    if (_params->debug)
      cerr << "*** Creating output file: " << _params->output_file_path << endl;
    
    Path path(_params->output_file_path);
    if (path.makeDirRecurse() != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating output directories for output file: "
	   << _params->output_file_path << endl;
      
      exit(-1);
    }
    
    if (_outputFile.fopen(_params->output_file_path, "w") == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening output file: " << _params->output_file_path << endl;
      perror(_params->output_file_path);
      
      exit(-1);
    }

    // Write the archive label to start the file.  This will be overwritten
    // at the end of the processing since we need to know where the directory
    // information starts.  But write it out here with that set to 0 to save
    // the bytes in the file.

    _archiveLabel.setTimeStamp(volume.getStartTimeSecs());
    _archiveLabel.write(_outputFile, false);
    
    _outputFileCreated = true;
  }
  
  // Convert the data in the volume to 16-bit integers since that's what we
  // want in the end.  Since the DeTect files always just contain a "count"
  // field, we can set the scale to 1.0 and offset to 0.0.

  volume.convertToSi16(1.0, 0.0);
  
  // Process each of the sweeps in the volume

  const vector< RadxSweep* > sweeps = volume.getSweeps();
  vector< RadxSweep* >::const_iterator sweep_iter;
  
  const vector< RadxRay* > rays = volume.getRays();
      
  for (sweep_iter = sweeps.begin(); sweep_iter != sweeps.end(); ++sweep_iter)
  {
    if (!_processSweep(volume, *sweep_iter, rays))
      return false;
  } /* endfor - sweep_iter */
  
  return true;
}


/*********************************************************************
 * _processSector()
 */

bool CfRadial2DeTect::_processSector(const RadxVol &volume,
				     const RadxSweep *sweep,
				     const vector< RadxRay* > &rays,
				     const size_t start_sector_index,
				     const size_t end_sector_index)
{
  static const string method_name = "CfRadial2DeTect::_processSector()";
  
  if (_params->debug)
  {
    cerr << "      Processing sector:" << endl;
    cerr << "         start_sector_index = " << start_sector_index << endl;
    cerr << "         end_sector_index = " << end_sector_index << endl;
    cerr << "         start_sector_az = "
	 << rays[start_sector_index]->getAzimuthDeg() << endl;
    cerr << "         end_sector_az = "
	 << rays[end_sector_index]->getAzimuthDeg() << endl;
  }
  
  // Initialize the DataObject which will be used for writing this sector to
  // the output file.

  DataObject data_object;
  RadxRay *first_ray = rays[start_sector_index];
  size_t num_gates = first_ray->getNGates();
  
  data_object.setStartRangeKm(volume.getStartRangeKm());
  data_object.setGateSpacingKm(volume.getGateSpacingKm());
  data_object.setNumGates(num_gates);
  data_object.setLatitude(volume.getLatitudeDeg());
  data_object.setLongitude(volume.getLongitudeDeg());
  data_object.setBitsPerDatum(16);
  data_object.setStartAngle(first_ray->getAzimuthDeg());
  data_object.setNumDegrees(90.0);
  data_object.setStartTime(first_ray->getTimeSecs(),
			   (int)((first_ray->getNanoSecs() / 1.0e6) + 0.5));
  data_object.setNumBeams(end_sector_index - start_sector_index + 1);

  data_object.setPulseFiltering(_params->pulse_filtering);
  data_object.setPulsePeriod(_params->pulse_period);
  data_object.setNMI(_params->nmi);
  data_object.setNominalThroughput(_params->nominal_throughput);
  
  // Add this entry to the archive directory written at the end of the
  // file

  _archiveDirectory.addDirectoryEntry(_outputFile.ftell(),
				      first_ray->getTimeSecs());

  // Process each of the rays in the sector

  for (size_t ray_index = start_sector_index, beam_index = 0;
       ray_index <= end_sector_index; ++ray_index, ++beam_index)
  {
    // Get a pointer to the ray for ease of coding

    RadxRay *ray = rays[ray_index];
      
    // Check that the number of gates hasn't changed.  We can't handle data
    // where this changes.

    if (ray->getNGates() != num_gates)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Number of gates has changed in the middle of a sector" << endl;
      cerr << "Expected " << num_gates << " gates" << endl;
      cerr << "Found " << ray->getNGates() << " gates" << endl;
      
      return false;
    }
    
    // Now get the field information and the data.  We will convert the data to
    // int values for writing to the DeTect file.  DeTect files actually only
    // contain count values, so kludging this a bit will allow me to test this
    // app with non-count datasets.

    RadxField *field;

    if ((field = ray->getField(_params->field_name)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting " << _params->field_name
	   << " from input file" << endl;
      
      return false;
    }
    
    const Radx::si16 *data;
    if ((data = field->getDataSi16()) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting data from input beam" << endl;
      
      return false;
    }
    
    vector< int > beam(num_gates);
    for (size_t i = 0; i < num_gates; ++i)
    {
      beam[i] = (int)data[i];
    }
    
    data_object.setBeam(beam_index,
			ray->getTimeSecs(),
			(int)((ray->getNanoSecs() / 1.0e6) + 0.5),
			ray->getAzimuthDeg(), beam);
  } /* endfor - ray_index */
    
  // Write the sector to the output file

  if (_params->debug)
    data_object.print(cerr);
  
  data_object.write(_outputFile);

  // Set the radar information in the archive label so it can be included in
  // the label written at the end of the file.

  if (_firstSector)
  {
    _archiveLabel.setRadarInformation(data_object);
    _firstSector = false;
  }
  
  return true;
}


/*********************************************************************
 * _processSweep()
 */

bool CfRadial2DeTect::_processSweep(const RadxVol &volume,
				    const RadxSweep *sweep,
				    const vector< RadxRay* > &rays)
{
  static const string method_name = "CfRadial2DeTect::_processSweep()";
  
  // Find the beginning and ending rays for the sweep, getting rid of any
  // transition rays.  I think there will only be transition rays at the end
  // of the sweep, but will do both sides just in case.

  size_t start_sweep_index = sweep->getStartRayIndex();
  size_t end_sweep_index = sweep->getEndRayIndex();
    
  if (_params->debug)
  {
    cerr << "   Processing volume " << sweep->getVolumeNumber()
	 << ", sweep " << sweep->getSweepNumber() << endl;
  }
    
  for (size_t ray_index = start_sweep_index; ray_index <= end_sweep_index;
       ++ray_index)
  {
    if (!rays[ray_index]->getAntennaTransition())
    {
      start_sweep_index = ray_index;
      break;
    }
  } /* endfor - ray_index */
    
  for (size_t ray_index = end_sweep_index; ray_index >= start_sweep_index;
       --ray_index)
  {
    if (!rays[ray_index]->getAntennaTransition())
    {
      end_sweep_index = ray_index;
      break;
    }
  } /* endfor - ray_index */
    
  if (start_sweep_index >= end_sweep_index)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No non-transition rays found in sweep" << endl;
    
    return false;
  }
  
  // Now that we have the sweep isolated, divide the sweep into 90 degree
  // sectors.  Each sector will be written to the output file as a DataObject.

  size_t start_sector_index = start_sweep_index;

  double end_sector_azimuth =
    rays[start_sweep_index]->getAzimuthDeg() + SECTOR_SIZE;
  size_t sectors_processed = 0;
  
  for (size_t ray_index = start_sweep_index; ray_index <= end_sweep_index;
       ++ray_index)
  {
    // If we found the end of the sector, process the sector and reset our
    // indices

    if (rays[ray_index]->getAzimuthDeg() >= end_sector_azimuth)
    {
      _processSector(volume, sweep, rays, start_sector_index, ray_index - 1);
      
      ++ sectors_processed;
      
      start_sector_index = ray_index;
      end_sector_azimuth += SECTOR_SIZE;
    }

  } /* endfor - ray_index */
  
  // Process the last sector in the sweep

  if (sectors_processed < (360 / SECTOR_SIZE))
    _processSector(volume, sweep, rays, start_sector_index, end_sweep_index);
  
  return true;
}
