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
/////////////////////////////////////////////////////////////
// RadxXml.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Simple XML support for reading and writing XML
//
// No IO actually occurs. All reads/writes are to/from strings.
// These strings are referred to as buffers in this class.
//
////////////////////////////////////////////////////////////////
//
// Reading:
//
// (a) External to this class, Load xml to be decoded into a
//     string buffer.
//
// (b) Run remove comments to generate a buffer stripped of comments.
//
// (c) Break the xml into sub-buffers using:
//        readString()
//        readStringArray()
//        readTagBuf()
//        readTagBufArray()
//
//  These sub-buffers should contain parts of the original buffer
//  for each complex tag (or element).
//
//  (d) From the sub-buffers, read individual elements using:
//        readString()
//        readBoolean()
//        readInt()
//        readLong()
//        readDouble()
//        readTime()
//
//  (e) If attributes are present, use the versions of these
//      methods which include an attribute vector. Then use
//      the readXXXAttr() methods to retrieve the attributes.
//
////////////////////////////////////////////////////////////////
//
// Writing:
//
// (a) Prepare an empty string buffer.
//
// (b) Use writeStartTag() to start a tags section.
//
// (c) Use the writeXXXX() methods to add elements, with or
//     without attributes.
//
// (d) Close a tag with writeEndTag().
//
// (e) Use writeTagClosed() to write a stand-alone tag with no
//     elements, only attributes.
//
////////////////////////////////////////////////////////////////

#ifndef RadxXml_HH
#define RadxXml_HH

#include <ctime>
#include <string>
#include <vector>
#include <Radx/RadxTime.hh>
using namespace std;

class RadxXml {

public:

  // tag limits, for reading efficiently

  class TagLimits {
  public:
    inline const string &getTag() const { return _tag; }
    inline size_t getStartPosn() const { return _startPosn; }
    inline size_t getEndPosn() const { return _endPosn; }
    inline void setTag(const string &tag) { _tag = tag; }
    inline void setStartPosn(const size_t start) { _startPosn = start; }
    inline void setEndPosn(const size_t end) { _endPosn = end; }
  private:
    string _tag;
    size_t _startPosn; // tag start in buffer
    size_t _endPosn;   // one past tag end in buffer
  };

  // attributes

  class attribute {
  public:
    attribute();
    attribute(const string &name,
              const string &val) :
            _name(name), _val(val) {}
    inline const string &getName() const { return _name; }
    inline const string &getVal() const { return _val; }
    inline void setName(const string &name) { _name = name; }
    inline void setVal(const string &val) { _val = val; }
  private:
    string _name;
    string _val;
  };

  // tag types

  typedef enum {
    TAG_OPEN,    // <tag> or <tag .... >
    TAG_CLOSE,   // </tag>
    TAG_BOTH     // <tag ... />
 } tag_type_t;
  
  ////////////////////////////////////////////////////////////
  // Remove comments from XML buffer.
  // Returns string with comments removed.
  // This should be done before parsing any buffer for content.

  static string removeComments(const string &xmlBuf);
  
  ///////////////////////////////////////////////////////////
  // remove the leading and trailine white space from a string
  // Can be used in cases where incoming XML has extra white space
  // that prevents parsing using TaXML methods
  //
  static string removeSurroundingWhite(const string &valStr);

  // Remove tags from the given tag buffer.
  // Returns string with tags removed.
  // You must be sure to have a single tag buffer in the string
  // (like what is returned by readTagBuf() and readTagBufArray()).

  static string removeTags(const string &tagBuf);
  
  // read the start tag from the given tag buffer.  The buffer must
  // start with the tag.
  //
  // returns 0 on success, -1 on failure

  static int readStartTag(const string &tagBuf,
			  string &startTag);
  
  // Read string value from XML buffer, given a tag.
  // No attributes.
  // Returns 0 on success, -1 on failure
  
  static int readString(const string &xmlBuf,
                        const string &tag,
                        string &val);
  
