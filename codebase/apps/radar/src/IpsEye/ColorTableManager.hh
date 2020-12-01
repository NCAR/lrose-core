#ifndef ColorTableManager_HH
#define ColorTableManager_HH

#include <algorithm>
#include <string>
#include <vector>
#include <map>

//#include <glib.h>

using namespace std;

class ColorTableManager
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~ColorTableManager();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static ColorTableManager *getInstance();
  

  // Access methods //

  int getNumColorTables() const
  {
    return _colorTableNames.size();
  }
  
  std::string getTableName(const int index) const
  {
    return _colorTableNames[index];
  }
  
  std::vector< std::string > getTableNames() const
  {
    return _colorTableNames;
  }
  

  std::string getTableNamesString()
  {
    std::string return_string = "";
  
    std::vector< std::string >::const_iterator table_name;
    for (table_name = _colorTableNames.begin();
	 table_name != _colorTableNames.end(); ++table_name)
    {
      return_string += *table_name;
      return_string += "\n";
    }
  
    return return_string;
  }

  void clearList()
  {
    _colorTableNames.clear();
  }
  
  /**
   * @brief Add the color table in the given file to the list.
   *
   * @param[in] filename   The color table full file path.
   *
   * @return Returns the name of the new color table on success, "" on failure.
   */

  std::string absorbTable(const char *filename);
  
  bool dumpTables(FILE *stream) const;
  
  // void putAsciiColorTable(const char *name, const char *table);
  void putAsciiColorTable(string name, vector<string> table);
  vector<string> getAsciiColorTable(string name); 
/* 
  char *getAsciiTable(const char *name) const
  {
    return (char *)g_tree_lookup(_asciiColorTables, (gpointer)name);
  }
 */ 

  ////////////////////
  // Public methods //
  ////////////////////

  // protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static ColorTableManager *_instance;
  
  /**
   * @brief The color table name list.
   */

  std::vector< std::string > _colorTableNames;
  
  /**
   * @brief The ascii color table list.
   */

  //GTree *_asciiColorTables;
  // map< color table name, list of strings containing color definitions > _asciiColorTables;
  std::map<std::string, std::vector<string> > _asciiColorTables;

  /**
   * @brief The list of color table names.
   */

  //GSList *_colorTableNamesList;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor.  This is private because this is a singleton object.
   */

  ColorTableManager();

  void _addTable(const char **at, int nn);
  
  void _initDefaultTables();
  
};

#endif
