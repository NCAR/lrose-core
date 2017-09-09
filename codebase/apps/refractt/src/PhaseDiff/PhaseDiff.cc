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
 * @file PhaseDiff.cc
 *
 * @class PhaseDiff
 *
 * PhaseDiff is the top level application class.
 *  
 * @date 2/18/2010
 *
 */
#include "PhaseDiff.hh"
#include <Refract/ParmApp.hh>
#include <Refract/FieldDataPair.hh>
#include <Refract/FieldWithData.hh>
#include <toolsa/toolsa_macros.h>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <algorithm>

// Global variables

PhaseDiff *PhaseDiff::_instance = (PhaseDiff *)NULL;
const fl32 PhaseDiff::MISSING_DATA_VALUE = -9999.9;
const double PhaseDiff::SNR_NOISE_MAX = 0.25;
const int PhaseDiff::VERY_LARGE = 2147483647;
// Noise threshold set to .2*10 dB above average
const double PhaseDiff::OFFSET_ABOVE_AVERAGE = 0.2;
const double PhaseDiff::DM_NOISE = -114.4132;

//------------------------------------------------------------------
static bool _isMultiElev(const time_t &t, const string &url,
			 const std::string &fieldName)
{

  DsMdvx D;
  D.setReadTime(Mdvx::READ_CLOSEST, url, 0, t);
  D.addReadField(fieldName);
  if (D.readVolume())
  {
    LOG(ERROR) << "Reading from " << url << " at " << DateTime::strn(t);
    return false;
  }

  Mdvx::field_header_t fh = D.getField(0)->getFieldHeader();
  return fh.nz > 1;
}

//------------------------------------------------------------------
static time_t _best_matching_scan_time(const time_t &target_time,
				       int margin_seconds,
				       const string &url,
				       const string &fieldName,
				       Params::scan_mode_t scan_mode)
{
  DsMdvx D;
  D.setTimeListModeValid(url, target_time - margin_seconds, target_time);
  D.compileTimeList();
  vector<time_t> times = D.getTimeList();
  if (times.empty())
  {
    LOG(ERROR) << "No times near the target time"
	       << DateTime::strn(target_time);
    return -1;
  }
  vector<time_t>::reverse_iterator r;
  for (r = times.rbegin(); r!=times.rend(); ++r)
  {
    if (scan_mode == Params::MULTIPLE_ELEV_ONLY)
    {
      if (_isMultiElev(*r, url, fieldName))
      {
	return *r;
      }
    }
    else if (scan_mode == Params::SINGLE_ELEV_ONLY)
    {
      if (!_isMultiElev(*r, url, fieldName))
      {
	return *r;
      }
    }
    else
    {
      return *r;
    }
  }
  LOG(ERROR) << "No times near target time that had correct scan mode";
  return -1;
}

/*********************************************************************
 * Constructor
 */

PhaseDiff::PhaseDiff(int argc, char **argv) : _dataTrigger(0)
{
  // Make sure the singleton wasn't already created.

  assert(_instance == (PhaseDiff *)NULL);
  
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

  // Get TDRP parameters.
  if (!parmAppInit(_params, argc, argv))
  {
    exit(0);
  }
  if (RefParms::isPrintMode())
  {
    _refparms.print(stdout, PRINT_VERBOSE);
  }
  else
  {
    if (_refparms.load(RefParms::parmPath().c_str(), NULL,
		       !RefParms::isParmPrint(), false))
    {
      LOG(ERROR) << "cant load " <<  RefParms::parmPath();
      okay = false;
    }
    else
    {
      _refparms.setOk();
    }
    if (RefParms::isPrintAndLoadMode())
    {
      _refparms.print(stdout, PRINT_VERBOSE);
    }  
  }
  parmAppFinish(_params, _refparms);

  if (_refparms.raw_iq_in_input)
  {
    _testField = _refparms.raw_i_field_name;
  }
  else
  {
    _testField = _refparms.niq_field_name;
  }
}


/*********************************************************************
 * Destructor
 */

PhaseDiff::~PhaseDiff()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

PhaseDiff *PhaseDiff::Inst(int argc, char **argv)
{
  if (_instance == (PhaseDiff *)NULL)
    new PhaseDiff(argc, argv);
  
  return(_instance);
}

