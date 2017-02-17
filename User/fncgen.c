#include "stm8s.h"
#include "fncgen.h"

/** Get 14-bit LSB from a 28-bit value */
#define FREQ_LSB(reg) (u16)(0x3FFF & (reg))

/** Get 14-bit MSB from a 28-bit value */
#define FREQ_MSB(reg) (u16)(0x3FFF & ((reg)>>14))

/** Mask applied to the FG CREG to clear any previous waveform preset */
#define FG_WFM_MASK (u16)(FG_OPBITEN | FG_DIV2 | FG_MODE)

static void FG_SPI_Send16(uint16_t word);

/**
 * @brief Write a single word over SPI to a FG
 * @param inst - FG instance
 * @param word - word to write (16 bits)
 */
static void FG_WriteWord(FG_Instance *inst, uint16_t word)
{
	inst->NSS_GPIO->ODR &= ~inst->NSS_PIN;
	FG_SPI_Send16(word);
	inst->NSS_GPIO->ODR |= inst->NSS_PIN;
}

/**
 * @brief Manually start a FG data frame
 * This function may be used for broadcasting register
 * changes to multiple devices (ie. FG_RESET release
 * for phase synchronisation)
 *
 * @param inst - FG instance
 */
void FG_JoinBroadcast(FG_Instance *inst)
{
	inst->NSS_GPIO->ODR &= ~inst->NSS_PIN;
}

/**
 * @brief Manually end a FG data frame
 * This function is used to terminate a broadcast session.
 *
 * @param inst - FG instance
 */
void FG_LeaveBroadcast(FG_Instance *inst)
{
	inst->NSS_GPIO->ODR |= inst->NSS_PIN;
}

/**
 * @brief Flush the CREG instance variable to the device
 * @param inst - FG instance
 */
static inline void FG_WriteCREG(FG_Instance *inst)
{
	FG_WriteWord(inst, inst->CREG);
}

/**
 * @brief Set frequency in a bank register
 * Change is immediately applied if the bank is active.
 *
 * @param inst - FG instance
 * @param bank - bank number
 * @param regval - new frequency register value (28 bits)
 */
void FG_SetFreq(FG_Instance *inst, FG_FreqBank bank, u32 regval)
{
	u16 word;

	// Ensure B28 is set
	if (!(inst->CREG & FG_B28)) {
		inst->CREG |= FG_B28;
		FG_WriteCREG(inst);
	}

	word = (bank == FREQ0 ? FG_ADDR_FREQ0 : FG_ADDR_FREQ1);
	FG_WriteWord(inst, word | FREQ_LSB(regval)); // 14 bits LSB
	FG_WriteWord(inst, word | FREQ_MSB(regval)); // 14 bits MSB
}

/**
 * @brief Set frequency MSB in a bank register
 * Change is immediately applied if the bank is active.
 *
 * @param inst - FG instance
 * @param bank - frequency bank to change
 * @param lsb14 - upper 14 bits of the frequency register
 */
void FG_SetFreqMSB(FG_Instance *inst, FG_FreqBank bank, u16 msb14)
{
	u16 word;

	// Ensure B28 is cleared
	if ((inst->CREG & (FG_B28 | FG_HLB)) != FG_HLB) {
		inst->CREG &= ~FG_B28;
		inst->CREG |= FG_HLB;
		FG_WriteCREG(inst);
	}

	word = (bank == FREQ0 ? FG_ADDR_FREQ0 : FG_ADDR_FREQ1);
	FG_WriteWord(inst, word | (u16)(msb14 & 0x3FFF)); // 14 bits LSB
}

/**
 * @brief Set frequency LSB in a bank register
 * Change is immediately applied if the bank is active.
 *
 * @param inst - FG instance
 * @param bank - frequency bank to change
 * @param lsb14 - lower 14 bits of the frequency register
 */
void FG_SetFreqLSB(FG_Instance *inst, FG_FreqBank bank, u16 lsb14)
{
	u16 word;

	// Ensure B28 is cleared
	if ((inst->CREG & (FG_B28 | FG_HLB)) != 0) {
		inst->CREG &= ~FG_B28;
		inst->CREG &= ~FG_HLB;
		FG_WriteCREG(inst);
	}

	word = (bank == FREQ0 ? FG_ADDR_FREQ0 : FG_ADDR_FREQ1);
	FG_WriteWord(inst, word | (u16)(lsb14 & 0x3FFF)); // 14 bits LSB
}

/**
 * @brief Select active frequency bank (for FSK)
 * @param inst - FG instance
 * @param bank - bank number
 */
