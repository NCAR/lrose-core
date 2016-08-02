// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// TaXml.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Simple XML support
//
////////////////////////////////////////////////////////////////

#include <cstdio>
#include <ctime>
#include <cerrno>
#include <iostream>
#include <toolsa/TaXml.hh>
using namespace std;

// set the default indent spaces per level

int TaXml::indentPerLevel = 2;

////////////////////////////////////////////////////////////
// Remove comments from XML buffer.
// Returns string with comments removed.
// This should be done before parsing any buffer for content.

string TaXml::removeComments(const string &xmlBuf)
  
{

  string noCommentBuf;

  // compute start and end tokens
  
  string startTok = "<!--";
  string endTok = "-->";
  
  size_t pos = 0;
  size_t startPos = xmlBuf.find(startTok, pos);
  size_t endPos = 0;

  if (startPos == string::npos) {
    return xmlBuf;
  }
  
  while (startPos != string::npos) {
    
    // find comment start position
  
    startPos = xmlBuf.find(startTok, pos);
    
    // find comment end position
    
    endPos = xmlBuf.find(endTok, startPos);
    if (endPos != string::npos) {
      endPos += endTok.size();
    }
    
    // append non-comments

    string noCommentStr = xmlBuf.substr(pos, startPos - pos);
    noCommentBuf.append(noCommentStr);

    // update position to after comment
    
    pos = endPos;

  }

  return noCommentBuf;

}

///////////////////////////////////////////////////////////
// remove the leading and trailine white space from a string
// Can be used in cases where incoming XML has extra white space
// that prevents parsing using TaXML methods
//
string TaXml::removeSurroundingWhite(const string &valStr)
{
  size_t s0, s1;
  string whitespaces (" \t\f\v\n\r");

  s0 = valStr.find_first_not_of(whitespaces);
  if (s0==string::npos)
  {
    cerr << "WARNING - TaXml::removeSurroundingWhite" << endl;
    cerr << "  All whitespace? :" << valStr << endl;
    return valStr;
  }

  s1=valStr.find_last_not_of(whitespaces);
  if (s1!=string::npos)
    return valStr.substr(s0, s1-s0+1);
  else
  {
    cerr << "WARNING - TaXml::removeSurroundingWhite" << endl;
    cerr << "  All whitespace? :" << valStr << endl;
    return valStr;
  }
}


////////////////////////////////////////////////////////////
// Remove tags from the given tag buffer.
// Returns string with tags removed.
// You must be sure to have a single tag buffer in the string
// (like what is returned by readTagBuf() and readTagBufArray()).

string TaXml::removeTags(const string &tagBuf)
  
{
  size_t stringStart = tagBuf.find(">") + 1;
  size_t stringEnd = tagBuf.rfind("<") - 1;
  
  if (stringStart == string::npos ||
      stringEnd == string::npos ||
      stringStart > stringEnd) {
    return "";
  }
  
  return tagBuf.substr(stringStart, stringEnd - stringStart + 1);
				   
}


//////////////////////////////////////
// read the start tag from the given tag buffer.  The buffer must
// start with the tag.
//
// returns 0 on success, -1 on failure

int TaXml::readStartTag(const string &tagBuf,
			string &startTag)
  
{
  // Make sure that we are starting at the tag.  If not, it is an
  // error.

  if (tagBuf[0] != '<')
    return -1;
  
  // Find the end of the tag.

  size_t endPos = tagBuf.find(">");
  
  if (endPos == string::npos)
    return -1;
  
  startTag = tagBuf.substr(0, endPos + 1);
  
  return 0;
}


//////////////////////////////////////
// read string value, no attributes
//
// returns 0 on success, -1 on failure

int TaXml::readString(const string &xmlBuf,
                      const string &tag,
                      string &val)
  
{
  
  // Get the tag buffer

  string tagBuf;
  
  if (readTagBuf(xmlBuf, tag, tagBuf) != 0) {
    return -1;
  }
  
  // Remove the tags from the tag buffer.

  val = removeTags(tagBuf);
  
  return 0;

}

///////////////////////////////////
// read string value and attributes
//
// returns 0 on success, -1 on failure

int TaXml::readString(const string &xmlBuf,
                      const string &tag,
                      string &val,
                      vector<attribute> &attributes)
  
