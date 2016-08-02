//
//
//
//
//
//

#ifndef DORADEFILE_H
#define DORADEFILE_H

#include <stdio.h>
#include <string>

#include <vector>
#include <map>

#include "Fault.h"
#include "Buffer.h"
#include "Decoder.h"

#include "vectorDefs.h"
#include "RaycTime.h"
#include "RaycAngle.h"

#include "RayFile.h"


#include "DoradeBlock.h"

class DoradeBlockSswb;
class DoradeBlockVold;
class DoradeBlockRadd;
class DoradeBlockParm;
class DoradeBlockXstf;

class DoradeFile : public RayFile {
public:
    DoradeFile();
    ~DoradeFile();

    virtual void open_file(const std::string &filename, 
			   const bool         newFile = false)                 throw(ForayUtility::Fault);
    virtual void close_file();
    virtual void reset_file();

    DoradeBlock *read_next_block()                                             throw(ForayUtility::Fault);

    virtual void read_headers()                                                throw(ForayUtility::Fault);


    virtual bool find_first_ray()                                              throw(ForayUtility::Fault);
    virtual bool find_next_ray()                                               throw(ForayUtility::Fault);

    virtual std::vector<double> get_cell_vector()                              throw(ForayUtility::Fault);
    virtual void           set_cell_spacing_vector(std::vector<double> &)      throw(ForayUtility::Fault);

    virtual int  get_field_index(std::string fieldName)                        throw(ForayUtility::Fault);
    virtual bool test_for_field (std::string fieldName)                        throw(ForayUtility::Fault);

    virtual void get_ray_data(int index, RayIntegers *ri)                      throw(ForayUtility::Fault);
    virtual void get_ray_data(int index, RayDoubles  *rd)                      throw(ForayUtility::Fault);
    
    const unsigned char * get_xstf_data();

    virtual void write_ground_headers()                                        throw(ForayUtility::Fault);
    virtual void write_ground_ray()                                            throw(ForayUtility::Fault);
    virtual void write_ground_tail()                                           throw(ForayUtility::Fault);

    virtual void set_ray_data(int index, RayIntegers &ri)                      throw(ForayUtility::Fault);
    virtual void set_ray_data(int index, RayDoubles  &rd)                      throw(ForayUtility::Fault);


protected:

    bool test_big_endian()                                             throw(ForayUtility::Fault);

    bool read_block_(ForayUtility::Buffer &)                           throw(ForayUtility::Fault);

    void read_sswb(DoradeBlockSswb &)                                  throw(ForayUtility::Fault);
    void read_vold(DoradeBlockVold &)                                  throw(ForayUtility::Fault);
    void read_radd(DoradeBlockRadd &)                                  throw(ForayUtility::Fault);
    void read_parm(DoradeBlockParm &,const int index)                  throw(ForayUtility::Fault);
    void read_csfd(DoradeBlockCsfd &)                                  throw(ForayUtility::Fault);
    void read_celv(DoradeBlockCelv &)                                  throw(ForayUtility::Fault);
    void read_swib(DoradeBlockSwib &)                                  throw(ForayUtility::Fault);
    void set_missing_values();

    void clear_rdatVector();

    void write_block(ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_sswb (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_vold (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_radd (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_parm (ForayUtility::Buffer &,const int index)           throw(ForayUtility::Fault);
    void build_csfd (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_celv (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_cfac (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_swib (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_ryib (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_rdat (ForayUtility::Buffer &,const int index)           throw(ForayUtility::Fault);
    void build_null (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);
    void build_rktb (ForayUtility::Buffer &)                           throw(ForayUtility::Fault);

    void calculate_file_offsets_and_size()                             throw(ForayUtility::Fault);

    FILE    *file_;
    ForayUtility::Decoder *decoder_;

    DoradeBlockSswb  *sswb_;
    DoradeBlockVold  *vold_;
    DoradeBlockRadd  *radd_;
    std::vector<DoradeBlockParm *> parmVector_;
    DoradeBlockCelv  *celv_;
    DoradeBlockCsfd  *csfd_;
    DoradeBlockSwib  *swib_;
    std::vector<DoradeBlockRdat *> rdatVector_;
    DoradeBlockXstf  *xstf_;
    
    std::map<std::string,int>      fieldIndex_;
    
    std::vector<double>            cellVector_;


    // Used for writing data.
    std::vector<RayIntegers>       rayIntegerData_;
    std::vector<RayDoubles>        rayDoubleData_;

    int startYear_;
    int startMonth_;
    int startDay_;

    bool headers_read_;
    bool newFile_;
    bool useBigEndian_;

};


#endif // DORADEFILE_H
