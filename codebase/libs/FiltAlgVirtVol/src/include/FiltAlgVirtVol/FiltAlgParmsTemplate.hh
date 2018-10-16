/**
 * @file FiltAlgParmsTemplate.hh
 * @brief 
 * @class FiltAlgParmsTemplate
 * @brief 
 */

# ifndef    FiltAlgParmsTempate_HH
# define    FiltAlgParmsTempate_HH

#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <tdrp/tdrp.h>

/*----------------------------------------------------------------*/
/**
 * parmAppInit The app has a parameters class T which is the parameters
 *
 * @tparam T  Params class for app, derived class of FiltAlgParms
 * @param[in,out] appParams  Parameters for the app (object of type T)
 * @param[in] argc  Command line number of args
 * @param[in] argv  Command line args
 *
 */
template <class T>
bool parmAppInit(T &appParams, int argc, char **argv)
{
  tdrp_print_mode_t printMode;
  int expandEnv;
  bool pp, po, pr;
  string fileName;
  pp = FiltAlgParms::isPrintParams(argc, argv, printMode, expandEnv);
  po = FiltAlgParms::isPrintOperators(argc, argv);
  pr = FiltAlgParms::isSetParams(argc, argv, fileName);

  if (po)
  {
    appParams.printOperators();
    return false;
  }

  if (pr)
  {
    if (pp)
    {
      appParams = T(fileName, expandEnv);
    }
    else
    {
      appParams = T(fileName, true);
    }
    appParams.setFiltersFromParms();
  }
  
  if (pp)
  {
    appParams.printParams(printMode);
    return false;
  }

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
}

#endif
