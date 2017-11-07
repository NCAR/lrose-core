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
#include "BufrTables.hh"

using namespace std;



TableMap::TableMap() {
  _debug = false;
}

TableMap::~TableMap() {
}

void TableMap::setDebug(bool debug) {
  if (debug) {
    _debug = true;
  } else {
    _debug = false;
  }
}

//int TableMap::ReadInternalTableB(unsigned int masterTableVersion,
//				 unsigned int generatingCenter,
//				 unsigned int localTableVersion) {
int TableMap::ReadInternalTableB(const char **internalBufrTable,
 size_t n) {

    //  static const char **internalBufrTable;
    //  size_t n;

  // TODO: need to call the same function with the local tables using
  /*   sprintf(fileName, "../share/bbufr/tables/localtabb_%u_%u.csv", generatingCenter,
	  localTableVersion);
      maybe send a switch to use either masterTable, or localTable???
   Hmm ... maybe just send the pointer to the table to traverse and the number
of entries?? and put this switch in the calling function???
  */

  // loop over table entries
  //for (std::string line; std::getline(filein, line); ) {
  for (size_t i=0; i<n; i++) { 
    std::string line(internalBufrTable[i]);
 
    if (_debug) std::cout << line << std::endl;
    std::vector<std::string> tokens;
    tokens = split(line, ';');

    if (0) {
      for (vector<std::string>::const_iterator s = tokens.begin();
        s!= tokens.end(); ++s) {
        cout << *s << endl; 
      }
    }

    unsigned char f,x,y;
    if (tokens.size() >= 8) {
      f = atoi(tokens[0].c_str());
      x = atoi(tokens[1].c_str());
      y = atoi(tokens[2].c_str());
      unsigned short key;
      //f = 1; x = 8;
      key = f << 6;
      key = key | x;
      key = key << 8;
      key = key | y;
      if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d %s \n", key, key, f,x,y, tokens[3].c_str()); 
      int scale;
      int referenceValue;
      int dataWidthBits;
      scale = atoi(tokens[5].c_str());
      referenceValue = atoi(tokens[6].c_str());
      dataWidthBits = atoi(tokens[7].c_str());
      table[key] = TableMapElement(tokens[3], scale, tokens[4], referenceValue,
				   dataWidthBits);
    } else {
      cerr << " discarding line: " << line << " from Table B "
	// << masterTableVersion 
	   << endl;
    }
  }
  return 0;
}


int TableMap::ReadTableB(string fileName) {

  std::ifstream filein(fileName.c_str());

  if (!filein.is_open()) {
    string _errString;
    Radx::addErrStr(_errString, "ERROR: cannot read BUFR table ", fileName, true);
    throw _errString.c_str();
  }

  for (std::string line; std::getline(filein, line); ) {

    if (_debug) std::cout << line << std::endl;
    std::vector<std::string> tokens;
    tokens = split(line, ';');

    if (0) {
      for (vector<std::string>::const_iterator s = tokens.begin();
        s!= tokens.end(); ++s) {
        cout << *s << endl; 
      }
    }

    unsigned char f,x,y;
    if (tokens.size() >= 8) {
      f = atoi(tokens[0].c_str());
      x = atoi(tokens[1].c_str());
      y = atoi(tokens[2].c_str());
      unsigned short key;
      //f = 1; x = 8;
      key = f << 6;
      key = key | x;
      key = key << 8;
      key = key | y;
      if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d %s \n", key, key, f,x,y, tokens[3].c_str()); 
      int scale;
      int referenceValue;
      int dataWidthBits;
      scale = atoi(tokens[5].c_str());
      referenceValue = atoi(tokens[6].c_str());
      dataWidthBits = atoi(tokens[7].c_str());
      table[key] = TableMapElement(tokens[3], scale, tokens[4], referenceValue,
				   dataWidthBits);
    } else {
      cerr << " discarding line: " << line << " from file: " <<
	fileName <<  endl;
    }
  }
  return 0;
}

