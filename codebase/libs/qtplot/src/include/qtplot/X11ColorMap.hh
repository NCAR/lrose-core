
#ifndef X11COLORMAP_H
#define X11COLORMAP_H

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <iostream>

using namespace std;

#define RGB_NUMBER_LENGTH 11
#define MAX_X11_COLOR_TABLE_ENTRY_LENGTH 580
#define NUMBER_OF_ENTRIES_X11_COLOR_MAP 753

class DLL_EXPORT X11ColorMap {


  public:

    static X11ColorMap *instance();

  	unsigned int x11Name2Rgb(string x11ColorName);
  	string lower(string data);


  private:

  	// TODO: make this a hash table



	//unsigned int *X112RGB_r;
	//unsigned int *X112RGB_g;
	//unsigned int *X112RGB_b;
	vector<string> X11_color_name;
	unsigned int X11RGB[NUMBER_OF_ENTRIES_X11_COLOR_MAP];

    static X11ColorMap *_instance;

  	X11ColorMap();
  	~X11ColorMap();	

	bool commentLine(char *line);
	int countLines(char *fileName);
	void readX11ColorTables();

};

#endif