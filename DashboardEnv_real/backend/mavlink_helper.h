#ifndef MAVLINK_HELPER_H
#define MAVLINK_HELPER_H

#include <QDebug>
#include <bits/stdc++.h>

#define X25_INIT_CRC 0xffff
#define X25_VALIDATE_CRC 0xf0b8

#ifndef MAVLINK_CRC_EXTRA
#define MAVLINK_CRC_EXTRA 1
#endif

#define MAVLINK_STX 254
#define MAVLINK_MAX_PAYLOAD_LEN 255

#define MAVLINK_MESSAGE_CRCS {155, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 253, 42, 203, 237, 51, 186, 169, 41, 60, 71, 40, 252, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 235, 0, 0, 209, 197, 173, 155, 251, 0, 18, 213, 0, 245, 215, 144, 93, 106, 217, 55, 0, 251, 52, 0, 163, 0, 0, 0, 161, 148, 0, 0, 0, 0, 128, 0, 134, 0, 0, 0, 89, 9, 178, 0, 0, 0, 0, 0, 0, 0, 106, 142, 209, 80, 0, 0, 0, 0, 0, 83, 13, 238}

#if MAVLINK_CRC_EXTRA
    #ifndef MAVLINK_MESSAGE_CRC_
        static const uint8_t mavlink_message_crcs_[256] = MAVLINK_MESSAGE_CRCS;
        #define MAVLINK_MESSAGE_CRC_(msgid) mavlink_message_crcs_[msgid]
    #endif
#endif

typedef struct _mavlink_message  {
    uint16_t checksum;
    uint8_t header;
    uint8_t len;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint8_t msgid;
    uint8_t payload64[MAVLINK_MAX_PAYLOAD_LEN];
    uint8_t checksum_calculator[2];
}mavlink_message_t;

#define _MAV_PAYLOAD(msg) ((const char *)(&((msg)->payload64[0])))
#define _MAV_PAYLOAD_NON_CONST(msg) ((char *)(&((msg)->payload64[0])))

typedef enum {
    MAVLINK_STATE_IDLE = 0,
    MAVLINK_STATE_GOT_STX,
    MAVLINK_STATE_GOT_LENGTH,
    MAVLINK_STATE_GOT_SEQ,
    MAVLINK_STATE_GOT_SYSID,
    MAVLINK_STATE_GOT_COMPID,
    MAVLINK_STATE_GOT_MSGID,
    MAVLINK_STATE_GOT_PAYLOAD,
    MAVLINK_STATE_GOT_CRC,
    MAVLINK_STATE_GOT_BAD_CRC
} mavlink_state_t;

typedef struct __mavlink_status {
    uint8_t msg_received;               ///< Number of received messages
    mavlink_state_t parse_state;        ///< Parsing state machine
    uint8_t packet_idx;                 ///< Index in current packet
} mavlink_status_t;


static inline void crc_accumulate (uint8_t data, uint16_t *crcAccum) {
    int8_t tmp;
    tmp = data ^ (uint8_t)(*crcAccum & 0xFF);
    tmp ^= (tmp<<4);
    *crcAccum = (*crcAccum >> 8) ^ (((uint8_t)tmp)<< 8) ^ (((uint8_t)tmp) << 3) ^ (((uint8_t)tmp) >> 4);
}
static inline void crc_init (uint16_t *crcAccum) {
    *crcAccum =  X25_INIT_CRC;
}
static inline void mavlink_start_checksum (mavlink_message_t *msg) {
    uint16_t crcTmp =0;
    crc_init(&crcTmp);
    msg->checksum = crcTmp;
}
static inline void mavlink_update_checksum (mavlink_message_t *msg, uint8_t c) {
    uint16_t checksum = msg->checksum;
    crc_accumulate (c, &checksum);
    msg->checksum = checksum;
}

