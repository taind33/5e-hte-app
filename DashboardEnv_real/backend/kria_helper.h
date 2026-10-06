#ifndef KRIA_HELPER_H
#define KRIA_HELPER_H

#include <QDebug>
#include <cstdint>
#include <cstring>

#define KRIA_MAX_PAYLOAD_LEN 255


#pragma pack(push, 1)

/* LEGACY attenuator payload (6 bytes, whole dB). No longer sent by this app
 * - kept only so the struct definitions stay identical to the firmware's
 * src/kria_helper.h, whose HandleAtt() still accepts it from older builds. */
typedef struct __annuation_msg {
    char msg[4];            // "ATT"
    uint8_t att_number;     // 1..3
    uint8_t value;          // whole dB
} annuation_msg_t;

/* Attenuator payload actually sent (msgid 38, 7 bytes) - 0.25 dB resolution.
 * F1956 = 7-bit, 0.25 dB/step, 0..31.75 dB (firmware board_config.h
 * ATT_STEP_DB / ATT_MAX_STEPS, device_attenuator.c), so the wire carries the
 * STEP COUNT: dB = value_qdb * 0.25, valid 0..127. Firmware decodes it in
 * HandleAtt() (cmd_dispatcher.c) by payload length. The length differs from
 * the legacy struct on purpose: an older firmware that only knows the 6-byte
 * format NACKs this one (length check) instead of misreading the step count
 * as whole dB (= 4x the requested attenuation). */
typedef struct __annuation_q_msg {
    char msg[4];            // "ATQ"
    uint8_t att_number;     // 1..3
    uint16_t value_qdb;     // 0.25 dB steps, 0..127
} annuation_q_msg_t;


typedef struct __timedelay_msg {
    char msg[4];
    uint16_t value;         /* delay in ns: 0/100/270/400 - must stay
                                uint16_t end-to-end (400 does not fit a
                                uint8_t); see applyDelayTime() */
} timedelay_msg_t;


typedef struct __fwi_msg {
    char msg[4];
    char isAC[3];
    uint16_t voltage;
    uint16_t frequency;     /* AC: integer Hz.
                                DC: reused field carries current, fixed-point
                                x100 (centiamps) so 2.35 A -> 235 - see
                                setDcPower() and firmware's HandleFwi() which
                                divides by 100.0f for the DC case. */
    uint8_t state;
} fwi_msg_t;


typedef struct __cmd_msg {
    char msg[64];
    uint8_t value[4];
} cmd_msg_t;

/* ADC7928 8-channel telemetry (device -> host only), msgid 40. Byte-for-byte
 * match of src/kria_helper.h's adc7928_msg_t on the firmware side - added so
 * the app can decode CmdDispatcher_SendAdcTelemetry() reports. */
typedef struct __adc7928_msg {
    char msg[4];
    uint16_t raw_code[8];
} adc7928_msg_t;

/* AD9361 RX configuration report (device -> host), msgid 39 + magic "RFST".
 * Byte-for-byte identical to rf_status_msg_t in firmware src/kria_helper.h
 * (sent by CmdDispatcher_SendRfStatus() every ~3 s, values cached once at
 * AD9361 bring-up). Used to put a real frequency axis under the "WAVE"
 * spectrum and to show which RF frequency the receiver is tuned to. */
typedef struct __rf_status_msg {
    char     msg[4];        // "RFST" (no NUL)
    uint64_t rx_lo_hz;
    uint32_t rx_fs_hz;
    uint32_t rx_bw_hz;
    int32_t  rx1_gain_db;
    uint8_t  gain_mode;     // 0=MGC 1=fast AGC 2=slow AGC 3=hybrid AGC
    uint8_t  fft_size;      // points per WAVE frame
    uint8_t  adc_bits;      // 12
    uint8_t  window;        // 0 = rectangular
} rf_status_msg_t;

// ── Báo cáo sức khoẻ board (msgid 39, magic "DIAG", ~3 s/lần và ngay khi
// app (re)connect) - byte-for-byte giống firmware src/kria_helper.h.
typedef struct __board_diag_msg {
    char     msg[4];          // "DIAG"
    uint8_t  version;         // 1
    uint8_t  ad9361_ok;
    uint8_t  rf_stream_ok;
    uint8_t  adc7928_ok;
    uint8_t  att_ok;
    uint8_t  odly_ok;
    uint8_t  ac_uart_ok;
    uint8_t  dc_uart_ok;
    int32_t  ad9361_status;   // mã trả về ad9361_init(), 0 = OK
    uint32_t uptime_s;
    uint32_t dma_ok;
    uint32_t dma_timeout;
    uint32_t dma_start_fail;
    uint16_t rx1_peak;        // max |I|,|Q| lần chụp gần nhất, 2047 = đầy thang (bão hoà)
    uint16_t reserved;
    uint32_t pc_rx_bytes;     // board nhận được từ PC
    uint32_t pc_frames_ok;
    uint32_t pc_frames_bad;
    uint32_t pc_rx_dropped;
    char     build[24];       // thời điểm build firmware
} board_diag_msg_t;