{

  attributes.clear();
  
  // Get the tag buffer

  string tagBuf;
  
  if (readTagBuf(xmlBuf, tag, tagBuf) != 0) {
    return -1;
  }
  
  // Retrieve the start tag from the tag buffer.  We will pull
  // the attributes from this tag.

  string startTag;
  
  if (readStartTag(tagBuf, startTag) != 0) {
    return -1;
  }
  
  // Remove the tags from the tag buffer.

  val = removeTags(tagBuf);
  
  // Extract the attributes from the start tag

  size_t blankPos = startTag.find(" ");
  
  if (blankPos == string::npos) {
    // There are no attributes so just return
    return 0;
  }
  
  string attrStr = startTag.substr(blankPos + 1,
				   startTag.length() - blankPos - 2);
  
  if (attrStr[attrStr.length()-1] == '/') {
    attrStr = attrStr.substr(0, attrStr.length()-1);
  }
  
  // decode attributes into vector
  
  attrDecode(attrStr, attributes);

  return 0;
  
}

//////////////////////////////
// read array of strings from XML buffer, given a tag.
// One entry in array for each tag found.
// Does not support attributes.
// returns 0 on success, -1 on failure

int TaXml::readStringArray(const string &xmlBuf,
                           const string &tag,
                           vector<string> &valArray)
  
{
  
  valArray.clear();
  
  // compute start and end tokens
  
  string startTok = "<" + tag + ">";
  string endTok = "</" + tag + ">";
  
  size_t startPos = 0;
  size_t endPos = 0;
  
  while (startPos != string::npos && endPos != string::npos) {
    
    // find start position
    
    startPos = xmlBuf.find(startTok, endPos);
    if (startPos == string::npos) {
      break;
    }
    startPos += startTok.size();
    
    // find end position
    
    endPos = xmlBuf.find(endTok, startPos);
    if (endPos == string::npos) {
      break;
    }
    
    string val = xmlBuf.substr(startPos, endPos - startPos);
    valArray.push_back(val);
    
    endPos += endTok.size();

  }

  if (valArray.size() == 0) {
    return -1;
  }

  return 0;

}

///////////////////////////////////
// Read tag buffer.
// Does support attributes.
// Loads buffer with string including start tag and end tag.
// Search starts at searchStart pos.
// As a side effect, if searchEnd is not null it is positioned
// 1 beyond the end of the data for the current tag.
// Returns 0 on success, -1 on failure

int TaXml::readTagBuf(const string &xmlBuf,
                      const string &tag,
                      string &tagBuf,
                      size_t searchStart /* = 0 */,
                      size_t *searchEnd /* = NULL */)
  
{

  size_t startPos, endPos;
  if (findTagLimits(xmlBuf, tag, searchStart, startPos, endPos)) {
    return -1;
  }

  // set buffer
  
  tagBuf = xmlBuf.substr(startPos, endPos - startPos);

  if (searchEnd != NULL) {
    *searchEnd = endPos;
  }

  return 0;

}

//////////////////////////////
// read array of tag buffers
// Does support attributes.
// returns 0 on success, -1 on failure

int TaXml::readTagBufArray(const string &xmlBuf,
                           const string &tag,
                           vector<string> &tagBufArray)
  
{
  
  tagBufArray.clear();

  size_t searchStart = 0;
  size_t searchEnd = 0;

  string tagBuf;
  while (readTagBuf(xmlBuf, tag, tagBuf, searchStart, &searchEnd) == 0) {
    tagBufArray.push_back(tagBuf);
    searchStart = searchEnd;
  }

  if (tagBufArray.size() > 0) {
    return 0;
  } else {
    return -1;
  }

}

///////////////////////////////////
// Find tag limits
// Search starts at searchStart.
// StartPos is set to beginning of start tag.
// Endpos is set 1 beyond the end of the closing tag
//
// Returns 0 on success, -1 on failure

int TaXml::findTagLimits(const string &xmlBuf,
                         const string &tag,
                         size_t searchStart,
                         size_t &startPos,
                         size_t &endPos,
			 const string prefix)
  
