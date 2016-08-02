/*

  levelTable.C

  Implementation of LevelTable class

*/

#include <stdlib.h>
#include <stdio.h>
#include "levelTable.h"
#include "palette.h"

LevelTable *StdBinaryReflLevelTbl = 0;

LevelTable *GetStdBinaryReflLevelTbl()
{

  if (!StdBinaryReflLevelTbl)	// table doesn't exist yet, construct it
    {
      StdBinaryReflLevelTbl = new LevelTable(256, -32.0, -31.5, 95.5);
      StdBinaryReflLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      StdBinaryReflLevelTbl->setMoment(e_refl);
    }
  return StdBinaryReflLevelTbl;
}

// StdBinaryVelLevelTbl is 256 level for normalised vels
// i.e. 0=no val, -1.0=-nyquist +1.0=+nyquist
LevelTable *StdBinaryVelLevelTbl = 0;

LevelTable *GetStdBinaryVelLevelTbl()
{

  if (!StdBinaryVelLevelTbl)	// table doesn't exist yet, construct it
    {
      StdBinaryVelLevelTbl = new LevelTable(256, 0.0, -1.0, 1.0);
      StdBinaryVelLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      StdBinaryVelLevelTbl->setMoment(e_vel);
    }
  return StdBinaryVelLevelTbl;
}

// StdBinarySpectWLevelTbl is 256 level for normalised spectral widths
// i.e. 0=no val, 1=0 +1.0=+nyquist
LevelTable *StdBinarySpectWLevelTbl = 0;

LevelTable *GetStdBinarySpectWLevelTbl()
{

  if (!StdBinarySpectWLevelTbl)	// table doesn't exist yet, construct it
    {
      StdBinarySpectWLevelTbl = new LevelTable(256, 0.0, 0, 1.0);
      StdBinarySpectWLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      StdBinarySpectWLevelTbl->setMoment(e_spectw);
    }
  return StdBinarySpectWLevelTbl;
}

// Std64LvlPartIDLevelTbl is 64 level for Particle ID enumerations
// and per enum type strings
LevelTable *Std64LvlPartIDLevelTbl = 0;

LevelTable *GetStd64LvlPartIDLevelTbl()
{

  if (!Std64LvlPartIDLevelTbl)	// table doesn't exist yet, construct it
    {
      Std64LvlPartIDLevelTbl = new LevelTable(64);
      Std64LvlPartIDLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      Std64LvlPartIDLevelTbl->setMoment(e_particle_id);
    }
  return Std64LvlPartIDLevelTbl;
}

LevelTable *StdAccumLevelTbl = 0;

LevelTable *GetStdAccumLevelTbl()
{

  if (!StdAccumLevelTbl)	// table doesn't exist yet, construct it
    {
      //      StdAccumLevelTbl = new LevelTable(32, STDAccumThresh32);

      // the following level interpolates a 24 entry table to 96 (4 times)
      StdAccumLevelTbl = new LevelTable(24, STDAccumThresh24, 4);
      StdAccumLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      StdAccumLevelTbl->setMoment(e_rainaccum);
    }
  return StdAccumLevelTbl;
}

LevelTable *StdTerrainHtLevelTbl = 0;

LevelTable *GetStdTerrainHtLevelTbl()
{

  if (!StdTerrainHtLevelTbl)	// table doesn't exist yet, construct it
    {
      StdTerrainHtLevelTbl = new LevelTable(256, 0, 1, terrainHtIncrement * 255);
      StdTerrainHtLevelTbl->setGlobal(true); // mark as a table which must not be deleted
      StdTerrainHtLevelTbl->setMoment(e_terrainht);
    }
  return StdTerrainHtLevelTbl;
}

LevelTable::LevelTable(short NUMLEVELS, float *InitTbl, int interpfactor) {
  index0Value = 0.0;
  Levels = new float[NUMLEVELS];
  //    next = prev = 0;
  moment = e_refl;
  numlevels = NUMLEVELS;
  minVal = 0.0; maxVal = 0.0; incVal = 0.0;
  SetLevels(NUMLEVELS, InitTbl, interpfactor);
  GlobalTable = false;    // by default table is not global
  mode = LTM_indexed;
}

// the following ctor is for use where the index0val is a special value
// and the rest of the range from index1 to NUMLEVELS-1 is linear
// from minval to maxval
LevelTable::LevelTable(short NUMLEVELS, float index0val, float index1val, float maxval) {
  index0Value = 0.0;
  Levels = NULL;
  //    next = prev = 0;
  moment = e_refl;
  numlevels = NUMLEVELS;
  SetLinear(NUMLEVELS, index0val, index1val, maxval);
  GlobalTable = false;    // by default table is not global
  mode = LTM_linear;
}

// the following ctor is for use where the table is simply linear 
// from minval to maxval
LevelTable::LevelTable(short NUMLEVELS, float minval, float maxval) {
  index0Value = minval;
  Levels = NULL;
  //    next = prev = 0;
  moment = e_refl;
  numlevels = NUMLEVELS;
  SetLinear(NUMLEVELS, minval, maxval);
  GlobalTable = false;    // by default table is not global
  mode = LTM_simple_linear;
}

