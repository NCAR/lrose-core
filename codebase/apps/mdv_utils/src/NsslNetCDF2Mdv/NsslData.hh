/**
 * @file NsslData.hh
 * @brief  Data file, parsed into parts
 * @class NsslData
 * @brief  Data file, parsed into parts
 *
 * Data files are assumed to have this format:
 *     <input_dir>/<field>/yyyymmddd-hhmmss-elev.netcdf
 */

#ifndef NsslData_HH
#define NsslData_HH

#include <string>
class Params;

class NsslData
{
public:

  /**
   * Constructor
   * @param[in] params  The parameters, with input_dir
   * @param[in] path    The full path:
   *                    <input_dir>/<field>/yyyymmdd/yyyymmdd-hhmmss-elev.netcdf
   */
  NsslData(const Params &params, std::string &path);
  
  /**
   * Destructor
   */
  virtual ~NsslData(void);
  
  /**
   * @return true if set
   */
  inline bool ok(void) const {return _ok;}

  /**
   * @return full  path
   */
  inline std::string getPath(void) const {return _fullPath;}

  /**
   * @return field name
   */
  inline std::string getField(void) const {return _field;}

  /**
   * @return time
   */
  inline time_t getTime(void) const {return _time;}

  /**
   * @return elevation
   */
  inline double getElev(void) const {return _elev;}

  /**
   * Adjust the path if need be to prepend input_dir
   * @param[in] input_dir
   *
   * @return true if able to set full path to an existing file
   */
  bool pathAdjust(const std::string &input_dir);

  /**
   * @return true if local = inputs
   * @param[in] name  Field
   * @param[in] time  Time
   * @param[in] elev
   */
  bool isMatch(const std::string &name, const time_t &time, double elev) const;

protected:
private:

  bool _ok;              /**< TRue if well formed */
  std::string _fullPath; /**< The full path */
  std::string _field;    /**< The field name */
  time_t _time;          /**< The time */
  double _elev;          /**< The elevation */

};


#endif
