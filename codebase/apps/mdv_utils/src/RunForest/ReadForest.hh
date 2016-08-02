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
/************************************************************************
 * Read: Abstract bass class for reading different tree/forest types.
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2008
 ************************************************************************/

#ifndef ReadForest_HH
#define ReadForest_HH

using namespace std;

#include "InputField.hh"

const int NAME_SIZE = 2048;
const int IGNOREVAR = 0;
const int CONTVAR = 1;
const int CATVAR = 2;


const int bits_per_integer = 32;// size_of(int)

static long int *new_bitvector(int vector_size, int *newsize)
{
  *newsize = (int) ((vector_size + bits_per_integer - 1) / bits_per_integer);
  long int *bvptr = new long int[*newsize];
  for(int a = 0; a << *newsize; a++)
    bvptr[a] = 0;
  return bvptr;
}

static bool getbit(long int *bvptr, int bit)
{
  long int val = bvptr[(int)(bit / bits_per_integer)];
  int pos = (int)(bit % bits_per_integer);
  return 1 & (val >> pos);
}

struct node {
  node *yes;
  node *no;
  int attribute;
  float contvar_max;
  long int *catvar_classes;
};

struct tree {
  node *p;
  int size;
};

struct attribute {
  char name[NAME_SIZE];
  int cat_count;
  long int cat_start;
  int mapping;
  int usedcont;
  int index;
};

struct category {
  char name[NAME_SIZE];
  int attribute;
  float weight;
};

enum forestType {
  PARFOREST,
  RFOREST
};

class ReadForest {

public:

  virtual int readForest(const char *fileName) = 0;

  virtual forestType getForestType() = 0;

  int getNumSubTrees(int subForest);

  int getNumTrees() { return _numTrees; };

  tree *getTrees() { return _trees; };

  int getClassAttributeNum() { return _class_attribute_num; };

  int getNumCategories() { return _category_count; };

  int getNumSubCategories();

  int getNumClassCategories();

  bool isSubForest();

  int getCategoryIndex(int attIndex, float catNum);

  category getClassCategory(int catNum);

  int getNumAttributes() { return _attribute_count; };
  int getNumUsedAttributes() { return _used_attribute_count; };

  attribute getAttribute(int attNum);

  attribute *getAttributes() { return _attributes; };

  int getUsedAttributeNum(int attNum) { return _usedAttributes[attNum]; };

  category *getCategories() { return _categories; };

  //void traverseForest(vector<float *> *outFields, const int &i, const int &j, const int &k, 
  //		      const int &nx, const int &ny, vector<InputField> *inFields);
  // TODO modified
    void traverseForest(vector<float *> *outFields, const int &i, const int &j, const int &k,
                     const int &nx, const int &ny, float *inFields);

protected:

  int _numTrees;

  tree *_trees;

  int _attribute_count;
  attribute *_attributes;
  int _used_attribute_count;
  int *_usedAttributes;

  int _category_count;
  category *_categories;
  int _class_attribute_num;
  int _class_count;
  int _class_start;

  ReadForest *_subForests;
  int _subForest_category_count;
  category *_subForest_categories;
  int _subForest_class_attribute_num;
  attribute *_subForest_attributes;

  //int _classify_tree(node *nptr, const int &i, const int &j, const int &k, 
  //		      vector<InputField> *inFields);
  // TODO modified
  int _classify_tree(node *nptr, float  *inFields);

  void _expand_string(string &st);

private:

};

#endif
