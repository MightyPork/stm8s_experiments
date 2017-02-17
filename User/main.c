#include "stm8s.h"
#include "bootstrap.h"
#include "fncgen.h"

FG_Instance fgi;

void main(void)
{
	u16 t;
	bool x = FALSE;
	SimpleInit();

	FG_SPI_Init();

	FG_Init(&fgi, GPIOC, GPIO_PIN_4);
	FG_SetFreq(&fgi, FREQ0, HZ_REG(4000));
	FG_SetFreq(&fgi, FREQ1, HZ_REG(2500));
	FG_SetWaveform(&fgi, WFM_SINE);
	FG_Cmd(&fgi, ENABLE);

	t = ms_now();
	while (1) {
		if (ms_loop_elapsed(&t, 100)) {
			LED_Toggle();

			// Switching freq banks - connect a speaker to hear 2-tone beeping!
			x = !x;
			FG_FreqSwitch(&fgi, x);
		}
	}
}