{

  int nOpen = 0;
  int nClose = 0;
  size_t searchPos = searchStart;

  // search buffer, looking for tags
  
  while (true) {

    // find next tag

    size_t tagStartPos, tagEndPos;
    tag_type_t tagType;
    if (findNextTag(xmlBuf, tag, searchPos,
                    tagStartPos, tagEndPos, tagType)) {
      return -1;
    }
    
    // we need an open tag to start us off
    
    if (nOpen == 0) {
      switch (tagType) {
        case TAG_BOTH: {
          // single <tag ... />
          startPos = tagStartPos;
          endPos = tagEndPos;
          nOpen++;
          nClose++;
          return 0;
        }
        case TAG_CLOSE: {
          // ignore, since we have not yet found a start tag
          break;
        }
        default: {
          // TAG_OPEN
          startPos = tagStartPos;
          nOpen++;
        }
      }

    } else { // if (nOpen == 0)

      // nOpen > 0
  
      switch (tagType) {
        case TAG_BOTH: {
          // single <tag ... />
          nOpen++;
          nClose++;
          break;
        }
        case TAG_OPEN: {
          nOpen++;
          break;
        }
        default: {
          // TAG_CLOSE
          nClose++;
          // check if we have same number of opens and closes
          if (nOpen == nClose) {
            // we are done
            endPos = tagEndPos;
            return 0;
          }
        }
      }
      
    }  // if (nOpen == 0)

    // set location for next search
    
    searchPos = tagEndPos;
    
  } // while

  return -1;

}

////////////////////////////////////////////////////
// Find next tag
//
// Search starts at searchStart.
// startPos is set to beginning of start tag.
// endpos is set 1 beyond the end of the end tag.
//
// Tag type is set appropriately.
//   TAG_OPEN:   <tag> or <tag .... >
//   TAG_CLOSE:  </tag>
//   TAG_BOTH:   <tag ... />
// 
// Returns 0 on success, -1 on failure

int TaXml::findNextTag(const string &xmlBuf,
                       const string &tag,
                       size_t searchStart,
                       size_t &startPos,
                       size_t &endPos,
                       tag_type_t &tagType)
  
{

  size_t bufSize = xmlBuf.size();
  size_t tagSize = tag.size();
  size_t searchPos = searchStart;
  
  while (true) {

    size_t tPos = xmlBuf.find(tag, searchPos);
    if (tPos == string::npos) {
      // no match
      return -1;
    }

    // move search position forwards over this match

    searchPos += tag.size();

    // =========== check for open tag ============

    if (tPos < 1) {
      // cannot be an XML tag
      continue;
    }

    if (xmlBuf[tPos-1] == '<') {
      
      // open tag
      
      size_t pos2 = tPos + tagSize;
      if (pos2 > bufSize - 1) {
        // no room left
        return -1;
      }

      if (xmlBuf[pos2] == '>') {
        // <tag>
        tagType = TAG_OPEN;
        startPos = tPos - 1;
        endPos = pos2 + 1;
        if (endPos > bufSize - 1) {
          endPos = string::npos;
        }
        return 0;
      }

      if (xmlBuf[pos2] != ' ') {
        // longer tag, not the correct one
        continue;
      }

      // <tag .... > or <tag .... />
      // find closing brace
      
      size_t cbPos = xmlBuf.find(">", pos2 + 1);
      if (cbPos == string::npos) {
        // no closing brace
        return -1;
      }

      if (xmlBuf[cbPos-1] == '/') {
        // <tag .... />
        tagType = TAG_BOTH;
        startPos = tPos - 1;
        endPos = cbPos + 1;
        if (endPos > bufSize - 1) {
          endPos = string::npos;
        }
        return 0;
      }

      // <tag ... >
      
      tagType = TAG_OPEN;
      startPos = tPos - 1;
      endPos = cbPos + 1;
      if (endPos > bufSize - 1) {
        endPos = string::npos;
      }
      return 0;

    } // end of if for open tag
    
    // =========== check for close tag ============

    if (tPos < 2) {
      // cannot be an XML close tag
      continue;
    }
    
    if (xmlBuf[tPos-2] == '<' &&
        xmlBuf[tPos-1] == '/') {
      
      // possible close tag
      
      size_t pos2 = tPos + tagSize;
      if (pos2 > bufSize - 1) {
        // no room left
        return -1;
      }
      
      if (xmlBuf[pos2] == '>') {
        // </tag>
        tagType = TAG_CLOSE;
        startPos = tPos - 2;
        endPos = pos2 + 1;
        if (endPos > bufSize - 1) {
          endPos = string::npos;
        }
        return 0;
      }
      
    }

  } // while
 
  return -1;

}

////////////////////////////////////
// read the tag limits for the buffer
//
// populates the vector of limits for this level

void TaXml::readTagLimits(const string &xmlBuf,
                          size_t searchStart,
                          vector<TagLimits> &limits)