// "LOGT" (msgid 39): 4 byte magic, 1 byte mức, rồi chuỗi (không NUL).
#define BOARD_LOG_INFO          0U
#define BOARD_LOG_WARN          1U
#define BOARD_LOG_ERROR         2U

// ── Đọc ngược nguồn chuẩn AC/DC (byte-for-byte giống firmware src/kria_helper.h)
#define PWR_DEV_AC              1U
#define PWR_DEV_DC              2U

#define PWR_REASON_READ         0U  // msgid 41 (nút refresh / F5)
#define PWR_REASON_SET          1U  // xác nhận lệnh msgid 35/36
#define PWR_REASON_RESET        2U  // xác nhận "RESET" (msgid 39)
#define PWR_REASON_RAW          3U  // lệnh thô từ Terminal (msgid 43 -> 44)

#define PWR_RESULT_OK           0U
#define PWR_RESULT_NO_REPLY     1U  // nguồn không trả lời (cáp/baud/nguồn tắt)
#define PWR_RESULT_DEV_ERROR    2U  // SYST:ERR? báo lỗi (dev_err)
#define PWR_RESULT_OUT_MISMATCH 3U  // OUTP? khác trạng thái yêu cầu
#define PWR_RESULT_NOT_READY    4U  // UART nguồn trên board chưa khởi tạo
#define PWR_RESULT_ABORTED      5U  // bị lệnh mới hơn thay thế
#define PWR_RESULT_BAD_REPLY    6U  // phản hồi không đọc được
#define PWR_RESULT_BUSY         7U  // nguồn đang bận giao dịch khác

#define PWR_VALID_VOLT          0x01U
#define PWR_VALID_CURR          0x02U
#define PWR_VALID_FREQ          0x04U
#define PWR_VALID_OUTPUT        0x08U
#define PWR_VALID_ERR           0x10U

// msgid 41, app -> board: đọc 1 nguồn ngay.
typedef struct __pwr_query_msg {
    char    msg[4];         // "PWQ" + NUL
    uint8_t dev;            // PWR_DEV_AC / PWR_DEV_DC
    uint8_t flags;          // 0
} pwr_query_msg_t;

// msgid 42, board -> app: kết quả 1 lần trao đổi với nguồn.
typedef struct __pwr_status_msg {
    char     msg[4];        // "PWS" + NUL
    uint8_t  dev;
    uint8_t  reason;        // PWR_REASON_*
    uint8_t  result;        // PWR_RESULT_*
    uint8_t  output;        // 0 OFF, 1 ON, 0xFF chưa biết
    uint8_t  valid;         // PWR_VALID_*
    uint8_t  req_seq;
    int16_t  dev_err;       // mã lỗi SCPI (AC)
    int32_t  volt_mv;
    int32_t  curr_ma;
    int32_t  freq_mhz;
} pwr_status_msg_t;

// msgid 43, app -> board: lệnh văn bản thô gửi nguyên văn tới 1 nguồn
// ("ac raw ..." / "dc raw ..." trong Terminal). Có '?' = câu hỏi, câu trả
// lời về bằng msgid 44.
#define PWR_RAW_TEXT_MAX        64U
typedef struct __pwr_raw_msg {
    char    msg[4];                     // "RAW" + NUL
    uint8_t dev;
    uint8_t len;
    char    text[PWR_RAW_TEXT_MAX];
} pwr_raw_msg_t;

// msgid 44, board -> app.
#define PWR_RAW_REPLY_MAX       96U
typedef struct __pwr_raw_reply_msg {
    char    msg[4];                     // "RAWR"
    uint8_t dev;
    uint8_t result;
    uint8_t output;                     // 0/1, 0xFF chưa biết (sau lệnh ghi AC)
    uint8_t valid;                      // PWR_VALID_OUTPUT / PWR_VALID_ERR
    int16_t dev_err;
    uint8_t len;
    char    text[PWR_RAW_REPLY_MAX];
} pwr_raw_reply_msg_t;

typedef struct _kria_message {
    uint8_t seq;
    uint8_t msgid;
    uint8_t len;
    uint8_t payload64[KRIA_MAX_PAYLOAD_LEN];
} kria_message_t;

#pragma pack(pop)

/* Wire-size guards - must match the other end of the link exactly (Qt
 * backend/kria_helper.h <-> firmware src/kria_helper.h). A mismatch here is a
 * protocol break, so fail the build instead of misparsing at runtime. */
