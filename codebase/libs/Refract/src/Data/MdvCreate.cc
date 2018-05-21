#include <Refract/MdvCreate.hh>
#include <Refract/RefractInput.hh>
#include <Refract/RefractConstants.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/str.h>

//-----------------------------------------------------------------------
MdvxField *MdvCreate::createMatchingField(const MdvxField *field,
					   const std::string &name,
					   const std::string &units)
{
  // Create the new field header

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  
  // Create the new vlevel header

  Mdvx::vlevel_header_t vlevel_hdr = field->getVlevelHeader();

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = refract::INVALID;
  field_hdr.missing_data_value = refract::INVALID;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.field_name_long, name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Create the new field
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


//-----------------------------------------------------------------------
MdvxField *
MdvCreate::createMatchingFieldWithData(const MdvxField *field,
					const std::string &field_name,
					const std::string &units)
{
  // Create the field
  MdvxField *fnew = createMatchingField(field, field_name, units);
  if (fnew == 0)
    return 0;
  
  // Set the initial data values
  fl32 *data = (fl32 *)fnew->getVol();
  fl32 *input_data = (fl32 *)field->getVol();
  Mdvx::field_header_t field_hdr = fnew->getFieldHeader();
  for (int i = 0; i < field_hdr.nx*field_hdr.ny; ++i)
    data[i] = input_data[i];
  return fnew;
}


//-----------------------------------------------------------------------
MdvxField *
MdvCreate::createMatchingFieldWithValue(const MdvxField *field,
					 const std::string &field_name,
					 const std::string &units,
					 double value)
{
  // Create the field
  MdvxField *fnew = createMatchingField(field, field_name, units);
  if (fnew == 0)
    return 0;
  
  // Set the initial data values
  fl32 *data = (fl32 *)fnew->getVol();
  Mdvx::field_header_t field_hdr = fnew->getFieldHeader();
  for (int i = 0; i < field_hdr.nx*field_hdr.ny; ++i)
    data[i] = value;
  return fnew;
}

fl32 * MdvCreate::dataPtr(MdvxField *field)
{
  return (fl32 *)field->getVol();
}
  
fl32 * MdvCreate::refPtr(MdvxField &field)
{
  return (fl32 *)field.getVol();
}
