#ifndef VIP_HELPER_H
#define VIP_HELPER_H

#include <stdint.h>
#include "kria_helper.h"



/* ==========================================================================
 * VIP (Vehicle/Inter-node Protocol) - OUTER, SYSTEM-WIDE FRAMING LAYER
 *
 * Boc them 1 lop BEN DUOI kria_message_t / protocol_parser.c hien co, giong
 * cach MAVLink boc msgid-based payload trong 1 envelope system_id/component_id
 * /CRC. Lop nay khong dinh nghia lai kria_message_t: moi handler trong
 * cmd_*.c va cmd_dispatcher.c giu nguyen 100%.
 *
 * Muc dich: cho phep dinh tuyen (routing) & nhan dien node khi he thong mo
 * rong ra nhieu board (khong chi 1 PS <-> 1 Host UART point-to-point nhu
 * hien tai), cong them lop kiem tra toan ven (CRC) o muc khung ngoai cung
 * truoc khi du lieu duoc dua vao protocol_parser tang trong.
 *
 * Module nay PURE C / HARDWARE-AGNOSTIC (giong protocol_parser.c, tx_queue.c):
 * khong include BSP header, khong tu goi driver vat ly. Viec "gui xuong tang
 * vat ly" duoc thuc hien qua 1 callback do caller cung cap (xem VIP_Send()),
 * de transport thuc te (UART hien tai, co the SPI/CAN/Ethernet sau nay) van
 * co the thay doi ma khong dung cham module nay, va module van unit-test
 * duoc off-target.
 * ========================================================================== */

#define VIP_SYNC_VALUE       0xEB90U
#define VIP_HEADER_LEN       9U    /* sizeof(vip_header_t) packed, ca 3 loai frame */
#define VIP_HEADER_CRC_LEN   2U
#define VIP_MSG_CRC_LEN      4U

/* Khung toi thieu: SYNC(2) + header(9) + headerCRC(2). Dung cho PING_STD/ACK_STD. */
#define VIP_MIN_FRAME_LEN    (2U + VIP_HEADER_LEN + VIP_HEADER_CRC_LEN)

/* So byte toi da cua vung "msg" (3-byte prefix seq/msgid/len + payload), khop
 * voi gioi han payload64[255] cua kria_message_t. */
#define VIP_MAX_MSG_LEN       (3U + 255U)

/* Buffer toi da 1 caller can cap phat de chua 1 frame da serialize (worst
 * case DATA_STD voi payload lon nhat). */
#define VIP_MAX_FRAME_LEN     (VIP_MIN_FRAME_LEN + VIP_MAX_MSG_LEN + VIP_MSG_CRC_LEN)

/* -------------------------------------------------------------------------
 * Frame types (vip_header_t.frameType)
 * ------------------------------------------------------------------------- */
typedef enum {
    VIP_FRAME_DATA_STD = 10U,  /* boc 1 kria_message_t application payload    */
    VIP_FRAME_PING_STD = 11U,  /* liveness probe, khong payload               */
    VIP_FRAME_ACK_STD  = 12U   /* phan hoi cho PING_STD, khong payload        */
} vip_frame_type_t;

/* control bit0: 1 = day la 1 REQUEST can phan hoi (hien chi PING_STD dung),
 * 0 = day la 1 ban tin phan hoi (khong can ai tra loi lai no nua). */
#define VIP_CTRL_PING_REQUEST_MASK   0x01U

/* -------------------------------------------------------------------------
 * Dinh danh node - bang dinh tuyen hardcode, phu hop so luong thiet bi it.
 * Override luc build (-D) neu firmware nay nap cho 1 board khac.
 * ------------------------------------------------------------------------- */
/* VIP_NODE_ID_SELF = node id cua CHINH chuong trinh dang chay file nay.
 * Ban sao nay nam trong ung dung Qt - ma ung dung Qt CHINH LA HOST - nen mac
 * dinh o day la 0x00 (= VIP_NODE_ID_HOST).
 *
 * SUA LOI (2026-10): truoc day mac dinh o day la 0x01 (id cua board, chep tu
 * phia firmware) va chi duoc "override" thanh 0x00 bang 1 #define dat trong
 * dashboardmanager.h. Nhung vip_helper.cpp la 1 translation unit RIENG, khong
 * include dashboardmanager.h -> ban than VIP_Process_Frame() van duoc bien
 * dich voi SELF = 0x01, bang dinh tuyen coi moi frame firmware gui cho host
 * (desId = 0x00) la FORWARD -> parseIncomingData() bo qua TOAN BO frame
 * DATA: ACK cua lenh AC/DC/ATT/Delay, telemetry ADC7928 (kich no), pho RF
 * "WAVE"... (chi PING/ACK_STD con chay vi duoc kiem tra truoc khi goi
 * VIP_Process_Frame()). Phat hien bang test end-to-end that (app <-> firmware
 * qua cong noi tiep ao). Dat mac dinh dung ngay tai day thi moi TU deu thay
 * cung 1 gia tri, khong phu thuoc thu tu include. */
