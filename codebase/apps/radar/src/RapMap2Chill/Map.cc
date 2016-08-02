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
//////////////////////////////////////////////////////////
// Map.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
//////////////////////////////////////////////////////////

#include "Map.hh"
#include <cerrno>
#include <toolsa/pjg.h>
using namespace std;

//////////////
// constructor

Map::Map(const Params &params) :
        _params(params)
{
}

//////////////
// destructor

Map::~Map()
{
  _freeUp();
}

///////////////////////////////////////////////
// Reads in map file
// Returns 0 on success, -1 on failure

int Map::readFile(const string &filePath)

{

  _filePath = filePath;
  _freeUp();
  
  // open map file
  
  FILE *mapFile;
  if ((mapFile = fopen(filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Map::readMapFile" << endl;
    cerr << "  Reading file: " << filePath << endl;
    strerror(errNum);
    return -1;
  }
  int lineNum = 0;
  
  char line[BUFSIZ];
  while (fgets(line, BUFSIZ, mapFile) != NULL) {
    
    char tline[BUFSIZ];
    strncpy(tline, line, BUFSIZ);
    lineNum++;
    if (tline[0] == '#') {
      continue;
    }

    if (!strncmp(line, ICONDEF, strlen(ICONDEF))) {

      // ICONDEF

      char *token = strtok(tline, " \n\t");
      
      // name
      
      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	_printError(lineNum,line, "Reading icondef name", filePath);
	continue;
      }
      
      IconDef iconDef;
      iconDef.name = token;

      // npts

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      char *end_pt;
      int npts = strtol(token, &end_pt, 10);
      if (errno) {
	_printError(lineNum,line, "Reading icondef npts", filePath);
	continue;
      }

      // coords for each point

      for (int ipt = 0; ipt < npts; ipt++) {

	if (fgets(line, BUFSIZ, mapFile) == NULL) {
	  break;
	}
	lineNum++;
	if (line[0] == '#') {
	  ipt--;
	  continue;
	}

	errno = 0;
        IntCoord pt;
	int icon_x = strtol(line, &end_pt, 10);
	if (errno) {
	  _printError(lineNum,line, "Reading icondef x value", filePath);
	  continue;
	}
        pt.x = icon_x;
    
	errno = 0;
	int icon_y = strtol(end_pt, &end_pt, 10);
	if (errno) {
	  _printError(lineNum,line, "Reading icondef y value", filePath);
	  continue;
	}
        pt.y = icon_y;

        iconDef.pts.push_back(pt);

      } // ipt

      _icondefs.push_back(iconDef);
      
    } else if (!strncmp(line, ICON, strlen(ICON))) {

      // ICON

      char *token = strtok(tline, " \n\t");

      // name
      
      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	_printError(lineNum,line, "Reading icon name", filePath);
	continue;
      }

      Icon icon;
      icon.name = token;

      // location

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      char *end_pt;
      double lat = strtod(token, &end_pt);
      if (errno) {
	_printError(lineNum,line, "Reading icon lat", filePath);
	continue;
      }
      
      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      double lon = strtod(token, &end_pt);
      if (errno) {
	_printError(lineNum,line, "Reading icon lon", filePath);
	continue;
      }
      icon.loc.x = lon;
      icon.loc.y = lat;
      
      // text offset
      
      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      int xoff = strtol(token, &end_pt, 10);
      if (errno) {
	_printError(lineNum,line, "Reading icon x text offset", filePath);
	continue;
      }
      icon.label_offset.x = xoff;

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      int yoff = strtol(token, &end_pt, 10);
      if (errno) {
	_printError(lineNum,line, "Reading icon y text offset", filePath);
	continue;
      }
      icon.label_offset.y = yoff;
      
      // label
      
      if (icon.label_offset.x != ICON_FLAG ||
	  icon.label_offset.x != ICON_FLAG) {

	token = strtok((char *) NULL, " \n\t");
	if (token == NULL) {
	  _printError(lineNum,line, "Reading icon label", filePath);
	  continue;
	}
        
        icon.label = token;
        
      } 

      _icons.push_back(icon);

    } else if (!strncmp(line, POLYLINE, strlen(POLYLINE))) {

      // POLYLINE

      char *token = strtok(tline, " \n\t");

      // name

      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	_printError(lineNum,line, "Reading polyline name", filePath);
	continue;
      }
      
      Polyline polyline;
      polyline.name = token;

      // npts
      
      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      char *end_pt;
      int npts = strtol(token, &end_pt, 10);
      if (errno) {
	_printError(lineNum,line, "Reading polyline npts", filePath);
	continue;
      }

      // coords for each point

      for (int ipt = 0; ipt < npts; ipt++) {
        
	if (fgets(line, BUFSIZ, mapFile) == NULL) {
	  break;
	}
	lineNum++;
	if (line[0] == '#') {
	  ipt--;
	  continue;
	}

	errno = 0;
	double lat = strtod(line, &end_pt);
	if (errno) {
	  _printError(lineNum,line, "Reading polyline lat", filePath);
	  continue;
	}
    
	errno = 0;
	double lon = strtod(end_pt, &end_pt);
	if (errno) {
	  _printError(lineNum,line, "Reading polyline lon", filePath);
	  continue;
	}
        
        DoubleCoord coord;
	if (lat < POLYLINE_FILE_FLAG && lon < POLYLINE_FILE_FLAG) {
	  coord.y = POLYLINE_PLOT_PENUP;
	  coord.x = POLYLINE_PLOT_PENUP;
	} else {
	  coord.y = lat;
	  coord.x = lon;
	}

        polyline.pts.push_back(coord);
        
      } // ipt

      _polylines.push_back(polyline);
      
    } else if (!strncmp(line, SIMPLELABEL, strlen(SIMPLELABEL))) {

      // SIMPLELABEL
      
      char *token = strtok(tline, " \n\t");

      // location

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      char *end_pt;
      double lat = strtod(token, &end_pt);
      if (errno) {
	_printError(lineNum,line, "Reading simplelabel lat", filePath);
	continue;
      }

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      double lon = strtod(token, &end_pt);
      if (errno) {
	_printError(lineNum,line, "Reading simplelabel lon", filePath);
	continue;
      }

      // text
      
      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	_printError(lineNum,line, "Reading simplelabel text", filePath);
	continue;
      }
      
      SimpleLabel simpleLabel;
      simpleLabel.loc.x = lon;
      simpleLabel.loc.y = lat;
      simpleLabel.text = token;

      _simplelabels.push_back(simpleLabel);

    } // if (!strncmp(line, ICONDEF, strlen(ICONDEF))) {

  } // while

  // set the icondef numbers associated with each icon

  for (int i = 0; i < (int) _icons.size(); i++) {
    
    bool def_found = false;

    for (int j = 0; j < (int) _icondefs.size(); j++) {

      if (_icons[i].name == _icondefs[j].name) {
        _icons[i].defnum = j;
	def_found = true;;
	break;
      }

    } // j
    
    if (!def_found) {
      _icons[i].defnum = -1;
      cerr << "Warning - icon name not defined: "
           << _icons[i].name << endl;
      cerr << "  Check file: " << filePath << endl;
    }

  } // i
  
  fclose(mapFile);

  return 0;

}

