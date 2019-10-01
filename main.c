#include "ht32.h"
#include "ht32_board.h"
#include "MB_Config.h"
#include "ws2812.h"

#define LEN1          48
#define LEN2          48
#define LEN3          32

const u16 BPM = 600;	
const u16 R = 0x00, C = 0x01, D   = 0x02,	// Rest, Do, Re 
		  E = 0x04,	F = 0x08, G   = 0x10,	// Mi  , Fa, Sol
		  A = 0x20, B = 0x40, C_H = 0x80;	// La  , Si, DoHigh'
const u32 WAIT_SERVO = 2000;

typedef struct{
	vu8 flag;
	vu16 tm;
} TimerStruct;

TimerStruct wait[8], beat, switchMode, modeDisplay, ws;

bool isCorrect = FALSE, didAddScore = FALSE, didChooceMusic = FALSE, didSetColor, musicStatus;
u8 btns_state, ledScore[4], score = 0, musicLen, musicNum = 1;
u8 i, music_index, music_buff, wait_index, mode = WS_GENERAL;
FlagStatus blinkStatus = RESET;
// Rest 8 beats at start and end of the music.
u8 LED1[LEN1 + 20] = {
	R, R, R, R, R, R, R, R, R, R,
	C, D, E, F, G, A, B, C_H,
	C, D, E, F, G, A, B, C_H,
	C, D, E, F, G, A, B, C_H,
	C, D, E, F, G, A, B, C_H,
	C, D, E, F, G, A, B, C_H,
	C, D, E, F, G, A, B, C_H,
	R, R, R, R, R, R, R, R, R, R
};
u8 LED2[LEN2 + 20] = {
	R, R, R, R, R, R, R, R, R, R,
	C, R, R, D, E, R, R, C,
	E, R, C, R, E, R, R, R,
	D, R, R, E, F, F, E, D,
	F, R, R, R, R, R, R, R,
	E, R, R, F, G, R, R, E,
	G, R, E, R, G, R, R, R,
	R, R, R, R, R, R, R, R, R, R
};
u8 LED3[LEN3 + 20] = {
	R, R, R, R, R, R, R, R, R, R,
	C_H, B, A, G, F, E, D, C,
	C_H, B, A, G, F, E, D, C,
	C_H, B, A, G, F, E, D, C,
	C_H, B, A, G, F, E, D, C,
	R, R, R, R, R, R, R, R, R, R
};

void Initialize(void);
void GPTM0_IRQHandler(void);
void GPTM1_IRQHandler(void);
void BFTM0_IRQHandler(void);
void setScore(u16);
void sendBTData(void);
void switchHandle(void);
void modeSwitch(void);
FlagStatus musicSwitch(void);
void wsSetMusicLED(u8 music,u8 buffer[]);
void setMusicStatus(void);

#if (ENABLE_CKOUT == 1)
	void CKOUTConfig(void);
#endif

int main(void) {
	Initialize();	
	NVIC_Configuration();
	CKCU_Configuration();
	RETARGET_Configuration();
	GPIO_Configuration();
	GPTM0_Configuration();
	GPTM0_IRQHandler();
	GPTM1_Configuration();
	GPTM1_IRQHandler();
	BFTM0_Configuration();
	BFTM0_IRQHandler();
	BTMasterConfiguration();
	wsInit();
	
	wsBlinkAll(100);
	
	printf("\r\n============Set up===========\r\n");
	while (1) {
		if (switchMode.flag == RESET) {
			modeSwitch();
		}
		else {
			if(mode == WS_GENERAL){
				
			}
			else if(mode == WS_GUIDE) {
				
			}
			else if(mode == WS_GAME) {
				if(didChooceMusic == FALSE) {
					musicSwitch();
				}else {
					setMusicStatus();
					for (music_index = 0; music_index < (musicLen + 10); music_index += 1) {
						// If x seconds passed, reset x to 0, break to print and rejudge next bar.
						wsSetMusicLED(music_index, LED1);
						beat.flag = RESET;
						isCorrect = FALSE;
						didAddScore = FALSE;
						music_buff = LED1[music_index];	// The botton of 8*8LED
						while (beat.flag == RESET) {
							// Read the buttons is either pressed or not, save it to btns_state.
							btns_state = ~(GPIO_ReadInData(HT_GPIOA) >> 8);
							if (music_buff == btns_state && music_buff != 0x00) isCorrect = TRUE;	
							if(isCorrect == TRUE && didAddScore == FALSE) {
								score++;            	// Add one point.
								setScore(score);		// Set score to 7Segment.
								didAddScore = TRUE; 	// Mark the score has been added.
								//printf("\n\rScore added. Score = %d", score);
							}
						}
						if(didChooceMusic == FALSE) break;
					}
					didChooceMusic = FALSE;
				}
			}
		}
	}// end while
}//end main

