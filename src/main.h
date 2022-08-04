#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <ch.h>


#include <msgbus/messagebus.h>
#include <parameter/parameter.h>
#include <camera/dcmi_camera.h>

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;


void test_captors (void);
#ifdef __cplusplus
}
#endif

#endif