{

  while (true) {

    // find the first available tag, by searching for 
    // <tag followed by space.
    
    size_t open = xmlBuf.find("<", searchStart);
    if (open == string::npos) {
      return;
    }
    size_t close = xmlBuf.find(">", open);
    if (close == string::npos) {
      return;
    }
    size_t space = xmlBuf.find(" ", open);
    
    string tag;
    if (space == string::npos || close < space) {
      tag = xmlBuf.substr(open + 1, close - open - 1);
    } else {
      tag = xmlBuf.substr(open + 1, space - open - 1);
    }
    
    // find the tag limits
    
    size_t startPos, endPos;
    if (findTagLimits(xmlBuf, tag, searchStart, startPos, endPos)) {
      return;
    }
    
    TagLimits limit;
    limit.setTag(tag);
    limit.setStartPosn(startPos);
    limit.setEndPosn(endPos);
    limits.push_back(limit);

    searchStart = endPos;
    
  } // while

}

/////////////////////////////////////////////
// read boolean
// methods return 0 on success, -1 on failure

// boolean from a string

int TaXml::readBoolean(const string &valStr, bool &val)
  
{
  string LvalStr = removeSurroundingWhite(valStr);
  if (LvalStr == "true" || LvalStr == "TRUE") {
    val = true;
  } else if (LvalStr == "false" || LvalStr == "FALSE") {
    val = false;
  } else {
    cerr << "ERROR - TaXml::readBoolean" << endl;
    cerr << "  Cannot decode string into boolean: " << LvalStr << endl;
    return -1;
  }
  return 0;
}

// boolean from xml buffer, given a tag

int TaXml::readBoolean(const string &xmlBuf,
                       const string &tag,
                       bool &val)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr)) {
    return -1;
  }
  if (readBoolean(valStr, val)) {
    return -1;
  }
  return 0;
}

// boolean with attributes from xml buffer, given a tag

int TaXml::readBoolean(const string &xmlBuf,
                       const string &tag,
                       bool &val,
                       vector<attribute> &attributes)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr, attributes)) {
    return -1;
  }
  if (readBoolean(valStr, val)) {
    return -1;
  }
  return 0;
}

/////////////////////////////////////////////
// read int
// methods return 0 on success, -1 on failure

// int from a string

int TaXml::readInt(const string &valStr, int &val)
  
{
  int ival;
  if (sscanf(valStr.c_str(), "%d", &ival) != 1) {
    cerr << "ERROR - TaXml::readInt" << endl;
    cerr << "  Cannot decode string into int: " << valStr << endl;
    return -1;
  }
  val = ival;
  return 0;
}

// int from xml buffer, given a tag

int TaXml::readInt(const string &xmlBuf,
                   const string &tag,
                   int &val)

{
  string valStr;
  if (readString(xmlBuf, tag, valStr)) {
    return -1;
  }
  if (readInt(valStr, val)) {
    return -1;
  }
  return 0;
}

// int with attributes from xml buffer, given a tag

int TaXml::readInt(const string &xmlBuf,
                   const string &tag,
                   int &val,
                   vector<attribute> &attributes)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr, attributes)) {
    return -1;
  }
  if (readInt(valStr, val)) {
    return -1;
  }
  return 0;
}

/////////////////////////////////////////////
// read long
// methods return 0 on success, -1 on failure

// long from a string

int TaXml::readLong(const string &valStr, long &val)
  
{
  long lval;
  if (sscanf(valStr.c_str(), "%ld", &lval) != 1) {
    cerr << "ERROR - TaXml::readLong" << endl;
    cerr << "  Cannot decode string into long: " << valStr << endl;
    return -1;
  }
  val = lval;
  return 0;
}

// long from xml buffer, given a tag

int TaXml::readLong(const string &xmlBuf,
                    const string &tag,
                    long &val)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr)) {
    return -1;
  }
  if (readLong(valStr, val)) {
    return -1;
  }
  return 0;
}

// long with attributes from xml buffer, given a tag

int TaXml::readLong(const string &xmlBuf,
                    const string &tag,
                    long &val,
                    vector<attribute> &attributes)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr, attributes)) {
    return -1;
  }
  if (readLong(valStr, val)) {
    return -1;
  }
  return 0;
}

/////////////////////////////////////////////
// read double
// methods return 0 on success, -1 on failure

// double from a string

int TaXml::readDouble(const string &valStr, double &val)
  
