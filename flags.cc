#include "flags.h"

DEFINE_string(CONFIG_FILE_PATH, "./conf.xml", "configure file path");
DEFINE_int32(BUFFER_SIZE, 1024, "buffer size");
DEFINE_int32(DC_HEAD_LEN, 10, "length of DC_HEAD"); 
DEFINE_int32(CAP_PACK_BUF_SIZE, 8196, "buffer size of capture packet");
DEFINE_int32(ZMQ_RCVHWM_SIZE, 1024*1024, "zmq receive high water line");
DEFINE_int32(ZMQ_SNDHWM_SIZE, 1024*1024, "zmq send high water line");
DEFINE_int32(RECOMBINED_HEADER_BUFFER_SIZE, 2048, "recombined header buffer size");
DEFINE_int32(ROLL_OVERTIME, 5, "over time");
