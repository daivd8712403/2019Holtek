#include "stdlib.h"
#include "ht32.h"
#include "ws2812.h"

// Data Type Definitions
TM_TimeBaseInitTypeDef 	TM_TimeBaseInitStructure;
TM_OutputInitTypeDef 		TM_OutputInitStructure;
PDMACH_InitTypeDef 			PDMACH_InitStructure;
HT_PDMA_TypeDef 				PDMA_TEST_Struct;

static u32 WS2812[WS_PIXEL + 2][WS_24BITS];
static u8 ws_i, ws_j;
extern bool didSetColor;

void wsInit(void) {
	wsAFIOConfig();
	wsPWMConfig();
	wsNVICConfig();
	MCTM_CHMOECmd(HT_MCTM0, ENABLE);
	for(ws_i = 0; ws_i < WS_24BITS; ws_i++) {
		WS2812[WS_PIXEL	+	0][ws_i] = 0;
		WS2812[WS_PIXEL + 1][ws_i] = 0;
		WS2812[WS_PIXEL + 2][ws_i] = 0;
	}
	wsClearAll();
}

void wsAFIOConfig(void) {
	AFIO_GPxConfig(GPIO_PB, AFIO_PIN_0, AFIO_MODE_4); 	//Port B -> Pin  0  >>>  TM_CH_1
}

void wsPWMConfig(void) {
	TM_TimeBaseInitStructure.CounterReload = WS_RELOAD;
	TM_TimeBaseInitStructure.Prescaler = WS_PRESCALER;
	TM_TimeBaseInitStructure.RepetitionCounter = 0;
	TM_TimeBaseInitStructure.CounterMode = TM_CNT_MODE_UP;
	TM_TimeBaseInitStructure.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;
	TM_TimeBaseInit(HT_MCTM0, &TM_TimeBaseInitStructure);
	TM_ClearFlag(HT_MCTM0, TM_FLAG_UEV);
	TM_Cmd(HT_MCTM0, ENABLE);
	
	TM_OutputInitStructure.Channel = TM_CH_1;
	TM_OutputInitStructure.OutputMode = TM_OM_PWM1;
	TM_OutputInitStructure.Control = TM_CHCTL_ENABLE;
	TM_OutputInitStructure.ControlN = TM_CHCTL_DISABLE;
	TM_OutputInitStructure.Polarity = TM_CHP_NONINVERTED;
	TM_OutputInitStructure.PolarityN = TM_CHP_NONINVERTED;
	TM_OutputInitStructure.IdleState = MCTM_OIS_LOW;
	TM_OutputInitStructure.IdleStateN = MCTM_OIS_HIGH;
	TM_OutputInitStructure.Compare = 0;
	TM_OutputInit(HT_MCTM0, &TM_OutputInitStructure);
}

void wsPDMAConfig(void) {
	PDMACH_InitStructure.PDMACH_SrcAddr = (u32)WS2812;
	PDMACH_InitStructure.PDMACH_DstAddr = (u32)&(HT_MCTM0 -> CH1CCR);
	PDMACH_InitStructure.PDMACH_BlkCnt = WS_BLOCK;
	PDMACH_InitStructure.PDMACH_BlkLen = 1;
	PDMACH_InitStructure.PDMACH_DataSize = WIDTH_32BIT;
	PDMACH_InitStructure.PDMACH_Priority = VH_PRIO;
	PDMACH_InitStructure.PDMACH_AdrMod = SRC_ADR_LIN_INC | DST_ADR_FIX | AUTO_RELOAD;
	PDMA_Config(PDMA_CH5, &PDMACH_InitStructure);
	PDMA_ClearFlag(PDMA_CH5, PDMA_FLAG_TC);
	PDMA_IntConfig(PDMA_CH5, PDMA_INT_TC, ENABLE);
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, ENABLE);
}

void wsNVICConfig(void) {
	NVIC_EnableIRQ(PDMACH2_5_IRQn);	// Enable PDMA Channel 2 to 5 interrupt
}

