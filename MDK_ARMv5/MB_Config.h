#include "ht32.h"
#include "ht32_board.h"

#define T500US_count    36000
#define T200US_count    14400

static const u32 GPIO_PIN_n[16] = {
		  GPIO_PIN_0 ,  GPIO_PIN_1 ,  GPIO_PIN_2 ,  GPIO_PIN_3 , 
		  GPIO_PIN_4 ,  GPIO_PIN_5 ,  GPIO_PIN_6 ,  GPIO_PIN_7 ,
		  GPIO_PIN_8 ,  GPIO_PIN_9 ,  GPIO_PIN_10,  GPIO_PIN_11,
		  GPIO_PIN_12,  GPIO_PIN_13,  GPIO_PIN_14,  GPIO_PIN_15
};

static TM_TimeBaseInitTypeDef TimeBaseInit;

void CKCU_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void GPTM0_Configuration(void);
void GPTM1_Configuration(void);
void BFTM0_Configuration(void);
void BTMasterConfiguration(void);
