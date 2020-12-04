#ifndef SPREADSHEETVIEW_HH
#define SPREADSHEETVIEW_HH

#include <iostream>
#include <fstream>
#include <string>

using namespace std; 

class SoloScriptTranslator
{

public:

  SoloScriptTranslator();
  ~SoloScriptTranslator();

  void format_it(string &command);
  bool process_from_to(string line, std::iostream& javascript);
  bool process_action_in(string line, std::iostream& javascript);
  bool process_when_above(string line, std::iostream& javascript);
  bool process_on_var_below(string line, std::iostream& javascript);
  bool process_copy(string line, std::iostream& javascript);
  bool process_assignment(string line, std::iostream& javascript);
  bool process_action_no_args(string line, std::iostream& javascript);

  void translate(ifstream& solo_script, std::iostream& javascript);


};

#endif