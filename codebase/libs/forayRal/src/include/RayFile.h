//
//
//
//
//
//

#ifndef RAYFILE_H
#define RAYFILE_H

#include <stdio.h>
#include <string>

#include <vector>
#include <map>

#include "Fault.h"
#include "Buffer.h"
#include "Decoder.h"

#include "vectorDefs.h"
#include "RayConst.h"
#include "KeyAccess.h"
#include "RaycTime.h"
#include "RaycAngle.h"

class RayFile : public ForayUtility::KeyAccess {
public:

    RayFile();
    virtual ~RayFile();  // C++ FAQS, FAQ 97 says this should be virtual

    virtual void open_file(const std::string &filename, const bool newFile = false) throw(ForayUtility::Fault) = 0;
    virtual void close_file() = 0;
    virtual void reset_file() = 0;

    virtual void read_headers()                                                throw(ForayUtility::Fault) = 0;

    virtual bool find_first_ray()                                              throw(ForayUtility::Fault) = 0;
    virtual bool find_next_ray()                                               throw(ForayUtility::Fault) = 0;

    virtual std::vector<double> get_cell_vector()                              throw(ForayUtility::Fault) = 0;
    virtual void                set_cell_spacing_vector(std::vector<double> &) throw(ForayUtility::Fault) = 0;

    virtual int  get_field_index(std::string fieldName)                        throw(ForayUtility::Fault) = 0;
    virtual bool test_for_field (std::string fieldName)                        throw(ForayUtility::Fault) = 0;

    virtual void get_ray_data(int index, RayIntegers *ri)                      throw(ForayUtility::Fault) = 0;
    virtual void get_ray_data(int index, RayDoubles  *rd)                      throw(ForayUtility::Fault) = 0;
    
    virtual void write_ground_headers()                                        throw(ForayUtility::Fault) = 0;
    virtual void write_ground_ray()                                            throw(ForayUtility::Fault) = 0;
    virtual void write_ground_tail()                                           throw(ForayUtility::Fault) = 0;

    virtual void set_ray_data(int index, RayIntegers &ri)                      throw(ForayUtility::Fault) = 0;
    virtual void set_ray_data(int index, RayDoubles  &rd)                      throw(ForayUtility::Fault) = 0;


    void copy_headers(RayFile &sourceRayFile)                                     throw(ForayUtility::Fault);
    void compute_2byte_scale_and_bias(int index, double minValue,double maxValue) throw(ForayUtility::Fault);

protected:

};


#endif // RAYFILE_H