void Initialize(void) {
	// Initialize parameters for time counting
	beat.flag = RESET;
	beat.tm = RESET;
	for(i = 0; i < 3; i++) ledScore[i] = 0xff;
	for(i = 0; i < 8; i++) wait[i].tm = RESET; wait[i].flag = SET;
	ws.flag = RESET;
	ws.tm = RESET;
	switchMode.flag = RESET;
	setScore(score);
}

void GPTM1_IRQHandler(void) {
	TM_ClearFlag(HT_GPTM1, TM_FLAG_UEV);
	if (switchMode.flag == RESET) {
		sendBTData();
	}
	switchHandle();
	ws.tm++;
	if(ws.tm >= WS_EXIT){
		ws.tm = 0;
<<<<<<< HEAD
		if(didSetColor == TRUE) wsShow();
		//wsShow();
=======
		//if(didSetColor == TRUE) wsShow();
		wsShow();
>>>>>>> parent of 5f7addf... clear
	}
}

void GPTM0_IRQHandler(void) {
	TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);
	
	// Wait servo timer.
	for(wait_index = 0; wait_index < 8; wait_index++) {
		if(wait[wait_index].flag == RESET) {
			wait[wait_index].tm++;
			if(wait[wait_index].tm >= WAIT_SERVO){
				wait[wait_index].flag = SET;
				wait[wait_index].tm = RESET;
			}
		}
	}
	
	// BPM Timer.
	if(beat.flag == RESET){
		beat.tm++;
		if (beat.tm >= BPM) {
			beat.tm = RESET;
			beat.flag = SET;
		}
	}
}

void BFTM0_IRQHandler(void) {
	BFTM_ClearFlag(HT_BFTM0);
	
	// button long click timer
	if (GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) == SET && switchMode.flag == SET){
		switchMode.tm++;
		if (switchMode.tm >= 3) {
			if(didChooceMusic == TRUE) {
				didChooceMusic = FALSE;
				printf("Switching music starting at %d\r\n", musicNum);
			}else {
				switchMode.flag = RESET;
				printf("Switching mode starting at %d\r\n", mode);
			}
<<<<<<< HEAD
			wsPrintBuffer();
=======
>>>>>>> parent of 5f7addf... clear
			switchMode.tm = RESET;
			wsClearAll();
			wsSetLeftArrow(10, 10, 10);
			wsSetRightArrow(10, 10, 10);
			wsSetColor(mode, 10, 10, 10);
			wsShow();
			while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) == SET);
		}
	} else {
		switchMode.tm = RESET;
	}
	
}

void setScore(u16 input) {
	static u8 i;
	
	if (input == 0) ledScore[0] = 0x00;
	else {
		for (i = 0; i < 4; i++) {
			ledScore[i] = input % 10;
			if (input == 0) ledScore[i] = 0xff;
			input /= 10;
		}
	}
}

void sendBTData(void){
	static FlagStatus buff[8] = {SET};
	static u8 BTData, btn_index;
	if (switchMode.flag == SET) {
		for (btn_index = 0; btn_index < 8; btn_index++) {
			// Button[btn_index] is PRESSING.
			if (buff[btn_index] > GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[btn_index + 8])) {
			}
			// Button[btn_index] is PRESSED.
			else if (buff[btn_index] < GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[btn_index + 8])) {
				// Save the Bluetooth data.
				BTData = 0x01 << btn_index;
				// If wait_flag[btn_index] is prepared, send bluetooth data.
				if(wait[btn_index].flag == SET) {
					USART_SendData(HT_UART1, BTData);
					while(USART_GetFlagStatus(HT_UART1, USART_FLAG_TXC) == RESET);
					wait[btn_index].flag = RESET;
					printf("\r\nSend %x success.", BTData);
				} else printf("\r\nSend %x failed.", BTData);
			}
			buff[btn_index] = GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[btn_index + 8]);
		}
	}
}

void switchHandle(void) {
	if (switchMode.flag == RESET) {
		modeDisplay.tm++;
		if(modeDisplay.tm >= WS_BLINK) {
			modeDisplay.tm = 0;
			modeDisplay.flag = SET;
			wsClearBetween(WS_GENERAL, WS_GAME);
			if(blinkStatus == SET) wsSetColor(mode, 10, 10, 10);
			else wsSetColor(mode, 0, 0, 0);
			blinkStatus = !blinkStatus;
		}
	}else {
		wsSetColor(mode, 10, 0, 0);
	}
}

