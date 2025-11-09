#include "RPLidar.h"
#include <stdio.h>
#include <io.h>
#include "system.h"

#define MOTOR_CTRL_PIO_BASE PIO_0_BASE
#define MAX_POINTS 0       // > 0 is debug mode

void set_motor(bool on) {
    IOWR_32DIRECT(MOTOR_CTRL_PIO_BASE, 0, on ? 1 : 0);
}

int main() {
    RPLidar lidar;
    int count = 0;

    printf("Starting RPLIDAR A1 scan...\n");

    if (!lidar.begin()) {
        printf("UART init failed\n");
        return 1;
    }

    set_motor(true);
    printf("Motor ON\n");

    rplidar_response_device_info_t info;
    if (lidar.getDeviceInfo(info, 1000) == RESULT_OK) {
        printf("Model: %d, FW: %d.%d, HW: %d\n",
               info.model,
               info.firmware_version >> 8, info.firmware_version & 0xFF,
               info.hardware_version);
    }

    if (lidar.startScan(false, 2000) != RESULT_OK) {
        printf("Scan start failed\n");
        set_motor(false);
        return 1;
    }

    printf("Angle    Distance  Quality\n");
    printf("-----    --------  -------\n");

    while (MAX_POINTS == 0 || count < MAX_POINTS) {
        if (lidar.waitPoint(1000) == RESULT_OK) {
            const RPLidarMeasurement &p = lidar.getCurrentPoint();

            // Skip if Quality == 0
            if (p.quality == 0) {
                continue;
            }

            printf("%3d.%02d    %4d     %3d\n",
                   (int)p.angle, (int)(p.angle * 100) % 100,
                   (int)p.distance,
                   p.quality);
            count++;
        }
    }

    lidar.stop();
    set_motor(false);
    printf("Stopped after %d valid points\n", count);

    return 0;
}
