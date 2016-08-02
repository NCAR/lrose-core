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
#ifndef DRAWLIB_HH
#define DRAWLIB_HH

typedef struct
{
	char R,G,B;
}
ColorType;

enum DRAWMODES {ORmode,SETmode};

void FillRect (int x1,int y1, int x2, int y2);
void SetLineType(int type);
void SetLineThick(int type);
void DrawRect (int x1,int y1, int x2, int y2);
void InitImage(int cx, int cy);
int  GetXSize();
int  GetYSize();
void SetColor(int r,int g, int b);
void aanbis(int x,int y);
void ORbit(int x,int y);
void DrawLine (int x1,int y1,int x2,int y2);
void pltsym(int x0,int y0,char *s,double sf,double rotation,int b);
void pltsymrot(int x0,int y0,char *s,double sf,double rotation,int b);
void plowind(double x,double y,double rigtin,int spoed,int skag,int vlag);
void WriteRGB(char *name);


char *sym_def(char a);
char *sym_defb(char a);

#ifdef DRAWLIB_MAIN
  int XSIZE,YSIZE;
  char *Rfield,*Gfield,*Bfield;
  ColorType curColor;
  int curlinetype;
  int curlinethick;
  int Drawmode;
#else
  extern int XSIZE,YSIZE;
  extern char *Rfield,*Gfield,*Bfield;
  extern ColorType curColor;
  extern int curlinetype;
  extern int curlinethick;
  extern int Drawmode;
#endif

#endif
