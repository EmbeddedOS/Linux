#pragma once

extern unsigned char g_param_enable_scaling;
extern unsigned short g_param_device_nodes;

int vcam_driver_init(const char *device_name);
void vcam_driver_exit(void);