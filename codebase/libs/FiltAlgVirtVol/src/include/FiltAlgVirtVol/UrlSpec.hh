/**
 * @file UrlSpec.hh
 * @brief One input or output location, specified by location, type of data,
 * and a set of data items
 *
 * @class UrlSpec
 * @brief One input or output location, specified by location, type of data,
 * and a set of data items
 */

#ifndef FILT_ALG_URL_H
#define FILT_ALG_URL_H

#include <FiltAlgVirtVol/VirtVolParams.hh>
#include <FiltAlgVirtVol/DataSpec.hh>
#include <vector>

class InputHandler;

class UrlSpec
{
public:

  /**
   * Constructor, values pulled from params struct
   *
   * @param[in] d  The struct with needed values
   *
   * Data items are added using add()
   */
  UrlSpec(const VirtVolParams::External_data_t &d);

  /**
   * Constructor, values passed in
   *
   * @param[in] url  Where
   * @param[in] type  What
   * 
   * Data items are added using add()
   */
  UrlSpec(const std::string &url, VirtVolParams::Url_t type);

  /**
   * Destructor
   */
  virtual ~UrlSpec(void);

  /**
   * @return true if input URL (location) equals local
   *
   * @param[in] url    The string with URL
   */
  inline bool urlEquals(const std::string &url) const {return _url == url; }

  /**
   * Add one data item to local state
   *
   * @param[in] d   Data to add
   */
  bool add(const DataSpec &d);

  /**
   * @return true if the input name is one of the internal names for this Url
   * @param[in] name  The name
   */
  bool internalNameMatch(const std::string &name) const;

  /**
   * @return true if the input name is one of the external names for this Url
   * @param[in] name  The name
   */
  bool externalNameMatch(const std::string &name) const;

  /**
   * Convert from external to internal names
   * @param[in] externalName  Input 
   * @param[in] internalName  Output
   *
   * @return true if converted, false if externalName not found
   */
   bool external2Internal(const std::string externalName,
   			 std::string &internalName) const;

  /**
   * Convert from internal to external names
   * @param[in] internalName  Input
   * @param[in] externalName  Output 
   *
   * @return true if converted, false if internalName not found
   */
  bool internal2External(const std::string internalName,
  			 std::string &externalName) const;

  /**
   * @return all the external names that are for gridded data
   */
  std::vector<std::string> externalFieldNames(void) const;

  /**
   * @return all the external names that are for non-gridded value data
   */
  std::vector<std::string> externalValueNames(void) const;

  /**
   * @return all the internal names that are for gridded data
   */
  std::vector<std::string> internalFieldNames(void) const;

  /**
   * @return all the internal names that are for non-gridded value data
   */
  std::vector<std::string> internalValueNames(void) const;
  
  /**
   * @return all the internal/external name pairs for gridded data
   */
  std::vector<NamePair> fieldNames(void) const;

  /**
   * @return all the internal/external name pairs for non-gridded value data
   */
  std::vector<NamePair> valueNames(void) const;

  /**
   * @return true if this is database data 
   */
  inline bool isDataBase(void) const {return _type == VirtVolParams::DATABASE;}

  std::string _url;                 /**< Thelocation */
  VirtVolParams::Url_t _type;       /**< The type of data at the location */
  std::vector<DataSpec> _data; /**< Data associated with this Url */

protected:
private:

  /**
   * @return true if input data item would be allowed at the URL, based on type
   *
   * @param[in] d  Data item whose type to compare to local _type
   *
   * If the local type is VIRTUAL_VOLUME, than any kind of data is ok,
   * otherwise GRID data is not allowed
   */
  bool _allowed(const DataSpec &d) const;
};

#endif
