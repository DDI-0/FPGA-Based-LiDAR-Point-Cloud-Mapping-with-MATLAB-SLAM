/*
 * RPLidar.h
 *
 *  Created on: Oct 29, 2025
 *      Author: DDI
 */

#pragma once
#include <alt_types.h>
#include "rplidar_cmd.h"
#include "system.h"
#include <stddef.h>

// Result codes
#define RESULT_OK                       0
#define RESULT_FAIL_BIT                 0x80000000
#define RESULT_INVALID_DATA             (0x8000 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_FAIL           (0x8001 | RESULT_FAIL_BIT)
#define RESULT_OPERATION_TIMEOUT        (0x8002 | RESULT_FAIL_BIT)

#define IS_OK(x)    (((x) & RESULT_FAIL_BIT) == 0)
#define IS_FAIL(x)  (((x) & RESULT_FAIL_BIT) != 0)

struct RPLidarMeasurement {
    float distance;
    float angle;
    alt_u8 quality;
    bool   startBit;
};

class RPLidar {
public:
    enum {
        RPLIDAR_SERIAL_BAUDRATE = 115200,
        RPLIDAR_DEFAULT_TIMEOUT = 500
    };

    RPLidar();
    ~RPLidar();

    bool begin();
    void end();
    bool isOpen();

    alt_u32 getHealth(rplidar_response_device_health_t &healthinfo, alt_u32 timeout = RPLIDAR_DEFAULT_TIMEOUT);
    alt_u32 getDeviceInfo(rplidar_response_device_info_t &info, alt_u32 timeout = RPLIDAR_DEFAULT_TIMEOUT);
    alt_u32 stop();
    alt_u32 startScan(bool force = false, alt_u32 timeout = RPLIDAR_DEFAULT_TIMEOUT * 2);
    alt_u32 waitPoint(alt_u32 timeout = RPLIDAR_DEFAULT_TIMEOUT);

    const RPLidarMeasurement &getCurrentPoint() { return _currentMeasurement; }

protected:
    alt_u32 _sendCommand(alt_u8 cmd, const void *payload, size_t payloadsize);
    alt_u32 _waitResponseHeader(rplidar_ans_header_t *header, alt_u32 timeout);

private:
    volatile alt_u32* _uart_base;
    RPLidarMeasurement _currentMeasurement;
};
