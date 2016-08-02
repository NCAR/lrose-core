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
/**
 * @file Attributes.hh
 * @brief Any number of generic named double and int values
 *
 * @class Attributes
 * @brief Any number of generic named double and int values
 */
# ifndef    ATTRIBUTES_HH
# define    ATTRIBUTES_HH

#include <map>
#include <string>
#include <vector>

class Attributes
{

public:

  /**
   * Constructor, no attributes in place
   */
  Attributes();

  /**
   * copy constructor
   *
   * @param[in] l
   */
  Attributes(const Attributes &l);

  /**
   * Construct the average or maximum of attributes from all inputs
   *
   * @param[in] a    Attributes to average together or find maximum of
   * @param[in] maxNames  Names of those attributes for which the maximum
   *                      should be taken, all other named attributes get
   *                      averaged.
   */
  Attributes(const std::vector<Attributes> &a,
	     const std::vector<std::string> &maxNames);

  /**
   * Destructor
   */
  virtual ~Attributes();
 
  /**
   * operator= method
   *
   * @param[in] a
   */
  Attributes & operator=(const Attributes &a);

  /**
   * operator== method
   *
   * @param[in] a
   */
  bool operator==(const Attributes &a) const;

  /**
   * @return an XML reprenstation of all attributes
   *
   * @param[in] tag
   */
  std::string writeAttXml(const std::string &tag) const;

  /**
   * parse an XML reprenstation of all attributes to set state
   *
   * @param[in] xml  The data
   * @param[in] tag
   *
   * @return true for success
   */
  bool readAttXml(const std::string &xml, const std::string &tag);

  /**
   * Add a double attribute
   *
   * @param[in] name   Name to give this attribute
   * @param[in] v  Value to give this attribute
   */
  void addDouble(const std::string &name, const double v);

  /**
   * Add an int attribute
   *
   * @param[in] name   Name to give this attribute
   * @param[in] i  Value to give this attribute
   */
  void addInt(const std::string &name, const int i);

  /**
   * Retrieve value for a double attribute
   *
   * @param[in] name   Name to look for
   * @param[out]  v  Value returned
   * @return true if an attribute with this name exists
   */
  bool getDouble(const std::string &name, double &v) const;

  /**
   * Retrieve value for an int attribute
   *
   * @param[in] name   Name to look for
   * @param[out] i  Value returned
   * @return true if an attribute with this name exists
   */
  bool getInt(const std::string &name, int &i) const;

  /**
   * Remove a double attribute
   *
   * @param[in] name   Name of attribute to remove
   */
  void removeDouble(const std::string &name);

  /**
   * Remove an int attribute
   *
   * @param[in] name   Name of attribute to remove
   */
  void removeInt(const std::string &name);

  /**
   * Merge input attributes to local state, overwriting values if already
   * part of local state
   * 
   * @param[in] a
   */
  void attributeUnion(const Attributes &a);

  /**
   * return values for all attributes, creating scaled ints for the
   * double attributes by multiplying by a scale factor
   *
   * @param[in] scale  Scale factor
   *
   * @return  The attribute values
   */
  std::vector<int> getAllValues(double scale) const;

  /**
   * Debug print to stdout
   */
  void printAtt(void) const;

  /**
   * Debug print to a specific output
   * @param[in] fp  File pointer
   */
  void printAtt(FILE *fp) const;

  /**
   * Debug print to a string
   * @return  The string
   */
  std::string sprintAtt(void) const;


protected:
private:
  
  /**
   * Double attributes with name key
   */
  std::map<std::string, double> _da;

  /**
   * Int attributes with name key
   */
  std::map<std::string, int> _di;

}; 

#endif