void modeSwitch() {
	wsSetLeftArrow(10, 10, 10);
	wsSetRightArrow(10, 10, 10);
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) != SET) {
		// left toutch button
		if(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[8]) == RESET) {
			wsSetLeftArrow(30, 30, 30);
			while(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[8]) == RESET);
			wsSetLeftArrow(10, 10, 10);
			if(mode > WS_GENERAL) mode--;
			else mode = WS_GAME;
			wsClearBetween(WS_GENERAL, WS_GAME);
			printf("Mode: %02d\r\n", mode);
			modeDisplay.tm = WS_BLINK;
			blinkStatus = SET;
		}

		// right toutch button
		if(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[15]) == RESET) {
			wsSetRightArrow(30, 30, 30);
			while(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[15]) == RESET);
			wsSetRightArrow(10, 10, 10);
			if(mode < WS_GAME) mode++;
			else mode = WS_GENERAL;
			wsClearBetween(WS_GENERAL, WS_GAME);
			printf("Mode: %02d\r\n", mode);
			modeDisplay.tm = WS_BLINK;
			blinkStatus = SET;
		}
	}
	switchMode.flag = SET;
	printf("Mode: %02d\r\n", mode);
	wsClearBetween(0, 79);
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) == SET);
}

FlagStatus musicSwitch(void) {
	wsClearAll();
	wsSetLeftArrow(10, 10, 10);
	wsSetRightArrow(10, 10, 10);
	wsSetNumber(musicNum, 10, 0, 0);
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) != SET){
		// left toutch button
		if(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[8]) == RESET) {
			wsSetLeftArrow(30, 30, 30);
			while(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[8]) == RESET);
			if(musicNum > 1) musicNum--;
			else musicNum = 3;
			wsClearBetween(0, 79);
			wsSetLeftArrow(10, 10, 10);
			wsSetRightArrow(10, 10, 10);
			wsSetNumber(musicNum, 10, 0, 0);
			printf("Music: %02d\r\n", musicNum);
		}

		// right toutch button
		if(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[15]) == RESET) {
			wsSetRightArrow(30, 30, 30);
			while(GPIO_ReadInBit(HT_GPIOA, GPIO_PIN_n[15]) == RESET);
			if(musicNum < 3) musicNum++;
			else musicNum = 1;
			wsClearBetween(0, 79);
			wsSetLeftArrow(10, 10, 10);
			wsSetRightArrow(10, 10, 10);
			wsSetNumber(musicNum, 10, 0, 0);
			printf("Music: %02d\r\n", musicNum);
		}
	}
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) == SET);
	if(switchMode.flag == SET) {
		didChooceMusic = TRUE;
		switch(musicNum) {
			case 1:
				musicLen = LEN1;
				break;
			case 2:
				musicLen = LEN2;
				break;
			case 3:
				musicLen = LEN3;
				break;
			default:
				break;
		}
		printf("Selected Music: %02d\r\n", mode);
		wsClearAll();
	}
	
}

void wsSetMusicLED(u8 music, u8 buffer[]) {
	static u8 i, j;
<<<<<<< HEAD
	wsClearAll();
=======
>>>>>>> parent of 5f7addf... clear
	for(i = 10; i > 0; i--) {
		for(j = 0; j < 8; j++) {
			if((buffer[music + i] >> j & 1) == 1) wsSetColor(80 - (i * 8 - j), 10, 10, 10);
			else wsSetColor(80 - (i * 8 - j), 0, 0, 0);
			//printf("(music + i) * 8 + (7 - j) = %d\r\n", (music + i) * 8 + (7 - j));
		}
	}
	//wsPrintBuffer();
}

void setMusicStatus(void) {
	wsSetMusicStatus(musicStatus, 10);
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) != SET);
	wsSetMusicStatus(musicStatus, 50);
	while(GPIO_ReadInBit(HT_GPIOC, GPIO_PIN_n[0]) == SET);
	musicStatus = !musicStatus;
}


#if (HT32_LIB_DEBUG == 1)
	/*********************************************************************************************************//**
	  * @brief  Report both the error name of the source file and the source line number.
	  * @param  filename: pointer to the source file name.
	  * @param  uline: error line source number.
	  * @retval None
	  ***********************************************************************************************************/
	void assert_error(u8* filename, u32 uline) {
		/*
			This function is called by IP library that the invalid parameters has been passed to the library API.
			Debug message can be added here.
			Example: printf("Parameter Error: file %s on line %d\r\n", filename, uline);
		*/
		printf("Parameter Error: file %s on line %d\r\n", filename, uline);
		while (1) {
		}
	}
#endif

#if (ENABLE_CKOUT == 1)
	/*********************************************************************************************************//**
	  * @brief  Configure the debug output clock.
	  * @retval None
	  ***********************************************************************************************************/
	void CKOUTConfig(void) {
		CKCU_CKOUTInitTypeDef CKOUTInit;
		
		AFIO_GPxConfig(GPIO_PA, AFIO_PIN_9, AFIO_MODE_15);
		CKOUTInit.CKOUTSRC = CKCU_CKOUTSRC_HCLK_DIV16;
		CKCU_CKOUTConfig(&CKOUTInit);
	}
#endif
