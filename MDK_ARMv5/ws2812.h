#include "ht32.h"

#define WS_RELOAD			(60)											// (48000000/800000)	
#define WS_PRESCALER	0
#define WS_PIXEL			83														// LED pixels, music(80) + mode(3)
#define WS_LOGIC_1		38														// High Bit Duty
#define WS_LOGIC_0		19														// Low Bit Duty
#define	WS_24BITS			24														// Number of Bits of each LED
#define WS_BLOCK			((WS_PIXEL + 3) * WS_24BITS)	// PDMA transfer blocks
#define WS_GENERAL		80
#define WS_GUIDE			81
#define WS_GAME				82
#define WS_EXIT				10
#define WS_BLINK			400	

void wsInit(void);

void wsAFIOConfig(void);

void wsPWMConfig(void);

void wsPDMAConfig(void);

void wsNVICConfig(void);

void wsSetColor(u8 pixelNum, u8 red, u8 green, u8 blue);

void wsShow(void);

void wsClearAll(void);

void wsClearBetween(u8 startPixelNum, u8 endPixelNum);

void wsBlinkAll(u32 wait);

void wsPrintBuffer(void);

void wsSetLeftArrow(u8 red, u8 green, u8 blue);

void wsSetRightArrow(u8 red, u8 green, u8 blue);

void wsSetNumber(u8 number,u8 red, u8 green, u8 blue);

void wsSetMusicStatus(bool play, u8 lightValue);