static_assert(sizeof(annuation_msg_t) == 6, "annuation_msg_t wire size changed");
static_assert(sizeof(annuation_q_msg_t) == 7, "annuation_q_msg_t wire size changed");
static_assert(sizeof(timedelay_msg_t) == 6, "timedelay_msg_t wire size changed");
static_assert(sizeof(fwi_msg_t) == 12, "fwi_msg_t wire size changed");
static_assert(sizeof(cmd_msg_t) == 68, "cmd_msg_t wire size changed");
static_assert(sizeof(adc7928_msg_t) == 20, "adc7928_msg_t wire size changed");
static_assert(sizeof(rf_status_msg_t) == 28, "rf_status_msg_t wire size changed");
static_assert(sizeof(board_diag_msg_t) == 76, "board_diag_msg_t wire size changed");
static_assert(sizeof(pwr_raw_msg_t) == 70, "pwr_raw_msg_t wire size changed");
static_assert(sizeof(pwr_raw_reply_msg_t) == 107, "pwr_raw_reply_msg_t wire size changed");
static_assert(sizeof(pwr_query_msg_t) == 6, "pwr_query_msg_t wire size changed");
static_assert(sizeof(pwr_status_msg_t) == 24, "pwr_status_msg_t wire size changed");


typedef enum {
    KRIA_STATE_IDLE = 0,
    KRIA_STATE_GOT_SEQ,
    KRIA_STATE_GOT_MSGID,
    KRIA_STATE_GOT_PAYLOAD
} kria_state_t;

typedef struct __kria_status {
    uint8_t msg_received;
    kria_state_t parse_state;
    uint8_t packet_idx;
} kria_status_t;

static inline uint8_t kria_decode(kria_message_t *rxmsg, kria_status_t *status, uint8_t c)
{
    status->msg_received = false;
    switch (status->parse_state) {
    case KRIA_STATE_IDLE:
        rxmsg->seq = c;
        status->parse_state = KRIA_STATE_GOT_SEQ;
        break;
    case KRIA_STATE_GOT_SEQ:
        rxmsg->msgid = c;
        status->parse_state = KRIA_STATE_GOT_MSGID;
        break;
    case KRIA_STATE_GOT_MSGID:
        rxmsg->len = c;
        status->packet_idx = 0;
        /* len is uint8_t (0..255) and KRIA_MAX_PAYLOAD_LEN is 255, so
         * "len > KRIA_MAX_PAYLOAD_LEN" can never be true - matches the
         * firmware-side kria_helper.h, which drops the same tautological
         * branch (gcc -Wtype-limits flags it as dead code). */
        if (rxmsg->len == 0) {
            status->msg_received = true;
            status->parse_state = KRIA_STATE_IDLE;
        } else {
            status->parse_state = KRIA_STATE_GOT_PAYLOAD;
        }
        break;
    case KRIA_STATE_GOT_PAYLOAD:
        rxmsg->payload64[status->packet_idx++] = c;
        if (status->packet_idx >= rxmsg->len) {
            status->msg_received = true;
            status->parse_state = KRIA_STATE_IDLE;
        }
        break;
    default:
        status->parse_state = KRIA_STATE_IDLE;
        break;
    }
    return status->msg_received;
}

static inline uint16_t kria_msg_to_send_buffer(uint8_t *buff, const kria_message_t *msg) {
    buff[0] = msg->seq;
    buff[1] = msg->msgid;
    buff[2] = msg->len;
    if (msg->len > 0) {
        std::memcpy(&buff[3], msg->payload64, msg->len);
    }
    return 3 + msg->len;
}


static inline void kria_msg_output_encode(int seq, int msg_id, const void *p_struct_data, kria_message_t *msg) {
    msg->seq = (uint8_t)seq;
    msg->msgid = (uint8_t)msg_id;
    msg->len = 0;

    if (p_struct_data == nullptr) return;


    switch (msg_id) {
    case 35:   // AC power - thêm mới, cùng struct với 36
    case 36:
        msg->len = sizeof(fwi_msg_t);
        break;
    case 37:
        msg->len = sizeof(timedelay_msg_t);
        break;
    case 38:
        msg->len = sizeof(annuation_q_msg_t);
        break;
    case 39:
        msg->len = sizeof(cmd_msg_t);
        break;
    case 40:
        msg->len = sizeof(adc7928_msg_t);
        break;
    case 41:
        msg->len = sizeof(pwr_query_msg_t);
        break;
    case 42:
        msg->len = sizeof(pwr_status_msg_t);
        break;
    case 43:
        msg->len = sizeof(pwr_raw_msg_t);
        break;
    case 44:
        msg->len = sizeof(pwr_raw_reply_msg_t);
        break;
    default:
        return;
    }

    std::memcpy(msg->payload64, p_struct_data, msg->len);
}

#endif // KRIA_HELPER_H