{
  double dval;
  if (sscanf(valStr.c_str(), "%lg", &dval) != 1) {
    cerr << "ERROR - TaXml::readDouble" << endl;
    cerr << "  Cannot decode string into double: " << valStr << endl;
    return -1;
  }
  val = dval;
  return 0;
}

// double from xml buffer, given a tag

int TaXml::readDouble(const string &xmlBuf,
                      const string &tag,
                      double &val)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr)) {
    return -1;
  }
  if (readDouble(valStr, val)) {
    return -1;
  }
  return 0;
}

// double with attributes from xml buffer, given a tag

int TaXml::readDouble(const string &xmlBuf,
                      const string &tag,
                      double &val,
                      vector<attribute> &attributes)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr, attributes)) {
    return -1;
  }
  if (readDouble(valStr, val)) {
    return -1;
  }
  return 0;
}

/////////////////////////////////////////////
// read time
// will decode either yyyy-mm-ddThh:mm:ss or unix time
// in secs since 1970
// methods return 0 on success, -1 on failure

// time from a string

int TaXml::readTime(const string &valStr, time_t &val)
  
{
  int year, month, day, hour, min, sec;
  if (sscanf(valStr.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
             &year, &month, &day, &hour, &min, &sec) == 6) {
    struct tm dt;
    dt.tm_year = year - 1900;
    dt.tm_mon = month - 1;
    dt.tm_mday = day;
    dt.tm_hour = hour;
    dt.tm_min = min;
    dt.tm_sec = sec;
    val = mktime(&dt);
    return 0;
  }
  time_t tval;
  if (sscanf(valStr.c_str(), "%ld", &tval) == 1) {
    val = tval;
    return 0;
  }
  cerr << "ERROR - TaXml::readTime" << endl;
  cerr << "  Cannot decode string into time_t: " << valStr << endl;
  return -1;
}

// time from xml buffer, given a tag

int TaXml::readTime(const string &xmlBuf,
                    const string &tag,
                    time_t &val)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr)) {
    return -1;
  }
  if (readTime(valStr, val)) {
    return -1;
  }
  return 0;
}

// time with attributes from xml buffer, given a tag

int TaXml::readTime(const string &xmlBuf,
                    const string &tag,
                    time_t &val,
                    vector<attribute> &attributes)
  