  // Read string value and attributes.
  // returns 0 on success, -1 on failure
  
  static int readString(const string &xmlBuf,
                        const string &tag,
                        string &val,
                        vector<attribute> &attributes);

  // read array of strings from XML buffer, given a tag.
  // One entry in array for each tag found.
  // Does not support attributes.
  // returns 0 on success, -1 on failure

  static int readStringArray(const string &xmlBuf,
                             const string &tag,
                             vector<string> &valArray);
  
  // Read tag buffer - buffer includes tags.
  // Does support attributes.
  // Loads buffer with string including start tag and end tag.
  // Search starts at searchStart pos.
  // As a side effect, if searchEnd is not null it is positioned
  // 1 beyond the end of the data for the current tag.
  // Returns 0 on success, -1 on failure
  
  static int readTagBuf(const string &xmlBuf,
                        const string &tag,
                        string &tagBuf,
                        size_t searchStart = 0,
                        size_t *searchEnd = NULL);
  
  // read array of tag buffers - buffers include tags.
  // Does support attributes.
  // returns 0 on success, -1 on failure
  
  static int readTagBufArray(const string &xmlBuf,
                             const string &tag,
                             vector<string> &tagBufArray);
  
  // Find tag limits
  // Search starts at searchStart.
  // StartPos is set to beginning of start tag.
  // Endpos is set 1 beyond the end of the closing tag
  //
  // Returns 0 on success, -1 on failure
  
  static int findTagLimits(const string &xmlBuf,
                           const string &tag,
                           size_t searchStart,
                           size_t &startPos,
                           size_t &endPos,
			   const string prefix = "");
  
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
  
  static int findNextTag(const string &xmlBuf,
                         const string &tag,
                         size_t searchStart,
                         size_t &startPos,
                         size_t &endPos,
                         tag_type_t &tagType);
  
  // read the tag tree for the buffer
  // populates the tree with information about tags found
  
  static void readTagLimits(const string &xmlBuf,
                            size_t searchStart,
                            vector<TagLimits> &limits);
  
  /////////////////////////////////////////////
  // read boolean
  // methods return 0 on success, -1 on failure
  
  // boolean from a string
  
  static int readBoolean(const string &valStr, bool &val);
  
  // boolean from xml buffer, given a tag
  
  static int readBoolean(const string &xmlBuf,
                         const string &tag,
                         bool &val);
  
  // boolean with attributes from xml buffer, given a tag
  
