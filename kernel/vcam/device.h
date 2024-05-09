#pragma once

typedef struct
{

} vcam_device_t;

vcam_device_t *create_vcam_device(void);
void clean_vcam_device(vcam_device_t *device);
