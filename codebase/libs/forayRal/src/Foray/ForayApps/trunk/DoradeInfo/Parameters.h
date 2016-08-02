//  $Id: Parameters.h,v 1.1 2008/10/23 05:06:18 dixon Exp $
//
//
//

#include <string>

class Parameters {
public:
    Parameters();
    ~Parameters();
    int init(int argc, char *argv[]);

    string inputFile_;
    bool   singleFile_;

    bool   dumpRktb_;

private:
    void set_defaults();
    int  read_command_line(int argc,char *argv[]);

};
