#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <Radx/TableMap.hh>
#include <Radx/TableMapKey.hh>

using namespace std;



TableMap::TableMap() {
  _debug = false;
}

TableMap::~TableMap() {
}

int TableMap::ReadTableB(string fileName) {

  std::ifstream filein(fileName.c_str());
  //std::fstream filein(fileName, ios::in);

  if (!filein.is_open()) {
    string _errString;
    Radx::addErrStr(_errString, "ERROR: cannot read BUFR table ", fileName, true);
    throw _errString.c_str();
  }

  for (std::string line; std::getline(filein, line); ) {

    //std::cout << line << std::endl;
    std::vector<std::string> tokens;
    tokens = split(line, ';');

    if (_debug) {
      for (vector<std::string>::const_iterator s = tokens.begin(); s!= tokens.end(); ++s) {
	//for (string s: tokens) {
        cout << *s << endl; 
      }
    }

    unsigned char f,x,y;
 
    f = atoi(tokens[0].c_str());  // try tokens[0].stoui
    x = atoi(tokens[1].c_str());
    y = atoi(tokens[2].c_str());
    unsigned short key;
    //f = 1; x = 8;
    key = f << 6;
    key = key | x;
    key = key << 8;
    key = key | y;
    if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d %s \n", key, key, f,x,y, tokens[3].c_str()); 
    //const char *fieldName;
    //fieldName = tokens[3].c_str();
    int scale;
    int referenceValue;
    int dataWidthBits;
    scale = atoi(tokens[5].c_str());
    referenceValue = atoi(tokens[6].c_str());
    dataWidthBits = atoi(tokens[7].c_str());
    table[key] = TableMapElement(tokens[3], scale, tokens[4], referenceValue,
			dataWidthBits);
  }
  return 0;
}

int TableMap::ReadTableD(string fileName) {

  unsigned short key;
  vector<unsigned short> currentList(0);

  // read table d which has pointers

  std::ifstream fileind(fileName.c_str());

  if (!fileind.is_open()) {
    throw "ERROR: cannot read BUFR table " + fileName;
  }

  for (std::string line; std::getline(fileind, line); ) {

    if (line[0] != '#') { // this is a comment skip it
     
      //std::cout << line << std::endl;
      std::vector<std::string> tokens;
      tokens = split(line, ';');

      if (_debug) {
        for (vector<std::string>::const_iterator s = tokens.begin(); s!= tokens.end(); ++s) {
	  //for (string s: tokens) {
          cout << *s << endl; 
        }
      }
      if (tokens.size() >= 6) { // handle blank lines and lines with only ;;;;;; 
	unsigned short subkey;      
	if (tokens[0].compare("  ") == 0) { // this is a continuation of the list
	  subkey = TableMapKey().EncodeKey(tokens[3], tokens[4], tokens[5]);
	  currentList.push_back(subkey);
	} else { // we have a new list starting
	  if (!currentList.empty()) {
	    table[key] = TableMapElement(currentList);
	  }
	  key = TableMapKey().EncodeKey(tokens[0], tokens[1], tokens[2]);
	  currentList.clear();

	  subkey = TableMapKey().EncodeKey(tokens[3], tokens[4], tokens[5]);
	  currentList.push_back(subkey);
	}
      }  // end if more than 6 tokens
    } // end if comment line
  }  // end for each line
  return 0;
}

// read the bufrtab_x.csv  (master) tables first, then the localtab_x_y.csv
// files second, overwriting any duplicate values provided by the master
// tables. 
int TableMap::ImportTables() {
  ReadTableB("../share/bbufr/tables/bufrtabb_16.csv");
  ReadTableD("../share/bbufr/tables/bufrtabd_16.csv");
  ReadTableB("../share/bbufr/tables/localtabb_41_2.csv");
  ReadTableD("../share/bbufr/tables/localtabd_41_2.csv");
  return 0;
}