static inline uint8_t mavlink_decode(mavlink_message_t *rxmsg, mavlink_status_t *status, uint8_t c)
{
    switch (status->parse_state) {
    case MAVLINK_STATE_IDLE:
    case MAVLINK_STATE_GOT_STX:
//        qDebug () << "MAVLINK_STATE_GOT_STX";
        if (c == MAVLINK_STX) {
            rxmsg->len = 0;
            rxmsg->header = c;
            mavlink_start_checksum(rxmsg);
            status->parse_state = MAVLINK_STATE_GOT_LENGTH;
        }
        break;
    case MAVLINK_STATE_GOT_LENGTH:
//        qDebug () << "MAVLINK_STATE_GOT_LENGTH";
        rxmsg->len = c;
        status->packet_idx = 0;
        status->msg_received = false;
        mavlink_update_checksum(rxmsg,c);
        status->parse_state = MAVLINK_STATE_GOT_SEQ;
        break;
    case MAVLINK_STATE_GOT_SEQ:
//        qDebug () << "MAVLINK_STATE_GOT_SEQ";
        rxmsg->seq = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_STATE_GOT_SYSID;
        break;
    case MAVLINK_STATE_GOT_SYSID:
//        qDebug () << "MAVLINK_STATE_GOT_SYSID";
        rxmsg->sysid = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_STATE_GOT_COMPID;
        break;
    case MAVLINK_STATE_GOT_COMPID:
//        qDebug () << "MAVLINK_STATE_GOT_COMPID";
        rxmsg->compid = c;
        mavlink_update_checksum(rxmsg, c);
        status->parse_state = MAVLINK_STATE_GOT_MSGID;
        break;
    case MAVLINK_STATE_GOT_MSGID:
//        qDebug () << c <<"MAVLINK_STATE_GOT_MSGID";
        rxmsg->msgid = c;
        mavlink_update_checksum(rxmsg, c);
        if (rxmsg->len == 0)
        {
            status->parse_state = MAVLINK_STATE_GOT_CRC;
        }
        else
        {
            status->parse_state = MAVLINK_STATE_GOT_PAYLOAD;
        }
        break;
    case MAVLINK_STATE_GOT_PAYLOAD:
//        qDebug () << "MAVLINK_STATE_GOT_PAYLOAD";
        _MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx++] = (char)c;
        mavlink_update_checksum(rxmsg, c);
        if (status->packet_idx == rxmsg->len)
        {
            status->parse_state = MAVLINK_STATE_GOT_CRC;
        }
        break;
    case MAVLINK_STATE_GOT_CRC:
//        qDebug () << "MAVLINK_STATE_GOT_CRC";
        #if MAVLINK_CRC_EXTRA
            mavlink_update_checksum(rxmsg, MAVLINK_MESSAGE_CRC_(rxmsg->msgid));
        #endif
//        qDebug () <<c<< (rxmsg->checksum & 0xFF);
        if (c != (rxmsg->checksum & 0xFF)) {
            qDebug () <<rxmsg->msgid <<(rxmsg->checksum & 0xFF) << c <<"Bad CRC1";
//            qDebug() << rxmsg->checksum << c
            status->msg_received = false;
            status->parse_state = MAVLINK_STATE_IDLE;
        } else {
            status->parse_state = MAVLINK_STATE_GOT_BAD_CRC;
        }
        break;
    case MAVLINK_STATE_GOT_BAD_CRC:
//        qDebug () << c << (rxmsg->checksum >> 8 );
        if (c != (rxmsg->checksum >> 8)) {
            // got a bad CRC message
            qDebug () <<"Bad CRC2";
            status->msg_received = false;
            status->parse_state = MAVLINK_STATE_IDLE;
        } else {
            // Successfully got message
            status->msg_received = true;
        }
            status->parse_state = MAVLINK_STATE_IDLE;
        break;
    default:
        break;
    }
    return status->msg_received;
}
static inline uint16_t mavlink_msg_to_send_buffer(uint8_t *buff,const mavlink_message_t *msg) {
    buff[0]= msg->header;
    buff[1]= msg->len;
    buff[2]= msg->seq;
    buff[3]= msg->sysid;
    buff[4]= msg->compid;
    buff[5]= msg->msgid;
    std::memcpy(&buff[6], _MAV_PAYLOAD(msg), msg->len);
    buff[6+msg->len]= msg->checksum & 0xFF;
    buff[7+msg->len]= msg->checksum >> 8;
    return 8 + msg->len;
}
static inline void mavlink_msg_output_encode(int seq, int msg_id, int msg_payload, mavlink_message_t *msg) {
    msg->header = MAVLINK_STX;
    mavlink_start_checksum(msg);
    msg->len = sizeof (msg_payload);
    mavlink_update_checksum(msg,msg->len);
    msg->seq = seq;
    mavlink_update_checksum(msg,msg->seq);
    msg->sysid = 0x03;
    mavlink_update_checksum(msg,msg->sysid);
    msg->compid = 0x04;
    mavlink_update_checksum(msg,msg->compid);
    msg->msgid = (uint8_t)msg_id;
    mavlink_update_checksum(msg,msg->msgid);
    msg->payload64[0] = (uint32_t)msg_payload & 0x000000ff;
    mavlink_update_checksum(msg,msg->payload64[0]);
    msg->payload64[1] = ((uint32_t)msg_payload>>8) & 0x0000ff;
    mavlink_update_checksum(msg,msg->payload64[1]);
    msg->payload64[2] = ((uint32_t)msg_payload>>16 & 0x00ff);
    mavlink_update_checksum(msg,msg->payload64[2]);
    msg->payload64[3] = ((uint32_t)msg_payload>>24);
    mavlink_update_checksum(msg,msg->payload64[3]);
    mavlink_update_checksum(msg, MAVLINK_MESSAGE_CRC_(msg->msgid));
}
#endif // MAVLINK_HELPER_H
