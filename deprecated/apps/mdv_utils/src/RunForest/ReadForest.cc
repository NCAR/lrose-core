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
/*********************************************************************
 * ReadForest: An Abstract base class for various forest types. 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2009
 *
 *********************************************************************/

#include <iostream>
#include <string.h>
#include <cstdlib>

#include "ReadForest.hh"
#include "ReadParf.hh"

using namespace std;

int ReadForest::getNumSubTrees(int subForest)
{
  if(_subForest_category_count != 0 && subForest < _class_count) {
      switch(getForestType()) {
      case PARFOREST:
	return ((ReadParf*)_subForests)[subForest].getNumTrees();
	break;
      defualt:
	return 0;
	break;
      }
  } else 
    return 0;
}

int ReadForest::getNumSubCategories() 
{ 
  if(_subForest_category_count == 0) 
    return _category_count;
  else 
    return _subForest_category_count-1; 
}

int ReadForest::getNumClassCategories()
{
  if(_subForest_category_count == 0)
    return _attributes[_class_attribute_num].cat_count;
  else
    return _subForest_attributes[_subForest_class_attribute_num].cat_count;    
}

bool ReadForest::isSubForest() 
{ 
  if(_subForest_category_count == 0) 
    return false;
  else 
    return true; 
}

int ReadForest::getCategoryIndex(int attIndex, float catNum)
{ 
  for(int a = 0; a < _attributes[attIndex].cat_count; a++) {
    if(catNum == atof(_categories[_attributes[attIndex].cat_start-1+a].name))
      return a;
  }
  cerr << "ERROR: Requested category " << catNum << " from attribute " << attIndex << " does not exist." << endl;
  return -1;
}

category ReadForest::getClassCategory(int catNum) 
{ 
  if(catNum >= getNumClassCategories()) {
    cerr << "ERROR: Requesting class category " << catNum << " greater than num categories " << 
      getNumClassCategories()  << endl;
    category a;
    return a;
  }
  if(_subForest_category_count == 0) {
    return _categories[_attributes[_class_attribute_num].cat_start-1+catNum];
  } else {
    return _subForest_categories[_subForest_attributes[_subForest_class_attribute_num].cat_start-1+catNum];
  }
}

attribute ReadForest::getAttribute(int attNum) 
{ 
  if(attNum < _attribute_count)
    return _attributes[attNum-1]; 
  else {
    cerr << "ERROR: Requesting attribute " << attNum << " greater than num attributes " << 
      _attribute_count << endl;
    attribute a;
    return a;
  }
};

void ReadForest::traverseForest(vector<float *> *outFields, const int &i, const int &j, const int &k,
                     const int &nx, const int &ny, float *inFields)
{
  int result, countIndex, r;
  for(int a = 0; a < _numTrees; a++)  {
    result = _classify_tree(_trees[a].p, inFields);

    // Missing value hook triggered with id = -1
    if(result == -1) {
      for(int c = 0; c < getNumClassCategories(); c++)
	(*outFields)[c][(k*ny*nx)+(j*nx)+i] = -999.0;
      return;
    // Sub Forests Hook triggered with cat weight = -1.0
    } else if(_categories[_class_start+result-1].weight == -1.0) {
      countIndex = getNumClassCategories();
      (*outFields)[countIndex][(k*ny*nx)+(j*nx)+i] = result;
      switch(getForestType()) {
      case PARFOREST:
	((ReadParf*)_subForests)[result-1].traverseForest(outFields, i, j, k, nx, ny, inFields);
	break;
      defualt:
	// Checks exist in RunForest to never get here.
	break;
      }
    } else
      (*outFields)[result-1][(k*ny*nx)+(j*nx)+i]++;
  }
}


int ReadForest::_classify_tree(node *nptr, float *inFields)
{
  int attNum = nptr->attribute;
  if(nptr->yes == 0) {
    return attNum;
  }

  attribute *curr_attr = &(_attributes[attNum-1]);
  float field_val = inFields[curr_attr->index];

  bool yes;
  if(curr_attr->cat_count == CONTVAR) {
    yes = (field_val <= nptr->contvar_max);
  } else
    { // Catagorical variable
      int bit = getCategoryIndex(curr_attr->index, field_val);
      if(bit > -1)
	yes = getbit(nptr->catvar_classes, bit);
      else
	return -1;
    }
  if(yes)
    return _classify_tree(nptr->yes, inFields);
  else
    return _classify_tree(nptr->no, inFields);
}


void ReadForest::_expand_string(string &st)
{
  int dollar_bracket = 0;
  int closing_bracket;
  char *env_val;

  /*
   * look for opening '$(' sequence
   */

  while ((dollar_bracket = st.find("$(", dollar_bracket)) != string::npos) {

    if ((closing_bracket = st.find(')', dollar_bracket)) == string::npos) {
      cerr << "ERROR: No closing bracket for env variable in string " << st << endl;
      return;
    }

    string env = st.substr(dollar_bracket+2, closing_bracket-(dollar_bracket+2));

    if((env_val = getenv(env.c_str())) == NULL) {
      cerr << "ERROR: Env variable " << env << " not set" << endl;
      return;
    }

    st.replace(dollar_bracket, closing_bracket-dollar_bracket+1, env_val);
    dollar_bracket = closing_bracket+1;
  }

}
