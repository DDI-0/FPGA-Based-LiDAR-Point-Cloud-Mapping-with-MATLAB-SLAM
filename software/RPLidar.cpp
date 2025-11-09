/*
 * RPLidar.cpp
 *
 *  Created on: Oct 29, 2025
 *      Author: DDI
 */

#include "RPLidar.h"
#include <sys/alt_alarm.h>
#include <string.h>
#include <altera_avalon_uart_regs.h>

#define UART_BASE ((volatile alt_u32*)UART_0_BASE)

// Global timing
static alt_u32 _ticks_per_ms = 0;

static inline alt_u32 get_ms() {
    return alt_nticks() / _ticks_per_ms;
}

static void init_timing() {
    _ticks_per_ms = alt_ticks_per_second() / 1000;
    if (!_ticks_per_ms) _ticks_per_ms = 1;
}

// UART register I/O
static inline int uart_read_byte(volatile alt_u32* base) {
    alt_u32 status = IORD_ALTERA_AVALON_UART_STATUS(base);
    if (status & ALTERA_AVALON_UART_STATUS_RRDY_MSK) {
        return IORD_ALTERA_AVALON_UART_RXDATA(base) & 0xFF;
    }
    return -1;
}

static inline void uart_write_byte(volatile alt_u32* base, alt_u8 byte) {
    while (!(IORD_ALTERA_AVALON_UART_STATUS(base) & ALTERA_AVALON_UART_STATUS_TRDY_MSK));
    IOWR_ALTERA_AVALON_UART_TXDATA(base, byte);
}

RPLidar::RPLidar() : _uart_base(nullptr) {
    _currentMeasurement.distance = 0;
    _currentMeasurement.angle = 0;
    _currentMeasurement.quality = 0;
    _currentMeasurement.startBit = false;
}

RPLidar::~RPLidar() { end(); }

bool RPLidar::begin() {
    if (isOpen()) end();
    _uart_base = UART_BASE;
    init_timing();
    return true;
}

void RPLidar::end() {
    _uart_base = nullptr;
}

bool RPLidar::isOpen() { return _uart_base != nullptr; }

alt_u32 RPLidar::_sendCommand(alt_u8 cmd, const void *payload, size_t payloadsize) {
    rplidar_cmd_packet_t pkt = { RPLIDAR_CMD_SYNC_BYTE, cmd };
    alt_u8 checksum = 0;

    if (payloadsize && payload) {
        cmd |= RPLIDAR_CMDFLAG_HAS_PAYLOAD;
        pkt.cmd_flag = cmd;
    }

    uart_write_byte(_uart_base, pkt.syncByte);
    uart_write_byte(_uart_base, pkt.cmd_flag);

    if (cmd & RPLIDAR_CMDFLAG_HAS_PAYLOAD) {
        checksum ^= RPLIDAR_CMD_SYNC_BYTE ^ cmd ^ (payloadsize & 0xFF);
        for (size_t i = 0; i < payloadsize; ++i)
            checksum ^= ((alt_u8*)payload)[i];
        alt_u8 sizebyte = payloadsize;
        uart_write_byte(_uart_base, sizebyte);
        for (size_t i = 0; i < payloadsize; ++i)
            uart_write_byte(_uart_base, ((alt_u8*)payload)[i]);
        uart_write_byte(_uart_base, checksum);
    }
    return RESULT_OK;
}

alt_u32 RPLidar::_waitResponseHeader(rplidar_ans_header_t *header, alt_u32 timeout) {
    alt_u8 *buf = (alt_u8*)header;
    int pos = 0;
    alt_u32 start = get_ms();

    while (get_ms() - start <= timeout) {
        int byte = uart_read_byte(_uart_base);
        if (byte < 0) continue;

        if (pos == 0 && byte != RPLIDAR_ANS_SYNC_BYTE1) continue;
        if (pos == 1 && byte != RPLIDAR_ANS_SYNC_BYTE2) { pos = 0; continue; }

        buf[pos++] = byte;
        if (pos == sizeof(rplidar_ans_header_t)) return RESULT_OK;
    }
    return RESULT_OPERATION_TIMEOUT;
}

