//
// Created by MightyPork on 2017/02/17.
//

#ifndef STM8S_FNCGEN_H
#define STM8S_FNCGEN_H

// See the C file for doxygen

#include "stm8s.h"

// Freq write - addr + 14 bits
// B28 and HLB allow sequential write, or repeated update of half-registers
// In sequential write, the half-registers are written in the order LSB, MSB
#define FG_ADDR_CR (u16)0x0000
#define FG_ADDR_FREQ0 (u16)0x4000
#define FG_ADDR_FREQ1 (u16)0x8000

// Phase write - addr + 12 bits
#define FG_ADDR_PHASE0 (u16)0xC000
#define FG_ADDR_PHASE1 (u16)0xE000

#define FG_B28     (u16)0x2000 // Enable sequential write of the freq registers
#define FG_HLB     (u16)0x1000 // Nonsequential write, register select (0-LSB, 1-MSB)
#define FG_FSELECT (u16)0x0800 // Select active frequency reg
#define FG_PSELECT (u16)0x0400 // Select active phase reg
#define FG_RESET   (u16)0x0100 // Reset counter and keep output at midpoint
#define FG_SLEEP1  (u16)0x0080 // Suspend
#define FG_SLEEP12 (u16)0x0040 // Shut down ADC (when using square output)
#define FG_OPBITEN (u16)0x0020 // Output counter MSb (0=SIN/TRI,1=SQUARE)
#define FG_DIV2    (u16)0x0008 // Divide counter MSb by 2
#define FG_MODE    (u16)0x0002 // If ADC enabled, 0 = SIN, 1 = TRI

/** Convert frequency in Hz to a register value (for 25 MHz MCLK) */
#define HZ_REG(hz) (u32)((float)hz * 10.73741824f)

/** Function generator instance object */
typedef struct {
	uint16_t CREG;
	GPIO_TypeDef *NSS_GPIO;
	GPIO_Pin_TypeDef NSS_PIN;
} FG_Instance;

/** FG frequency bank number */
typedef enum {
	FREQ0 = 0,
	FREQ1 = 1
} FG_FreqBank;

/** FG phase bank number */
typedef enum {
	PHASE0 = 0,
	PHASE1 = 1
} FG_PhaseBank;

/**
 * Function generator waveform presets
 */
typedef enum {
	WFM_SINE = (u16) 0,
	WFM_TRIANGLE = (u16) FG_MODE,
	WFM_SQUARE = (u16) (FG_OPBITEN | FG_DIV2),
	WFM_SQUARE_DIV2 = (u16) FG_OPBITEN
} FG_Waveform;

void FG_JoinBroadcast(FG_Instance *inst);
void FG_LeaveBroadcast(FG_Instance *inst);
void FG_SetFreq(FG_Instance *inst, FG_FreqBank bank, u32 regval);
void FG_SetFreqMSB(FG_Instance *inst, FG_FreqBank bank, u16 msb14);
void FG_SetFreqLSB(FG_Instance *inst, FG_FreqBank bank, u16 lsb14);
void FG_FreqSwitch(FG_Instance *inst, FG_FreqBank bank);
void FG_SetPhase(FG_Instance *inst, FG_PhaseBank bank, u16 regval);
void FG_PhaseSwitch(FG_Instance *inst, FG_PhaseBank bank);
void FG_SetWaveform(FG_Instance *inst, FG_Waveform wfm);
void FG_Suspend(FG_Instance *inst);
void FG_Resume(FG_Instance *inst);
void FG_Cmd(FG_Instance *inst, FunctionalState enable);
void FG_Reset(FG_Instance *inst);
void FG_Init(FG_Instance *inst, GPIO_TypeDef *NSS_GPIO, GPIO_Pin_TypeDef NSS_PIN);
void FG_SPI_Init(void);

#endif //STM8S_FNCGEN_H