int TableMap::ReadInternalTableD(const char **internalBufrTable,
 size_t n) {

  unsigned short key;
  vector<unsigned short> currentList(0);

  // read table d which has pointers

  if (internalBufrTable == NULL) {
    throw "ERROR: cannot read BUFR table ";
  }

  //for (std::string line; std::getline(fileind, line); ) {
  for (size_t i=0; i<n; i++) { 
    std::string line(internalBufrTable[i]);

    if (line[0] != '#') { // this is a comment skip it
     
      if (_debug) std::cout << line << std::endl;
      std::vector<std::string> tokens;
      tokens = split(line, ';');

      if (0) {
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
      } else { // end if more than 6 tokens
	cerr << " discarding line: " << line 
	  // << " from file: " <<
	  //fileName 
	     <<  endl;
      }
    } // end if comment line
  }  // end for each line
  // hanlde the end case; check if we have one more key,value to insert
  if (!currentList.empty()) {
    table[key] = TableMapElement(currentList);
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
     
      if (_debug) std::cout << line << std::endl;
      std::vector<std::string> tokens;
      tokens = split(line, ';');

      if (0) {
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
      } else { // end if more than 6 tokens
	cerr << " discarding line: " << line << " from file: " <<
	fileName <<  endl;
      }
    } // end if comment line
  }  // end for each line
  // hanlde the end case; check if we have one more key,value to insert
  if (!currentList.empty()) {
    table[key] = TableMapElement(currentList);
  }
  return 0;
}

