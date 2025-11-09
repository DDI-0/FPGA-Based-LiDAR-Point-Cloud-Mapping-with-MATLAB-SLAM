/*
 * rplidar_protocol.h
 *
 *  Created on: Oct 29, 2025
 *      Author: DDI
 */

#pragma once
#include <alt_types.h>

#define RPLIDAR_CMD_SYNC_BYTE        0xA5
#define RPLIDAR_CMDFLAG_HAS_PAYLOAD  0x80

#define RPLIDAR_ANS_SYNC_BYTE1       0xA5
#define RPLIDAR_ANS_SYNC_BYTE2       0x5A
#define RPLIDAR_ANS_PKTFLAG_LOOP     0x1

typedef struct _rplidar_cmd_packet_t {
    alt_u8 syncByte;
    alt_u8 cmd_flag;
    alt_u8 size;
    alt_u8 data[0];
} __attribute__((packed)) rplidar_cmd_packet_t;

typedef struct _rplidar_ans_header_t {
    alt_u8  syncByte1;
    alt_u8  syncByte2;
    alt_u32 size:30;
    alt_u32 subType:2;
    alt_u8  type;
} __attribute__((packed)) rplidar_ans_header_t;
