/******************************************************************************
 *
 *****************************************************************************/
#ifndef SHARP_DISPLAY_H
#define SHARP_DISPLAY_H

bool SharpMemInit(uint8_t clk, uint8_t mosi, uint8_t ss, uint16_t w = 96, uint16_t h = 96);
bool SharpMemSetPixel(int16_t x, int16_t y, uint16_t color);
bool SharpMemGetPixel(uint16_t x, uint16_t y);
void SharpMemClearDisplay();
void SharpMemRefresh(void);

#endif /* SHARP_DISPLAY_H */