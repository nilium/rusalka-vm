#include "vm_value.cpp"
#include <iostream>


int main(int argc, char const *argv[])
{
  #define HEAD(L) std::clog << std::endl << "## " << #L << std::endl;

  #define LOG_VALUES do { \
    std::clog << "------------" << std::endl; \
    std::clog << "from_float        =>  " << from_float << std::endl; \
    std::clog << "from_double       =>  " << from_double << std::endl; \
    std::clog << "from_uint64_t     =>  " << from_uint64_t << std::endl; \
    std::clog << "from_uint32_t     =>  " << from_uint32_t << std::endl; \
    std::clog << "from_uint16_t     =>  " << from_uint16_t << std::endl; \
    std::clog << "from_uint8_t      =>  " << from_uint8_t << std::endl; \
    std::clog << "from_int64_t      =>  " << from_int64_t << std::endl; \
    std::clog << "from_int32_t      =>  " << from_int32_t << std::endl; \
    std::clog << "from_int16_t      =>  " << from_int16_t << std::endl; \
    std::clog << "from_int8_t       =>  " << from_int8_t << std::endl; \
    } while (0)

  #define LOG_PREFIX(PREFIX) do { \
    std::clog << "------------" << std::endl; \
    std::clog << #PREFIX " from_float       =>  " << (PREFIX from_float) << std::endl; \
    std::clog << #PREFIX " from_double      =>  " << (PREFIX from_double) << std::endl; \
    std::clog << #PREFIX " from_uint64_t    =>  " << (PREFIX from_uint64_t) << std::endl; \
    std::clog << #PREFIX " from_uint32_t    =>  " << (PREFIX from_uint32_t) << std::endl; \
    std::clog << #PREFIX " from_uint16_t    =>  " << (PREFIX from_uint16_t) << std::endl; \
    std::clog << #PREFIX " from_uint8_t     =>  " << (PREFIX from_uint8_t) << std::endl; \
    std::clog << #PREFIX " from_int64_t     =>  " << (PREFIX from_int64_t) << std::endl; \
    std::clog << #PREFIX " from_int32_t     =>  " << (PREFIX from_int32_t) << std::endl; \
    std::clog << #PREFIX " from_int16_t     =>  " << (PREFIX from_int16_t) << std::endl; \
    std::clog << #PREFIX " from_int8_t      =>  " << (PREFIX from_int8_t) << std::endl; \
    } while (0)

  #define LOG_SUFFIX(SUFFIX) do { \
    std::clog << "------------" << std::endl; \
    std::clog << "from_float    " #SUFFIX "    =>  " << (from_float SUFFIX) << std::endl; \
    std::clog << "from_double   " #SUFFIX "    =>  " << (from_double SUFFIX) << std::endl; \
    std::clog << "from_uint64_t " #SUFFIX "    =>  " << (from_uint64_t SUFFIX) << std::endl; \
    std::clog << "from_uint32_t " #SUFFIX "    =>  " << (from_uint32_t SUFFIX) << std::endl; \
    std::clog << "from_uint16_t " #SUFFIX "    =>  " << (from_uint16_t SUFFIX) << std::endl; \
    std::clog << "from_uint8_t  " #SUFFIX "    =>  " << (from_uint8_t SUFFIX) << std::endl; \
    std::clog << "from_int64_t  " #SUFFIX "    =>  " << (from_int64_t SUFFIX) << std::endl; \
    std::clog << "from_int32_t  " #SUFFIX "    =>  " << (from_int32_t SUFFIX) << std::endl; \
    std::clog << "from_int16_t  " #SUFFIX "    =>  " << (from_int16_t SUFFIX) << std::endl; \
    std::clog << "from_int8_t   " #SUFFIX "    =>  " << (from_int8_t SUFFIX) << std::endl; \
    } while (0)

  vm_value from_float { 0.5f };
  vm_value from_double { 0.5 };
  vm_value from_uint64_t { uint64_t { 100 } };
  vm_value from_uint32_t { uint32_t { 100 } };
  vm_value from_uint16_t { uint16_t { 100 } };
  vm_value from_uint8_t { uint8_t { 100 } };
  vm_value from_int64_t { int64_t { 100 } };
  vm_value from_int32_t { int32_t { 100 } };
  vm_value from_int16_t { int16_t { 100 } };
  vm_value from_int8_t { int8_t { 100 } };
  LOG_VALUES;
  LOG_SUFFIX(.type);

  HEAD(+= 10);
  from_float     += 10;
  from_double    += 10;
  from_uint64_t  += 10;
  from_uint32_t  += 10;
  from_uint16_t  += 10;
  from_uint8_t   += 10;
  from_int64_t   += 10;
  from_int32_t   += 10;
  from_int16_t   += 10;
  from_int8_t    += 10;
  LOG_VALUES;

  HEAD(-= 20);
  from_float     -= 20;
  from_double    -= 20;
  from_uint64_t  -= 20;
  from_uint32_t  -= 20;
  from_uint16_t  -= 20;
  from_uint8_t   -= 20;
  from_int64_t   -= 20;
  from_int32_t   -= 20;
  from_int16_t   -= 20;
  from_int8_t    -= 20;
  LOG_VALUES;

  HEAD(= 20);
  from_float     = 20;
  from_double    = 20;
  from_uint64_t  = 20;
  from_uint32_t  = 20;
  from_uint16_t  = 20;
  from_uint8_t   = 20;
  from_int64_t   = 20;
  from_int32_t   = 20;
  from_int16_t   = 20;
  from_int8_t    = 20;
  LOG_VALUES;

  HEAD(VALUES)
  LOG_SUFFIX(.i64());
  LOG_SUFFIX(.ui64());
  LOG_SUFFIX(.f64());
  LOG_PREFIX((float));
  LOG_PREFIX((double));
  LOG_PREFIX((uint64_t));
  LOG_PREFIX((uint32_t));
  LOG_PREFIX((uint16_t));
  LOG_PREFIX((uint8_t));
  LOG_PREFIX((int64_t));
  LOG_PREFIX((int32_t));
  LOG_PREFIX((int16_t));
  LOG_PREFIX((int8_t));

  HEAD(= 20 + self)
  from_float     = 20 + from_float;
  from_double    = 20 + from_double;
  from_uint64_t  = 20 + from_uint64_t;
  from_uint32_t  = 20 + from_uint32_t;
  from_uint16_t  = 20 + from_uint16_t;
  from_uint8_t   = 20 + from_uint8_t;
  from_int64_t   = 20 + from_int64_t;
  from_int32_t   = 20 + from_int32_t;
  from_int16_t   = 20 + from_int16_t;
  from_int8_t    = 20 + from_int8_t;
  LOG_VALUES;

  return 0;
}