///////////////
// free up map

void Map::_freeUp()
{

  _icondefs.clear();
  _icons.clear();
  _polylines.clear();
  _simplelabels.clear();

}

/////////////////////////////////
// translate to GTK display format

void Map::translate2GtkDisplay()
{

  double radarLat = _params.radar_latitude;
  double radarLon = _params.radar_longitude;

  // polylines

  cout << "# Tranlated from RAP map file: " << _filePath << endl;
  cout << "#" << endl;
  cout << "# Polylines" << endl;
  cout << "#" << endl;
  for (int ii = 0; ii < (int) _polylines.size(); ii++) {
    const Polyline &polyline = _polylines[ii];
    cout << "# Polyline: " << polyline.name << endl;
    for (int jj = 0; jj < (int) polyline.pts.size(); jj++) {
      double lat = polyline.pts[jj].y;
      double lon = polyline.pts[jj].x;
      if (_params.debug) {
	cout << "lat, lon: " << lat << ", " << lon << endl;
      }
      double xx, yy;
      PJGLatLon2DxDy(radarLat, radarLon,
                     lat, lon,
                     &xx, &yy);
      if (jj < (int) polyline.pts.size() - 1) {
	// if penup next, draw what we have
	if (polyline.pts[jj+1].y < -999 ||
	    polyline.pts[jj+1].x < -999) {
	  cout << xx << "   " << yy << "  1" << endl;
	  jj++;
	} else {
	  // push point onto stack
	  cout << xx << "   " << yy << "  0" << endl;
	}
      } else {
        // draw line using stack pts
        cout << xx << "   " << yy << "  1" << endl;
      }
    }
  }

  // town labels

  cout << "#" << endl;
  cout << "# Towns" << endl;
  cout << "#" << endl;
  for (int ii = 0; ii < (int) _simplelabels.size(); ii++) {
    const SimpleLabel &simplelabel = _simplelabels[ii];
    double lat = simplelabel.loc.y;
    double lon = simplelabel.loc.x;
    double xx, yy;
    PJGLatLon2DxDy(radarLat, radarLon,
                   lat, lon,
                   &xx, &yy);
    cout << xx << "   " << yy << "  6   " << simplelabel.text << endl;
  }
  
}

