#ifndef __RAPICCG_H__
#define __RAPICCG_H__

#include <Cg/cg.h>    /* Can't include this?  Is Cg Toolkit installed! */
#include <Cg/cgGL.h>
#include <string>

enum cgProgramStringType { CGPS_FILE, CGPS_SOURCE, CGPS_OBJ };
extern bool globalCgReload;

class rapicCgProgram
{
 public:
  CGcontext   cgContext;
  CGprofile   
    cgVertexProfile,
    cgFragmentProfile;
  CGprogram   
    cgVertexProgram,
    cgFragmentProgram;

  std::string
    vertexProgramFileName,
    vertexProgramEntryName,
    fragmentProgramFileName,
    fragmentProgramEntryName;

  cgProgramStringType vertexProgramStringType;
  cgProgramStringType fragmentProgramStringType;

  rapicCgProgram(char *_vertexProgramFileName = NULL, 
		 char *_vertexProgramEntryName = NULL,
		 char *_fragmentProgramFileName = NULL,
		 char *_fragmentProgramName = NULL,
		 cgProgramStringType stringtype = CGPS_FILE);

  virtual ~rapicCgProgram();

  virtual void init(char *_vertexProgramFileName = NULL, 
		    char *_vertexProgramEntryName = NULL,
		    char *_fragmentProgramFileName = NULL,
		    char *_fragmentProgramName = NULL,
		    cgProgramStringType stringtype = CGPS_FILE);

  virtual void reloadPrograms();
  virtual CGerror checkForCgError(const char *situation);

  virtual CGprogram loadVertexProgram(char *_vertexProgramFileName = NULL, 
			      char *_vertexProgramEntryName = NULL,
			      cgProgramStringType stringtype = CGPS_FILE);
  virtual CGprogram loadVertexProgramFile(char *_vertexProgramFileName = NULL, 
				  char *_vertexProgramEntryName = NULL);
  virtual CGprogram loadVertexProgramSource(char *_vertexProgramSource = NULL, 
				    char *_vertexProgramEntryName = NULL);
  virtual CGprogram loadVertexProgramObj(char *_vertexProgramObj = NULL, 
				 char *_vertexProgramEntryName = NULL);
  virtual CGprogram loadFragmentProgram(char *_fragmentProgramFileName = NULL, 
				char *_fragmentProgramEntryName = NULL,
				cgProgramStringType stringtype = CGPS_FILE);
  virtual CGprogram loadFragmentProgramFile(char *_fragmentProgramFileName = 
					    NULL, 
				    char *_fragmentProgramEntryName = NULL);
  virtual CGprogram loadFragmentProgramSource(char *_fragmentProgramSource = 
					      NULL, 
				      char *_fragmentProgramEntryName = NULL);
  virtual CGprogram loadFragmentProgramObj(char *_fragmentProgramObj = NULL, 
				   char *_fragmentProgramEntryName = NULL);

  virtual CGerror enablePrograms();
  virtual CGerror disablePrograms();
};

#endif
