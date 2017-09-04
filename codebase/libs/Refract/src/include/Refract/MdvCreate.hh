/**
 * @file MdvCreate.hh
 * @class MdvCreate
 */

#ifndef MdvCreate_H
#define MdvCreate_H

#include <dataport/port_types.h>
#include <string>

class MdvxField;

class MdvCreate
{
public:
  /**
   * @return pointer to MdvxField, owned by caller
   * @param[in] field  Pointer to field to match
   * @param[in] name  Name to give field
   * @param[in] units  Units to give field
   */
  static MdvxField *createMatchingField(const MdvxField *field,
					 const std::string &name,
					 const std::string &units);

  /**
   * @return pointer to MdvxField, owned by caller.  Data values same as input
   * @param[in] field  Pointer to field to match
   * @param[in] name  Name to give field
   * @param[in] units  Units to give field
   */
  static MdvxField *createMatchingFieldWithData(const MdvxField *field,
						const std::string &name,
						const std::string &units);
  /**
   * @return pointer to MdvxField, owned by caller.  Data values set to constant
   * @param[in] field  Pointer to field to match
   * @param[in] name  Name to give field
   * @param[in] units  Units to give field
   * @param[in] value  Value to set data to everywhere
   */
  static MdvxField *
  createMatchingFieldWithValue(const MdvxField *field,
			       const std::string &name,
			       const std::string &units,
			       double value);
  /**
   * Interpret volume data as a pointer to float
   * @param[in]  field  MdvxField pointer
   * @return pointer to float
   */
  static fl32 * dataPtr(MdvxField *field);
  
  /**
   * Interpret volume data as a pointer to float
   * @param[in]  field  MdvxField reference
   * @return pointer to float
   */
  static fl32 * refPtr(MdvxField &field);
  
protected:
private:
};

#endif