{
  string valStr;
  if (readString(xmlBuf, tag, valStr, attributes)) {
    return -1;
  }
  if (readTime(valStr, val)) {
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// decode attributes from attribute string into a vector

void TaXml::attrDecode(const string &attrStr,
                       vector<attribute> &attributes)

{

  size_t nameStart = 0;
  while (nameStart < attrStr.size() && nameStart != string::npos) {

    // move past white space before name
    
    char next = attrStr[nameStart];
    if (isspace(next)) {
      nameStart++;
      continue;
    }

    // find '='

    size_t equalsPos = attrStr.find('=', nameStart);
    if (equalsPos == string::npos) {
      return;
    }

    // back up past any white space

    size_t nameEnd = equalsPos - 1;
    for (size_t pos = nameEnd; pos > nameStart; pos--) {
      if (!isspace(attrStr[nameEnd])) {
        nameEnd = pos + 1;
        break;
      }
    }

    // set name

    int nameLen = nameEnd - nameStart;
    if (nameLen < 1) {
      return;
    }
    string name = attrStr.substr(nameStart, nameLen);

    // find first quote

    size_t startQuotePos = attrStr.find_first_of("\'\"", equalsPos + 1);
    if (startQuotePos == string::npos) {
      return;
    }

    // find next quote

    char quote = attrStr[startQuotePos];
    size_t endQuotePos = attrStr.find(quote, startQuotePos + 1);
    if (endQuotePos == string::npos) {
      return;
    }

    // set value

    int valueLen = endQuotePos - startQuotePos - 1;
    if (valueLen < 1) {
      return;
    }
    string val = attrStr.substr(startQuotePos + 1, valueLen);

    // add attribute

    attributes.push_back(attribute(name, val));

    // jump forward

    nameStart = endQuotePos + 1;

  } // while

}

//////////////////////////////////////////
// read attributes of various types
// return 0 on success, -1 on failure

// string

int TaXml::readStringAttr(const vector<attribute> &attributes,
                          const string &name,
                          string &val)

{

  for (int ii = 0; ii < (int) attributes.size(); ii++) {
    if (name == attributes[ii].getName()) {
      val = attributes[ii].getVal();
      return 0;
    }
  } // ii

  return -1;

}

// boolean

int TaXml::readBooleanAttr(const vector<attribute> &attributes,
                           const string &name,
                           bool &val)

{

  string valStr;
  if (readStringAttr(attributes, name, valStr)) {
    return -1;
  }
  for (int ii = 0; ii < (int) valStr.size(); ii++) {
    valStr[ii] = tolower(valStr[ii]);
  }
    
  if (valStr == "true") {
    val = true;
    return 0;
  } else if (valStr == "false") {
    val = false;
    return 0;
  }

  return -1;

}

// int

int TaXml::readIntAttr(const vector<attribute> &attributes,
                       const string &name,
                       int &val)
  
{

  string valStr;
  if (readStringAttr(attributes, name, valStr)) {
    return -1;
  }
  for (int ii = 0; ii < (int) valStr.size(); ii++) {
    valStr[ii] = tolower(valStr[ii]);
  }

  int ival;
  if (sscanf(valStr.c_str(), "%d", &ival) == 1) {
    val = ival;
    return 0;
  }

  return -1;

}

// long

int TaXml::readLongAttr(const vector<attribute> &attributes,
                        const string &name,
                        long &val)

{

  string valStr;
  if (readStringAttr(attributes, name, valStr)) {
    return -1;
  }
  for (int ii = 0; ii < (int) valStr.size(); ii++) {
    valStr[ii] = tolower(valStr[ii]);
  }

  long lval;
  if (sscanf(valStr.c_str(), "%ld", &lval) == 1) {
    val = lval;
    return 0;
  }

  return -1;

}

// double

int TaXml::readDoubleAttr(const vector<attribute> &attributes,
                          const string &name,
                          double &val)

{

  string valStr;
  if (readStringAttr(attributes, name, valStr)) {
    return -1;
  }
  for (int ii = 0; ii < (int) valStr.size(); ii++) {
    valStr[ii] = tolower(valStr[ii]);
  }

  double dval;
  if (sscanf(valStr.c_str(), "%lg", &dval) == 1) {
    val = dval;
    return 0;
  }

  return -1;

}

// time - represented as yyyy-mm-ddThh:mm:ss

int TaXml::readTimeAttr(const vector<attribute> &attributes,
                        const string &name,
                        time_t &val)

{

  string valStr;
  if (readStringAttr(attributes, name, valStr)) {
    return -1;
  }

  for (int ii = 0; ii < (int) valStr.size(); ii++) {
    valStr[ii] = tolower(valStr[ii]);
  }

  // xml format

  int year, month, day, hour, min, sec;
  if (sscanf(valStr.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
             &year, &month, &day, &hour, &min, &sec) == 6) {
    struct tm dt;
    dt.tm_year = year - 1900;
    dt.tm_mon = month - 1;
    dt.tm_mday = day;
    dt.tm_hour = hour;
    dt.tm_min = min;
    dt.tm_sec = sec;
    val = mktime(&dt);
    return 0;
  }

  // unix time format

  time_t tval;
  if (sscanf(valStr.c_str(), "%ld", &tval) == 1) {
    val = tval;
    return 0;
  }

  cerr << "ERROR - TaXml::readUtime" << endl;
  cerr << "  Cannot decode string into time_t: " << valStr << endl;
  return -1;

}

///////////////////////////////
// write a tag at a given level
//
// An indent of 2 spaces is added for each level

string TaXml::writeStartTag(const string &tag, int level)
  
{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">\n";

  return str;

}

// write an end tag

string TaXml::writeEndTag(const string &tag, int level)
  
{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}

/////////////////////////////////////////
// write start tag with attributes
//
// New line after is optional.

string TaXml::writeStartTag(const string &tag,
                            int level,
                            const vector<attribute> &attrs,
                            bool addNewLine)
  
{
  
  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;

  for (int ii = 0; ii < (int) attrs.size(); ii++) {
    str += " ";
    str += attrs[ii].getName();
    str += "=\"";
    str += attrs[ii].getVal();
    str += "\"";
  }

  str += ">";

  if (addNewLine) {
    str += "\n";
  }

  return str;

}

/////////////////////////////////////////
// write tag with attributes
// tag is closed

string TaXml::writeTagClosed(const string &tag,
                             int level,
                             const vector<attribute> &attrs)
  
{
  
  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;

  for (int ii = 0; ii < (int) attrs.size(); ii++) {
    str += " ";
    str += attrs[ii].getName();
    str += "=\"";
    str += attrs[ii].getVal();
    str += "\"";
  }

  str += " />\n";

  return str;

}

/////////////////////////////////////////////////////////////////
// Write a value, without tags, to a string.
// If a format arg is available, you may specify the sprintf format.
// Otherwise %d, %ld or %g will be used as appropriate.
// Returns a string with the value written to it.

string TaXml::writeString(const string &val)

{
  return val;
}
  
string TaXml::writeBoolean(bool val)

{
  string str;
  if (val) {
    str = "true";
  } else {
    str = "false";
  }
  return str;
}
  
string TaXml::writeInt(int val, const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%d", val);
  } else {
    sprintf(buf, format, val);
  }
  string str = buf;
  return str;
}
  
string TaXml::writeLong(long int val, const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%ld", val);
  } else {
    sprintf(buf, format, val);
  }
  string str = buf;
  return str;
}