LevelTable::LevelTable(short NUMLEVELS) {
  index0Value = 0;
  Levels = NULL;
  minVal = 0;
  maxVal = NUMLEVELS - 1;
  incVal = 1.0;
  moment = e_refl;
  numlevels = NUMLEVELS;
  GlobalTable = false;    // by default table is not global
  mode = LTM_enum;
}

LevelTable::LevelTable(LevelTable *lvltable) {
  index0Value = 0.0;
  Levels = NULL;
  numlevels = 0;
  init(lvltable);
}

void LevelTable::init(LevelTable *lvltable) {
  if (Levels) {
    delete[] Levels;
    Levels = 0;
    numlevels = 0;
  }
  if (lvltable)
    {
      mode = lvltable->mode;
      numlevels = lvltable->numLevels();
      moment = lvltable->moment;
      minVal = lvltable->minVal; 
      maxVal = lvltable->maxVal; 
      incVal = lvltable->incVal;
      index0Value = lvltable->index0Value;
    }
  else 
    {
      mode = LTM_indexed;
      numlevels = 0;
      moment  = e_refl;
      minVal = 0.0; maxVal = 0.0; incVal = 0.0; index0Value = 0.0;
    }
  if (lvltable && (lvltable->mode == LTM_indexed))
    {
      SetLevels(lvltable->numLevels(), lvltable->Levels);
    }
  else
    Levels = NULL;
  //    next = prev = 0;
  GlobalTable = false;    // by default table is not global
}

LevelTable::~LevelTable() {
  if (GlobalTable)
    fprintf(stderr, "LevelTable::~LevelTable WARNING - Deleting a GlobalTable\n");
  if (Levels) {
    delete[] Levels;
    Levels = 0;
  }
}

// the mode for use where the index0val is a special value
// and the rest of the range from index1 to NUMLEVELS-1 is linear
// from minval to maxval 
void LevelTable::SetLinear(short NUMLEVELS, float index0val, float index1val, float maxval)
{
  mode = LTM_linear;
  minVal = index1val;
  maxVal = maxval;
  numlevels = NUMLEVELS;
  if (numlevels < 2)
    numlevels = 2;
  incVal = (maxVal - minVal) / (numlevels - 2);
  index0Value = index0val;  // 
  if (Levels) {
    delete[] Levels;
    Levels = 0;
  }
}

// the following ctor is for use where the table is simply linear 
// from minval to maxval
void LevelTable::SetLinear(short NUMLEVELS, float minval, float maxval)
{
  mode = LTM_simple_linear;
  minVal = minval;
  maxVal = maxval;
  numlevels = NUMLEVELS;
  if (numlevels < 2)
    numlevels = 2;
  incVal = (maxVal - minVal) / (numlevels - 1);
  index0Value = minVal;  // 
  if (Levels) {
    delete[] Levels;
    Levels = 0;
  }
}

void LevelTable::SetLevels(short initTblSize, float *InitTbl, int interpfactor) {
  float interpbase = 0.0,
    interpinc = 0.0;
  int   initTblIndex = 0;
  int interplevels = initTblSize * interpfactor;
  int interpstep = 0;
  
  if (interplevels != numlevels) {
    fprintf(stderr, "LevelTable::SetLevels ERROR: Old levels=%d. Setting to new levels=%d\n", 
	    numlevels, interplevels);
    if (Levels && (interplevels > numlevels)) {
      delete[] Levels;
      Levels = 0;
    }
    numlevels = interplevels;
  }
  if (!Levels)
    {
      Levels = new float[interplevels];
      numlevels = interplevels;
    }
  if (InitTbl)
    index0Value = InitTbl[0];
  else
    index0Value = 0.0;
  for (int x = 0; x < numlevels; x++) {
    if (InitTbl)
      {
	if (!interpstep) // whole step
	  {
	    interpbase = InitTbl[initTblIndex]; // base value
	    Levels[x] = interpbase;
	    if (initTblIndex < initTblSize-1)    // calc incremental interp step, last value continues using prev inc
	      interpinc = float(InitTbl[initTblIndex+1] - interpbase) / float(interpfactor);
	    initTblIndex++;
	    if (interpfactor > 1)
	      interpstep = 1;
	  }
	else    // calc interpolated value
	  {
	    Levels[x] = interpbase + (interpstep * interpinc);
	    interpstep++;
	    if (interpstep == interpfactor)
	      interpstep = 0;
	  }
	if (Levels[x] > maxVal)
	  maxVal = Levels[x];
	if (Levels[x] < minVal)
	  minVal = Levels[x];
      }
    else Levels[x] = 0;
  }
}

void LevelTable::SetLevel(short index, float val)
{
  if (mode != LTM_indexed)
    {
      fprintf(stderr, "LevelTable::SetLevel ERROR - table in linear mode\n");
      return;
    }
  if (index >= numlevels)
    index = numlevels - 1;
  if (index < 0)
    index = 0;
  if (index == 0)
    index0Value = val;
  Levels[index] = val;
}

