#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//#include <se_utils.hh>
//#include <sii_utils.hh>
//#include <solo2.hh>

#include "ColorTableManager.hh"

// initialize instance

ColorTableManager *ColorTableManager::_instance = nullptr;

/*********************************************************************
 * Constructor
 */

ColorTableManager::ColorTableManager()
{

  if (_instance != nullptr) {
    return;
  }
  
  // Create the color tables

  _initDefaultTables();

  // Set the singleton instance pointer

  _instance = this;

}


/*********************************************************************
 * Destructor
 */

ColorTableManager::~ColorTableManager()
{
}


/*********************************************************************
 * Inst()
 */

ColorTableManager *ColorTableManager::getInstance()
{
  if (_instance == nullptr) {
    new ColorTableManager();
  }
  
  return _instance;
}


/*********************************************************************
 * getAsciiColorTable()
 */
// ok, make this a dictionary instead of a GTree

vector<string> ColorTableManager::getAsciiColorTable(string name)
{
  vector<string> colorsAsStrings;
  colorsAsStrings = _asciiColorTables[name];
  return colorsAsStrings;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addTable()
 */

void ColorTableManager::_addTable(const char **at, int nn)
{
  // The name should be the second token

  // std::vector< std::string > tokens;
  // tokenize(at[0], tokens);

  char firstLine[256];
  strncpy(firstLine, at[0], 255);

  char *token;
  token = strtok(firstLine, " ");

  if (token == NULL) {
    cerr << "Error reading internal color table " << at[0] << endl;
    cerr << " discarding it " << endl;
    return;
  }

  // get the next token, which should be the table name 
  token = strtok(NULL, " ");

  if (token == NULL) {
    cerr << "Error reading internal color table " << at[0] << endl;
    cerr << " discarding it " << endl;
    return;
  }
  
}

/*********************************************************************
 * _initDefaultTables()
 */

void ColorTableManager::_initDefaultTables()
{
  //_asciiColorTables = g_tree_new((GCompareFunc)strcmp);
  
  int nn;
  
  // test1

  const char * test1[] = {
    "colortable test1",
    "	0.0	0.0	0.",
    "	0.5	0.5	0.5",
    "	1.0	1.0	1.0",
    "endtable",
  };

  nn = sizeof(test1)/sizeof(char *);
  _addTable(test1, nn);


  // carbone17

  const char * carbone17[] = {
    "colortable carbone17",
    "	0.539	0.066	0.559",
    "	0.293	0.121	0.621",
    "	0.461	0.363	0.887",
    "	0.437	0.684	0.809",
    "	0.066	0.543	0.066",
    "	0.125	0.652	0.125",
    "	0.430	0.770	0.430",
    "	0.633	0.816	0.633",
    "	0.906	0.902	0.906",
    "	0.984	0.859	0.050",
    "	0.984	0.750	0.113",
    "	0.879	0.641	0.211",
    "	0.750	0.535	0.289",
    "	0.629	0.441	0.344",
    "	0.871	0.375	0.469",
    "	0.945	0.277	0.418",
    "	0.809	0.156	0.254",
    "endtable",
  };

  nn = sizeof(carbone17)/sizeof(char *);
  _addTable(carbone17, nn);
  

  // carbone11

  const char * carbone11[] = {
    "colortable carbone11",
    "        0.539   0.066   0.559",
    "        0.465   0.363   0.891",
    "        0.434   0.687   0.812",
    "        0.125   0.652   0.125",
    "        0.637   0.816   0.637",
    "        0.906   0.906   0.906",
    "        0.965   0.863   0.051",
    "        0.945   0.691   0.238",
    "        0.633   0.441   0.348",
    "        0.891   0.441   0.531",
    "        0.793   0.129   0.238",
    "endtable",
  };

  nn = sizeof(carbone11)/sizeof(char *);
  _addTable(carbone11, nn);
  
  // rrate11

  const char * rrate11[] = {
    "colortable rrate11",
    "        0.504   0.000   0.504",
    "        0.301   0.176   0.629",
    "        0.426   0.363   0.937",
    "        0.039   0.523   0.031",
    "        0.367   0.746   0.348",
    "        0.945   0.937   0.922",
    "        0.996   0.855   0.035",
    "        0.937   0.609   0.156",
    "        0.590   0.391   0.309",
    "        0.934   0.387   0.434",
    "        0.832   0.066   0.238",
    "endtable",
  };

  nn = sizeof(rrate11)/sizeof(char *);
  _addTable(rrate11, nn);
  
  // bluebrown10

  const char * bluebrown10[] = {
    "colortable bluebrown10",
    "	0.004	0.379	0.754",
    "	0.005	0.517	0.815",
    "	0.006	0.654	0.877",
    "	0.007	0.792	0.938",
    "	0.008	0.930	1.000",
    "	1.000	0.902	0.695",
    "	0.902	0.764	0.606",
    "	0.805	0.625	0.517",
    "	0.707	0.487	0.429",
    "	0.609	0.348	0.340",
    "endtable",
  };

  nn = sizeof(bluebrown10)/sizeof(char *);
  _addTable(bluebrown10, nn);

  // bluebrown11

  const char * bluebrown11[] = {
    "colortable bluebrown11",
    "	0.004	0.379	0.754",
    "	0.005	0.517	0.815",
    "	0.006	0.654	0.877",
    "	0.007	0.792	0.938",
    "	0.008	0.930	1.000",
    "	0.906	0.902	0.906",
    "	1.000	0.902	0.695",
    "	0.902	0.764	0.606",
    "	0.805	0.625	0.517",
    "	0.707	0.487	0.429",
    "	0.609	0.348	0.340",
    "endtable",
  };

  nn = sizeof(bluebrown11)/sizeof(char *);
  _addTable(bluebrown11, nn);

  // theodore16

  const char * theodore16[] = {
    "colortable theodore16",
    "	0.676	0.676	0.996",
    "	0.559	0.559	0.996",
    "	0.453	0.453	0.828",
    "	0.309	0.312	0.672",
    "	0.133	0.570	0.000",
    "	0.367	0.707	0.367",
    "	0.594	0.801	0.594",
    "	0.824	0.895	0.824",
    "	0.938	0.891	0.586",
    "	0.938	0.797	0.004",
    "	0.938	0.703	0.066",
    "	0.758	0.531	0.180",
    "	0.645	0.117	0.117",
    "	0.742	0.355	0.352",
    "	0.863	0.250	0.250",
    "	0.973	0.293	0.469",
    "endtable",
  };

  nn = sizeof(theodore16)/sizeof(char *);
  _addTable(theodore16, nn);

  // ewilson17

  const char * ewilson17[] = {
    "colortable ewilson17",
    "	0.938	0.000	0.977",
    "	0.684	0.391	0.977",
    "	0.000	0.992	0.938",
    "	0.000	0.625	0.977",
    "	0.000	0.000	0.898",
    "	0.000	0.977	0.000",
    "	0.312	0.742	0.312",
    "	0.000	0.539	0.000",
    "	0.938	0.938	0.938",
    "	0.703	0.488	0.273",
    "	0.742	0.625	0.469",
    "	0.977	0.488	0.000",
    "	0.977	0.645	0.000",
    "	0.938	0.820	0.000",
    "	0.996	0.684	0.684",
    "	0.977	0.469	0.469",
    "	0.996	0.000	0.000",
    "endtable",
  };

  nn = sizeof(ewilson17)/sizeof(char *);
  _addTable(ewilson17, nn);

  // scook18

  const char * scook18[] = {
    "colortable scook18",
    "	0.762	0.074	0.915",
    "	0.578	0.215	0.914",
    "	0.328	0.363	0.887",
    "	0.086	0.523	0.855",
    "	0.008	0.715	0.692",
    "	0.008	0.883	0.425",
    "	0.004	0.782	0.058",
    "	0.008	0.668	0.012",
    "	0.387	0.586	0.387",
    "	0.703	0.582	0.453",
    "	0.754	0.543	0.312",
    "	0.859	0.559	0.192",
    "	0.953	0.590	0.070",
    "	0.973	0.676	0.059",
    "	0.899	0.516	0.223",
    "	0.972	0.492	0.344",
    "	0.996	0.074	0.074",
    "	0.855	0.078	0.078",
    "endtable",
  };

  nn = sizeof(scook18)/sizeof(char *);
  _addTable(scook18, nn);

  // pd17

  const char * pd17[] = {
    "colortable pd17",
    "	0.817	0.813	0.715",
    "	1.000	0.848	0.610",
    "	1.000	0.590	0.000",
    "	0.805	0.469	0.043",
    "	1.000	0.187	0.187",
    "	0.984	0.984	0.184",
    "	0.145	0.981	0.148",
    "	0.105	0.778	0.031",
    "	0.000	0.574	0.000",
    "	0.305	0.875	1.000",
    "	0.387	0.582	0.941",
    "	0.375	0.332	0.754",
    "	1.000	0.793	1.000",
    "	0.969	0.606	0.715",
    "	0.973	0.973	0.973",
    "	0.629	0.629	0.504",
    "	0.770	0.117	0.648",
    "endtable",
  };

  nn = sizeof(pd17)/sizeof(char *);
  _addTable(pd17, nn);

  // gray5

  const char * gray5[] = {
    "colortable gray5",
    "	0.426	0.426	0.426",
    "	0.535	0.535	0.535",
    "	0.660	0.660	0.660",
    "	0.816	0.816	0.816",
    "	0.941	0.941	0.941",
    "endtable",
  };

  nn = sizeof(gray5)/sizeof(char *);
  _addTable(gray5, nn);

  // gray9

  const char * gray9[] = {
    "colortable gray9",
    "	0.379	0.379	0.379",
    "	0.454	0.454	0.454",
    "	0.528	0.528	0.528",
    "	0.603	0.603	0.603",
    "	0.678	0.678	0.678",
    "	0.753	0.753	0.753",
    "	0.827	0.827	0.827",
    "	0.902	0.902	0.902",
    "	0.977	0.977	0.977",
    "endtable",
  };

  nn = sizeof(gray9)/sizeof(char *);
  _addTable(gray9, nn);

  // sym_gray12

  const char * sym_gray12[] = {
    "colortable sym_gray12",
    "	0.753	0.753	0.753",
    "	0.800	0.800	0.800",
    "	0.846	0.846	0.846",
    "	0.892	0.892	0.892",
    "	0.938	0.938	0.938",
    "	0.985	0.985	0.985",
    "	0.691	0.691	0.691",
    "	0.636	0.636	0.636",
    "	0.580	0.580	0.580",
    "	0.525	0.525	0.525",
    "	0.470	0.470	0.470",
    "	0.415	0.415	0.415",
    "endtable",
  };

  nn = sizeof(sym_gray12)/sizeof(char *);
  _addTable(sym_gray12, nn);

  // carbone42

  const char * carbone42[] = {
    "colortable carbone42",
    "	0.469	0.020	0.640",
    "	0.403	0.227	0.559",
    "	0.164	0.055	0.582",
    "	0.227	0.055	0.672",
    "	0.289	0.055	0.766",
    "	0.352	0.141	0.898",
    "	0.414	0.375	0.996",
    "	0.445	0.559	0.996",
    "	0.281	0.590	0.602",
    "	0.188	0.523	0.371",
    "	0.004	0.445	0.000",
    "	0.000	0.492	0.000",
    "	0.000	0.539	0.000",
    "	0.059	0.586	0.059",
    "	0.176	0.633	0.176",
    "	0.289	0.680	0.289",
    "	0.402	0.723	0.402",
    "	0.520	0.770	0.520",
    "	0.633	0.816	0.633",
    "	0.750	0.863	0.750",
    "	0.816	0.894	0.816",
    "	0.926	0.894	0.691",
    "	0.938	0.859	0.352",
    "	0.938	0.812	0.000",
    "	0.938	0.766	0.023",
    "	0.938	0.719	0.055",
    "	0.926	0.672	0.086",
    "	0.871	0.625	0.117",
    "	0.816	0.578	0.148",
    "	0.758	0.531	0.180",
    "	0.703	0.484	0.211",
    "	0.648	0.438	0.242",
    "	0.590	0.391	0.250",
    "	0.535	0.344	0.250",
    "	0.485	0.328	0.297",
    "	0.629	0.312	0.375",
    "	0.625	0.003	0.000",
    "	0.718	0.086	0.188",
    "	0.813	0.148	0.273",
    "	0.879	0.211	0.355",
    "	0.949	0.273	0.355",
    "	1.000	0.012	0.000",
    "endtable",
  };

  nn = sizeof(carbone42)/sizeof(char *);
  _addTable(carbone42, nn);
}
