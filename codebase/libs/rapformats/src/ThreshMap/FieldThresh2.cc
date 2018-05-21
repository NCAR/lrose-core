/**
 * @file FieldThresh2.cc
 */

#include <rapformats/FieldThresh2.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

const std::string FieldThresh2::_tag2 = "FldThr2";

//-------------------------------------------------------------------------
FieldThresh2::FieldThresh2(const std::string &xml, const std::string &fname) :
   FieldThresh(xml, fname), _thresh2(-999.99)
{
  if (TaXml::readDouble(xml, "T2", _thresh2))
  {
    LOG(ERROR) << "No tag T2 in string";
    _ok = false;
  }
}

//-------------------------------------------------------------------------
std::string FieldThresh2::toXml2(int indent) const
{
  std::string ret = TaXml::writeStartTag(_tag2, indent);
  ret += toXmlContent(indent+1);
  ret += TaXml::writeDouble("T2", indent+1, _thresh2, "%.10lf");
  ret += TaXml::writeEndTag(_tag2, indent);
  return ret;
}

//-------------------------------------------------------------------------
std::string FieldThresh2::sprint2(void) const
{
  char buf[1000];
  sprintf(buf, "%s:[%05.2f,%05.2f]", _fieldName.c_str(), _thresh, _thresh2);
  string s = buf;
  return s;
}

//-------------------------------------------------------------
void FieldThresh2::print2(void) const
{
  printf("%s[%lf,%lf]\n", _fieldName.c_str(), _thresh, _thresh2);
}

//-------------------------------------------------------------
void FieldThresh2::printNoNewline2(void) const
{
  printf("%s[%lf,%lf] ", _fieldName.c_str(), _thresh, _thresh2);
}

//-------------------------------------------------------------
std::string FieldThresh2::dataFieldName2(int nameChars, int precision) const
{
  char buf[1000];
  switch (precision)
  {
  case 0:
    sprintf(buf, "%s%.0lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  case 1:
    sprintf(buf, "%s%.1lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  case 2:
    sprintf(buf, "%s%.2lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  case 3:
    sprintf(buf, "%s%.3lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  case 4:
    sprintf(buf, "%s%.4lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  case 5:
    sprintf(buf, "%s%.5lf", _fieldName.substr(0,nameChars).c_str(), _thresh2);
    break;
  default:
    LOG(ERROR) << "Maximum digits=5, had " << precision;
    sprintf(buf, "ERROR");
    break;
  }
  std::string ret = buf;
  return ret;
}

