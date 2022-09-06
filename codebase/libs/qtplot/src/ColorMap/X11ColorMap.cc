#include <qtplot/X11ColorMap.hh>
#include <algorithm>
#include <cctype>
#include <string>
#include <cstring>
//#include <QApplication>
//#include <cstdio>
//#include <iostream>
//#include <cstdlib>
//#include "PrintMdv.hh"

//#include <iostream>

//#define RGB_NUMBER_LENGTH 11
//#define MAX_X11_COLOR_TABLE_ENTRY_LENGTH 180

//int *X112RGB_r;
//int *X112RGB_g;
//int *X112RGB_b;
//vector<string> X11_color_name;
//unsigned int *X11RGB;


// TODO: need to sort the colors by name. 
// and convert to lower case

X11ColorMap *X11ColorMap::_instance = NULL;

X11ColorMap *X11ColorMap::instance() {
    if (_instance == NULL) {
        _instance = new X11ColorMap();
    }
    return _instance;
}

X11ColorMap::X11ColorMap() {
  readX11ColorTables();
}

X11ColorMap::~X11ColorMap() {
    //delete[] X112RGB_r;
    //delete[] X112RGB_g;
    //delete[] X112RGB_b;
    //delete[] X11RGB;
}


//  return a new string
string X11ColorMap::lower(string data) {

   std::transform(data.begin(), data.end(), data.begin(),
    [](unsigned char c){ return std::tolower(c); });    
   //string *strippedAndLowered = new string;
   size_t startIdx=0;
   while (data[startIdx] == ' ') startIdx += 1;
   size_t endIdx = data.size() - 1;
   while (data[endIdx] == ' ') endIdx -= 1;

   //if (startIdx > endIdx) ???

   return data.substr(startIdx, endIdx-startIdx+1);
}

// string must be lower case
// 
unsigned int X11ColorMap::x11Name2Rgb(string x11ColorName) {
    string slColorName = lower(x11ColorName);
    int startingIdx = (slColorName[0] - 'a')/26 * NUMBER_OF_ENTRIES_X11_COLOR_MAP;
    int result;
    do {
      result = slColorName.compare(X11_color_name[startingIdx]);
      if (result < 0) startingIdx -= 1;
      if (result > 0) startingIdx += 1;
    } while ((result != 0) &&  (startingIdx < NUMBER_OF_ENTRIES_X11_COLOR_MAP) &&
        (startingIdx >= 0));
    if (result == 0) return X11RGB[startingIdx];
    else  {
        string msg = "color not found ";
        msg.append(slColorName);
        throw std::invalid_argument(msg);
    }
}

bool X11ColorMap::commentLine(char *line) {
    if (strlen(line) > 0) {
        if (line[0] == '!') {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

int X11ColorMap::countLines(char *fileName) {
    char line[MAX_X11_COLOR_TABLE_ENTRY_LENGTH];
    int count = 0;
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp != NULL) {
        while (!feof(fp)) {
            fgets(line, MAX_X11_COLOR_TABLE_ENTRY_LENGTH, fp);
            if (!commentLine(line)) {
                count += 1;
            }
        }
        fclose(fp);
    }
    return count;

}

void X11ColorMap::readX11ColorTables() {
    unsigned int r,g,b;
    char name[MAX_X11_COLOR_TABLE_ENTRY_LENGTH];
    char line[MAX_X11_COLOR_TABLE_ENTRY_LENGTH];
    char fileName[] = "x11_rgb_sorted.txt";

    //_numberOfEntries = 755; // countLines(fileName);
   // X112RGB_r = new unsigned int(_numberOfEntries);
    //X112RGB_g = new unsigned int(_numberOfEntries);
    //X112RGB_b = new unsigned int[_numberOfEntries];
    //X11RGB = new unsigned int[_numberOfEntries];
    X11_color_name.resize(NUMBER_OF_ENTRIES_X11_COLOR_MAP);


    cerr << "sizeof unsigned int = " << sizeof(unsigned int) << endl;


    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp != NULL) {
        int idx = 0;
        while (!feof(fp) && idx < NUMBER_OF_ENTRIES_X11_COLOR_MAP) {
            fgets(line, MAX_X11_COLOR_TABLE_ENTRY_LENGTH, fp);
            if (!commentLine(line)) {
                int nAssignments = sscanf(line, "%u%u%u", &r, &g, &b);

                if (nAssignments != 3) cout << "ERROR! three colors NOT read" << endl;
                //X112RGB_r[idx] = r;
                //X112RGB_g[idx] = g;
                //X112RGB_b[idx] = b;
                X11RGB[idx] = r << 16 | g << 8 | b;

                // remove white space
                int start = RGB_NUMBER_LENGTH;
                bool done = false;
                while ( start<(int)strlen(line) && !done ) {
                  if (line[start] == ' ' || line[start]=='\t') {
                   start++;
                  } else {
                    done = true;
                  }
                }
                int end = strlen(line) - 1;
                done = false;
                while ( end > RGB_NUMBER_LENGTH && !done ) {
                    if (line[end] == '\t' || line[end] == ' ' || line[end] == '\n' ) {
                        end -= 1;
                    } else {
                        done = true;
                    }
                }
                int nchars = end - start + 1;
                cout << "nchars = " << nchars << endl;
                strncpy(name, &line[start], nchars);
                name[nchars] = '\0';

                X11_color_name[idx] = lower(name);

                // TODO strip line of white space \t, etc.
                printf("%3d: %u %u %u %s\n", idx, r, g, b, name);
                idx += 1;
            }
            
        }
        if (!feof(fp)) {
            cerr << "WARNING: extra entries in X11 color table not read" << endl;
        }
        fclose(fp);
    }
}