  static int readBoolean(const string &xmlBuf,
                         const string &tag,
                         bool &val,
                         vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read int
  // methods return 0 on success, -1 on failure
  
  // int from a string
  
  static int readInt(const string &valStr, int &val);
  
  // int from xml buffer, given a tag
  
  static int readInt(const string &xmlBuf,
                     const string &tag,
                     int &val);
  
  // int with attributes from xml buffer, given a tag
  
  static int readInt(const string &xmlBuf,
                     const string &tag,
                     int &val,
                     vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read long
  // methods return 0 on success, -1 on failure
  
  // long from a string
  
  static int readLong(const string &valStr, long &val);
  
  // long from xml buffer, given a tag
  
  static int readLong(const string &xmlBuf,
                      const string &tag,
                      long &val);
  
  // long with attributes from xml buffer, given a tag
  
  static int readLong(const string &xmlBuf,
                      const string &tag,
                      long &val,
                      vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read double
  // methods return 0 on success, -1 on failure
  
  // double from a string
  
  static int readDouble(const string &valStr, double &val);
  
  // double from xml buffer, given a tag
  
  static int readDouble(const string &xmlBuf,
                        const string &tag,
                        double &val);
  
  // double with attributes from xml buffer, given a tag
  
  static int readDouble(const string &xmlBuf,
                        const string &tag,
                        double &val,
                        vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read float
  // methods return 0 on success, -1 on failure
  
  // float from a string
  
  static int readFloat(const string &valStr, float &val);
  
  // float from xml buffer, given a tag
  
  static int readFloat(const string &xmlBuf,
                       const string &tag,
                       float &val);
  
  // float with attributes from xml buffer, given a tag
  
  static int readFloat(const string &xmlBuf,
                       const string &tag,
                       float &val,
                       vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read time from a string
  // fill out time_t &val
  // will decode either yyyy-mm-ddThh:mm:ss or
  // unix time in secs since 1970
  // returns 0 on success, -1 on failure
  
  // time from a string
  
  static int readTime(const string &valStr, time_t &val);
  
  // time from xml buffer, given a tag
  
  static int readTime(const string &xmlBuf,
                      const string &tag,
                      time_t &val);
  
  // time with attributes from xml buffer, given a tag
  
  static int readTime(const string &xmlBuf,
                      const string &tag,
                      time_t &val,
                      vector<attribute> &attributes);
  
  /////////////////////////////////////////////
  // read time
  // fill out RadxTime &val
  // will decode:
  //   yyyy-mm-ddThh:mm:ss or
  //   yyyy-mm-ddThh:mm:ss.frac or
  //   unix time in secs since 1970
  // returns 0 on success, -1 on failure
  
  // time from a string
  
  static int readTime(const string &valStr, RadxTime &val);
    
  // time from xml buffer, given a tag
  
  static int readTime(const string &xmlBuf,
                      const string &tag,
                      RadxTime &val);
  
  // time with attributes from xml buffer, given a tag
  
  static int readTime(const string &xmlBuf,
                      const string &tag,
                      RadxTime &val,
                      vector<attribute> &attributes);
  
  ////////////////////////////////////////////////////////////////
  // decode attributes from attribute string into a vector
  
  static void attrDecode(const string &attrStr,
                         vector<attribute> &attributes);
  
  // read attributes of various types
  // return 0 on success, -1 on failure
  
  // string
  
  static int readStringAttr(const vector<attribute> &attributes,
                            const string &name,
                            string &val);
  
  // boolean
  
  static int readBooleanAttr(const vector<attribute> &attributes,
                             const string &name,
                             bool &val);
  
  // int
  
  static int readIntAttr(const vector<attribute> &attributes,
                         const string &name,
                         int &val);
  
  // long
  
  static int readLongAttr(const vector<attribute> &attributes,
                          const string &name,
                          long &val);
  
  // double
  
  static int readDoubleAttr(const vector<attribute> &attributes,
                            const string &name,
                            double &val);
  
  // time
  
  static int readTimeAttr(const vector<attribute> &attributes,
                          const string &name,
                          time_t &val);

  //////////////////////////////////////////////////////////
  // Write methods fill strings with XML text for one line
  // An indent of 2 spaces is the default added for each level.
  // The default may be overridden with setIndentPerLevel()

  static int indentPerLevel;
  static void setIndentPerLevel(int n) { indentPerLevel = n; }

  static string writeStartTag(const string &tag, int level);
  static string writeEndTag(const string &tag, int level);

  // Write start tage with attributes
  // New line is appended if requested.
  
  static string writeStartTag(const string &tag,
                              int level,
                              const vector<attribute> &attrs,
                              bool addNewLine);

  // write tag with attributes, close tag
  
  static string writeTagClosed(const string &tag,
                               int level,
                               const vector<attribute> &attrs);

  /////////////////////////////////////////////////////////////////
  // Write a value, without tags, to a string.
  // If a format arg is available, you may specify the sprintf format.
  // Otherwise %d, %ld or %g will be used as appropriate.
  // Returns a string with the value written to it.

  static string writeString(const string &val);
  static string writeBoolean(bool val);
  static string writeInt(int val, const char *format = NULL);
  static string writeLong(long int val, const char *format = NULL);
  static string writeDouble(double val, const char *format = NULL);

  // unix time

  static string writeUtime(time_t val, const char *format = NULL);

  // time as yyyy-mm-ddThh:mm:ss
  
  static string writeTime(time_t val);

  /////////////////////////////////////////////////////////////////
  // Write a value, with tags, to a string.
  // If a format arg is available, you may specify the sprintf format.
  // Otherwise %d, %ld or %g will be used as appropriate.
  // Returns a string with the value written to it.

  static string writeString(const string &tag, int level,
                            const string &val);

  static string writeBoolean(const string &tag, int level,
                             bool val);

  static string writeInt(const string &tag, int level,
                         int val, const char *format = NULL);
  
  static string writeLong(const string &tag, int level,
                          long int val, const char *format = NULL);
  
  static string writeDouble(const string &tag, int level,
                            double val, const char *format = NULL);
  
  // unix time

  static string writeUtime(const string &tag, int level,
                           time_t val, const char *format = NULL);

  // write time as yyyy-mm-ddThh:mm:ss

  static string writeTime(const string &tag, int level,
                          time_t val);
  
  /////////////////////////////////////////////////////////////////
  // Write a value, with tags and attributes, to a string.
  // Returns a string with the value written to it.

  static string writeString(const string &tag,
                            int level,
                            const vector<attribute> &attrs,
                            const string &val);

  static string writeBoolean(const string &tag,
                             int level,
                             const vector<attribute> &attrs,
                             bool &val);

  static string writeInt(const string &tag,
                         int level,
                         const vector<attribute> &attrs,
                         int val,
                         const char *format = NULL);

  static string writeLong(const string &tag,
                          int level,
                          const vector<attribute> &attrs,
                          long val,
                          const char *format = NULL);
  
  static string writeDouble(const string &tag,
                            int level,
                            const vector<attribute> &attrs,
                            double val,
                            const char *format = NULL);

  // unix time
  
  static string writeUtime(const string &tag,
                           int level,
                           const vector<attribute> &attrs,
                           time_t val,
                           const char *format = NULL);
  
  // write time as yyyy-mm-ddThh:mm:ss
  
  static string writeTime(const string &tag,
                          int level,
                          const vector<attribute> &attrs,
                          time_t val);

  //////////////////////////////////////////////////////////////////
  // add attribute to an attribute vector
  // The set methods clear the attribute vector and then add a
  // single attribute

  // string

  static void addStringAttr(const string &name,
                            const string &val,
                            vector<attribute> &attributes);

  static void setStringAttr(const string &name,
                            const string &val,
                            vector<attribute> &attributes);

  // boolean

  static void addBooleanAttr(const string &name,
                             bool val,
                             vector<attribute> &attributes);

  static void setBooleanAttr(const string &name,
                             bool val,
                             vector<attribute> &attributes);

  // int

  static void addIntAttr(const string &name,
                         int val,
                         vector<attribute> &attributes,
			 const char *format = NULL);

  static void setIntAttr(const string &name,
                         int val,
                         vector<attribute> &attributes,
			 const char *format = NULL);

  // long

  static void addLongAttr(const string &name,
                          long val,
                          vector<attribute> &attributes,
			  const char *format = NULL);
  
  static void setLongAttr(const string &name,
                          long val,
                          vector<attribute> &attributes,
			  const char *format = NULL);
  
  // double

  static void addDoubleAttr(const string &name,
                            double val,
                            vector<attribute> &attributes,
                            const char *format = NULL);
  
  static void setDoubleAttr(const string &name,
                            double val,
                            vector<attribute> &attributes,
                            const char *format = NULL);
  
  // time - represented as yyyy-mm-ddThh:mm:ss
  
  static void addTimeAttr(const string &name,
                          time_t val,
                          vector<attribute> &attributes);
  
  static void setTimeAttr(const string &name,
                          time_t val,
                          vector<attribute> &attributes);
  
  // time - represented as unix time long int
  
  static void addUtimeAttr(const string &name,
                           time_t val,
                           vector<attribute> &attributes);

  static void setUtimeAttr(const string &name,
                           time_t val,
                           vector<attribute> &attributes);

protected:
private:

};

#endif