PhaseDiff *PhaseDiff::Inst()
{
  assert(_instance != (PhaseDiff *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool PhaseDiff::init()
{
  // Create the colorscale files, if requested

  if (_params.create_phase_diff_colorscale)
    _createPhaseColorscale();

  if (_params.create_niq_colorscale)
    _createNiqColorscale();
  
  // Initialize the data trigger
  if (!_refparms.initTrigger(&_dataTrigger))
  {
    return false;
  }
  _input = _refparms.initInput();

  // initialize process registration

  PMU_auto_init(_progName, _refparms.instance, PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void PhaseDiff::run()
{
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      LOG(ERROR) << "getting next trigger time";
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      LOG(ERROR) << "processing data for time: " << trigger_time;
      continue;
    }
  } /* endwhile - !_dataTrigger->endOfData() */
  
}

/*********************************************************************
 * _processData()
 */

bool PhaseDiff::_processData(const DateTime &trigger_time)
{
  LOG(DEBUG)<< "*** Processing data for time: " << trigger_time;

  // see if input data matches requirements
  time_t t = trigger_time.utime();
  if (_params.scan_mode == Params::MULTIPLE_ELEV_ONLY)
  {
    if (!_isMultiElev(t, _refparms.input_url, _testField))
    {
      LOG(DEBUG) << "Not multi elev..ignore";
      return true;
    }
  }
  else if (_params.scan_mode == Params::SINGLE_ELEV_ONLY)
  {
    if (_isMultiElev(t, _refparms.input_url, _testField))
    {
      LOG(DEBUG) << "Multi elev..ignore";
      return true;
    }
  }
  else
  {
    // proceed
  }
  
  
  // Read the radar file from the trigger time

  DsMdvx mdvx2;
  
  if (!_readInputFile(mdvx2, trigger_time))
    return false;
  
  // Read in the matching file

  DsMdvx mdvx1;
  
  time_t best_time =
    _best_matching_scan_time(t - _params.lookback_secs,
			     _params.lookback_search_margin,
			     _refparms.input_url, _testField,
			     _params.scan_mode);
  if (best_time == -1)
  {
    return false;
  }

  if (!_readInputFile(mdvx1, best_time))
    return false;
  
  LOG(DEBUG) << "  Base MDV file time: "
	     << DateTime::str(mdvx1.getMasterHeader().time_centroid);
  LOG(DEBUG) << "  Matching MDV file time: "
	     << DateTime::str(mdvx2.getMasterHeader().time_centroid);
  
  // Calculate the phase diff fields.  This adds the calculated fields to
  // the base input file -- mdvx2.

  if (!_calcPhaseDiff(mdvx1, mdvx2))
    return false;
  
  // Write the output file
  if (mdvx2.writeToDir(_refparms.output_url) != 0)
  {
    LOG(ERROR) << "writing file to URL: " << _refparms.output_url;
    LOG(ERROR) << mdvx2.getErrStr();
    return false;
  }
  return true;
}

/*********************************************************************
 * _calcPhaseDiff()
 */

bool PhaseDiff::_calcPhaseDiff(DsMdvx &mdvx1, DsMdvx &mdvx2) const
{
  FieldWithData I1 = _input->getI(mdvx1);
  FieldWithData Q1 = _input->getQ(mdvx1);
  FieldDataPair iq1(I1, Q1);
  FieldWithData I2 = _input->getI(mdvx2);
  FieldWithData Q2 = _input->getQ(mdvx2);
  FieldDataPair iq2(I2, Q2);

  FieldDataPair change_iq(iq1, "change_i", "none", 0.0, "change_q", "none",0.0);
  FieldDataPair diff_phase_iq(iq1, "diff_phase_i", "none", 0.0,
			      "diff_phase_q", "none", 0.0);
  FieldWithData phase_field(iq1.getI(), "phase_diff", "none", 0.0);
  FieldWithData niq_field(iq1.getI(), "niq", "none", 0.0);

  // Create the blank output fields and get the needed information
  // MdvxField *change_i_field = I1.createMatchingBlankField("change_i");
  // MdvxField *change_q_field = I1.createMatchingBlankField("change_q");
  // MdvxField *diff_phase_i_field = I1.createMatchingBlankField("diff_phase_i");
  // MdvxField *diff_phase_q_field = I1.createMatchingBlankField("diff_phase_q");
  // MdvxField *phase_field = I1.createMatchingBlankField("phase_diff");
  // MdvxField *niq_field = I1.createMatchingBlankField("niq");
  // if (change_i_field == 0 || change_q_field == 0 ||
  //     diff_phase_i_field == 0 || diff_phase_q_field == 0 ||
  //     phase_field == 0 || niq_field == 0)
  // {
  //   LOG(ERROR) << "creating blank fields for the output fields";

  //   // hey, this won't work! says me dave
  //   delete change_i_field;
  //   delete change_q_field;
  //   delete diff_phase_i_field;
  //   delete diff_phase_q_field;
  //   delete phase_field;
  //   delete niq_field;
  //   return false;
  // }
  
  // fl32 *change_i_data = (fl32 *)change_i_field->getVol();
  // fl32 *change_q_data = (fl32 *)change_q_field->getVol();
  // fl32 *diff_phase_i_data = (fl32 *)diff_phase_i_field->getVol();
  // fl32 *diff_phase_q_data = (fl32 *)diff_phase_q_field->getVol();
  // fl32 *phase_data = (fl32 *)phase_field->getVol();
  // fl32 *niq_data = (fl32 *)niq_field->getVol();
  
  // Calculate the phase difference fields

  int data_size = I1.scanSize();
  for (int i = 0; i < data_size; ++i)
  {
    bool debug = _input->isDebugPt(i);

    // Check for missing data
    if (iq1.missingIorQ(i) || iq2.missingIorQ(i))
    // if (I1.isBadAtIndex(i) || Q1.isBadAtIndex(i) ||
    // 	I2.isBadAtIndex(i) || Q2.isBadAtIndex(i))
    {
      continue;
    }
    
    // Calculate the change fields
    // change_i_data[i] = (I2._data[i]*I1._data[i]) + (Q2._data[i]*Q1._data[i]);
    // change_q_data[i] = (I2._data[i]*Q1._data[i]) - (Q2._data[i]*I1._data[i]);
    change_iq[i] = iq2[i].phaseDiffC(iq1[i]);
    // Calculate the phase diff fields
    // diff_phase_i_data[i] = I1._data[i] * I1._data[i];
    // diff_phase_q_data[i] = Q1._data[i] * Q1._data[i];
    diff_phase_iq[i] = iq1[i].iqProduct(iq2[i]);
    
    // Calculate the phase field
    if (!change_iq[i].isZero())
    // if (change_i_data[i] != 0.0 || change_q_data[i] != 0.0)
    {
      phase_field[i] = -change_iq[i].phase();
      //atan2(change_q_data[i], change_i_data[i]) / DEG_TO_RAD;
    }
    
    // if (debug)
    // {
    //   printf("delta_i = i2*i1 + q2*q1[%d] = %lf\n", i, change_i_data[i]);
    //   printf("delta_q = i2*q1 - q2*i1[%d] = %lf\n", i, change_q_data[i]);
    //   printf("Phase[%d] = %lf\n", i, phase_data[i]);
    // }

    // Calculate the niq field
    if (!diff_phase_iq[i].isZero())
    // if (diff_phase_i_data[i] != 0.0 || diff_phase_q_data[i] != 0)
    {
      niq_field[i] = 10.0*log10(diff_phase_iq[i].norm());
	// 10.0 * log10(sqrt((diff_phase_i_data[i] * diff_phase_i_data[i]) +
	// 		  (diff_phase_q_data[i] * diff_phase_q_data[i])));
    }
  } /* endfor - i */
  
  // Add the calculated fields to the second (base) MDV file

  change_iq.addToOutput(mdvx2);
  diff_phase_iq.addToOutput(mdvx2);
  mdvx2.addField(phase_field.fieldCopy());
  mdvx2.addField(niq_field.fieldCopy());
  
  return true;
}

/*********************************************************************
 * _createNiqColorscale()
 */

void PhaseDiff::_createNiqColorscale() const
{
  FILE *file;

  if ((file = fopen(_params.niq_colorscale_path, "w")) == 0)
  {
    LOG(ERROR) << "opening colorscale file for output";
    perror(_params.niq_colorscale_path);
    return;
  }
  
  fprintf(file, "%8.2f  %8.2f   #800080\n", -100.0, -50.0);

  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)(119 - i) / 120.0);
    
    short int red = 0, green = 0, blue = 0;
    
    if (step <= 1.0)
    {
      // violet to red

      red = (short int)(128 + (step * 127));
      green = 0;
      blue = (short int)(128 - (step * 128));
    }
    else if (step < 2.0)
    {
      red = 255;
      green = (short int)((step - 1.0) * 255);
      blue = 0;
    }
    else if (step == 2.0)
    {
      red = 247;
      green = 247;
      blue = 0;
    }
    else if (step <= 3.0)
    {
      red = (short int)((3.0 - step) * 255);
      green = 255;
      blue = 0;
    }
    else if (step <= 4.0)
    {
      red = 0;
      green = (short int)(255 - ((step - 3.0) * 127));
      blue = (short int)((step - 3.0) * 128);
    }
    else if (step <= 5.0)
    {
      red = 0;
      green = (short int)((5.0 - step) * 128);
      blue = 255 + (short int)((step - 5.0) * 127);
    }
    else if (step <= 6.0)
    {
      red = (short int)((step - 5.0) * 128);
      green = 0;
      blue = 255 - (short int)((step - 5.0) * 127);
    }
    else
    {
      fprintf(file, "*** step = %f\n", step);
    }
    
    double min_value = ((60.0 / 120.0) * (double)i) - 50.0;
    double max_value = ((60.0 / 120.0) * (double)(i+1)) - 50.0;
    
    fprintf(file, "%8.2f  %8.2f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fprintf(file, "%8.2f  %8.2f   #ffffff\n", 10.0, 100.0);

  fclose(file);
}


/*********************************************************************
 * _createPhaseColorscale()
 */

void PhaseDiff::_createPhaseColorscale() const
{
  FILE *file;

  if ((file = fopen(_params.phase_diff_colorscale_path, "w")) == 0)
  {
    LOG(ERROR) << "opening colorscale file for output";
    perror(_params.phase_diff_colorscale_path);
    return;
  }
  
  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)(119 - i) / 120.0);
    
    short int red = 0, green = 0, blue = 0;

    if (step <= 1.0)
    {
      // violet to red

      red = (short int)(128 + (step * 127));
      green = 0;
      blue = (short int)(128 - (step * 128));
    }
    else if (step < 2.0)
    {
      red = 255;
      green = (short int)((step - 1.0) * 255);
      blue = 0;
    }
    else if (step == 2.0)
    {
      red = 247;
      green = 247;
      blue = 0;
    }
    else if (step <= 3.0)
    {
      red = (short int)((3.0 - step) * 255);
      green = 255;
      blue = 0;
    }
    else if (step <= 4.0)
    {
      red = 0;
      green = (short int)(255 - ((step - 3.0) * 127));
      blue = (short int)((step - 3.0) * 128);
    }
    else if (step <= 5.0)
    {
      red = 0;
      green = (short int)((5.0 - step) * 128);
      blue = 255 + (short int)((step - 5.0) * 127);
    }
    else if (step <= 6.0)
    {
      red = (short int)((step - 5.0) * 128);
      green = 0;
      blue = 255 - (short int)((step - 5.0) * 127);
    }
    else
    {
      fprintf(file, "*** step = %f\n", step);
    }
    
    double min_value = ((360.0 / 120.0) * (double)i) - 180.0;
    double max_value = ((360.0 / 120.0) * (double)(i+1)) - 180.0;
    
    fprintf(file, "%5.0f  %5.0f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);
}

/*********************************************************************
 * _readInputFile()
 */

bool PhaseDiff::_readInputFile(DsMdvx &mdvx, const DateTime &data_time)
{
  if (!_input->getScan(data_time, 0, _refparms.input_url, mdvx))
  {
    LOG(ERROR) << "Could not read in data";
    return false;
  }
  return true;
}



