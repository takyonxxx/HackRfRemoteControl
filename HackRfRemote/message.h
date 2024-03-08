#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#define mHeader     0xa0 //Fix header
#define mWrite      0x01 //Write request
#define mRead       0x02 //Read request
#define mData       0xa0 //data
#define mGetFreq    0xb0
#define mSetFreq    0xb1
#define mGetFreqMod 0xb2
#define mSetFreqMod 0xb3
#define mIncFreq    0xb4
#define mDecFreq    0xb5
#define mGetPtt     0xc0
#define mSetPtt     0xc1
#define mGetDeMod   0xd0
#define mSetDeMod   0xd1
#define mGetIp      0xe0
#define mSetIp      0xe1
#define MaxPayload  1024

//message len max is 256, header, command, rw and cheksum total len is 8, therefore payload max len is 248
//max input bluetooth buffer in this chip allows a payload max 0x38

typedef struct {
    uint8_t header;
    uint8_t len;
    uint8_t rw;
    uint8_t command;
    uint8_t data[MaxPayload];
    uint8_t CheckSum[2];
    uint32_t data32;
} MessagePack;

class Message
{
public:
    Message();
    bool parse(uint8_t *dataUART, uint8_t size, MessagePack *message);
    uint8_t create_pack(uint8_t RW,uint8_t command, QByteArray dataSend, uint8_t *dataUART);

};

#endif // MESSAGE_H
