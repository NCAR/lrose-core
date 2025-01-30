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
  
  vector<string> getAsciiColorTable(string name); 

private:

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

  std::map<std::string, std::vector<string> > _asciiColorTables;

  /**
   * @brief Constructor. This is private because this is a singleton object.
   */
  
  ColorTableManager();

  void _addTable(const char **at, int nn);
  
  void _initDefaultTables();
  
};

#endif
