//
//  Encoder.h
//
//

#ifndef ENCODER_H
#define ENCODER_H

#include <string>

namespace ForayUtility {

   class Encoder {
   public:
      Encoder();
      ~Encoder();
      
      void  buffers_big_endian(bool bigEndian);
      bool  machine_big_endian();
      
      void  four_byte_integer         (unsigned char * const buffer, const int            encodeValue);
      void  four_byte_unsigned_integer(unsigned char * const buffer, const unsigned int   encodeValue);
      void  four_byte_float           (unsigned char * const buffer, const double         encodeValue);
      void  eight_byte_float          (unsigned char * const buffer, const double         encodeValue);
      void  two_byte_integer          (unsigned char * const buffer, const int            encodeValue);
      void  two_byte_unsigned_integer (unsigned char * const buffer, const int            encodeValue);
      void  one_byte_integer          (unsigned char * const buffer, const char           encodeValue) const;
      void  one_byte_unsigned_integer (unsigned char * const buffer, const unsigned char  encodeValue) const;
      void  string                    (unsigned char * const buffer, const char          *encodeValue, const int size);
      void  string                    (unsigned char * const buffer, const unsigned char *encodeValue, const int size);
      
      // These functions not converted yet.
      
      long long  eight_byte(const unsigned char *buffer);
      int        one_byte(const unsigned char *buffer);
      
      
   private:
      void set_swapBytes();
      
      bool machineBigEndian_;
      bool buffersBigEndian_;
      bool swapBytes_;
      
   };
   
}

#endif // ENCODER_H