alt_u32 RPLidar::getHealth(rplidar_response_device_health_t &healthinfo, alt_u32 timeout) {
    if (!isOpen()) return RESULT_OPERATION_FAIL;
    if (_sendCommand(RPLIDAR_CMD_GET_DEVICE_HEALTH, NULL, 0) != RESULT_OK)
        return RESULT_OPERATION_FAIL;

    rplidar_ans_header_t header;
    if (_waitResponseHeader(&header, timeout) != RESULT_OK)
        return RESULT_OPERATION_TIMEOUT;
    if (header.type != RPLIDAR_ANS_TYPE_DEVHEALTH || header.size < sizeof(healthinfo))
        return RESULT_INVALID_DATA;

    alt_u8 *buf = (alt_u8*)&healthinfo;
    int pos = 0;
    alt_u32 start = get_ms();
    while (get_ms() - start <= timeout) {
        int byte = uart_read_byte(_uart_base);
        if (byte < 0) continue;
        buf[pos++] = byte;
        if (pos == sizeof(healthinfo)) return RESULT_OK;
    }
    return RESULT_OPERATION_TIMEOUT;
}

alt_u32 RPLidar::getDeviceInfo(rplidar_response_device_info_t &info, alt_u32 timeout) {
    if (!isOpen()) return RESULT_OPERATION_FAIL;
    if (_sendCommand(RPLIDAR_CMD_GET_DEVICE_INFO, NULL, 0) != RESULT_OK)
        return RESULT_OPERATION_FAIL;

    rplidar_ans_header_t header;
    if (_waitResponseHeader(&header, timeout) != RESULT_OK)
        return RESULT_OPERATION_TIMEOUT;
    if (header.type != RPLIDAR_ANS_TYPE_DEVINFO || header.size < sizeof(info))
        return RESULT_INVALID_DATA;

    alt_u8 *buf = (alt_u8*)&info;
    int pos = 0;
    alt_u32 start = get_ms();
    while (get_ms() - start <= timeout) {
        int byte = uart_read_byte(_uart_base);
        if (byte < 0) continue;
        buf[pos++] = byte;
        if (pos == sizeof(info)) return RESULT_OK;
    }
    return RESULT_OPERATION_TIMEOUT;
}

alt_u32 RPLidar::stop() {
    if (!isOpen()) return RESULT_OPERATION_FAIL;
    return _sendCommand(RPLIDAR_CMD_STOP, NULL, 0);
}

alt_u32 RPLidar::startScan(bool force, alt_u32 timeout) {
    if (!isOpen()) return RESULT_OPERATION_FAIL;
    stop();

    alt_u8 cmd = force ? RPLIDAR_CMD_FORCE_SCAN : RPLIDAR_CMD_SCAN;
    if (_sendCommand(cmd, NULL, 0) != RESULT_OK)
        return RESULT_OPERATION_FAIL;

    rplidar_ans_header_t header;
    if (_waitResponseHeader(&header, timeout) != RESULT_OK)
        return RESULT_OPERATION_TIMEOUT;
    if (header.type != RPLIDAR_ANS_TYPE_MEASUREMENT ||
        header.size < sizeof(rplidar_response_measurement_node_t))
        return RESULT_INVALID_DATA;

    return RESULT_OK;
}

alt_u32 RPLidar::waitPoint(alt_u32 timeout)
{
    rplidar_response_measurement_node_t node;
    alt_u8 *buf = (alt_u8*)&node;
    int pos = 0;
    alt_u32 start = get_ms();

    while (get_ms() - start <= timeout) {
        int byte = uart_read_byte(_uart_base);
        if (byte < 0) continue;

        // --- byte 0: S (bit0) and !S (bit1) must differ ---
        if (pos == 0) {
            if ((byte & 1) == ((byte >> 1) & 1)) {
                continue;               // invalid start pattern, drop
            }
            buf[pos++] = byte;
            continue;
        }

        // --- byte 1: check-bit must be 1 ---
        if (pos == 1) {
            if (!(byte & RPLIDAR_RESP_MEASUREMENT_CHECKBIT)) {
                pos = 0;                // malformed, restart
                continue;
            }
        }

        buf[pos++] = byte;

        if (pos == sizeof(node)) {
            _currentMeasurement.distance = node.distance_q2 / 4.0f;
            _currentMeasurement.angle    = (node.angle_q6_checkbit >>
                                            RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f;
            _currentMeasurement.quality  = node.sync_quality >>
                                            RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;
            _currentMeasurement.startBit = node.sync_quality &
                                            RPLIDAR_RESP_MEASUREMENT_SYNCBIT;
            return RESULT_OK;
        }
    }
    return RESULT_OPERATION_TIMEOUT;
}
