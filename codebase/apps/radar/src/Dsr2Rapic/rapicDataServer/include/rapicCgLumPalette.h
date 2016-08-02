#ifndef __RAPICCGLUMPALETTE_H__
#define __RAPICCGLUMPALETTE_H__

#include "rapicCg.h"
typedef unsigned char uchar;
#include "palette.h"

class rapicCgLumPalette : public rapicCgProgram
{
 public:
  CGparameter cgFragmentParam_palette;
  GLuint      boundPaletteID;     // bound palette 1D texture "name"
  CGparameter cgFragmentParam_texture;
  GLuint      boundtexID;     // bound palette 2D texture
  CGparameter cgFragmentParam_palSize;
  float         palSize;
  
  rapicCgLumPalette();
  
  virtual ~rapicCgLumPalette();

  virtual CGprogram loadFragmentProgram(char *_fragmentProgramFileName = NULL, 
					char *_fragmentProgramEntryName = NULL,
					cgProgramStringType stringtype = 
					CGPS_FILE);
  virtual void reloadPrograms();
  virtual void setPalette(RGBPalette *newpal);
  virtual CGerror enablePrograms();
  virtual CGerror disablePrograms();
};

extern rapicCgLumPalette *_globalCgLumPalette;

inline rapicCgLumPalette *globalCgLumPalette()
{
  if (_globalCgLumPalette) return _globalCgLumPalette;
  else
    {
      _globalCgLumPalette = new rapicCgLumPalette();
      return _globalCgLumPalette;
    }
};

#endif
