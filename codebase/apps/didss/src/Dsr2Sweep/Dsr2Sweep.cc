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
////////////////////////////////////////////////////////////////////////
// Dsr2Sweep.cc
//
// Dsr2Sweep object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////////////
//
// Dsr2Sweep reads an input radar FMQ and saves it out as a sweep file,
// either DORADE sweep or netCDF sweep.
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>

#include "DoradeFileHandler.hh"
#include "NcFileHandler.hh"

#include "Dsr2Sweep.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Dsr2Sweep::Dsr2Sweep(int argc, char **argv) :
  _fileHandler(0)
{
  isOK = true;

  // set programe name

  _progName = "Dsr2Sweep";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }
}


/*********************************************************************
 * Destructor
 */

Dsr2Sweep::~Dsr2Sweep()
{
  delete _fileHandler;
  
  // unregister process

  PMU_auto_unregister();

}


/*********************************************************************
 * init() - Initialize the program.
 */

bool Dsr2Sweep::init()
{

  // Create the file handler

  switch (_params.output_format)
  {
  case Params::DORADE_OUTPUT :
    _fileHandler = new DoradeFileHandler(_params);
    break;
    
  case Params::NC_RADAR_OUTPUT :
    _fileHandler = new NcFileHandler(_params);
    break;
  }
  
  // create field refs vector

  for (int i = 0; i < _params.output_fields_n; i++)
  {
    Params::output_field_t &outField = _params._output_fields[i];

    int byteWidth = 4; // FL32
    if (outField.output_encoding == Params::ENCODING_SI16) {
      byteWidth = 2;
    }

    FieldInfo finfo(outField.dsr_name,
		    outField.output_name,
		    outField.output_units,
                    byteWidth);

    finfo.outputScale = outField.output_scale;
    finfo.outputBias = outField.output_bias;

    _fileHandler->addField(finfo);
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * Run() - Run the program.
 */

int Dsr2Sweep::Run()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true)
  {
    _run();
    if (_params.debug)
    {
      cerr << "Dsr2Sweep::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    sleep(2);
  }

  return 0;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _run() - Run the program.
 */

int Dsr2Sweep::_run ()
{
  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;

  if (_params.seek_to_end_of_input)
  {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::END ))
    {
      fprintf(stderr, "ERROR - %s:Dsr2Sweep::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }
  else
  {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Sweep::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }
  
  // read beams from the queue and process them
  
  while (true)
  {
    int contents;

    if (radarQueue.getDsMsg(radarMsg, &contents) == 0)
    {
      PMU_auto_register("Processing message");
      
      // Process the message

      _fileHandler->processMsg(radarMsg, contents);
      
    } /* if (radarQueue.getDsMsg(... */
    
  } /* while (true) */
  
  return 0;
}
