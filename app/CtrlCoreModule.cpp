/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "CtrlCoreModule.hpp"
#include <cstdint>
#include <cstring>
#include <string.h>

#define GIT_COMMAND_ID  "@GIT"
#define TIME_COMMAND_ID "@TIM"

CtrlCoreModule::CtrlCoreModule(int buffer_capacity,
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage**)>
    try_get_for_cb,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
    const char *name) :
  Module(priority, stack_size, stack_mem, name), _buffer_capacity(buffer_capacity),
   _try_get_for_cb(try_get_for_cb) {
    _count = 0;
  }

CtrlCoreModule::~CtrlCoreModule() {}

void CtrlCoreModule::_task() {
  while(true) {
    bool status;
    CtrlIntfModuleMessage *p_ctrl_intf_mod_msg;
    char *cmd_token = NULL;
    char *saveptr = NULL;

    /* Reads messages */
    status = _try_get_for_cb(rtos::Kernel::wait_for_u32_forever,
        &p_ctrl_intf_mod_msg);
    assert(status);

    strtok_r(p_ctrl_intf_mod_msg->buff, " \n", &saveptr);

    /* Processes command */
    if(!strcmp(p_ctrl_intf_mod_msg->buff, "@INC")) {
      ++_count;
      memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
      snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
        "%lu", _count);
    } else if(!strcmp(p_ctrl_intf_mod_msg->buff, "@DEC")) {
      if(_count > 0) {
        --_count;
      }
      memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
      snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
        "%lu", _count);
    } else if(!strcmp(p_ctrl_intf_mod_msg->buff, "@ZER")) {
      _count = 0;
      memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
      snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
        "%lu", _count);
    }
    else if(strstr(p_ctrl_intf_mod_msg->buff, "@GIT") != nullptr) {
      cmd_token = strtok_r(NULL, " ", &saveptr);
      if (*cmd_token == 'H') {
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s",GIT_HASH_DESCRIBE);
      }
      else if (*cmd_token == 'B') {
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s",GIT_BRANCH);
      }
      else if (*cmd_token == 'T') {
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s",GIT_TAG);
      }
      else{
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s","NAK");
      }
    }else if (strstr(p_ctrl_intf_mod_msg->buff, "@TIM") != nullptr) {
      cmd_token = strtok_r(NULL, " ", &saveptr);
      if (*cmd_token == 'U') {
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%lu",static_cast<unsigned long>(BUILD_TIMESTAMP));
      }
      else if (*cmd_token == 'D') {
         set_time(BUILD_TIMESTAMP);
        time_t localtime = time(NULL);
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s",ctime(&localtime));
      }
       else if (*cmd_token == 'L') {
         set_time(BUILD_TIMESTAMP);
        time_t localtime = time(NULL) - (3 * 60 * 60); /* UTC-3 */
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s",ctime(&localtime));
      }
      else{
        memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
        snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
            "%s","NAK");
      }
    }else {
      memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
      snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
        "%s", "NAK");
    }

    /* Signalizes that response is ready */
    p_ctrl_intf_mod_msg->p_ready->release();
  }
}