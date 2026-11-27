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
 * @file ParmApp.hh
 * @brief Template functions to mix reading of app params with reading of 
 *        shared library params
 * @class ParmApp
 * @brief Template functions to mix reading of app params with reading of 
 *        shared library params
 *
 *   The constructor source code has a "Params" object specific to the app.
 *
 *   The developer has code something like this:
 *
 *     Params p;
 *     if (!parmAppInit(p, argc, argv))
 *     {
 *       error();
 *     }
 *
 *     intermediate_steps();
 *
 *     parmAppFinish(p);
 *
 */

# ifndef    ParmApp_HH
# define    ParmApp_HH

#include <stdlib.h>
#include <string>
#include <tdrp/tdrp.h>  
#include "AppParams.hh"
#include "App.hh"
using std::string;

/*----------------------------------------------------------------*/
/**
 * parmAppInit The app has a parameters class T which is the parameters
 *
 * @tparam T  Params class for app
 * @param[in,out] appParams  Parameters for the app (object of type T)
 * @param[in] argc  Command line number of args
 * @param[in] argv  Command line args
 *
 * @note: appParams is a tdrp class
 */
template <class T>
bool parmAppInit(T &appParams, App &r, int argc, char **argv)
{
  AppArgs a;
  std::string appName;
  if (a.parse(argc, argv, appName))
  {
    printf("ERROR\n");
    return false;
  }

  TDRP_warn_if_extra_params(FALSE);
  char * path;
  if (appParams.loadFromArgs(argc, argv, a.override.list, &path, true))
  {
    printf("Problem with Application TDRP parameters.\n");
    return false;
  }    

  AppParams p;
  if (p.loadFromArgs(argc, argv, a.override.list, &path, true))
  {
    printf("Problem with AppParams TDRP parameters.\n");
    return false;
  }
  TDRP_warn_if_extra_params(TRUE);
  if (a.tdrpExit)
  {
    exit(0);
  }
  r.setValues(a, appName, path, p);
  return true;
}

/*----------------------------------------------------------------*/
/**
 * parmAppFinish has an app parameters class object appParams
 *
 * @tparam T  Parameter class for calling app
 * @param[in,out] appParams      parameters for the app
 *
 * NOTE: appParams is a tdrp class
 */
template <class T, class S>
void parmAppFinish(T &appParams)
{
  // // ask interface if command args specified printing of parameters
  // if (AppParms::isParmPrint())
  // {
  //   // call the app print method
  //   appParams.print(stdout, PRINT_VERBOSE);
  // }

  // // do the interface finish step.
  // AppParms::parmFinish();
}

#endif
