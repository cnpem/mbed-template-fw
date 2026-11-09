/*
 * Copyright (C) 2023 CNPEM (cnpem.br)
 * Author: Guilherme Ricioli <guilherme.ricioli@lnls.br>
 */

#include "CtrlCoreModule.hpp"
#include "version_info.h"

CtrlCoreModule::CtrlCoreModule(const uint8_t max_cmd_len,
    mbed::Callback<bool(Kernel::Clock::duration_u32, CtrlIntfModuleMessage**)>
    try_get_for_cb,
    /* Module params */
    osPriority priority, uint32_t stack_size, unsigned char *stack_mem,
    const char *name) :
  Module(priority, stack_size, stack_mem, name), _buffer_capacity(max_cmd_len),
  _try_get_for_cb(try_get_for_cb) {
    _count = 0;
  }

CtrlCoreModule::~CtrlCoreModule() {}

void CtrlCoreModule::_task() {
  while(true) {
    bool status;
    CtrlIntfModuleMessage *p_ctrl_intf_mod_msg;
    char *cmd_token = nullptr;
    char *saveptr = nullptr;

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
    } else if(!strcmp(p_ctrl_intf_mod_msg->buff, "@VER")) {
      memset(p_ctrl_intf_mod_msg->buff, 0, _buffer_capacity);
      snprintf(p_ctrl_intf_mod_msg->buff, _buffer_capacity,
               "%s", VERSION);
    }

    /* Signalizes that response is ready */
    p_ctrl_intf_mod_msg->p_ready->release();
  }
}
