#ifndef MONITOR_FLAGS_H_
#define MONITOR_FLAGS_H_

#include <gflags/gflags.h>

DECLARE_string(CONFIG_FILE_PATH);
DECLARE_int32(BUFFER_SIZE);
DECLARE_int32(DC_HEAD_LEN); 
DECLARE_int32(CAP_PACK_BUF_SIZE);
DECLARE_int32(ZMQ_RCVHWM_SIZE);
DECLARE_int32(ZMQ_SNDHWM_SIZE);
DECLARE_int32(RECOMBINED_HEADER_BUFFER_SIZE);
DECLARE_int32(ROLL_OVERTIME);
DECLARE_string(LOG_FILE_MAX_SIZE);

#endif
