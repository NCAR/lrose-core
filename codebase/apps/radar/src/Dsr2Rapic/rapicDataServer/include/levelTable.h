#ifndef __LEVELTABLE_H__
#define __LEVELTABLE_H__

/*
 * levelTable.h
 *
 */

#include "rdrtypes.h"
#include "rdr.h"
#include "utils.h"
#include <vector>
#include <string>

enum levelTableMode 
  { LTM_indexed, LTM_linear, LTM_simple_linear, LTM_enum };
/*
 * In both indexed & linear mode, index value 0 is always nil echo, i.e. a "special" value
 * Linear mode notes:
 *   numlevels includes the nil echo 0 value
 *   minVal is the value for index value of 1
 *   maxVal is the value for index value of (numlevels-1)
 * Enum mode
 *   Indexes are enumerations of classifications etc.
 *   May include a list of strings for the enumerations which may be
 *   passed n the LVLTBL: field of the rapic header
 */

class LevelTable {
  e_data_type moment;	// moment this table represents
    
  //    LevelTable	*next, *prev;	// linked list of tables
  short	numlevels;
  float	minVal, maxVal, incVal;	// Level in signed floating point units
  float       index0Value;    // value to assign to index value 0
  bool	GlobalTable;	// if true, this is a global table and MUST NOT be deleted
  levelTableMode mode;
 public:
  float	*Levels;	// Level in signed floating point units
  std::vector<std::string> enumTypeStrings; // strings for enum mode types

  // pass array of threshold points - LTM_indexed
  LevelTable(short NUMLEVELS, float *InitTbl, int interpfactor = 1);

  // the following ctor is for use where the index0val is a special value
  // and the rest of the range from index1 to NUMLEVELS-1 is linear
  // from minval to maxval  - LTM_linear
  LevelTable(short NUMLEVELS, float index0val, float index1val, float maxval);

  // the following ctor is for use where the table is simply linear 
  // from minval to maxval - LTM_simple_linear
  LevelTable(short NUMLEVELS, float minval, float maxval);

  // the following ctor is for use where the table is a series 
  // of enumerators from 0 to numlevels-1 - LTM_enum
  LevelTable(short NUMLEVELS);

  // copy leveltable 
  LevelTable(LevelTable *lvltable);
  ~LevelTable();
  void	init(LevelTable *lvltable);
  bool	getGlobal() { return GlobalTable; };
  void	setGlobal(bool state = true) { GlobalTable = state; };
  void	setIndex0Value(float val = 0.0) { index0Value = val; };
  float	getIndex0Value() { return index0Value; };
  e_data_type getMoment() { return moment; };
  void	setMoment(e_data_type newmoment) { moment = newmoment; };
  int		numLevels() { return numlevels; };
  int		maxIndex() { return numlevels - 1; };
  inline float val(int index)	// return value corresponding to index
    {
      if (index == 0)
	return index0Value;
      if (index >= numlevels)
	index = numlevels - 1;
      if (index < 0)
	index = 0;
      if (mode == LTM_indexed)
	return *(Levels+index);
      else if (mode == LTM_linear)
	return minVal + ((index-1) * incVal);
      else if (mode == LTM_simple_linear)
	return minVal + (index * incVal);
      else if (mode == LTM_enum)
	return clip(index, int(minVal), int(maxVal));
      return 0.0;
    };
  inline int	threshold(float val)	// return thresholded index for value
    {
      int	lastlvl = maxIndex();
      int	index = 0;
      float	*nextlvl;
      if (mode == LTM_indexed)
	{
	  nextlvl = Levels+1;
	  while ((index < lastlvl) &&
		 (val >= *nextlvl)) {
	    index++;
	    nextlvl++;	// don't step past last level entry
	    // it is > last level, and will drop through
	  }
		
	  return index;
	}
      if (mode == LTM_linear)
	{
	  if (val < minVal)
	    return 0;
	  else if (val >= maxVal)
	    return lastlvl;  
	  else
	    return int((val-minVal)/incVal)+1;
	}
      if (mode == LTM_simple_linear)
	{
	  if (val < minVal)
	    return 0;
	  else if (val >= maxVal)
	    return lastlvl;  
	  else
	    return int(val/incVal);
	}
      if (mode == LTM_enum)
	{
	  return clip(int(val), int(minVal), int(maxVal));
	}
      return index;
    };
  //    float	*levelArray() { return Levels; }; // return pointer to level array, NULL if LTM_linear
  /*
   * if InitTbl not defined,  clear all values to 0
   */
  // the interpfactor will force interpolation btwn the InitTbl values
  // initTblSize is the total number of levels in the InitTbl, 
  // e.g. the accum level table is 24 levels with interp factor of 4 will give a 96 level table
  void	SetLevels(short initTblSize, float *InitTbl = 0, int interpfactor = 1);
  void	SetLevel(short idx, float val);
  void	SetLinear(short NUMLEVELS, float index0val, float index1val, float maxval);
  void	SetLinear(short NUMLEVELS, float minval, float maxval);
  levelTableMode getMode() { return mode; };
};

extern LevelTable *StdBinaryReflLevelTbl;
extern LevelTable *StdBinaryVelLevelTbl;
extern LevelTable *StdBinarySpectWLevelTbl;
extern LevelTable *Std64LvlPartIDLevelTbl;
extern LevelTable *StdAccumLevelTbl;
extern LevelTable *StdTerrainHtLevelTbl;

LevelTable *GetStdBinaryReflLevelTbl();
LevelTable *GetStdBinaryVelLevelTbl();
LevelTable *GetStdBinarySpectWLevelTbl();
LevelTable *GetStd64LvlPartIDLevelTbl();
LevelTable *GetStdAccumLevelTbl();
LevelTable *GetStdTerrainHtLevelTbl();

#endif