#ifndef VIP_NODE_ID_SELF
#define VIP_NODE_ID_SELF        0x00U   /* ung dung Qt = host                 */
#endif
#define VIP_NODE_ID_HOST        0x00U   /* Host Qt dashboard                   */
#define VIP_NODE_ID_BOARD       0x01U   /* id co dinh cua board 5E-HTE, khong
                                          * bi anh huong boi override o tren  */
#define VIP_NODE_ID_BROADCAST   0xFFU

/* -------------------------------------------------------------------------
 * Cau truc tren day (packed 1-byte). Day la representation LOGIC trong bo
 * nho; ham Encode/Decode van serialize tung field mot cach explicit little-
 * endian (khong dua vao memcpy struct thuan tuy), de layout dung ngay ca
 * khi 1 cap toolchain/target khac khong cung endianness voi Cortex-A53 <->
 * x86 Host hien tai.
 * ------------------------------------------------------------------------- */
#pragma pack(push, 1)

typedef struct {
    uint8_t  frameType;   /* vip_frame_type_t                                  */
    uint8_t  headerSize;  /* luon = VIP_HEADER_LEN (9); dung de bao ve version  */
    uint8_t  control;     /* bit0 = VIP_CTRL_PING_REQUEST_MASK                  */
    uint16_t frameSeq;    /* bo dem rieng cua tung sender, tran tu nhien ve 0   */
    uint16_t dataLen;     /* so byte cua "msg" theo sau headerCRC; 0 = khong co */
    uint8_t  desId;
    uint8_t  srcId;
} vip_header_t;

typedef struct {
    uint16_t       SYNC;      /* VIP_SYNC_VALUE                                */
    vip_header_t   header;
    uint16_t       headerCRC; /* CRC16 tren 9 byte header                      */
    kria_message_t msg;       /* chi co y nghia khi header.dataLen > 0         */
    uint32_t       msg_crc;   /* CRC32 tren dung dataLen byte thuc te tren day */
} vip_msg_t;

#pragma pack(pop)

/* ==========================================================================
 * (1) ENCODING - dong goi 1 frame hoan chinh san sang gui di.
 * ========================================================================== */

/* Serialize 1 khung DATA_STD boc 1 kria_message_t da duoc build san (vi du
 * tu Protocol_Frame_Serialize / ACK cua cmd_dispatcher). `inner_msg`/
 * `inner_len` la byte da serialize cua kria_message_t (3 + payload_len),
 * KHONG PHAI con tro struct - nen ham nay khong phu thuoc cach tang trong
 * build ra chung.
 * Tra ve tong so byte da ghi vao `out`, hoac 0 neu loi (tham so sai / `out`
 * qua nho / inner_len vuot VIP_MAX_MSG_LEN). */
uint32_t VIP_Encode_Data(uint8_t des_id, uint8_t src_id, uint16_t frame_seq,
                         const uint8_t *inner_msg, uint16_t inner_len,
                         uint8_t *out, uint32_t out_cap);

/* Serialize 1 khung PING_STD (request, khong payload). */
uint32_t VIP_Encode_Ping(uint8_t des_id, uint8_t src_id, uint16_t frame_seq,
                         uint8_t *out, uint32_t out_cap);

/* Serialize 1 khung ACK_STD (reply, khong payload). Thuong goi voi
 * des_id/frame_seq lay tu chinh khung PING_STD dang duoc tra loi. */
uint32_t VIP_Encode_Ack(uint8_t des_id, uint8_t src_id, uint16_t frame_seq,
                        uint8_t *out, uint32_t out_cap);

/* Tra ve frameSeq tiep theo cho node nay va tang bo dem noi bo. Tran tu
 * nhien cua uint16_t (65535 -> 0) chinh la hanh vi reset-khi-tran mong muon. */
uint16_t VIP_NextTxSeq(void);

/* ==========================================================================
 * (4) DECODING FSM - state machine byte-by-byte, phong cach/chu ky giong
 * kria_decode() trong kria_helper.h.
 * ========================================================================== */

typedef enum {
    VIP_STATE_SYNC_1 = 0,
    VIP_STATE_SYNC_2,
    VIP_STATE_HEADER,
    VIP_STATE_HEADER_CRC,
    VIP_STATE_MSG_PAYLOAD,
    VIP_STATE_MSG_CRC
} vip_state_t;