string TaXml::writeDouble(double val, const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%g", val);
  } else {
    sprintf(buf, format, val);
  }
  string str = buf;
  return str;
}
  
// write unix time tag

string TaXml::writeUtime(time_t val, const char *format /* = NULL */)
  
{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%ld", (long int) val);
  } else {
    sprintf(buf, format, (long int) val);
  }
  string str = buf;
  return str;
}
  
// write time as yyyy-mm-ddThh:mm:ss

string TaXml::writeTime(time_t val)
  
{
  char buf[64];
  struct tm dt;
  gmtime_r(&val, &dt);
  sprintf(buf, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d",
          dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday,
          dt.tm_hour, dt.tm_min, dt.tm_sec);
  string str = buf;
  return str;
}

/////////////////////////////////////////////////////////////////
// Write a value, with tags, to a string.
// If a format arg is available, you may specify the sprintf format.
// Otherwise %d, %ld or %g will be used as appropriate.
// Returns a string with the value written to it.

// write string tag

string TaXml::writeString(const string &tag,
                          int level,
                          const string &val)

{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += val;
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
// write boolean tag

string TaXml::writeBoolean(const string &tag,
                           int level,
                           bool val)

{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeBoolean(val);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
// write int tag

string TaXml::writeInt(const string &tag, int level,
                       int val, const char *format /* = NULL */)

{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeInt(val, format);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
// write long tag

string TaXml::writeLong(const string &tag, int level,
                        long int val, const char *format /* = NULL */)

{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeLong(val, format);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}

// write double tag

string TaXml::writeDouble(const string &tag, int level,
                          double val, const char *format /* = NULL */)

{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeDouble(val, format);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
// write unix time tag

string TaXml::writeUtime(const string &tag, int level,
                         time_t val, const char *format /* = NULL */)
  
{

  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeUtime(val, format);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
// write time as yyyy-mm-ddThh:mm:ss

string TaXml::writeTime(const string &tag, int level, time_t val)
  
{
  
  string str;
  for (int ii = 0; ii < level * indentPerLevel; ii++) {
    str += " ";
  }
  str += "<";
  str += tag;
  str += ">";
  str += writeTime(val);
  str += "</";
  str += tag;
  str += ">\n";

  return str;

}
  
/////////////////////////////////////////////////////////////////
// Write a value, with tags and attributes, to a string.
// Returns a string with the value written to it.

string TaXml::writeString(const string &tag,
                          int level,
                          const vector<attribute> &attrs,
                          const string &val)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeString(val);
  str += writeEndTag(tag, 0);
  return str;

}
  
string TaXml::writeBoolean(const string &tag,
                           int level,
                           const vector<attribute> &attrs,
                           bool &val)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeBoolean(val);
  str += writeEndTag(tag, 0);
  return str;

}
  
string TaXml::writeInt(const string &tag,
                       int level,
                       const vector<attribute> &attrs,
                       int val,
                       const char *format /* = NULL */)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeInt(val, format);
  str += writeEndTag(tag, 0);
  return str;

}

string TaXml::writeLong(const string &tag,
                       int level,
                        const vector<attribute> &attrs,
                        long val,
                        const char *format /* = NULL */)
  
{
  
  string str = writeStartTag(tag, level, attrs, false);
  str += writeLong(val, format);
  str += writeEndTag(tag, 0);
  return str;

}

string TaXml::writeDouble(const string &tag,
                          int level,
                          const vector<attribute> &attrs,
                          double val,
                          const char *format /* = NULL */)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeDouble(val, format);
  str += writeEndTag(tag, 0);
  return str;

}

string TaXml::writeUtime(const string &tag,
                         int level,
                         const vector<attribute> &attrs,
                         time_t val,
                         const char *format /* = NULL */)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeUtime(val, format);
  str += writeEndTag(tag, 0);
  return str;

}

