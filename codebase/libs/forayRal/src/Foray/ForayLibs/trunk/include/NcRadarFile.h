//
//
//
//
//

#include "netcdf.h"
#include "RayFile.h"

#ifndef NCRADARFILE
#define NCRADARFILE

class NcRadarFile : public RayFile {
public:
    NcRadarFile();
    ~NcRadarFile();

    virtual void open_file(const std::string &filename, const bool newFile = false) throw(ForayUtility::Fault);
    virtual void close_file();
    virtual void reset_file();

    virtual void read_headers()                                                   throw(ForayUtility::Fault);

    virtual bool find_first_ray()                                                 throw(ForayUtility::Fault);
    virtual bool find_next_ray()                                                  throw(ForayUtility::Fault);
 
    virtual std::vector<double> get_cell_vector()                                 throw(ForayUtility::Fault);
    virtual void                set_cell_spacing_vector(std::vector<double> &)    throw(ForayUtility::Fault);

    virtual int  get_field_index(std::string fieldName)                           throw(ForayUtility::Fault);
    virtual bool test_for_field (std::string fieldName)                           throw(ForayUtility::Fault);

    virtual void get_ray_data(int index, RayIntegers *ri)                         throw(ForayUtility::Fault);
    virtual void get_ray_data(int index, RayDoubles  *rd)                         throw(ForayUtility::Fault);
    
    virtual void write_ground_headers()                                           throw(ForayUtility::Fault);
    virtual void write_ground_ray()                                               throw(ForayUtility::Fault);
    virtual void write_ground_tail()                                              throw(ForayUtility::Fault);

    virtual void set_ray_data(int index, RayIntegers &ri)                         throw(ForayUtility::Fault);
    virtual void set_ray_data(int index, RayDoubles  &rd)                         throw(ForayUtility::Fault);

protected:
    int    ncFileId_;
    bool   writeFile_;

    int    timeDimensionId_;
    int    maxCellsDimensionId_;
    int    numSystemsDimensionId_;
    int    numFieldsDimensionId_;
    int    shortStringDimensionId_;
    int    longStringDimensionId_;

    int         process_instrument_type (const std::string &type)                         throw(ForayUtility::Fault);
    int         process_scan_mode       (const std::string &mode)                         throw(ForayUtility::Fault);
    void        process_field_info      ()                                                throw(ForayUtility::Fault);    
 
    bool        test_for_variable       (const std::string &name)                         throw(ForayUtility::Fault);

    int         get_nc_variable_id      (const std::string &name)                         throw(ForayUtility::Fault);
    int         get_nc_dimension_id     (const std::string &name)                         throw(ForayUtility::Fault);

    int         get_nc_integer          (const std::string &name)                         throw(ForayUtility::Fault);

    double      get_nc_double           (const std::string &name)                         throw(ForayUtility::Fault);
    double      get_nc_double           (const std::string &name,const int index)         throw(ForayUtility::Fault);

    std::string get_nc_short_string     (const std::string &name,const int index)         throw(ForayUtility::Fault);

    int         get_nc_dimension_length (const std::string &name)                         throw(ForayUtility::Fault);

    std::string get_nc_string_attribute (const std::string &name)                         throw(ForayUtility::Fault);
    std::string get_nc_string_attribute (const int varialbeId, const std::string &name)   throw(ForayUtility::Fault);


    bool        test_nc_double_attribute(const int variableId, const std::string &name)   throw(ForayUtility::Fault);
    
    double      get_nc_double_attribute (const int variableId, const std::string &name)   throw(ForayUtility::Fault);
    int         get_nc_integer_attribute(const int variableId, const std::string &name)   throw(ForayUtility::Fault);

    virtual void        define_nc_file()                                                          throw(ForayUtility::Fault);
    virtual void        define_nc_dimensions()                                                    throw(ForayUtility::Fault);
    virtual void        define_nc_variables()                                                     throw(ForayUtility::Fault);

    void        set_nc_dimension         (const std::string &name,const int len, int &di) throw(ForayUtility::Fault);
    int         create_nc_scalar         (const std::string &name,const nc_type type)     throw(ForayUtility::Fault);
    int         create_nc_system_variable(const std::string &name,const nc_type type)     throw(ForayUtility::Fault);
    int         create_nc_time_variable  (const std::string &name,const nc_type type)     throw(ForayUtility::Fault);
    int         create_nc_data_variable  (const std::string &name,const nc_type type)     throw(ForayUtility::Fault);

    void       create_fields_variable   ()                                                throw(ForayUtility::Fault);

    void       set_nc_text_attribute          (const int varid,const std::string &name,
					       const std::string &value)                  throw(ForayUtility::Fault);
    void       set_nc_integer_attribute       (const int varid,const std::string &name,
					       const int &value)                          throw(ForayUtility::Fault);
    void       set_nc_float_attribute         (const int varid,const std::string &name,
					       const double &value)                       throw(ForayUtility::Fault);
    void       set_nc_double_attribute        (const int varid,const std::string &name,
					       const double &value)                       throw(ForayUtility::Fault);
    void       set_nc_double_attribute        (const int varid,const std::string &name,
					       const double &v1, const double &v2)        throw(ForayUtility::Fault);
    void       set_nc_short_attribute         (const int varid,const std::string &name,
					       const short &value)                        throw(ForayUtility::Fault);
    void       set_nc_int_attribute           (const int varid,const std::string &name,
					       const int   &value)                        throw(ForayUtility::Fault);
    void       set_missing_and_fill_attributes(const int varid)                           throw(ForayUtility::Fault);
    void       set_missing_and_fill_attributes(const int varid,const double value)        throw(ForayUtility::Fault);
    void       set_nc_field_attributes        (const int varid,const int fieldIndex)      throw(ForayUtility::Fault);

    virtual void       write_nc_header_variables()                                                throw(ForayUtility::Fault);

    void       set_nc_integer          (const std::string &name,const int    value)       throw(ForayUtility::Fault);
    void       set_nc_float            (const std::string &name,const float  value)       throw(ForayUtility::Fault);
    void       set_nc_float            (const std::string &name,const float  value,
					const int index)                                  throw(ForayUtility::Fault);
    void       set_nc_double           (const std::string &name,const double value)       throw(ForayUtility::Fault);
    void       set_nc_double           (const std::string &name,const double value,
					const int index)                                  throw(ForayUtility::Fault);
    void       set_fields_variable     ()                                                 throw(ForayUtility::Fault);


    int              currentRayIndex_;
    std::vector<int> indexToVariableId_;

    //    static const double LIGHTSPEED_ = 299792458.0;
    static const double LIGHTSPEED_;
    
};

#endif  // NCRADARFILE