void wsSetColor(u8 pixelNum, u8 red, u8 green, u8 blue) {
	u8 color_bit;
	didSetColor = FALSE;
	for (color_bit = 0; color_bit < 8; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (green >> (7 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	for (color_bit = 7; color_bit < 16; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (red >> (15 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	for (color_bit = 15; color_bit < 24; color_bit += 1) {
		WS2812[pixelNum][color_bit] = (blue >> (23 - color_bit) & 1) == 1 ? WS_LOGIC_1 : WS_LOGIC_0;
	}
	didSetColor = TRUE;
}

void wsShow(void) {
	wsPDMAConfig();
	PDMA_ClearFlag(PDMA_CH5, PDMA_FLAG_TC);												// Clear PDMA_FLAG_TC of PDMA_CH5 before transferring data
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, ENABLE);
	TM_Cmd(HT_MCTM0, ENABLE);
	PDMA_EnaCmd(PDMA_CH5, ENABLE);
	while (PDMA_GetFlagStatus(PDMA_CH5, PDMA_FLAG_TC) == RESET);	// Wait for transferring data complete
	TM_PDMAConfig(HT_MCTM0, TM_PDMA_UEV, DISABLE);
	TM_Cmd(HT_MCTM0, DISABLE);
	PDMA_EnaCmd(PDMA_CH5, DISABLE);
}

void wsClearAll(void) {
	for(ws_i = 0; ws_i < WS_PIXEL; ws_i++) wsSetColor(ws_i, 0, 0, 0);
}

void wsClearBetween(u8 startPixelNum, u8 endPixelNum) {
	for(ws_i = startPixelNum; ws_i <= endPixelNum; ws_i++) wsSetColor(ws_i, 0, 0, 0);
}

void wsBlinkAll(u32 wait) {
	wait *= 10000;
	for(ws_i = 0; ws_i < WS_PIXEL; ws_i++) wsSetColor(ws_i, 50, 50, 50);
	while(wait--);
	wsClearAll();
}

void wsPrintBuffer(void) {
	for(ws_i = 0; ws_i < WS_PIXEL; ws_i++) {
		printf("WS2812[%d]: ", ws_i);
		for(ws_j = 0; ws_j < WS_24BITS; ws_j++) {
			if(ws_j == 8 || ws_j == 16) printf(" ");
			printf("%d ", (WS2812[ws_i][ws_j] == 38)? 1: 0 );
		}
		printf("\r\n");
	}
}

void wsSetLeftArrow(u8 red, u8 green, u8 blue) {
	wsSetColor(57, red, green, blue);
	wsSetColor(64, red, green, blue);
	wsSetColor(73, red, green, blue);
}

void wsSetRightArrow(u8 red, u8 green, u8 blue) {
	wsSetColor(62, red, green, blue);
	wsSetColor(71, red, green, blue);
	wsSetColor(78, red, green, blue);
}

void wsSetNumber(u8 number, u8 red, u8 green, u8 blue) {
	if(number == 1) {
		wsSetColor( 4, red, green, blue);
		wsSetColor(11, red, green, blue);
		wsSetColor(12, red, green, blue);
		wsSetColor(18, red, green, blue);
		wsSetColor(20, red, green, blue);
		wsSetColor(28, red, green, blue);
		wsSetColor(36, red, green, blue);
		wsSetColor(44, red, green, blue);
		wsSetColor(52, red, green, blue);
		wsSetColor(60, red, green, blue);
		wsSetColor(66, red, green, blue);
		wsSetColor(67, red, green, blue);
		wsSetColor(68, red, green, blue);
		wsSetColor(69, red, green, blue);
	}else if (number == 2) {
		wsSetColor( 3, red, green, blue);
		wsSetColor( 4, red, green, blue);
		wsSetColor(10, red, green, blue);
		wsSetColor(13, red, green, blue);
		wsSetColor(21, red, green, blue);
		wsSetColor(29, red, green, blue);
		wsSetColor(37, red, green, blue);
		wsSetColor(44, red, green, blue);
		wsSetColor(51, red, green, blue);
		wsSetColor(58, red, green, blue);
		wsSetColor(66, red, green, blue);
		wsSetColor(67, red, green, blue);
		wsSetColor(68, red, green, blue);
		wsSetColor(69, red, green, blue);
	}else if(number == 3) {
		wsSetColor( 3, red, green, blue);
		wsSetColor( 4, red, green, blue);
		wsSetColor(10, red, green, blue);
		wsSetColor(13, red, green, blue);
		wsSetColor(21, red, green, blue);
		wsSetColor(29, red, green, blue);
		wsSetColor(34, red, green, blue);
		wsSetColor(35, red, green, blue);
		wsSetColor(36, red, green, blue);
		wsSetColor(45, red, green, blue);
		wsSetColor(53, red, green, blue);
		wsSetColor(58, red, green, blue);
		wsSetColor(61, red, green, blue);
		wsSetColor(67, red, green, blue);
		wsSetColor(68, red, green, blue);
	}else wsSetColor( 0, red, green, blue);
}

void wsSetMusicStatus(bool play, u8 lightValue) {
	if(play == TRUE) {
		wsSetColor(18, 0, lightValue, 0);
		wsSetColor(26, 0, lightValue, 0);
		wsSetColor(27, 0, lightValue, 0);
		wsSetColor(34, 0, lightValue, 0);
		wsSetColor(35, 0, lightValue, 0);
		wsSetColor(36, 0, lightValue, 0);
		wsSetColor(42, 0, lightValue, 0);
		wsSetColor(43, 0, lightValue, 0);
		wsSetColor(44, 0, lightValue, 0);
		wsSetColor(45, 0, lightValue, 0);
		wsSetColor(50, 0, lightValue, 0);
		wsSetColor(51, 0, lightValue, 0);
		wsSetColor(52, 0, lightValue, 0);
		wsSetColor(58, 0, lightValue, 0);
		wsSetColor(59, 0, lightValue, 0);
		wsSetColor(66, 0, lightValue, 0);
	}else {
		wsSetColor(18, lightValue, 0, 0);
		wsSetColor(21, lightValue, 0, 0);
		wsSetColor(26, lightValue, 0, 0);
		wsSetColor(29, lightValue, 0, 0);
		wsSetColor(34, lightValue, 0, 0);
		wsSetColor(37, lightValue, 0, 0);
		wsSetColor(42, lightValue, 0, 0);
		wsSetColor(45, lightValue, 0, 0);
		wsSetColor(50, lightValue, 0, 0);
		wsSetColor(53, lightValue, 0, 0);
		wsSetColor(58, lightValue, 0, 0);
		wsSetColor(61, lightValue, 0, 0);
		wsSetColor(66, lightValue, 0, 0);
		wsSetColor(69, lightValue, 0, 0);
	}
}