string TaXml::writeTime(const string &tag,
                        int level,
                        const vector<attribute> &attrs,
                        time_t val)

{

  string str = writeStartTag(tag, level, attrs, false);
  str += writeTime(val);
  str += writeEndTag(tag, 0);
  return str;

}

//////////////////////////////////////////////////////////////////
// add attributes of various types to an attribute vector

// string

void TaXml::addStringAttr(const string &name,
                          const string &val,
                          vector<attribute> &attributes)
  
{
  attribute attr(name, val);
  attributes.push_back(attr);
}

// boolean

void TaXml::addBooleanAttr(const string &name,
                           bool val,
                           vector<attribute> &attributes)

{
  string valStr;
  if (val) {
    valStr = "true";
  } else {
    valStr = "false";
  }
  addStringAttr(name, valStr, attributes);
}

// int

void TaXml::addIntAttr(const string &name,
                       int val,
                       vector<attribute> &attributes,
		       const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%d", val);
  } else {
    sprintf(buf, format, val);
  }
  string valStr = buf;
  addStringAttr(name, valStr, attributes);
}

// long

void TaXml::addLongAttr(const string &name,
                        long val,
                        vector<attribute> &attributes,
			const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%ld", val);
  } else {
    sprintf(buf, format, val);
  }
  string valStr = buf;
  addStringAttr(name, valStr, attributes);
}

// double

void TaXml::addDoubleAttr(const string &name,
                          double val,
                          vector<attribute> &attributes,
			  const char *format /* = NULL */)

{
  char buf[1024];
  if (format == NULL) {
    sprintf(buf, "%g", val);
  } else {
    sprintf(buf, format, val);
  }
  string valStr = buf;
  addStringAttr(name, valStr, attributes);
}

// time - represented as yyyy-mm-ddThh:mm:ss

void TaXml::addTimeAttr(const string &name,
                        time_t val,
                        vector<attribute> &attributes)

{
  struct tm dt;
  gmtime_r(&val, &dt);
  char str[64];
  sprintf(str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d",
          dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday,
          dt.tm_hour, dt.tm_min, dt.tm_sec);
  string valStr = str;
  addStringAttr(name, valStr, attributes);
}

// time - represented as unix time long int

void TaXml::addUtimeAttr(const string &name,
                         time_t val,
                         vector<attribute> &attributes)

{
  char str[64];
  sprintf(str, "%ld", val);
  string valStr = str;
  addStringAttr(name, valStr, attributes);
}

//////////////////////////////////////////////////////////////////
// set attributes of various types to an attribute vector
// These clear the attribute vector and then add a single attribute

// string

void TaXml::setStringAttr(const string &name,
                          const string &val,
                          vector<attribute> &attributes)
  
{
  attributes.clear();
  addStringAttr(name, val, attributes);
}

// boolean

void TaXml::setBooleanAttr(const string &name,
                           bool val,
                           vector<attribute> &attributes)

{
  attributes.clear();
  addBooleanAttr(name, val, attributes);
}

// int

void TaXml::setIntAttr(const string &name,
                       int val,
                       vector<attribute> &attributes,
		       const char *format /* = NULL */)

{
  attributes.clear();
  addIntAttr(name, val, attributes, format);
}

// long

void TaXml::setLongAttr(const string &name,
                        long val,
                        vector<attribute> &attributes,
			const char *format /* = NULL */)

{
  attributes.clear();
  addLongAttr(name, val, attributes, format);
}

// double

void TaXml::setDoubleAttr(const string &name,
                          double val,
                          vector<attribute> &attributes,
			  const char *format /* = NULL */)

{
  attributes.clear();
  addDoubleAttr(name, val, attributes, format);
}

// time - represented as yyyy-mm-ddThh:mm:ss

void TaXml::setTimeAttr(const string &name,
                        time_t val,
                        vector<attribute> &attributes)

{
  attributes.clear();
  addTimeAttr(name, val, attributes);
}

// time - represented as unix time long int

void TaXml::setUtimeAttr(const string &name,
                         time_t val,
                         vector<attribute> &attributes)

{
  attributes.clear();
  addUtimeAttr(name, val, attributes);
}