///////////////
// print map

void Map::print(ostream &out)
{

  cerr << "=================================" << endl;
  cerr << "Map file: " << _filePath << endl;

  cerr << "N icondefs: " << _icondefs.size() << endl;
  for (int ii = 0; ii < (int) _icondefs.size(); ii++) {
    const IconDef &icondef = _icondefs[ii];
    cerr << "  Icondef name: " << icondef.name << endl;
    for (int jj = 0; jj < (int) icondef.pts.size(); jj++) {
      cerr << "        x, y: "
           << icondef.pts[jj].x << ", "
           << icondef.pts[jj].y << endl;
    }
  }
  
  cerr << "N icons: " << _icons.size() << endl;
  for (int ii = 0; ii < (int) _icons.size(); ii++) {
    const Icon &icon = _icons[ii];
    cerr << "  Icondef   name: " << icon.name << endl;
    cerr << "          defnum: " << icon.defnum << endl;
    cerr << "           loc.x: " << icon.loc.x << endl;
    cerr << "           loc.y: " << icon.loc.y << endl;
    cerr << "  label_offset.x: " << icon.label_offset.x << endl;
    cerr << "  label_offset.y: " << icon.label_offset.y << endl;
  }
  
  cerr << "N polylines: " << _polylines.size() << endl;
  for (int ii = 0; ii < (int) _polylines.size(); ii++) {
    const Polyline &polyline = _polylines[ii];
    cerr << "  Polyline name: " << polyline.name << endl;
    for (int jj = 0; jj < (int) polyline.pts.size(); jj++) {
      cerr << "         x, y: "
           << polyline.pts[jj].x << ", "
           << polyline.pts[jj].y << endl;
    }
  }
  
  cerr << "N simplelabels: " << _simplelabels.size() << endl;
  for (int ii = 0; ii < (int) _simplelabels.size(); ii++) {
    const SimpleLabel &simplelabel = _simplelabels[ii];
    cerr << "  Simplelabel text: " << simplelabel.text << endl;
    cerr << "             loc.x: " << simplelabel.loc.x << endl;
    cerr << "             loc.y: " << simplelabel.loc.y << endl;
  }
  
}

//////////////////////////////////////////////
// print_error
//

void Map::_printError(int lineNum,
                      const string &line,
                      const string &message,
                      const string &filePath)
  
{
  
  cerr << "ERROR - reading map file: " << filePath << endl;
  cerr << "  Decoding line number: " << lineNum << endl;
  cerr << "  Line: " << line;
  cerr << "  " << message << endl;
  
}

