#ifndef __RAPICCGALPHAPALETTE_H__
#define __RAPICCGALPHAPALETTE_H__

#include "rapicCg.h"
typedef unsigned char uchar;
#include "palette.h"

class rapicCgAlphaPalette : public rapicCgProgram
{
 public:
  CGparameter cgFragmentParam_palette;
  GLuint      boundPaletteID;     // bound palette 1D texture "name"
  CGparameter cgFragmentParam_palSize;
  float         palSize;
  
  rapicCgAlphaPalette();
  
  virtual ~rapicCgAlphaPalette();

  virtual CGprogram loadFragmentProgram(char *_fragmentProgramFileName = NULL, 
					char *_fragmentProgramEntryName = NULL,
					cgProgramStringType stringtype = 
					CGPS_FILE);
  virtual void reloadPrograms();
  virtual void setPalette(RGBPalette *newpal);
  virtual CGerror enablePrograms();
  virtual CGerror disablePrograms();
};

extern rapicCgAlphaPalette *_globalCgAlphaPalette;

inline rapicCgAlphaPalette *globalCgAlphaPalette()
{
  if (_globalCgAlphaPalette) return _globalCgAlphaPalette;
  else
    {
      _globalCgAlphaPalette = new rapicCgAlphaPalette();
      return _globalCgAlphaPalette;
    }
};

#endif
