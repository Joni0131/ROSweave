#ifndef GENERALROSDEFINITIONS_H
#define GENERALROSDEFINITIONS_H

#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>

void error_loop();

#define RCCHECK(fn)               \
  {                               \
    rcl_ret_t temp_rc = fn;       \
    if ((temp_rc != RCL_RET_OK))  \
    {                             \
      log_e("An error occured %d", fn); \
      error_loop();               \
    }                             \
  }
  
#define RCSOFTCHECK(fn)          \
  {                              \
    rcl_ret_t temp_rc = fn;      \
    if ((temp_rc != RCL_RET_OK)) \
    {                            \
    }                            \
  }

#endif