#ifndef SPDB_SIM_PAYLOAD_RETIMER_HH
#define SPDB_SIM_PAYLOAD_RETIMER_HH

#include <cstdint>
#include <string>
#include <vector>

class PayloadRetimer {
public:
  static bool supports(int productId, int dataType2);

  static int retime(int productId,
                    int dataType2,
                    const void *input,
                    int inputLen,
                    int64_t offset,
                    std::vector<unsigned char> &output,
                    std::string &error);
};

#endif

