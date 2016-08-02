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
///////////////////////////////////////////////////////////////
// RainGage.cc
//
// RainGage object
//
// Alex Baia, August 2000
//
// code created from BasinPrecip.cc, Mike Dixon, Sept 1998
//
///////////////////////////////////////////////////////////////
#include <string>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>

#include "RainGage.hh"
#include <cstdio>
using namespace std;

//string RainGage::tdf = "gage.txt";



/*********************************************************************
 * Constructors
 */

RainGage::RainGage(int argc, char **argv)
{
  const string routine_name = "Constructor";
  
  OK = true;

  // set programe name

  _progName = STRdup("RainGage");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK)
  {
    cerr << "ERROR: " /*<< _className()*/ << "::" << routine_name << endl;
    cerr << "Problem with command line args" << endl;
    
    OK = false;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath))
  {
    cerr << "ERROR: " /*<< _className()*/ << "::" << routine_name << endl;
    cerr << "Problem with TDRP parameters" << endl;
    
    OK = false;
    return;
  }

  // check start and end in ARCHIVE mode

  if ((_params->mode == Params::ARCHIVE) &&
      (_args->startTime == -1 || _args->endTime == -1))
  {
    cerr << "ERROR: " /*<< _className()*/ << "::" << routine_name << endl;
    cerr << "In ARCHIVE mode start and end times must be specified." << endl;
    cerr << "Run '" << _progName << " -h' for usage" << endl;
    
    OK = false;
    return;
  }



  //
  // Initialize the object used for finding input files.
  //



  // init process mapper registration

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}


/*********************************************************************
 * Destructor
 */

RainGage::~RainGage()
{
  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_params);
  delete(_args);
  STRfree(_progName);
}


/*********************************************************************
 * Run() - Run the RainGage program.
 *
 * Returns the return code for the program (0 if successful, error code
 * otherwise).
 */

int RainGage::Run ()
{

  cout << "TEST!!!" << endl;

  ParseGageTData();

  return 0;

}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/


/********************************************************************
 * ParseGageTData() - Function parses a rain gage ASCII data file.
 *                    
 *
 */

void RainGage::ParseGageTData()
{

  const int maxline = 80;

  //char fname[] = "./gage_data/20000808/120000.txt";
  //char fname[] = "dat.dat";


  // get list of input files in the running time period
  
  time_t periodStart = _args->startTime;
  time_t periodEnd = _args->endTime;

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "RunningDriver: period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }

  DsInputPath *inputFiles =
    new DsInputPath(_progName, _params->debug,
		    _params->input_gage_url,
		    periodStart, periodEnd);


  char *inputPath;

  while ((inputPath = inputFiles->next()) != NULL) 

    {

      char line[80];
      FILE *fp;
      char comma;
      fp = fopen(inputPath,"r");
      int id, nc;
      float five_min, one_hour, six_hour, one_day;
      char date[9];
      char time[9];
      char SHEF[6];
      char  *input;
      fgets(line, maxline, fp);
      fgets(line, maxline, fp);
      
      while(fgets(line,maxline,fp) != NULL)
	{
	  
	  if ( (nc = sscanf(line,"%d %c %8s %8s%c %f %c %f %c %f %c %f %c %5s",
			    &id, &comma, date, time, &comma,
			    &five_min, &comma, &one_hour, &comma,
			    &six_hour, &comma, &one_day, &comma, 
			    SHEF)) == 14) 
	    {
	      printf("%d\n",id) ; 
	      printf("%s\n",date); 
	      printf("%s\n",time); 
	      printf("%f\n",five_min); 
	      printf("%f\n",one_hour); 
	      printf("%f\n",six_hour); 
	      printf("%f\n",one_day); 
	      printf("%s\n",SHEF); 
	    }
	  
	}  // end while
      
      fclose(fp); 

    }  // end while inputPath

}