// read the bufrtab_x.csv  (master) tables first, then the localtab_x_y.csv
// files second, overwriting any duplicate values provided by the master
// tables. 
int TableMap::ImportTablesOld() {

  // read table b

  std::ifstream filein("/h/eol/brenda/bufr/src/bbufr/tables/bufrtabb_11.csv");

  for (std::string line; std::getline(filein, line); ) {

    //std::cout << line << std::endl;
    std::vector<std::string> tokens;
    tokens = split(line, ';');

    for (vector<std::string>::const_iterator s = tokens.begin(); s!= tokens.end(); ++s) {
      //for (string s: tokens) {
      cout << *s << endl; 
    }

    unsigned char f,x,y;
 
    f = atoi(tokens[0].c_str());  // try tokens[0].stoui
    x = atoi(tokens[1].c_str());
    y = atoi(tokens[2].c_str());
    unsigned short key;
    //f = 1; x = 8;
    key = f << 6;
    key = key | x;
    key = key << 8;
    key = key | y;
    if (_debug) 
      printf("key = %d (x%x) for f;x;y %d;%d;%d %s \n", key, key, f,x,y, tokens[3].c_str()); 
    //const char *fieldName;
    //fieldName = tokens[3].c_str();
    int scale;
    int referenceValue;
    int dataWidthBits;
    scale = atoi(tokens[5].c_str());
    referenceValue = atoi(tokens[6].c_str());
    dataWidthBits = atoi(tokens[7].c_str());
    table[key] = TableMapElement(tokens[3], scale, tokens[4], referenceValue,
			dataWidthBits);
  }

  unsigned short key;
  vector<unsigned short> currentList(0);

  // read table d which has pointers

  std::ifstream fileind("/h/eol/brenda/bufr/src/bbufr/tables/localtabd_41_2.csv");

  for (std::string line; std::getline(fileind, line); ) {

    if (line[0] != '#') { // this is a comment skip it
     
      //std::cout << line << std::endl;
      std::vector<std::string> tokens;
      tokens = split(line, ';');

      for (vector<std::string>::const_iterator s = tokens.begin(); s!= tokens.end(); ++s) {
	//for (string s: tokens) {
        cout << *s << endl; 
      }

      unsigned short subkey;      
      if (tokens[0].compare("  ") == 0) { // this is a continuation of the list
        subkey = TableMapKey().EncodeKey(tokens[3], tokens[4], tokens[5]);
        currentList.push_back(subkey);
      } else { // we have a new list starting
        if (!currentList.empty()) {
          table[key] = TableMapElement(currentList);
	}
        key = TableMapKey().EncodeKey(tokens[0], tokens[1], tokens[2]);
        currentList.clear();

        subkey = TableMapKey().EncodeKey(tokens[3], tokens[4], tokens[5]);
        currentList.push_back(subkey);
      }
    }
  }
  return 0;
}

// // TODO: handle exception ...
// int  TableMap::Retrieve(unsigned short key, TableMapElement *tableMapElement) {

//   int result = 0;
//   TableMapElement val1;
//   val1 = table.at(key);

//     cout << " Found " << endl;
//     val1 = result->second;
//     *tableMapElement = val1;  // TODO: is this ok? or should I copy??

//     if (val1._whichType == TableMapElement::TableMapElementType::DESCRIPTOR) {
//       cout << "value for key " << key << ":" << val1._descriptor.fieldName << "," << 
//         val1._descriptor.scale << endl;
//     } else if (val1._whichType == TableMapElement::TableMapElementType::KEYS) {
//       vector<unsigned short> theList;
//       theList = val1._listOfKeys; 
//       cout << "value for key " << key << ": " << theList.size() << endl; 
//       for (vector<unsigned short>::const_iterator i = theList.begin(); i!= theList.end(); i++)
//         cout << *i << ' ';
//     } else {
//       result = -1;
//     }
// }

// TODO:  handle exception

TableMapElement TableMap::Retrieve(unsigned short key) {

  TableMapElement val1;
  val1 = table.at(key);

  //cout << " Found " << endl;
    if (val1._whichType == TableMapElement::DESCRIPTOR) {
      //cout << "value for key " << key << ":" << val1._descriptor.fieldName << "," << 
      //val1._descriptor.scale << endl;
      ;
    } else if (val1._whichType == TableMapElement::KEYS) {
      vector<unsigned short> theList;
      theList = val1._listOfKeys; 
      //cout << "value for key " << key << ": "; 
      for (vector<unsigned short>::const_iterator i = theList.begin(); i!= theList.end(); i++) {
        unsigned char f, x, y;
	TableMapKey().Decode(*i, &f, &x, &y);
        //cout << *i << ' ';
        //cout << f << ";" << x << ";" << y << "; ";
        printf("key(%d)=%d;%d;%d ",*i, f, x, y);
      }
      //cout << endl;
    } else {
      // TODO: do something
    }
  return val1;
}

vector<string>  TableMap::split(const std::string &s, char delim) {
  vector<string> result;
     std::stringstream ss;
     ss.str(s);
     std::string item;
     while (std::getline(ss, item, delim)) {
       result.push_back(item);
     }
     return result;
 }

