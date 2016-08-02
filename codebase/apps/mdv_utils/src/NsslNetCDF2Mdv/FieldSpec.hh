/**
 * @file FieldSpec.hh
 * @brief  Struct containing params for a field
 * @class FieldSpec
 * @brief  Struct containing params for a field
 */

#ifndef FieldSpec_HH
#define FieldSpec_HH

#include <string>

class FieldSpec
{
public:

  /**
   * Constructor
   */
  inline FieldSpec(void) {}
  
  /**
   * Destructor
   */
  inline virtual ~FieldSpec(void) {}
  
  std::string input_path;
  std::string field_name;
  bool missing_value_is_global;
  std::string missing_data_value_att_name;
  bool bias_specified;
  std::string bias_att_name;
  std::string scale_att_name;
  std::string units_att_name;
  bool override_missing;
  double missing_data_value;
  bool fix_missing_beams;

protected:
private:

};


#endif
