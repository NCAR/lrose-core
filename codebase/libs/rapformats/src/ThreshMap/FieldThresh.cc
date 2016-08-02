/**
 * @file FieldThresh.cc
 */

#include <rapformats/FieldThresh.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

const std::string FieldThresh::_tag = "FldThr";

//-------------------------------------------------------------------------
FieldThresh::FieldThresh(const std::string &xml, const std::string &fname) :
  _ok(true),
  _fieldName("Unknown"),
  _thresh(-99.99)
{
  if (TaXml::readString(xml, "F", _fieldName))
  {
    LOG(ERROR) << "No tag F in string";
    _ok = false;
  }
  if (TaXml::readDouble(xml, "T", _thresh))
  {
    LOG(ERROR) << "No tag T in string";
    _ok = false;
  }
  if (_fieldName != fname)
  {
    LOG(ERROR) << "Names don't match " << _fieldName << " " << fname;
    _ok = false;
  }
}

//-------------------------------------------------------------------------
std::string FieldThresh::toXml(void) const
{
  std::string ret = TaXml::writeString("F", 0, _fieldName);
  ret += TaXml::writeDouble("T", 0, _thresh, "%.10lf");
  return TaXml::writeString(_tag, 0, ret);
}

//-------------------------------------------------------------------------
std::string FieldThresh::sprint(void) const
{
  char buf[1000];
  sprintf(buf, "%s:%05.2f", _fieldName.c_str(), _thresh);
  string s = buf;
  return s;
}

//-------------------------------------------------------------
void FieldThresh::print(void) const
{
  printf("%s[%lf]\n", _fieldName.c_str(), _thresh);
}

//-------------------------------------------------------------
void FieldThresh::printNoNewline(void) const
{
  printf("%s[%lf] ", _fieldName.c_str(), _thresh);
}

//-------------------------------------------------------------
std::string FieldThresh::dataFieldName(int nameChars, int precision) const
{
  char buf[1000];
  switch (precision)
  {
  case 0:
    sprintf(buf, "%s%.0lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  case 1:
    sprintf(buf, "%s%.1lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  case 2:
    sprintf(buf, "%s%.2lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  case 3:
    sprintf(buf, "%s%.3lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  case 4:
    sprintf(buf, "%s%.4lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  case 5:
    sprintf(buf, "%s%.5lf", _fieldName.substr(0,nameChars).c_str(), _thresh);
    break;
  default:
    LOG(ERROR) << "Maximum digits=5, had " << precision;
    sprintf(buf, "ERROR");
    break;
  }
  std::string ret = buf;
  return ret;
}

