#include "MB_Config.h"

u8 config_i;

void CKCU_Configuration(void) {
#if 1
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
    CKCUClock.Bit.PA         = 1;
    CKCUClock.Bit.PB         = 1;
    CKCUClock.Bit.PC         = 1;
    CKCUClock.Bit.PD         = 1;
    CKCUClock.Bit.USART0     = 1;
    CKCUClock.Bit.USART1     = 1;
    CKCUClock.Bit.UART0      = 0;
    CKCUClock.Bit.UART1      = 1;
    CKCUClock.Bit.AFIO       = 1;
    CKCUClock.Bit.EXTI       = 1;
    CKCUClock.Bit.GPTM0      = 1;
    CKCUClock.Bit.GPTM1      = 1;
    CKCUClock.Bit.ADC        = 0;
		CKCUClock.Bit.BFTM0      = 1;
		CKCUClock.Bit.MCTM0			 = 1;
		CKCUClock.Bit.PDMA			 = 1;
    
    CKCU_PeripClockConfig(CKCUClock, ENABLE);
#endif
#if (ENABLE_CKOUT == 1)
    CKOUTConfig();
#endif
}

void GPIO_Configuration(void) {
	// Port A Config. 
	// PA1~3: output of 7 Segment's switch.
	for (config_i = 1; config_i <= 3; config_i += 1) GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_n[config_i],  GPIO_DIR_OUT);
	// PA8~15: Input of music button.
	for (config_i = 8; config_i <= 15; config_i += 1) {
		GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_n[config_i],  GPIO_DIR_IN);
		GPIO_InputConfig(HT_GPIOA, GPIO_PIN_n[config_i], ENABLE);
	}
	
	// Port C Config.  
	// PC0: Switch mode button
	GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_n[0],  GPIO_DIR_IN);
	GPIO_InputConfig(HT_GPIOC, GPIO_PIN_n[0], ENABLE);
	// PC8~11: output to ic 7447's BCD signal
	for (config_i = 8; config_i <= 11; config_i += 1) GPIO_DirectionConfig(HT_GPIOC, GPIO_PIN_n[config_i],  GPIO_DIR_OUT);
	
	// Port D Config. 
	//Output to ic 74138 decoder to switch 8*8 LED rows.
	for (config_i = 0; config_i <= 2; config_i += 1) GPIO_DirectionConfig(HT_GPIOD, GPIO_PIN_n[config_i],  GPIO_DIR_OUT);
}

void NVIC_Configuration(void) {
	NVIC_EnableIRQ(GPTM0_IRQn);
	NVIC_EnableIRQ(GPTM1_IRQn);
	NVIC_EnableIRQ(BFTM0_IRQn);
}

void GPTM1_Configuration(void) {
	TM_TimeBaseStructInit(&TimeBaseInit);                       // Init GPTM1 time-base
	TimeBaseInit.CounterMode = TM_CNT_MODE_UP;                  // up count mode
	TimeBaseInit.CounterReload = T500US_count;                  // interrupt in every 500us
	TimeBaseInit.Prescaler = 0;
	TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;          // reload immediately
	TM_TimeBaseInit(HT_GPTM1, &TimeBaseInit);                   // write the parameters into GPTM1
	TM_ClearFlag(HT_GPTM1, TM_FLAG_UEV);                        // Clear Update Event Interrupt flag
	TM_IntConfig(HT_GPTM1, TM_INT_UEV, ENABLE);                 // interrupt by GPTM update
	TM_Cmd(HT_GPTM1, ENABLE);                                   // enable the counter 1
}

void GPTM0_Configuration(void) {
	TM_TimeBaseStructInit(&TimeBaseInit);                       // Init GPTM1 time-base
	TimeBaseInit.CounterMode = TM_CNT_MODE_UP;                  // up count mode
	TimeBaseInit.CounterReload = T500US_count;                  // interrupt in every 500us
	TimeBaseInit.Prescaler = 0;
	TimeBaseInit.PSCReloadTime = TM_PSC_RLD_IMMEDIATE;          // reload immediately
	TM_TimeBaseInit(HT_GPTM0, &TimeBaseInit);                   // write the parameters into GPTM1
	TM_ClearFlag(HT_GPTM0, TM_FLAG_UEV);                        // Clear Update Event Interrupt flag
	TM_IntConfig(HT_GPTM0, TM_INT_UEV, ENABLE);                 // interrupt by GPTM update
	TM_Cmd(HT_GPTM0, ENABLE);                                   // enable the counter 1
}

void BFTM0_Configuration(void) {
	BFTM_SetCompare(HT_BFTM0, SystemCoreClock/10*8);
	BFTM_SetCounter(HT_BFTM0, 0);
	BFTM_IntConfig(HT_BFTM0, ENABLE);
	BFTM_EnaCmd(HT_BFTM0, ENABLE);
}

void BTMasterConfiguration(void){
	// Init Master Structure
	USART_InitTypeDef USART_InitMaster;

	// Config Master AFIO mode as UART1_Rx and UART1_Tx function.
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_12, AFIO_MODE_6);
	AFIO_GPxConfig(GPIO_PC, AFIO_PIN_13, AFIO_MODE_6);

	// Master UART1 Configuration
	USART_InitMaster.USART_BaudRate = 9600;
	USART_InitMaster.USART_WordLength = USART_WORDLENGTH_8B;
	USART_InitMaster.USART_StopBits = USART_STOPBITS_1;
	USART_InitMaster.USART_Parity = USART_PARITY_NO;
	USART_InitMaster.USART_Mode = USART_MODE_NORMAL;
	USART_Init(HT_UART1, &USART_InitMaster);
	USART_TxCmd(HT_UART1, ENABLE);
	USART_RxCmd(HT_UART1, ENABLE);
}