void FG_FreqSwitch(FG_Instance *inst, FG_FreqBank bank)
{
	if (bank == FREQ0) {
		inst->CREG &= ~FG_FSELECT;
	} else {
		inst->CREG |= FG_FSELECT;
	}
	FG_WriteCREG(inst);
}

/**
 * @brief Set phase in a bank register
 * Change is immediately applied if the bank is active.
 *
 * @param inst - FG instance
 * @param bank - bank number
 * @param regval - new phase register value (12 bits)
 */
void FG_SetPhase(FG_Instance *inst, FG_PhaseBank bank, u16 regval)
{
	u16 word = (bank == PHASE0 ? FG_ADDR_PHASE0 : FG_ADDR_PHASE1);
	FG_WriteWord(inst, word | (u16) (regval & 0xFFF)); // 12 bits LSB
}

/**
 * @brief Select active phase register (for PSK)
 * @param inst - FG instance
 * @param bank - bank number
 */
void FG_PhaseSwitch(FG_Instance *inst, FG_PhaseBank bank)
{
	if (bank == PHASE0) {
		inst->CREG &= ~FG_PSELECT;
	} else {
		inst->CREG |= FG_PSELECT;
	}
	FG_WriteCREG(inst);
}

/**
 * @brief Select a function generator waveform preset
 * @param inst - FG instance
 * @param wfm - waveform preset (enum)
 */
void FG_SetWaveform(FG_Instance *inst, FG_Waveform wfm)
{
	inst->CREG &= ~FG_WFM_MASK;
	inst->CREG |= wfm;
	FG_WriteCREG(inst);
}

/**
 * @brief Suspend a function generator
 * Stops the counter, keeping the output constant
 *
 * @param inst - FG instance
 */
void FG_Suspend(FG_Instance *inst)
{
	inst->CREG |= FG_SLEEP1;
	FG_WriteCREG(inst);
}

/**
 * @brief Resume a function generator from suspend state *
 * @param inst - FG instance
 */
void FG_Resume(FG_Instance *inst)
{
	inst->CREG &= ~FG_SLEEP1;
	FG_WriteCREG(inst);
}

/**
 * @brief Enable or disable a function generator
 * When the FG is disabled, the output goes to a midpoint and
 * counting registers are reset.
 *
 * @param inst - FG instance
 * @param enable - enable status
 */
void FG_Cmd(FG_Instance *inst, FunctionalState enable)
{
	if (enable == ENABLE) {
		inst->CREG &= ~FG_RESET;
	} else {
		inst->CREG |= FG_RESET;
	}
	FG_WriteCREG(inst);
}

void FG_Reset(FG_Instance *inst)
{
	// Stop, enable fullsize freq writes
	inst->CREG = FG_B28 | FG_RESET;
	FG_WriteCREG(inst);

	FG_SetFreq(inst, FREQ0, HZ_REG(500));
	FG_SetFreq(inst, FREQ1, 0);
	FG_SetPhase(inst, PHASE0, 0);
	FG_SetPhase(inst, PHASE1, 0);
}

/**
 * @brief Configure a function generator *
 * Sets up the GPIO pin and resets the target.
 * SPI must already be configured!
 *
 * @param inst - FG instance
 * @param NSS_GPIO - port with the slave select pin
 * @param NSS_PIN - slave slect pin position
 */
void FG_Init(FG_Instance *inst, GPIO_TypeDef *NSS_GPIO, GPIO_Pin_TypeDef NSS_PIN)
{
	GPIO_Init(NSS_GPIO, NSS_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);

	inst->NSS_GPIO = NSS_GPIO;
	inst->NSS_PIN = NSS_PIN;

	FG_Reset(inst);
}

void FG_SPI_Init(void)
{
	// MOSI, SCK
	GPIO_Init(GPIOC, GPIO_PIN_5 | GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);

	SPI_Init(SPI_FIRSTBIT_MSB,
			 SPI_BAUDRATEPRESCALER_2,
			 SPI_MODE_MASTER,
			 SPI_CLOCKPOLARITY_HIGH,
			 SPI_CLOCKPHASE_1EDGE,
			 SPI_DATADIRECTION_1LINE_TX,
			 SPI_NSS_SOFT,
			 0);

	SPI_Cmd(ENABLE);
}

static void FG_SPI_Send16(uint16_t word)
{
	SPI_SendData((u8)(word >> 8));
	while(!SPI_GetFlagStatus(SPI_FLAG_TXE));
	SPI_SendData((u8)(word));
	while(SPI_GetFlagStatus(SPI_FLAG_BSY));
}
