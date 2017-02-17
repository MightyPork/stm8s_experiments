#include "stm8s.h"
#include <stdio.h>
#include "bootstrap.h"

#define LEVEL_TOP 400
#define LEVEL_MAX 370

volatile uint16_t level = 200;

/**
 * Set PWM level
 */
void PWM_Write(void)
{
	uint16_t tmp = level;
	if (tmp > LEVEL_MAX) tmp = LEVEL_MAX;
	TIM1_SetCompare4(tmp);
}

void PWM_Cmd(FunctionalState fs)
{
	TIM1_CtrlPWMOutputs(fs);
}

/**
 * Set up the PWM generation
 */
void PWM_Setup()
{
	// open drain, fast
	GPIOC->DDR |= GPIO_PIN_4; // out
	//GPIOC->CR1 &= ~GPIO_PIN_4; // open drain
	GPIOC->CR2 |= GPIO_PIN_4; // fast

	TIM1_TimeBaseInit(0, TIM1_COUNTERMODE_UP, LEVEL_TOP, 0);

	TIM1_OC4Init(TIM1_OCMODE_PWM1,
				 TIM1_OUTPUTSTATE_ENABLE,
				 level,
				 TIM1_OCPOLARITY_HIGH,
				 TIM1_OCIDLESTATE_SET);

	TIM1_Cmd(ENABLE);
}

/**
 * Set up the analog input
 */
void AIN_Setup()
{
	ADC1_ConversionConfig(ADC1_CONVERSIONMODE_CONTINUOUS,
						  ADC1_CHANNEL_3,
						  ADC1_ALIGN_RIGHT);
	ADC1_Cmd(ENABLE);
	ADC1_StartConversion();
}

void main(void)
{
	uint16_t cnt = 0;
	uint16_t conv;

	SimpleInit();
	PWM_Setup();
	AIN_Setup();

	// Go
	PWM_Cmd(ENABLE);

	while (1) {
		if (cnt++ == 65535) {
			cnt = 0;
			LED_Toggle();
		}

		// adjust level
		if (ADC1_GetFlagStatus(ADC1_FLAG_EOC)) {
			conv = ADC1_GetConversionValue();
			level = conv;
			PWM_Write();
		}
	}
}
