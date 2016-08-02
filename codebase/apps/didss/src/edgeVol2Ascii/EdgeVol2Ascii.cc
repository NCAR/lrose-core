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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:42 $
//   $Id: EdgeVol2Ascii.cc,v 1.6 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeVol2Ascii.cc: EdgeVol2Ascii program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <edge/load_data.h>
#include <edge/vol.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "EdgeVol2Ascii.hh"
using namespace std;



// Global variables

EdgeVol2Ascii *EdgeVol2Ascii::_instance =
     (EdgeVol2Ascii *)NULL;


/*********************************************************************
 * Constructor
 */

EdgeVol2Ascii::EdgeVol2Ascii(int argc, char **argv)
{

  const string method_name = "EdgeVol2Ascii::EdgeVol2Ascii()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (EdgeVol2Ascii *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = progname_parts.base;
  
  // Display ucopright message.

  ucopyright(_progName.c_str());

  // parse the args

  if (_args.parse(argc, argv, _progName)) {
    okay = false;
    return;
  }

}


/*********************************************************************
 * Destructor
 */

EdgeVol2Ascii::~EdgeVol2Ascii()
{
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

EdgeVol2Ascii *EdgeVol2Ascii::Inst(int argc, char **argv)
{
  if (_instance == (EdgeVol2Ascii *)NULL)
    new EdgeVol2Ascii(argc, argv);
  
  return(_instance);
}

EdgeVol2Ascii *EdgeVol2Ascii::Inst()
{
  assert(_instance != (EdgeVol2Ascii *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run() - run the program.
 */

void EdgeVol2Ascii::run()
{

  if (_args.debug) {
    cerr << "n input files: " << _args.inputFileList.size() << endl;
  }

  // Process each of the volume files

  vector<string>::iterator file_iter;
  
  for (file_iter = _args.inputFileList.begin();
       file_iter != _args.inputFileList.end();
       ++file_iter)
  {
    cerr << "*** Processing file: " << *file_iter << endl;
    
    struct vol_struct *ptr = NULL;
    
    /*
    ** Load the product
    */
    load_data((char **)&ptr,(char *)(file_iter->c_str()),VOL_FILE);
    /*
    ** Display max range see prod.h for structure definition
    ** and other fields available in prod structure
    */
    printf("Number of sweeps in volume is %d\n",ptr->num_sweeps);

    // Inspect each sweep
    for (int i=0;i<ptr->num_sweeps;i++)
    {
      printf("sweep:%d\n",i);
      printf("number of range bins in each ray is %d\n",	
	     GATES(ptr,i));
      printf("range bin size is %G meters\n",GATE_SIZE(ptr,i));
      printf("Number of rays is %d\n",RAYS(ptr,i));
	
      // inspect each ray (radial)
      for(int j=0;j<RAYS(ptr,i);j++)
      {
	// Get the pointer to the data for this ray
	unsigned char *ray_ptr=RAY_PTR(ptr,i,j);
	
	printf("ray %d: Az range %G-%G "
	       "Elev range %G-%G\n", j,
	       START_AZ_DEGS(ray_ptr),
	       END_AZ_DEGS(ray_ptr),
	       START_EL_DEGS(ray_ptr),
	       END_EL_DEGS(ray_ptr));
	// Inspect each range bin
	for(unsigned int k=0;k<GATES(ptr,i);k++)
	{
	  unsigned char data_value = *(ray_ptr+8+k*4);
	  printf("Range bin:%d Slant range = %G ",
		 k,k*GATE_SIZE(ptr,i));
	  if (data_value==0)
	  {
	    printf("no data\n");
	  }
	  else if (data_value==255)
	  {
	    printf("did not look here(treat as no data)\n");
	  }
	  else
	  {
	    printf("data value = %d(%G dBz)\n",data_value,data_value/2.0-32.0);
	  }
	}
      }
    }
    
    /*
    ** Free memory
    */
    free(ptr);
    ptr=NULL;
  } /* endfor - file_iter */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

