//
// Created by MightyPork on 2017/02/10.
//

#include <stm8s.h>
#include "bootstrap.h"

/** Global time base */
volatile uint16_t time_ms;

/**
 * Putchar for printf
 * @param c - char to print
 */
void putchar(char c)
{
	while ((UART1->SR & UART1_SR_TXE) == 0);
	UART1->DR = (u8)c;
}

/**
 * Init for the chinese STM8 board
 * - enable LED
 * - enable UART @ 115200
 * - set up UART rx irq
 */
void SimpleInit(void)
{
	// Disable default div/8 HSI prescaller
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

	// LED
	GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);

	// UART init & enable IRQ
	UART_SimpleInit(UART_BAUD_115200);
	UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);

	// Timebase generation counter
	TIM4_UpdateRequestConfig(TIM4_UPDATESOURCE_REGULAR);
	TIM4_PrescalerConfig(TIM4_PRESCALER_64, TIM4_PSCRELOADMODE_IMMEDIATE);
	TIM4_SetAutoreload(0xFF);
	TIM4_ARRPreloadConfig(ENABLE);
	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
	TIM4_Cmd(ENABLE);

	enableInterrupts();
}

/**
 * @brief Timer4 Update/Overflow Interrupt routine.
 * @param  None
 * @retval None
 */
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
	time_ms++;
	TIM4_ClearITPendingBit(TIM4_IT_UPDATE);

}

/** Delay ms */
void delay_ms(uint16_t ms)
{
	uint16_t start = time_ms;
	uint16_t t2;
	while (1) {
		t2 = time_ms;
		if ((t2 - start) >= ms) {
			break;
		}
	}
}

/** Helper for looping with periodic branches */
bool ms_loop_elapsed(uint16_t *start, uint16_t duration)
{
	if (time_ms - *start >= duration) {
		*start = time_ms;
		return TRUE;
	}

	return FALSE;
}

/**
 * UART1 RX Interrupt routine.
 */
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)
{
	if (UART1->SR & UART1_SR_RXNE) {
		UART_HandleRx(UART1->DR);
	}
}

// Comment out if custom rx handler is added
#if 1
void UART_HandleRx(char c)
{
	// echo
	putchar(c);
}
#endif
