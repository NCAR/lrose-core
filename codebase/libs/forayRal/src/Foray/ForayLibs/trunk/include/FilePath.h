//
//
//
//

#ifndef FILEPATH_H
#define FILEPATH_H

#include <list>
#include <string>
#include "Fault.h"
namespace ForayUtility {

    class FilePath {
    public:
	FilePath();
	~FilePath();
	void        file(const std::string file)                                          throw (Fault);
	void        directory(const std::string dirPath,const std::string pattern = ".*") throw (Fault);
	void        sort_files();
	int         file_count();
    
	void        first_file()                                                          throw (Fault);
	bool        next_file();
	std::string get_full_name()                                                       throw (Fault);
	std::string get_name()                                                            throw (Fault);
	std::string get_directory()                                                       throw (Fault);

	bool        file_exist(const std::string filename)                                throw (Fault);

	bool        singleFile_;

	std::string combine(const std::string dir, const std::string name)                throw (Fault);
    
    private:

	std::string dir_;

	std::list<std::string> filenames_;
	std::list<std::string>::iterator filenameIterator_;

	bool is_file(const std::string filename)                                throw (Fault);

    };
}



#endif //FILEPATH
