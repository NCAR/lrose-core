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
#include <toolsa/copyright.h>

/**
 * @file ParmFiltAlgApp.hh
 * @brief template functions to allow application params function calls
 * @class ParmFiltAlgApp
 * @brief template functions to allow application params function calls
 */

# ifndef    ParmFiltAlgApp_HH
# define    ParmFiltAlgApp_HH

#include <tdrp/tdrp.h>
#include <FiltAlg/InterfaceFiltAlgParm.hh>

/*----------------------------------------------------------------*/
template <class T>
bool parmFiltAlgAppSet(T &app_params, FiltAlgParms &filt_params, int argc,
		       char **argv)
{
  if (!parmFiltAlgAppInit(app_params, argc, argv))
  {
    return false;
  }
  if (!InterfaceFiltAlgParm::load_params(filt_params))
  {
    printf("Fatal..Problem with TDRP parameters\n");
    return false;
  }
  parmFiltAlgAppFinish(app_params, filt_params);
  return true;
}


/*----------------------------------------------------------------*/
template <class T>
bool parmFiltAlgAppInit(T &app_params, int argc, char **argv)
{
  if (!InterfaceFiltAlgParm::parm_init(argc, argv))
  {
    return false;
  }
  if (InterfaceFiltAlgParm::is_parm_load())
  {
    if (app_params.load(InterfaceFiltAlgParm::parm_path(), NULL,
			!InterfaceFiltAlgParm::is_parm_print(), false))
    {
      printf("ERROR loading app params from file %s\n",
	     InterfaceFiltAlgParm::parm_path());
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
template <class T, class S>
void parmFiltAlgAppFinish(T &app_params, S &main_params)
{
  if (InterfaceFiltAlgParm::is_parm_print())
  {
    app_params.print(stdout, PRINT_VERBOSE);
  }
  InterfaceFiltAlgParm::parm_finish();
}

#endif