bool TableMap::filled() {
  return table.size() > 0;
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

int TableMap::ImportTables(unsigned int masterTableVersion,
			   unsigned int generatingCenter,
			   unsigned int localTableVersion) {
  /*
  if (masterBTables.size() <= 0) {
    // fill with the internal tables

    masterBTables.clear();
    masterBTables[16] =  BufrTables::bufrtabb_16;
    //	    table[key] = TableMapElement(currentList);

    masterBTablesSize[16] = BufrTables::N_bufrtabb_16;
    masterDTables[16] = BufrTables::bufrtabd_16;
    masterDTablesSize[16] = BufrTables::N_bufrtabd_16;

    
    localBTables.insert(make_pair(41,2), BufrTables::localtabb_41_2);
    localBTablesSize.insert(make_pair(41,2), BufrTables::N_localtabb_41_2);
    localDTables.insert(make_pair(41,2), BufrTables::localtabd_41_2);
    localDTablesSize.insert(make_pair(41,2), BufrTables::N_localtabd_41_2);
    

  }
  */
  // BufrTables bufrTables;

  const char **internalBufrTable;
  size_t n;

  internalBufrTable = BufrTables::bufrtabb_16;
  n = BufrTables::N_bufrtabb_16;
  ReadInternalTableB(internalBufrTable, n);

  // TODO: need to call the same function with the local tables using
  /*   sprintf(fileName, "../share/bbufr/tables/localtabb_%u_%u.csv", generatingCenter,
	  localTableVersion);
      maybe send a switch to use either masterTable, or localTable???
   Hmm ... maybe just send the pointer to the table to traverse and the number
of entries?? and put this switch in the calling function???
  */
      /*
      try {
	internalBufrTable = masterBTables[masterTableVersion];
	n = masterBTablesSize[masterTableVersion];
	ReadInternalTableB(internalBufrTable, n);

	internalBufrTable = masterDTables[masterTableVersion];
	n = masterDTablesSize[masterTableVersion];
	ReadInternalTableD(internalBufrTable, n);

	pair<unsigned int, unsigned int> key;
        key = make_pair(generatingCenter, localTableVersion);
	internalBufrTable = localBTables[key];
	n = localBTablesSize[key];
	ReadInternalTableB(internalBufrTable, n);


	internalBufrTable = localDTables[key];
	n = localDTablesSize[key];
	ReadInternalTableD(internalBufrTable, n);


      } catch (const std::out_of_range& e) {
	Radx::addErrStr(_errString, "", "ERROR - TableMap::", true);
	Radx::addErrInt(_errString, "   unknown BUFR table: ", generatingCenter , true);
	cerr << _errString;
	throw _errString.c_str();
      }
      */
      
      
  switch (masterTableVersion) {
  case 2:
    //                  bufrtabb_2
    internalBufrTable = BufrTables::bufrtabb_2;
    n = BufrTables::N_bufrtabb_2;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_2;
    n = BufrTables::N_bufrtabd_2;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 6:
    //                  bufrtabb_6
    internalBufrTable = BufrTables::bufrtabb_6;
    n = BufrTables::N_bufrtabb_6;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_6;
    n = BufrTables::N_bufrtabd_6;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 11:
    //                  bufrtabb_11
    internalBufrTable = BufrTables::bufrtabb_11;
    n = BufrTables::N_bufrtabb_11;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_11;
    n = BufrTables::N_bufrtabd_11;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 12:
    //                  bufrtabb_12
    internalBufrTable = BufrTables::bufrtabb_12;
    n = BufrTables::N_bufrtabb_12;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_12;
    n = BufrTables::N_bufrtabd_12;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 13:
    //                  bufrtabb_13
    internalBufrTable = BufrTables::bufrtabb_13;
    n = BufrTables::N_bufrtabb_13;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_13;
    n = BufrTables::N_bufrtabd_13;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 14:
    //                  bufrtabb_14
    internalBufrTable = BufrTables::bufrtabb_14;
    n = BufrTables::N_bufrtabb_14;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_14;
    n = BufrTables::N_bufrtabd_14;
    ReadInternalTableD(internalBufrTable, n);
    break;
  case 15:
    //                  bufrtabb_15
    internalBufrTable = BufrTables::bufrtabb_15;
    n = BufrTables::N_bufrtabb_15;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_15;
    n = BufrTables::N_bufrtabd_15;
    ReadInternalTableD(internalBufrTable, n);
    break;
    
  case 16:
    /// works!
    /*
    internalBufrTable = BufrTables::bufrtabb_16;
    n = BufrTables::N_bufrtabb_16;
    ReadInternalTableB(internalBufrTable, n);
    */
    //                  bufrtabb_16
    
    internalBufrTable = BufrTables::bufrtabb_16;
    n = BufrTables::N_bufrtabb_16;
    ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::bufrtabd_16;
    n = BufrTables::N_bufrtabd_16;
    ReadInternalTableD(internalBufrTable, n);
    
    break;

  default:
    string _errString;
    Radx::addErrInt(_errString, "ERROR: unrecognized master BUFR table ", masterTableVersion , true);
    throw _errString.c_str();
  }


  switch (generatingCenter) {
  case 41:
    //                  localtabb_41
    /*
    internalBufrTable = BufrTables::localtabb_41_2; // localtabb_41_2;
    n = BufrTables::N_localtabb_41_2; // N_localtabb_41_2;
    ReadInternalTableB(internalBufrTable, n);
    */

    internalBufrTable = BufrTables::localtabb_41_2;
    n = BufrTables::N_localtabb_41_2;
   ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::localtabd_41_2;
    n = BufrTables::N_localtabd_41_2;
    ReadInternalTableD(internalBufrTable, n);
    
    break;
  case 85:
    //                  localtabb_85
    switch (localTableVersion) {
    case 0:
      internalBufrTable = BufrTables::localtabb_85_0;
      n = BufrTables::N_localtabb_85_0;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_0;
      n = BufrTables::N_localtabd_85_0;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 1:
      internalBufrTable = BufrTables::localtabb_85_1;
      n = BufrTables::N_localtabb_85_1;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_1;
      n = BufrTables::N_localtabd_85_1;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 2:
      internalBufrTable = BufrTables::localtabb_85_2;
      n = BufrTables::N_localtabb_85_2;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_2;
      n = BufrTables::N_localtabd_85_2;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 10:
      internalBufrTable = BufrTables::localtabb_85_10;
      n = BufrTables::N_localtabb_85_10;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_10;
      n = BufrTables::N_localtabd_85_10;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 12:
      internalBufrTable = BufrTables::localtabb_85_12;
      n = BufrTables::N_localtabb_85_12;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_12;
      n = BufrTables::N_localtabd_85_12;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 14:
      internalBufrTable = BufrTables::localtabb_85_14;
      n = BufrTables::N_localtabb_85_14;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_85_14;
      n = BufrTables::N_localtabd_85_14;
      ReadInternalTableD(internalBufrTable, n);
      break;
    default:
      string _errString;
      Radx::addErrInt(_errString, "ERROR: unrecognized BUFR local table generating center ",
		      generatingCenter , true);
      Radx::addErrInt(_errString, "  local table version ", localTableVersion , true);
      throw _errString.c_str();
    }
    break;
  case 247:
    //                  localtabb_247
    switch (localTableVersion) {
    case 7:
      internalBufrTable = BufrTables::localtabb_247_7;
      n = BufrTables::N_localtabb_247_7;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_247_7;
      n = BufrTables::N_localtabd_247_7;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 8:
      internalBufrTable = BufrTables::localtabb_247_8;
      n = BufrTables::N_localtabb_247_8;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_247_8;
      n = BufrTables::N_localtabd_247_8;
      ReadInternalTableD(internalBufrTable, n);
      break;
    case 9:
      internalBufrTable = BufrTables::localtabb_247_9;
      n = BufrTables::N_localtabb_247_9;
      ReadInternalTableB(internalBufrTable, n);
      internalBufrTable = BufrTables::localtabd_247_9;
      n = BufrTables::N_localtabd_247_9;
      ReadInternalTableD(internalBufrTable, n);
      break;
    default:
      string _errString;
      Radx::addErrInt(_errString, "ERROR: unrecognized BUFR local table generating center ",
		      generatingCenter , true);
      Radx::addErrInt(_errString, "  local table version ", localTableVersion , true);
      throw _errString.c_str();
    }
    break;
  case 255:
    //                  localtabb_255
    internalBufrTable = BufrTables::localtabb_255_1;
    n = BufrTables::N_localtabb_255_1;
   ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::localtabd_255_1;
    n = BufrTables::N_localtabd_255_1;
    ReadInternalTableD(internalBufrTable, n);    
    break;
  case 65535:
    //                  localtabb_65535
    internalBufrTable = BufrTables::localtabb_65535_6;
    n = BufrTables::N_localtabb_65535_6;
   ReadInternalTableB(internalBufrTable, n);
    internalBufrTable = BufrTables::localtabd_65535_6;
    n = BufrTables::N_localtabd_65535_6;
    ReadInternalTableD(internalBufrTable, n);    
    break;
  default:
    string _errString;
    Radx::addErrInt(_errString, "ERROR: unrecognized BUFR local table generating center ",
		    generatingCenter , true);
    Radx::addErrInt(_errString, "  local table version ", localTableVersion , true);
    throw _errString.c_str();
    } 
  return 0;
      
}


int TableMap::ImportTables2(unsigned int masterTableVersion,
			   unsigned int generatingCenter,
			   unsigned int localTableVersion) {
  char fileName[1024];

  //ReadInternalTableB(masterTableVersion, generatingCenter, localTableVersion);
  sprintf(fileName, "../share/bbufr/tables/bufrtabb_%u.csv", masterTableVersion);
  if (_debug)
    cerr << "reading master Table B from " << fileName << endl;
  ReadTableB(fileName);

  sprintf(fileName, "../share/bbufr/tables/bufrtabd_%u.csv", masterTableVersion);
  if (_debug)
    cerr << "reading master Table D from " << fileName << endl;
  ReadTableD(fileName);

  sprintf(fileName, "../share/bbufr/tables/localtabb_%u_%u.csv", generatingCenter,
	  localTableVersion);
  if (_debug)
    cerr << "reading local Table B from " << fileName << endl;
  ReadTableB(fileName);

  sprintf(fileName, "../share/bbufr/tables/localtabd_%u_%u.csv", generatingCenter,
	  localTableVersion);
  if (_debug)
    cerr << "reading local Table D from " << fileName << endl;
  ReadTableD(fileName); 

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

TableMapElement TableMap::Retrieve(unsigned short key) {

  TableMapElement val1;
  val1 = table.at(key);
  if (_debug) {
    if (key != 7878) {
      if (val1._whichType == TableMapElement::DESCRIPTOR) {
	cout << "value for key " << key << ":" << val1._descriptor.fieldName
	     << endl;
      } else if (val1._whichType == TableMapElement::KEYS) {
	vector<unsigned short> theList;
	theList = val1._listOfKeys; 
 
	for (vector<unsigned short>::const_iterator i = theList.begin(); 
	     i!= theList.end(); i++) {
	  unsigned char f, x, y;
	  TableMapKey().Decode(*i, &f, &x, &y);
	  printf("key(%d)=%d;%d;%d ",*i, f, x, y);
	}
      } else {
	// ignore
	;
      }
    } // end if key != byte element of compressed array
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

