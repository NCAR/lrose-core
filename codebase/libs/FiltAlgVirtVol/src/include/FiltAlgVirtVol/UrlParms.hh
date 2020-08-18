/**
 * @file UrlParms.hh 
 * @brief Parameters, derived from UrlParams
 * @class UrlParms
 * @brief Parameters, derived from UrlParams
 */

#ifndef URL_PARMS_H
#define URL_PARMS_H

#include <FiltAlgVirtVol/UrlParams.hh>
#include <tdrp/tdrp.h>
#include <vector>
#include <string>

class Algorithm;

//------------------------------------------------------------------
class UrlParms  : public UrlParams
{
public:
  /**
   * Constructor empty.
   */
  UrlParms(void);

  /**
   * Constructor..reads in base class param file
   *
   * @param[in] p  file name
   */
  UrlParms(const std::string &p);

  /**
   * Constructor..sets base class to input
   *
   * @param[in] p  Base class values to use
   */
  UrlParms(const UrlParams &p);

  /**
   * Destructor
   */
  virtual ~UrlParms(void);

  /**
   * @return true if object well formed
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * Set base values to input, then initialize derived values
   * @param[in] v  Base class object
   */
  inline void set(const UrlParams &v)
  {
    *((UrlParams *)this) = v;
    _init();
  }

  /**
   * return true if the url is a database type
   */
  inline bool isDataBase(void) const
  {
    return (url_type == UrlParams::DATABASE);
  }

  /**
   * @return names of all the fields associated with this Url
   */
  inline std::vector<std::string> getNames(void) const {return _dataNames;}

  /**
   * @return true if name input matches a name in local state
   * @param[in] name
   */
  bool nameMatch(const std::string &name) const;

  /**
   * @return a string for a particular URL type 
   * @param[in] t  The URL type
   */
  static std::string sprintUrl(UrlParams::Url_t t);

  /**
   * @return a string for a particular data type
   * @param[in] t  The data type
   */
  static std::string sprintData(UrlParams::Data_t t);

protected:
private:

  bool _ok;      /**< True if object well formed */
  std::vector<std::string> _dataNames;  /**< The names of the fields */


  bool _init(void);

};

#endif
