/*
 * rplidar_cmd.h
 *
 *  Created on: Oct 29, 2025
 *      Author: DDI
 */

#pragma once
#include "rplidar_protocol.h"
#include <alt_types.h>

#define RPLIDAR_CMD_STOP               0x25
#define RPLIDAR_CMD_SCAN               0x20
#define RPLIDAR_CMD_FORCE_SCAN         0x21
#define RPLIDAR_CMD_RESET              0x40
#define RPLIDAR_CMD_GET_DEVICE_INFO    0x50
#define RPLIDAR_CMD_GET_DEVICE_HEALTH  0x52

#define RPLIDAR_ANS_TYPE_MEASUREMENT   0x81
#define RPLIDAR_ANS_TYPE_DEVINFO       0x4
#define RPLIDAR_ANS_TYPE_DEVHEALTH     0x6

#define RPLIDAR_STATUS_OK              0x0
#define RPLIDAR_STATUS_WARNING         0x1
#define RPLIDAR_STATUS_ERROR           0x2

#define RPLIDAR_RESP_MEASUREMENT_SYNCBIT        (0x1<<0)
#define RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT  2
#define RPLIDAR_RESP_MEASUREMENT_CHECKBIT       (0x1<<0)
#define RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT    1

typedef struct _rplidar_response_measurement_node_t {
    alt_u8    sync_quality;
    alt_u16   angle_q6_checkbit;
    alt_u16   distance_q2;
} __attribute__((packed)) rplidar_response_measurement_node_t;

typedef struct _rplidar_response_device_info_t {
    alt_u8   model;
    alt_u16  firmware_version;
    alt_u8   hardware_version;
    alt_u8   serialnum[16];
} __attribute__((packed)) rplidar_response_device_info_t;

typedef struct _rplidar_response_device_health_t {
    alt_u8   status;
    alt_u16  error_code;
} __attribute__((packed)) rplidar_response_device_health_t;
