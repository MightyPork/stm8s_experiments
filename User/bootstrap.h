//
// Created by MightyPork on 2017/02/10.
//

#ifndef STM8S_STDINIT_H
#define STM8S_STDINIT_H

/**
 * Simple init (UART, LED, timebase)
 */
void SimpleInit(void);

//region Timing

/** Global timebase */
extern volatile uint16_t time_ms;

/** SysTick handler */
void TIM4_UPD_OVF_IRQHandler(void) INTERRUPT(23);

/**
 * Millisecond delay
 *
 * @param ms - nr of milliseconds
 */
void delay_ms(uint16_t ms);

/**
 * Seconds delay
 *
 * @param ms - nr of milliseconds
 */
inline void delay_s(uint16_t s)
{
	while (s != 0) {
		delay_ms(1000);
		s--;
	}
}

/** Get milliseconds elapsed since start timestamp */
inline uint16_t ms_elapsed(uint16_t start)
{
	return time_ms - start;
}

/** Get current timestamp. */
inline uint16_t ms_now(void)
{
	return time_ms;
}

/** Helper for looping with periodic branches */
bool ms_loop_elapsed(uint16_t *start, uint16_t duration);

//endregion

//region UART

/** Uart IRQ handler */
void UART1_RX_IRQHandler(void) INTERRUPT(18);

/** putchar, used by the SDCC stdlib */
void putchar(char c);

/**
 * User UART rx handler
 *
 * If adding custom handler, comment out the defualt echo impl in bootstrap.c
 *
 * @param c
 */
extern void UART_HandleRx(char c);

//endregion

//region LED

/** Toggle indicator LED */
inline void LED_Toggle(void)
{
	GPIOB->ODR ^= GPIO_PIN_5;
}

/** Set indicator LED */
inline void LED_Set(bool state)
{
	if (state) {
		GPIOB->ODR &= ~GPIO_PIN_5;
	} else {
		GPIOB->ODR |= GPIO_PIN_5;
	}
}

//endregion

#endif //STM8S_DEBUG_H