typedef enum {
    VIP_ERR_NONE = 0,
    VIP_ERR_HEADER_CRC,
    VIP_ERR_MSG_CRC,
    VIP_ERR_BAD_HEADER_SIZE
} vip_decode_error_t;

typedef struct {
    uint8_t             msg_received; /* 1 dung 1 lan khi 1 frame day du + CRC hop le duoc rap xong */
    vip_state_t         parse_state;
    vip_decode_error_t  last_error;   /* set (va reset state) khi CRC/format sai */

    uint8_t  header_idx;
    uint8_t  header_raw[VIP_HEADER_LEN];

    uint8_t  crc_buf[4];   /* scratch dung chung cho ca header CRC(dung 2B) va msg CRC(dung 4B) */
    uint8_t  crc_idx;

    uint16_t payload_idx;               /* offset trong vung msg dai dataLen byte      */
    uint8_t  msg_raw[VIP_MAX_MSG_LEN];  /* luu tho de CRC32 kiem tra dung tung byte    */
} vip_status_t;

void    VIP_Decode_Init(vip_status_t *status);

/* Nap 1 byte nhan duoc vao state machine. Tra ve khac 0 dung 1 lan, tai byte
 * hoan tat 1 frame CRC hop le (giong quy uoc tra ve cua kria_decode());
 * `rxmsg` chi hop le doc tai thoi diem do. Khi CRC/format sai, frame bi drop
 * am tham va parser tu dong resync ve VIP_STATE_SYNC_1 - kiem tra
 * status->last_error neu can telemetry loi. */
uint8_t VIP_Decode(vip_msg_t *rxmsg, vip_status_t *status, uint8_t byte);

/* ==========================================================================
 * (2) NHAN DIEN BAN TIN, DINH TUYEN & AUTO-ACK
 * ========================================================================== */

typedef enum {
    VIP_ROUTE_LOCAL = 0,  /* dich la node nay - xu ly/tieu thu tai day        */
    VIP_ROUTE_FORWARD,    /* dich la 1 node khac da biet - can relay          */
    VIP_ROUTE_UNKNOWN     /* dich khong co trong bang dinh tuyen - drop       */
} vip_route_action_t;

/* Tra cuu dinh tuyen hardcode (bang nho, phu hop so thiet bi it hien tai). */
vip_route_action_t VIP_Route_Lookup(uint8_t des_id);

typedef enum {
    VIP_PROCESS_IGNORE = 0,   /* khong phai cho ta / khong co gi de lam       */
    VIP_PROCESS_FORWARD,      /* caller nen relay nguyen byte tho di noi khac */
    VIP_PROCESS_REPLY_READY,  /* 1 ACK tu dong da san sang trong `reply_buf`  */
    VIP_PROCESS_DATA_READY    /* frame la DATA_STD cho node nay; rxmsg->msg da
                                * la 1 kria_message_t san sang dua vao pipeline
                                * hien co (Cmd_Dispatch_Frame)                */
} vip_process_result_t;

/* Voi 1 frame da giai ma xong (tu VIP_Decode()), nhan dien loai, ap dung
 * dinh tuyen, va - voi 1 PING_STD request cho node nay - tu dong build san
 * ACK_STD phan hoi. Khong bao gio dung toi BSP/transport API. */
vip_process_result_t VIP_Process_Frame(const vip_msg_t *rxmsg,
                                       uint8_t *reply_buf, uint32_t reply_cap,
                                       uint32_t *reply_len);

/* ==========================================================================
 * (3) BAN GIAO XUONG TANG VAT LY
 * ========================================================================== */

/* Ham "sink" byte khong-block do caller cung cap, vi du BSL_UART_WriteNonBlocking.
 * Tra ve so byte thuc te da nhan (co the < len). Giu day la 1 con tro ham
 * duoc "tiem" vao, thay vi #include thang 1 BSP header o day, chinh la dieu
 * giu vip_helper hardware-agnostic va test duoc. */
typedef uint32_t (*vip_write_fn_t)(const uint8_t *data, uint32_t len);

/* Tien ich mot-buoc: encode + ban giao xuong tang vat ly trong 1 lan goi.
 * Tra ve so byte `write_fn` da nhan (0 neu loi encode/tham so). */
uint32_t VIP_Send(const uint8_t *frame, uint32_t frame_len, vip_write_fn_t write_fn);

/* ==========================================================================
 * CRC primitives - expose ra ngoai phong khi caller can validate doc lap
 * (test, tooling). CRC16 phu header; CRC32 phu vung msg (du lon cho payload
 * toi da 258 byte).
 * ========================================================================== */
uint16_t VIP_CRC16(const uint8_t *data, uint32_t len);
uint32_t VIP_CRC32(const uint8_t *data, uint32_t len);




#endif /* VIP_HELPER_H */