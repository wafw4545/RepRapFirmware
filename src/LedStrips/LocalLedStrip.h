/*
 * LocalLedStrip.h
 *
 *  Created on: 30 Apr 2023
 *      Author: David
 */

#ifndef SRC_LEDSTRIPS_LOCALLEDSTRIP_H_
#define SRC_LEDSTRIPS_LOCALLEDSTRIP_H_

#include "LedStripBase.h"

#if SUPPORT_LED_STRIPS

// Define which types of LED strip this hardware supports
#define SUPPORT_DMA_NEOPIXEL		(defined(DUET3_MB6HC) || defined(DUET3_MB6XD) || defined(DUET3MINI) || defined(PCCB_10))
#define SUPPORT_DMA_DOTSTAR			(defined(DUET3_MB6HC) || defined(DUET3_MB6XD) || defined(PCCB_10))
#define SUPPORT_BITBANG_NEOPIXEL	(defined(DUET3MINI_V04) || defined(DUET_NG))

// Temporarily we use a static chunk buffer if fixed size. Define the size of this buffer.
// We would do better to allocate it dynamically, however on the SAME70 it needs to be in non-cached RAM.
#if defined(DUET3_MB6HC) || defined(DUET3_MB6XD)
// We have plenty of non-cached RAM left on Duet 3
constexpr size_t ChunkBufferSize = 240 * 16;						// DotStar LEDs use 4 bytes/LED, NeoPixel RGBW use 16 bytes/LED
#elif defined(DUET3MINI)
constexpr size_t ChunkBufferSize = 80 * 16;							// NeoPixel RGBW use 16 bytes/LED (increased to 80 LEDs for Justin)
#elif defined(DUET_NG)
constexpr size_t ChunkBufferSize = 80 * 3;							// NeoPixel RGB use 3 bytes/LED
#else
constexpr size_t ChunkBufferSize = 60 * 16;							// DotStar LEDs use 4 bytes/LED, NeoPixel RGBW use 16 bytes/LED
#endif

#if SUPPORT_DMA_NEOPIXEL || SUPPORT_DMA_DOTSTAR

// We need an SPI port and DMA
# if LEDSTRIP_USES_USART
#  include <pdc/pdc.h>
#  include <pmc/pmc.h>
#  include <usart/usart.h>
# else
#  include <DmacManager.h>
#  if SAME5x
#   include <Hardware/IoPorts.h>
#   include <hri_mclk_e54.h>
#  elif SAME70
#   include <xdmac/xdmac.h>
#   include <pmc/pmc.h>
#  endif
# endif

# if SAME70
#  define USE_16BIT_SPI	1		// set to use 16-bit SPI transfers instead of 8-bit
# else
#  define USE_16BIT_SPI	0		// set to use 16-bit SPI transfers instead of 8-bit
# endif

# if USE_16BIT_SPI && LEDSTRIP_USES_USART
#  error Invalid combination
# endif

#endif	// SUPPORT_DMA_NEOPIXEL || SUPPORT_DMA_DOTSTAR

class LocalLedStrip : public LedStripBase
{
public:
	LocalLedStrip(uint32_t p_freq) noexcept;

protected:
	virtual bool IsNeopixel() const noexcept = 0;

#if SUPPORT_DMA_NEOPIXEL || SUPPORT_DMA_DOTSTAR
	void DmaSendChunkBuffer(size_t numBytes) noexcept;					// DMA the data. Must be a multiple of 2 bytes if USE_16BIT_SPI is true.
	bool DmaInProgress() noexcept;										// Return true if DMA to the LEDs is in progress
	void SetupSpi() noexcept;											// Setup the SPI peripheral. Only call this when the busy flag is not set.
#endif

private:
	uint32_t currentFrequency;											// the SPI frequency we are using

#if SUPPORT_DMA_NEOPIXEL || SUPPORT_DMA_DOTSTAR
	uint32_t whenDmaFinished = 0;										// the time in step clocks when we determined that the DMA had finished
	bool busy = false;													// true if DMA was started and is not known to have finished
#endif

#if SAME70
	alignas(4) static __nocache uint8_t chunkBuffer[ChunkBufferSize];	// buffer for sending data to LEDs
#else
	alignas(4) static uint8_t chunkBuffer[ChunkBufferSize];				// buffer for sending data to LEDs
#endif

};

#endif

#endif /* SRC_LEDSTRIPS_LOCALLEDSTRIP_H_ */