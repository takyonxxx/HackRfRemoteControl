#ifndef IHACKRFDATA_H
#define IHACKRFDATA_H
#include <stdint.h>

class IHackRFData
{
public:
    IHackRFData() {};
    ~IHackRFData() {};

    virtual int onData(int8_t* buffer, uint32_t length) = 0;
};

#endif // IHACKRFDATA_H
