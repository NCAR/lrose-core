/**
 * @file DataSpec.hh 
 * @brief One input or output data item, as indicated by naming and type
 * @class DataSpec
 * @brief One input or output data item, as indicated by naming and type
 *
 */

#ifndef DATA_SPEC_H
#define DATA_SPEC_H
#include <FiltAlgVirtVol/VirtVolParams.hh>
#include <FiltAlgVirtVol/NamePair.hh>

class DataSpec
{
public:

  /**
   * Constructor, pulls values from FiltAlgParams struct
   *
   * @param[in] d  External_data_t  Struct with needed values
   */
  inline DataSpec(const VirtVolParams::External_data_t &d) :
    _name(d.internal_name, d.external_name), _type(d.data_type) {}

  /**
   * Constructor, values passed in individually
   *
   * @param[in] intName  Internal name
   * @param[in] extName  External name
   * @param[in] type  What kind of data it is
   */
  inline DataSpec(const std::string &intName, const std::string &extName,
		  VirtVolParams::Data_t type) :
    _name(intName, extName), _type(type) {}
  
  /**
   * Destructor
   */
  inline virtual ~DataSpec(void) {}

  NamePair _name;                 /**< internal/external names */
  VirtVolParams::Data_t _type;    /**< Type of data, public */

protected:
private:
};

#endif
