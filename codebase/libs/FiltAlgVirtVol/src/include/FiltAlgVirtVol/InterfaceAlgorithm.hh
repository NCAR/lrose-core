/**
 * @file InterfaceAlgorithm.hh
 * @brief static functions for standard application actions.
 * @class InterfaceAlgorithm
 * @brief static functions for standard application actions.
 */
# ifndef    INTERFACE_ALGORITHM_HH
# define    INTERFACE_ALGORITHM_HH

#include <ctime>
#include <string>

class AlgorithmParams;

//------------------------------------------------------------------
class InterfaceAlgorithm
{
public:

  /**
   * default driver initialization steps for apps.
   *
   * @param[in] appName  Name of the app
   * @param[in] p  base class params used by all FiltAlg apps
   * @param[in] cleanup a cleanup function to call on various exit signals.
   *
   * @return true if init was successful
   */
  static bool algInit(const std::string &appName, const AlgorithmParams &p,
			 void cleanup(int));

  /**
   * Inform any monitoring software that the app is terminating.
   */
  static void algFinish(void);

protected:
private:  

};

# endif
