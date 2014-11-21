#include "timerdo.h" 
#include "stm32f4xx.h" 
#include <math.h>

/* Do stuff when timer interrupt happens */

/* Keep track of amount of time that has gone by */
static uint64_t timeElapsed;
static uint32_t ledTime;
static uint32_t buttonStatus; 
static uint32_t dacVal;
static uint32_t freq=100;

static void timerdo_update_time(void)
{
    timeElapsed += 1;
	if(timeElapsed >= 1000)
	{
		timeElapsed -= 1000;
	}
	
	dacVal += 1;
	if(dacVal >= 4095)
	{
		dacVal -= 4095;
	}
}

static int getSin(void)
{
	
	float temp = sin(2.0f * (float)M_PI * ((float)(freq*timeElapsed)/1000.0f));
	temp *= 1024.0f;
	temp += 2048;
	return (int)temp; 
	
	//return (int) 1024.*sin(2.*M_PI*(float)timeElapsed/1000.)+2048.;
	
//	return dacVal;
}

/* Make leds do something every so often */
static void timerdo_setup_leds(void)
{
	// init leds 
    GPIO_InitTypeDef GPIO_TIM_InitStruct;
	GPIO_TIM_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_TIM_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_TIM_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_TIM_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_TIM_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_Init(GPIOD, &GPIO_TIM_InitStruct);

    ledTime = LED_TIME_RESET_VAL; /* every 1000 ticks, do something with the leds */
	GPIO_ResetBits(GPIOD,GPIO_Pin_14);
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	GPIO_ResetBits(GPIOD,GPIO_Pin_15);
}

static void timerdo_setup_button(void)
{
	GPIO_InitTypeDef buttonInitStruct;
	buttonInitStruct.GPIO_Pin = GPIO_Pin_0;
	buttonInitStruct.GPIO_Mode = GPIO_Mode_IN;
	
	EXTI_InitTypeDef buttonExtiInitStruct;
	buttonExtiInitStruct.EXTI_Line = EXTI_Line0;
	buttonExtiInitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	buttonExtiInitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	
	NVIC_InitTypeDef buttonNvicInitStruct;
	buttonNvicInitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_Init(GPIOA, &buttonInitStruct);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	EXTI_Init(&buttonExtiInitStruct);
	NVIC_Init(&buttonNvicInitStruct);
	
}	

/* Do something with the leds */
static void timerdo_led_do(void)
{
	if(buttonStatus) {
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12); // green
		DAC_SetChannel1Data(DAC_Align_12b_R, getSin()); // set to 1.4 V
	}else{
		GPIO_ResetBits(GPIOD, GPIO_Pin_12); // green
		DAC_SetChannel1Data(DAC_Align_12b_R, 0); // set to 0 V
	}
}

void timerdo_button_do(void)
{
	buttonStatus = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
}

void timerdo_setup(void)
{
	buttonStatus = 0;
    timeElapsed = 0;
    timerdo_setup_leds();
    timerdo_setup_button();
}

void timerdo_update_leds(void)
{
    
    if (--ledTime == 0) {
        timerdo_led_do();
        ledTime = LED_TIME_RESET_VAL;
    }
}

void timerdo_timerdo(void)
{
    timerdo_update_time();
    timerdo_update_leds();
	
	if(buttonStatus) {
		DAC_SetChannel1Data(DAC_Align_12b_R, getSin()); // set to 1.4 V
	}else{
		DAC_SetChannel1Data(DAC_Align_12b_R, 0); // set to 0 V
	}
}
