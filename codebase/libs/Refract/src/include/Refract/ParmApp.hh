// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>

/**
 * @file ParmApp.hh
 * @brief Template functions to allow application parameters function calls
 * @class ParmApp
 * @brief Template functions to allow application parameters function calls
 *
 *   Developer typically creates a "Parms" class for the app.
 *   The constructor source code has a "Params" object specific to the app.
 *
 *   The developer has code something like this:
 *   method(int argc, char **argv)
 *   {
 *     Params p;  // app params
 *     RefParms r;
 *     if (!parmAppInit(p, argc, argv))
 *       exit(0);
 *   
 *     if (RefParms::isPrintMode())
 *     {
 *        r.print(stdout, PRINT_VERBOSE);
 *     }
 *     else
 *     {
 *       if r.load(RefParms::parmPath().c_str(), NULL, !RefParams.isParmPrint(),
 *                 false)
 *       {
 *         LOG(ERROR) "cant load Refparms.parmPath();
 *         ok = false;
 *       }
 *       if (RefParms::isPrintAndLoadMode())
 *       {
 *          r.print(stdout, PRINT_VERBOSE);
 *       }  
 *     }
 *     parmAppFinish(p);
 *   }
 */

# ifndef    ParmApp_HH
# define    ParmApp_HH

#include <stdlib.h>
#include <string>
#include <tdrp/tdrp.h>  // Needed for PRINT_VERBOSE below
#include <Refract/RefParms.hh>
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
 * @note: appParams must have a method called load:
 *          bool load(const char *path, char **overrideList, bool expandEnv,
 *                    bool debug);
 */
template <class T>
bool parmAppInit(T &appParams, int argc, char **argv)
{
  // ask interface if arguments are good for param loading, yes or no
  if (!RefParms::parmInit(argc, argv))
  {
    return false;
  }

  // ask interface if settings are such that a parmfile should be loaded or not.
  if (RefParms::isParmLoad())
  {
    string s = RefParms::parmPath();
    // a param file should be loaded. Call the apps load method, using
    // the path from the interface and param printing status from the interface
    if (appParams.load(s.c_str(), NULL, !RefParms::isParmPrint(), false))
    {
      printf("ERROR loading app params from file %s\n", s.c_str());
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
/**
 * parmAppFinish has an app parameters class object appParams and
 * a ParmMain object mainParams
 *
 * @tparam T  Parameter class for calling app
 * @param[in,out] appParams      parameters for the app
 *
 * NOTE: appParams must have a method called print:
 *          void print(FILE *f, tdrp_print_mode_t mode);
 */
template <class T, class M>
void parmAppFinish(T &appParams, const M &refParams)
{
  // ask interface if command args specified printing of parameters
  if (RefParms::isParmPrint())
  {
    // call the app print method
    appParams.print(stdout, PRINT_VERBOSE);
  }
  // do the interface finish step.
  RefParms::parmFinish(refParams);
}

#endif
