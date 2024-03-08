#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DEFAULT_SAMPLE_RATE             48*1000
#define DEFAULT_FREQUENCY               100*1000*1000

typedef enum {
    DEMOD_AM,
    DEMOD_WFM,
} Demod;

typedef enum {
    HZ,
    KHZ,
    MHZ,
    GHZ
} FreqMod;

#endif // CONSTANTS_H
