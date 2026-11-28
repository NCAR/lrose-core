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
 * ReadParf: 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2008
 *
 *********************************************************************/

#include <iostream>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ReadParf.hh"

using namespace std;

ReadParf::ReadParf()
{
  _numTrees = 0;
  _attribute_count = 0;
  _used_attribute_count = 0;
  _category_count = 0;
  _class_attribute_num = 0;
  _subForest_category_count = 0;
  read_success = false;
}

ReadParf::~ReadParf()
{
  free_parf();
}

int ReadParf::readParfForest(const char *baseFileName)
{
  int ret;
  char charBuffer[NAME_SIZE];
  int catvar_count, contvar_count;

  FILE *inFile;

  strcpy(charBuffer, baseFileName);
  strcat(charBuffer, ".forest");

  inFile = fopen(charBuffer, "rb");

  if(inFile == NULL) {
    cerr << "ERROR: Input file " << baseFileName << " doest not exist" << endl;
    return 1;
  }

  //
  // Read the header information
  fscanf(inFile, "%d", &_numTrees);
  ret = fscanf(inFile, "%d %d %d %ld %d", &_attribute_count, &_category_count, &_class_attribute_num,
	       &_instance_count, &_used_attribute_count);
  if(ret != 5) {
    cerr << "ERROR: Input file " << baseFileName << ", unable to read header information" << endl;
    return 1;
  }
  _class_attribute_num--;

  _attributes = new attribute[_attribute_count];
  _categories = new category[_category_count];
  _usedAttributes = new int[_used_attribute_count];

  _subForests = NULL;
  _subForest_category_count = 0;

  contvar_count = 0;
  catvar_count = 0;

  //
  // Read the variables
  for(int a = 0; a < _attribute_count; a++)
  {
    fscanf(inFile, "%s", charBuffer);
    strncpy(_attributes[a].name, charBuffer, NAME_SIZE-1);

    ret = fscanf(inFile, "%d %ld %d", &(_attributes[a].cat_count), &(_attributes[a].cat_start),
		 &(_attributes[a].mapping) );
    if(ret != 3) {
      cerr << "ERROR: Input file " << baseFileName << ", unable to read attribute number " << a << endl;
      return 1;
    }
    _attributes[a].usedcont = 0;

    if(_attributes[a].cat_count == CONTVAR)
      contvar_count++;
    if(_attributes[a].cat_count >= CATVAR)
      catvar_count++;
  }
  _class_count = _attributes[_class_attribute_num].cat_count;
  _class_start = _attributes[_class_attribute_num].cat_start-1;

  //
  // Read the indexes of the used variabes
  for(int a = 0; a < _used_attribute_count; a++) {
    ret = fscanf(inFile, "%d", &(_usedAttributes[a]) );
    if(ret != 1) {
      cerr << "ERROR: Input file " << baseFileName << ", unable to read used vars array " << a << endl;
      return 1;
    }
  }

  //
  // Create a list of the used variables
  for(int a = 1; a <= _attribute_count; a++) {
    _attributes[a-1].index = -1;
    for(int b = 0; b < _used_attribute_count; b++)
      if(_usedAttributes[b] == a) {
	_attributes[a-1].index = a-1;
	break;
      }
  }

  //
  // Read the categories
  for(int a = 0; a < _category_count; a++) {
    fscanf(inFile, "%s", charBuffer);
    strncpy(_categories[a].name, charBuffer, NAME_SIZE-1);

    ret = fscanf(inFile, "%d %f", &(_categories[a].attribute), &(_categories[a].weight) );
    if(ret != 2) {
      cerr << "ERROR: Input file " << baseFileName << ", unable to read category number " << a << endl;
      return 1;
    }

    //
    // Sub Forests Hook triggered with weight = -1.0
    if(_categories[a].weight == -1.0) {
      if(a == _class_start)
	_subForests = new ReadParf[_class_count];
      if(!_subForests) {
	cerr << "ERROR: Input file " << baseFileName << ", confussion over subforests with category " << a << endl;
	return 1;
      }

      string subforest(_categories[a].name);
      _expand_string(subforest);

      cout << "\tLoading sub forest: " << subforest << endl;

      if(a >= _class_count) {
	cerr << "ERROR: Input file " << baseFileName << ", number of sub forests greater than listed number of " << _class_count << endl;
	return 1;
      }

      if( ((ReadParf*)_subForests)[a-_class_start].readForest(subforest.c_str()))
	return 1;

      cout << "\tFinished loading sub forest." << endl;
    }
  }
  // Verify Sub Forests against each other
  if(_subForests) {
    if(_numTrees != 1) {
      cerr << "WARNING: SubForests detected and main forest has " << _numTrees << ", number of trees." << endl;
    }
    int nattributes = _subForests[0].getNumAttributes();
    int ncategories = _subForests[0].getNumCategories();
    if(_attribute_count != nattributes) {
      cerr << "ERROR: Main Forest and " << _categories[_class_start+0].name << ", number of attributes do not match." << endl;
      return 1;
    }
    for(int a = 1; a < _class_count; a++) {
      if(((ReadParf*)_subForests)[a].getNumAttributes() != nattributes) {
	cerr << "ERROR: SubForest " << _categories[_class_start+a].name << ", number of attributes does not match." << endl;
	return 1;
      }
      if(((ReadParf*)_subForests)[a].getNumCategories() != ncategories) {
	cerr << "ERROR: SubForest " << _categories[_class_start+a].name << ", number of categories does not match." << endl;
	return 1;
      }
    }

    _subForest_category_count = ncategories;
    _subForest_categories = _subForests[0].getCategories();
    _subForest_attributes = _subForests[0].getAttributes();
    _subForest_class_attribute_num = _subForests[0].getClassAttributeNum();
  }


  if(_instance_count > 0) 
  {
    int *estimated_class = new int[_instance_count];
    //
    // Read the training sets estimated classes
    for(int a = 0; a < _instance_count; a++) {
      ret = fscanf(inFile, "%d", &(estimated_class[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input file " << baseFileName << ", unable to read instance array " << a << endl;
	return 1;
      }
    }
    // Free the estimated classes as we don't need it
    delete [] estimated_class;
  }

  if(_subForests == NULL)
  {
    int *class_populations = new int[_class_count+1];
    //
    // Read the training sets category class populations
    for(int a = 0; a < _class_count+1 ; a++) {
      ret = fscanf(inFile, "%d", &(class_populations[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input file " << baseFileName << ", unable to read class populations array " << a << endl;
      return 1;
      }
    }

    delete [] class_populations;
  }

  fclose(inFile);
  read_success = true;

  _trees = new tree[_numTrees];
  int nTrees = _numTrees;

  //
  // OpenMP OPT
#ifdef _OPENMP
#pragma omp parallel
 {
   int thread = omp_get_thread_num();
   //if(thread == 0)
   //cout << "Number of Threads = " << omp_get_num_threads() << endl;
#else
   int thread = 0;
#endif

  char charBuffer2[2048];

#ifdef _OPENMP
#pragma omp for
#endif
   for(int a = 1; a <= nTrees; a++) {
     if(nTrees >= 100)
       sprintf(charBuffer2, "%s.%03d.tree", baseFileName, a);
      else if(nTrees >= 10)
       sprintf(charBuffer2, "%s.%02d.tree", baseFileName, a);
     else
       sprintf(charBuffer2, "%s.%01d.tree", baseFileName, a);

     if(load_tree(&(_trees[a-1]), charBuffer2) != 0)
       read_success = false;

   }

#ifdef _OPENMP
  } // End OMP Parallel section
#endif

 if(read_success == false)
   return 1;
 else
   return 0;
}


void ReadParf::free_parf()
{

  for(int a = 0; a < _numTrees; a++)
    free_tree( &(_trees[a]) );

  delete [] _trees;
  delete [] _attributes;
  delete [] _categories;
  delete [] _usedAttributes;
}

void ReadParf::free_tree(tree* thetree)
{
  free_node(thetree->p);
}

void ReadParf::free_node(node *nodeptr)
{
  if(nodeptr->yes) {
    if(_attributes[nodeptr->attribute-1].cat_count != CONTVAR) 
      delete [] nodeptr->catvar_classes;
    free_node(nodeptr->yes);
    free_node(nodeptr->no);
  }
  delete nodeptr;
}


int ReadParf::load_tree(tree* thetree, char *fileName)
{

  FILE *inFile;
  int ret;

  inFile = fopen(fileName, "rb");
  if(inFile == NULL) {
    cerr << "ERROR: Can not find file: " << fileName << endl;
    return 1;
  }

  //
  // Read the tree size
  ret = fscanf(inFile, "%d", &(thetree->size) );
  if(ret != 1) {
    cerr << "ERROR: Input tree " << fileName << ", unable to read tree size" << endl;
    read_success = false;
    return 1;
  }

  if(_instance_count > 0) 
  {

    int *leaf_bounds = new int[thetree->size +1];
    int *leaf_index = new int[_instance_count];
    int *leaf_pop = new int[thetree->size];
    
    //
    // Read the indices in leaf_index for each leaf
    for(int a = 0; a <= thetree->size; a++) {
      ret = fscanf(inFile, "%d", &(leaf_bounds[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input tree " << fileName << ", unable to read tree leaf bounds" << a << endl;
	read_success = false;
	return 1;
      }
    }
    
    //
    // Read the instance leaf block permutation table
    for(int a = 0; a < _instance_count; a++) {
      ret = fscanf(inFile, "%d", &(leaf_index[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input tree " << fileName << ", unable to read tree leaf index" << a << endl;
	read_success = false;
	return 1;
      }
    }

    //
    // Read the population count from the training set of each leaf
    for(int a = 0; a < thetree->size; a++) {
      ret = fscanf(inFile, "%d", &(leaf_pop[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input tree " << fileName << ", unable to read tree leaf pop" << a << endl;
	read_success = false;
	return 1;
      }
    }

    int oob_size;
    long int *oob = new_bitvector(_instance_count, &oob_size);
    //
    // Read the 
    for(int a = 0; a < oob_size; a++) {
      ret = fscanf(inFile, "%d", &(oob[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input tree " << fileName << ", unable to read tree leaf bitvector" << a << endl;
	read_success = false;
	return 1;
      }
    }
    
    int *leaf_id = new int[_instance_count];
    //
    // Read the 
    for(int a = 0; a < _instance_count; a++) {
      ret = fscanf(inFile, "%d", &(leaf_id[a]) );
      if(ret != 1) {
	cerr << "ERROR: Input tree " << fileName << ", unable to read tree leaf id" << a << endl;
	read_success = false;
	return 1;
      }
    }
    
    delete [] leaf_bounds;
    delete [] leaf_index;
    delete [] leaf_pop;
    delete [] oob;
    delete [] leaf_id;
    
  }

  thetree->p = load_tree_internal(inFile);

  if(read_success == false) {
    cerr << "Failed to read tree " << fileName << endl;
    return 1;
  }

  fclose(inFile);

  return 0;
}

node *ReadParf::load_tree_internal(FILE *inFile)
{
  int ret;
  char charBuffer[3];
  int id;
  float total_weight;
  int num_instances;

  node *nodeptr = new node;

  fscanf(inFile, "%s", charBuffer);

  ret = fscanf(inFile, "%d %d %f %d", &(id), &(nodeptr->attribute),
	       &(total_weight), &(num_instances) );
  if(ret != 4) {
    cerr << "ERROR: Unable to read input node" << endl;
    read_success = false;
    return nodeptr;
  }

  if(charBuffer[0] == 'b') {

    if(_attributes[nodeptr->attribute-1].cat_count != CONTVAR) 
    {
      int catvar_classes_size;
      nodeptr->catvar_classes = new_bitvector(_attributes[nodeptr->attribute-1].cat_count, &catvar_classes_size);
      
      for(int a = 0; a < catvar_classes_size; a++) {
	ret = fscanf(inFile, "%d", &(nodeptr->catvar_classes[a]) );
	if(ret != 1) {
	  cerr << "ERROR: Unable to read branch catvar bitvector" << a << endl;
	  read_success = false;
	  return nodeptr;
	}
      }

    } else 
    {
      nodeptr->catvar_classes = 0;

      ret = fscanf(inFile, "%e", &(nodeptr->contvar_max) );
      if(ret != 1) {
	cerr << "ERROR: Unable to read contvar_max" << endl;
	read_success = false;
	return nodeptr;
      }
    }

    nodeptr->yes = load_tree_internal(inFile);
    nodeptr->no = load_tree_internal(inFile);

  } else if(charBuffer[0] == 'l') {
    nodeptr->yes = 0;
    nodeptr->no = 0;
    nodeptr->catvar_classes = 0;
  } else {
    cerr << "ERROR: Unable to read node, unknown node type: " << charBuffer << endl;
    read_success = false;
    return nodeptr;
  }

  return nodeptr;
}
