//
//
//
//


#ifndef BUFFER_H
#define BUFFER_H

#include <string>

#include "Fault.h"
#include "Decoder.h"
#include "Encoder.h"

namespace ForayUtility {

   class Buffer {
   public:
       Buffer( const int size = 0);
       Buffer( const unsigned char *stream, const int size );
       Buffer(const Buffer &src);

       ~Buffer();
      
       Buffer & operator=(const Buffer &src);
      
       unsigned char*       new_data      (const int size);
       const unsigned char* data          (const int loc)       const            throw(Fault);
       int                  current_size  ()                    const;
       void                 is_big_endian (const bool bigEndian);
      
       std::string get_string_from_char          (const int loc, const int size) throw (Fault);
       int         get_one_byte_integer          (const int loc)                 throw (Fault);
       int         get_one_byte_unsigned_integer (const int loc)                 throw (Fault);
       int         get_two_byte_integer          (const int loc)                 throw (Fault);
       int         get_two_byte_unsigned_integer (const int loc)                 throw (Fault);
       int         get_four_byte_integer         (const int loc)                 throw (Fault);
       int         get_four_byte_unsigned_integer(const int loc)                 throw (Fault);
       double      get_four_byte_float           (const int loc)                 throw (Fault);
       double      get_eight_byte_float          (const int loc)                 throw (Fault);
       long long   get_eight_byte_integer        (const int loc)                 throw (Fault);
      

       int      set_four_byte_integer         (const int loc, const int            value)                 throw (Fault);
       int      set_four_byte_unsigned_integer(const int loc, const unsigned int   value)                 throw (Fault);
       int      set_two_byte_integer          (const int loc, const int            value)                 throw (Fault);
       int      set_two_byte_unsigned_integer (const int loc, const unsigned int   value)                 throw (Fault);
       int      set_four_byte_float           (const int loc, const double         value)                 throw (Fault);
       int      set_eight_byte_float          (const int loc, const double         value)                 throw (Fault);
       int      set_one_byte_integer          (const int loc, const char           value)                 throw (Fault);
       int      set_one_byte_unsigned_integer (const int loc, const unsigned char  value)                 throw (Fault);
       int      set_string                    (const int loc, const std::string   &value, const int size) throw (Fault);
       int      set_string                    (const int loc, const unsigned char *value, const int size) throw (Fault);
      
   protected:
       const Encoder& encoder( void ) const { return( encoder_ ); }
      
       const Decoder& decoder( void ) const { return( decoder_ ); }
      
   private:
      
      int size_;
      unsigned char *data_;
      Decoder decoder_;
      Encoder encoder_;
   };
}

#endif  //  BUFFER_H
