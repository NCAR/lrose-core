/**
 * @file RadxAppParmsTemplate.hh
 * @brief 
 * @class RadxAppParmsTemplate
 * @brief 
 */

# ifndef    RadxAppParmsTempate_HH
# define    RadxAppParmsTempate_HH

#include <radar/RadxAppParms.hh>
#include <tdrp/tdrp.h>

/*----------------------------------------------------------------*/
/**
 * parmAppInit The app has a parameters class T which is the parameters
 *
 * @tparam T  Params class for app, derived class of RadxAppParms
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
  bool pp, po, pr, files, help;
  string fileName;
  vector<string> fileList;
  pp = RadxAppParms::isPrintParams(argc, argv, printMode, expandEnv);
  po = RadxAppParms::isPrintOperators(argc, argv);
  help = RadxAppParms::isHelp(argc, argv);
  pr = RadxAppParms::isSetParams(argc, argv, fileName);
  files = RadxAppParms::isFileList(argc, argv, fileList);

  if (help)
  {
    appParams.printHelp();
    return false;
  }

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
      if (files)
      {
	appParams = T(fileName, fileList, true);
      }
      else
      {
	appParams = T(fileName, true);
      }
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
