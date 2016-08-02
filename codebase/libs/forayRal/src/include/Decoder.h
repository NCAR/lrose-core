//
//  Decoder.h
//
//

#ifndef DECODER_H
#define DECODER_H

#include <string>

#include <string.h> 

namespace ForayUtility {

   class Decoder {
   public:
      Decoder();
      ~Decoder();
      
       void         buffers_big_endian(bool bigEndian);
       bool         machine_big_endian();

       int          four_byte         (const unsigned char *buffer);
       unsigned int four_byte_unsigned(const unsigned char *buffer);
       long long    eight_byte        (const unsigned char *buffer);
       double       four_byte_float   (const unsigned char *buffer);
       double       eight_byte_float  (const unsigned char *buffer);
       int          two_byte          (const unsigned char *buffer);
       int          two_byte_unsigned (const unsigned char *buffer);
       int          one_byte          (const unsigned char *buffer);
       int          one_byte_unsigned (const unsigned char *buffer);
       std::string  char_string       (const unsigned char *buffer, int size);
      
   private:
      
      
      void set_swapBytes();
      
      bool machineBigEndian_;
      bool buffersBigEndian_;
      bool swapBytes_;
      
   };

}

#endif // DECODER_H
