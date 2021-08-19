/*
 * st7789.c
 *
 *  Created on: Jun 29, 2021
 *      Author: Liboras_VA
 */

#include "stm32l4xx_hal.h"
#include <st7789.h>
#include <gpio.h>
#include <stdlib.h>
#include "spi.h"
#include "font.h"
#include <string.h>
#include "display.h"
#include "tim.h"

uint16_t ST7789_Width, ST7789_Height;

void ST7789_Init(uint16_t Width, uint16_t Height)
{
	ST7789_Width = Width;
	ST7789_Height = Height;

	ST7789_SoftReset();
	ST7789_SleepModeExit();

	ST7789_ColorModeSet(ST7789_ColorMode_16bit);
	HAL_Delay(10);
	ST7789_MemAccessModeSet(3, 0, 0, 0);
	HAL_Delay(10);
	ST7789_InversionMode(1);
	HAL_Delay(10);
	ST7789_FillScreen(0);
	ST7789_SetBL(100);
	ST7789_DisplayPower(1);
}

void ST7789_InitFirst(void)
{
	ST7789_HardReset();
	ST7789_SoftReset();
	ST7789_SleepModeEnter();
}

void ST7789_HardReset(void)
{
	SPI_RESET_OFF();
	HAL_Delay(10);
	SPI_RESET_ON();
	HAL_Delay(1);
	SPI_RESET_OFF();
	HAL_Delay(120);
}

void ST7789_SoftReset(void)
{
	ST7789_SendCmd(ST7789_Cmd_SWRESET);
	HAL_Delay(120);
}

void ST7789_SendCmd(uint8_t Cmd)
{
	uint8_t CmdTrst = Cmd;
	SPI_DC_COMMAND();
	NSS_ON();
	if (READ_BIT(SPI3->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
		SET_BIT(SPI3->CR1, SPI_CR1_SPE);
	*((__IO uint8_t*) &SPI3->DR) = CmdTrst;
	NSS_OFF();
}

inline void ST7789_SendParametr(uint8_t Data)
{
	SPI_DC_DATA();
	NSS_ON();
	if (READ_BIT(SPI3->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
		SET_BIT(SPI3->CR1, SPI_CR1_SPE);
	*((__IO uint8_t*) &SPI3->DR) = Data;
	NSS_OFF();
}

void ST7789_RamWriteData(uint16_t Data, uint32_t Number)
{
	uint16_t DataTrst = Data;
	SPI_DC_DATA();
	NSS_ON();
	SPI_Transmit_Imp(&hspi3, (uint8_t*) &DataTrst, Number, 0xFFFF);
	NSS_OFF();
}

void ST7789_SleepModeEnter(void)
{
	ST7789_SendCmd(ST7789_Cmd_SLPIN);
	HAL_Delay(120);
}

void ST7789_SleepModeExit(void)
{
	ST7789_SendCmd(ST7789_Cmd_SLPOUT);
	//HAL_Delay(120);
	HAL_Delay(5);
}

void ST7789_ColorModeSet(uint8_t ColorMode)
{
	ST7789_SendCmd(ST7789_Cmd_COLMOD);
	ST7789_SendParametr(ColorMode & 0x77);
}

void ST7789_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror,
		uint8_t HorizMirror, uint8_t IsBGR)
{
	uint8_t Value;
	Rotation &= 7;

	if (READ_BIT(SPI3->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
		SET_BIT(SPI3->CR1, SPI_CR1_SPE);

	ST7789_SendCmd(ST7789_Cmd_MADCTL);

	switch (Rotation)
	{
	case 0:
		Value = 0;
		break;
	case 1:
		Value = ST7789_MADCTL_MX;
		break;
	case 2:
		Value = ST7789_MADCTL_MY;
		break;
	case 3:
		Value = ST7789_MADCTL_MX | ST7789_MADCTL_MY;
		break;
	case 4:
		Value = ST7789_MADCTL_MV;
		break;
	case 5:
		Value = ST7789_MADCTL_MV | ST7789_MADCTL_MX;
		break;
	case 6:
		Value = ST7789_MADCTL_MV | ST7789_MADCTL_MY;
		break;
	case 7:
		Value = ST7789_MADCTL_MV | ST7789_MADCTL_MX | ST7789_MADCTL_MY;
		break;
	}

	if (VertMirror)
		Value |= ST7789_MADCTL_ML;
	if (HorizMirror)
		Value |= ST7789_MADCTL_MH;

	if (IsBGR)
		Value |= ST7789_MADCTL_BGR;

	ST7789_SendParametr(Value);
}

void ST7789_InversionMode(uint8_t Mode)
{
	if (READ_BIT(SPI3->CR1, SPI_CR1_SPE) != (SPI_CR1_SPE))
		SET_BIT(SPI3->CR1, SPI_CR1_SPE);

	if (Mode)
		ST7789_SendCmd(ST7789_Cmd_INVON);
	else
		ST7789_SendCmd(ST7789_Cmd_INVOFF);
}

void ST7789_FillScreen(uint16_t color)
{
	ST7789_FillRect(0, 80, ST7789_Width, ST7789_Height, color);
}

void ST7789_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	if ((x >= ST7789_Width) || (y >= ST7789_Height))
		return;
	if ((x + w) > ST7789_Width)
		w = ST7789_Width - x;
	if ((y + h) > ST7789_Height)
		h = ST7789_Height - y;

	ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
	uint32_t j = 2 * h * w;		//115200 bit max
	ST7789_RamWriteData(color, j);
}

void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{

	ST7789_ColumnSet(x0, x1);
	ST7789_RowSet(y0, y1);
	ST7789_SendCmd(ST7789_Cmd_RAMWR);
}

void ST7789_RamWrite(uint16_t *pBuff)	//, uint16_t Len
{
	uint8_t col1 = *pBuff >> 8;
	uint8_t col2 = *pBuff & 0xFF;

	ST7789_SendParametr(col1);
	ST7789_SendParametr(col2);
}

void ST7789_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd)
{
	if (ColumnStart > ColumnEnd)
		return;
	if (ColumnEnd > ST7789_Width)
		return;

	ColumnStart += ST7789_X_Start;
	ColumnEnd += ST7789_X_Start;

	ST7789_SendCmd(ST7789_Cmd_CASET);
	ST7789_SendParametr(ColumnStart >> 8);
	ST7789_SendParametr(ColumnStart & 0xFF);
	ST7789_SendParametr(ColumnEnd >> 8);
	ST7789_SendParametr(ColumnEnd & 0xFF);
}

void ST7789_RowSet(uint16_t RowStart, uint16_t RowEnd)
{
	if (RowStart > RowEnd)
		return;
	if (RowEnd > ST7789_Height)
		return;

	RowStart += ST7789_Y_Start;
	RowEnd += ST7789_Y_Start;

	ST7789_SendCmd(ST7789_Cmd_RASET);
	ST7789_SendParametr(RowStart >> 8);
	ST7789_SendParametr(RowStart & 0xFF);
	ST7789_SendParametr(RowEnd >> 8);
	ST7789_SendParametr(RowEnd & 0xFF);
}

void ST7789_SetBL(uint8_t Value)
{
	if (Value > 100)
		Value = 100;

	if (Value)
		LIGHT_ON();
	else
		LIGHT_OFF();
}

void ST7789_DisplayPower(uint8_t On)
{

	if (On)
		ST7789_SendCmd(ST7789_Cmd_DISPON);
	else
		ST7789_SendCmd(ST7789_Cmd_DISPOFF);
}

void ST7789_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color)
{

	ST7789_DrawLine(x1, y1, x1, y2, color);
	ST7789_DrawLine(x2, y1, x2, y2, color);
	ST7789_DrawLine(x1, y1, x2, y1, color);
	ST7789_DrawLine(x1, y2, x2, y2, color);
}

void ST7789_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t fillcolor)
{
	if (x1 > x2)
		SwapInt16Values(&x1, &x2);
	if (y1 > y2)
		SwapInt16Values(&y1, &y2);
	ST7789_FillRect(x1, y1, x2 - x1, y2 - y1, fillcolor);
}

void ST7789_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color)
{
	// vertical line
	if (x1 == x2)
	{
		// Drawing the line with the quick method
		if (y1 > y2)
			ST7789_FillRect(x1, y2, 1, y1 - y2 + 1, color);
		else
			ST7789_FillRect(x1, y1, 1, y2 - y1 + 1, color);
		return;
	}

	// horizontal line
	if (y1 == y2)
	{
		// Drawing the line with the quick method
		if (x1 > x2)
			ST7789_FillRect(x2, y1, x1 - x2 + 1, 1, color);
		else
			ST7789_FillRect(x1, y1, x2 - x1 + 1, 1, color);
		return;
	}

	ST7789_DrawLine_Slow(x1, y1, x2, y2, color);
}

void SwapInt16Values(int16_t *pValue1, int16_t *pValue2)
{
	int16_t TempValue = *pValue1;
	*pValue1 = *pValue2;
	*pValue2 = TempValue;
}

void ST7789_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color)
{
	const int16_t deltaX = abs(x2 - x1);
	const int16_t deltaY = abs(y2 - y1);
	const int16_t signX = x1 < x2 ? 1 : -1;
	const int16_t signY = y1 < y2 ? 1 : -1;

	int16_t error = deltaX - deltaY;

	ST7789_DrawPixel(x2, y2, color);

	while (x1 != x2 || y1 != y2)
	{
		ST7789_DrawPixel(x1, y1, color);
		const int16_t error2 = error * 2;

		if (error2 > -deltaY)
		{
			error -= deltaY;
			x1 += signX;
		}
		if (error2 < deltaX)
		{
			error += deltaX;
			y1 += signY;
		}
	}
}

void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= ST7789_Width) || (y < 0) || (y >= ST7789_Height))
		return;

	ST7789_SetWindow(x, y, x, y);
	ST7789_RamWrite(&color);
}

void ST7789_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius,
		uint16_t fillcolor)
{
	int x = 0;
	int y = radius;
	int delta = 1 - 2 * radius;
	int error = 0;

	while (y >= 0)
	{
		ST7789_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
		ST7789_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
		error = 2 * (delta + y) - 1;

		if (delta < 0 && error <= 0)
		{
			++x;
			delta += 2 * x + 1;
			continue;
		}

		error = 2 * (delta - x) - 1;

		if (delta > 0 && error > 0)
		{
			--y;
			delta += 1 - 2 * y;
			continue;
		}

		++x;
		delta += 2 * (x - y);
		--y;
	}
}

void ST7789_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
{
	int x = 0;
	int y = radius;
	int delta = 1 - 2 * radius;
	int error = 0;

	while (y >= 0)
	{
		ST7789_DrawPixel(x0 + x, y0 + y, color);
		ST7789_DrawPixel(x0 + x, y0 - y, color);
		ST7789_DrawPixel(x0 - x, y0 + y, color);
		ST7789_DrawPixel(x0 - x, y0 - y, color);
		error = 2 * (delta + y) - 1;

		if (delta < 0 && error <= 0)
		{
			++x;
			delta += 2 * x + 1;
			continue;
		}

		error = 2 * (delta - x) - 1;

		if (delta > 0 && error > 0)
		{
			--y;
			delta += 1 - 2 * y;
			continue;
		}

		++x;
		delta += 2 * (x - y);
		--y;
	}
}

void ST7789_DrawChar_7x11(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j;
	uint8_t buffer[11];

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return;

	// Copy selected simbol to buffer
	memcpy(buffer, &font7x11[(c - 32) * 11], 11);
	for (j = 0; j < 11; j++)
	{
		for (i = 0; i < 7; i++)
		{
			if ((buffer[j] & (1 << i)) == 0)
			{
				if (TransparentBg == 0)
					ST7789_DrawPixel(x + i, y + j, BgColor);
			}
			else
				ST7789_DrawPixel(x + i, y + j, TextColor);
		}
	}
}

void ST7789_DrawChar_11x18(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j;
	uint16_t buffer[18];

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return;

	// Copy selected simbol to buffer
	memcpy(buffer, &font11x18[(c - 32) * 18], 36);
	for (j = 0; j < 18; j++)
	{
		for (i = 0; i < 16; i++)
		{
			if ((buffer[j] & (1 << (16 - i))) == 0)
			{
				//if (TransparentBg == 0) ST7789_DrawPixel(x + (16 - i), y + j, BgColor);
				if (TransparentBg == 0)
					ST7789_DrawPixel(x + i, y + j, BgColor);
			}
			//else ST7789_DrawPixel(x + (16 - i), y + j, TextColor);
			else
				ST7789_DrawPixel(x + i, y + j, TextColor);
		}
	}
}

void ST7789_DrawChar_8x13(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j;
	uint8_t buffer[13];

	uint16_t CharBuffer[104] =
	{ 0 };

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return;

	// Copy selected simbol to buffer
	memcpy(buffer, &Arial_8x13_Table[(c - 32) * 13], 13);
	for (j = 0; j < 13; j++)
	{
		for (i = 0; i < 8; i++)
		{
			if ((buffer[j] & (1 << (7 - i))) == 0)
			{
				CharBuffer[i + 8 * j] = BgColor;
				//if (TransparentBg == 0) ST7789_DrawPixel(x + (7 - i), y + j, BgColor);
			}
			else
			{
				CharBuffer[i + 8 * j] = TextColor;
				//ST7789_DrawPixel(x + (7 - i), y + j, TextColor);
			}
		}
	}

	ST7789_SetWindow(x, y, x + 7, y + 12);
	SPI_DC_DATA();
	NSS_ON();
	HAL_SPI_Transmit(&hspi3, (uint8_t*) CharBuffer, 208, 0xFFFF);
	NSS_OFF();
}

void ST7789_print_7x11(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128)
		x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_7x11(x, y, TextColor, BgColor, TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128)
			x = x + 8;
		else
			x = x + 8;
	}
}

void ST7789_print_11x18(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128)
		x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_11x18(x, y, TextColor, BgColor, TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128)
			x = x + 11;
		else
			x = x + 11;
	}
}

void ST7789_print_8x13(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128)
		x = x - 3;
	while (*str)
	{
		ST7789_DrawChar_8x13(x, y, TextColor, BgColor, TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128)
			x = x + 9;
		else
			x = x + 9;
	}
}

static uint8_t height = 0;
static uint8_t width = 0;
;
static void selectWHchargen(FONT_TYPE font)
{
	switch (font)
	{
	case FONT_TYPE_5x8:
		width = 1;
		height = 1;
		break;
	case FONT_TYPE_5x15:
		width = 1;
		height = 2;
		break;
	case FONT_TYPE_5x23:
		width = 1;
		height = 3;
		break;
	case FONT_TYPE_5x31:
		width = 1;
		height = 4;
		break;
	case FONT_TYPE_10x8:
		width = 2;
		height = 1;
		break;
	case FONT_TYPE_10x15:
		width = 2;
		height = 2;
		break;
	case FONT_TYPE_10x23:
		width = 2;
		height = 3;
		break;
	case FONT_TYPE_10x31:
		width = 2;
		height = 4;
		break;
	case FONT_TYPE_15x8:
		width = 3;
		height = 1;
		break;
	case FONT_TYPE_15x15:
		width = 3;
		height = 2;
		break;
	case FONT_TYPE_15x23:
		width = 3;
		height = 3;
		break;
	case FONT_TYPE_15x31:
		width = 3;
		height = 4;
		break;
	case FONT_TYPE_20x8:
		width = 4;
		height = 1;
		break;
	case FONT_TYPE_20x15:
		width = 4;
		height = 2;
		break;
	case FONT_TYPE_20x23:
		width = 4;
		height = 3;
		break;
	case FONT_TYPE_20x31:
		width = 4;
		height = 4;
		break;
	default:
		width = 1;
		height = 1;
	}
}

void ST7789_DrawChar(uint16_t x, uint16_t y, FONT_TYPE font, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t h_temp, w_temp;
	uint8_t buffer[5];
	selectWHchargen(font);
	//uint16_t CharBuffer[5 * 8 * 4 * 4] = {0};	//new

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return;

	// Copy selected simbol to buffer
	memcpy(buffer, &chargen[(c - 0x20) * 5], 5);

	uint8_t i = 0;
	uint8_t j = 0;

	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < 8; i++)
		{
			h_temp = 0;
			if ((buffer[j] & (1 << (7 - i))) == 0)
			{
				while (h_temp < height)
				{
					w_temp = 0;
					while (w_temp < width)
					{
						if (TransparentBg == 0)
						{
							//CharBuffer[(i * height + h_temp) + (j * 8 * width + w_temp * 8 * height)] = BgColor;
							ST7789_DrawPixel(x + (j * width + w_temp),
									y + (i * height + h_temp), BgColor);
						}
						w_temp++;
					}
					h_temp++;
				}
			}
			else
			{
				while (h_temp < height)
				{
					w_temp = 0;
					while (w_temp < width)
					{
						//CharBuffer[(i * height + h_temp) + (j * 8 * width + w_temp * 8 * height)] = TextColor;
						ST7789_DrawPixel(x + (j * width + w_temp),
								y + (i * height + h_temp), TextColor);
						w_temp++;
					}
					h_temp++;
				}
			}
		}
	}
	/*	ST7789_SetWindow(x, y, x + 4 * width, y + 7 * height);
	 SPI_DC_DATA();
	 NSS_ON();
	 HAL_SPI_Transmit(&hspi3, (uint8_t*) CharBuffer, (5 * width * 8 * height), 0xFFFF);
	 NSS_OFF();*/
}

void ST7789_print(uint16_t x, uint16_t y, FONT_TYPE font, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	while (*str)
	{
		ST7789_DrawChar(x, y, font, TextColor, BgColor, TransparentBg, *str++);
		x = x + (6 * width);
	}
}

void ST7789_DrawCharArial8x13(uint16_t x, uint16_t y, FONT_TYPE font,
		uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg,
		unsigned char c)
{
	uint8_t h_temp, w_temp;
	uint8_t buffer[13];
	selectWHchargen(font);

	uint16_t CharBuffer[8 * 13 * 4 * 4] =
	{ 0 };
	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return;

	// Copy selected simbol to buffer
	memcpy(buffer, &Arial_8x13_Table[(c - 0x20) * 13], 13);

	uint8_t i = 0;
	uint8_t j = 0;

	for (j = 0; j < 13; j++)
	{
		for (i = 0; i < 8; i++)
		{
			h_temp = 0;
			if ((buffer[j] & (1 << (7 - i))) == 0)
			{
				while (h_temp < height)
				{
					w_temp = 0;
					while (w_temp < width)
					{
						if (TransparentBg == 0)
						{
							CharBuffer[(i * height + h_temp)
									+ (j * 8 * height * width
											+ w_temp * 8 * height)] = BgColor;
							//ST7789_DrawPixel(x + (i * height + h_temp),	y + (j * width + w_temp), BgColor);
						}
						w_temp++;
					}
					h_temp++;
				}
			}
			else
			{
				while (h_temp < height)
				{
					w_temp = 0;
					while (w_temp < width)
					{
						CharBuffer[(i * height + h_temp)
								+ (j * 8 * height * width + w_temp * 8 * height)] =
								TextColor;
						//ST7789_DrawPixel(x + (i * height + h_temp),	y + (j * width + w_temp), TextColor);
						w_temp++;
					}
					h_temp++;
				}
			}
		}
	}
	ST7789_SetWindow(x, y, x + 8 * width - 1, y + 13 * height - 1);
	SPI_DC_DATA();
	NSS_ON();
	HAL_SPI_Transmit(&hspi3, (uint8_t*) CharBuffer,
			2 * (8 * width * 13 * height), 0xFFFF);
	NSS_OFF();
}

void ST7789_print_Arial8x13(uint16_t x, uint16_t y, FONT_TYPE font,
		uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	while (*str)
	{
		ST7789_DrawCharArial8x13(x, y, font, TextColor, BgColor, TransparentBg,
				*str++);
		x = x + (8 * width);
	}
}

uint16_t ST7789_DrawCharVerdana20x21(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j, b;
	uint8_t buffer[60];

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return 0;

	uint16_t wSymbol = (c - 32) * 61;
	uint8_t longColumn = 3;
	uint8_t byte = 8;

	// Copy selected simbol to buffer
	memcpy(buffer, &Verdana20x21[wSymbol + 1],
			longColumn * Verdana20x21[wSymbol]);

	for (j = 0; j < Verdana20x21[wSymbol]; j++)
	{
		b = 0;
		while (b < longColumn)
		{
			for (i = 0; i < 8; i++)
			{

				if ((buffer[longColumn * j + b] & (1 << i)) == 0)
				{
					if (TransparentBg == 0)
						ST7789_DrawPixel(x + j, y + (i + byte * b), BgColor);
				}
				else
				{
					ST7789_DrawPixel(x + j, y + (i + byte * b), TextColor);
				}
			}
			b++;
		}
	}
	return Verdana20x21[wSymbol];
}

void ST7789_print_Verdana20x21(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128)
		x = x - 3;
	while (*str)
	{
		uint16_t wSimbol = ST7789_DrawCharVerdana20x21(x, y, TextColor, BgColor,
				TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128)
			x = x + wSimbol + 2;
		else
			x = x + wSimbol + 2;
	}
}

uint16_t ST7789_DrawCharTahoma17x19(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j, b;
	uint8_t buffer[51];

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return 0;

	uint16_t wSymbol = (c - 32) * 52;
	uint8_t longColumn = 3;
	uint8_t byte = 8;

	// Copy selected simbol to buffer
	memcpy(buffer, &Tahoma17x19[wSymbol + 1],
			longColumn * (Tahoma17x19[wSymbol] + 2));

	uint16_t CharBuffer[byte * longColumn * (Tahoma17x19[wSymbol] + 2)];

	b = 0;
	while (b < longColumn)
	{
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < Tahoma17x19[wSymbol] + 2; j++)
			{
				if ((buffer[j * longColumn + b] & (1 << i)) == 0)
				{
					if (TransparentBg == 0)
					{
						CharBuffer[j + i * (Tahoma17x19[wSymbol] + 2)
								+ b * byte * (Tahoma17x19[wSymbol] + 2)] =
								BgColor;
					}
				}
				else
				{
					CharBuffer[j + i * (Tahoma17x19[wSymbol] + 2)
							+ b * byte * (Tahoma17x19[wSymbol] + 2)] =
							TextColor;
				}
			}
		}
		b++;
	}
	ST7789_SetWindow(x, y, x + Tahoma17x19[wSymbol] + 2 - 1,
			y + longColumn * byte - 1);
	SPI_DC_DATA();
	NSS_ON();
	HAL_SPI_Transmit(&hspi3, (uint8_t*) CharBuffer,
			2 * (byte * longColumn * (Tahoma17x19[wSymbol] + 2)), 0xFFFF);
	NSS_OFF();
	return Tahoma17x19[wSymbol] + 2;
}

void
ST7789_print_Tahoma17x19 (uint16_t x, uint16_t y, uint16_t TextColor,
			   uint16_t BgColor, uint8_t TransparentBg, char *str)
{
  unsigned char type = *str;
  if (type >= 128)
    x = x - 3;
  while (*str)
    {
      uint16_t wSimbol = ST7789_DrawCharTahoma17x19 (x, y, TextColor, BgColor, TransparentBg, *str++);
      unsigned char type = *str;
      if (type >= 128)
	x = x + wSimbol;
      else
	x = x + wSimbol;
    }
  ST7789_DrawCharTahoma17x19 (x, y, TextColor, BgColor, TransparentBg, ' ');
}

uint16_t ST7789_DrawCharTahoma42x50(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, unsigned char c)
{
	uint8_t i, j, b;
	uint8_t buffer[294];

	if ((x >= ST7789_Width) || (y >= ST7789_Height) || ((x + 4) < 0)
			|| ((y + 7) < 0))
		return 0;

	uint16_t wSymbol = (c - 32) * 295;
	uint8_t longColumn = 7;
	uint8_t byte = 8;

	// Copy selected simbol to buffer
	memcpy(buffer, &Tahoma42x50[wSymbol + 1],
			longColumn * (Tahoma42x50[wSymbol] + 3));

	uint16_t CharBuffer[byte * longColumn * (Tahoma42x50[wSymbol] + 3)];

	b = 0;
	while (b < longColumn)
	{
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < Tahoma42x50[wSymbol] + 3; j++)
			{
				if ((buffer[j * longColumn + b] & (1 << i)) == 0)
				{
					if (TransparentBg == 0)
					{
						CharBuffer[j + i * (Tahoma42x50[wSymbol] + 3)
								+ b * byte * (Tahoma42x50[wSymbol] + 3)] =
								BgColor;
					}
				}
				else
				{
					CharBuffer[j + i * (Tahoma42x50[wSymbol] + 3)
							+ b * byte * (Tahoma42x50[wSymbol] + 3)] =
							TextColor;
				}
			}
		}
		b++;
	}
	ST7789_SetWindow(x, y, x + Tahoma42x50[wSymbol] + 3 - 1,
			y + longColumn * byte - 1);
	SPI_DC_DATA();
	NSS_ON();
	HAL_SPI_Transmit(&hspi3, (uint8_t*) CharBuffer,
			2 * (byte * longColumn * (Tahoma42x50[wSymbol] + 3)), 0xFFFF);
	NSS_OFF();
	return Tahoma42x50[wSymbol] + 3;
}

void ST7789_print_Tahoma42x50(uint16_t x, uint16_t y, uint16_t TextColor,
		uint16_t BgColor, uint8_t TransparentBg, char *str)
{
	unsigned char type = *str;
	if (type >= 128)
		x = x - 3;
	while (*str)
	{
		uint16_t wSimbol = ST7789_DrawCharTahoma42x50(x, y, TextColor, BgColor,
				TransparentBg, *str++);
		unsigned char type = *str;
		if (type >= 128)
			x = x + wSimbol;
		else
			x = x + wSimbol;
	}
	ST7789_DrawCharTahoma42x50(x, y, TextColor, BgColor, TransparentBg, ' ');
	ST7789_DrawCharTahoma42x50(x + 16, y, TextColor, BgColor, TransparentBg, ' ');
	ST7789_DrawCharTahoma42x50(x + 32, y, TextColor, BgColor, TransparentBg, ' ');
	ST7789_DrawCharTahoma42x50(x + 48, y, TextColor, BgColor, TransparentBg, ' ');
	ST7789_DrawCharTahoma42x50(x + 64, y, TextColor, BgColor, TransparentBg, ' ');
	ST7789_DrawCharTahoma42x50(x + 80, y, TextColor, BgColor, TransparentBg, ' ');
}

void ST7789_DrawBatteryLevel(uint16_t x, uint16_t y, uint16_t color)
{
	uint8_t x1, y1, x2, y2;
	uint8_t x3, y3, x4, y4;
	uint8_t x5, y5, x6, y6;

	float ChargeLevel = 55;

	ChargeLevel = 55 / 100.0 * 25;

	x1 = x;
	y1 = y;
	x2 = x1 + 28;
	y2 = y1 + 12;

	x3 = x2;
	y3 = y1 + 2;
	x4 = x3 + 3;
	y4 = y3 + 9;

	x5 = x1 + 3;	//holl rectangle:
	y5 = y1 + 3;
	x6 = x1 + ChargeLevel;	//x6[x1; x1 + 25];
	y6 = y1 + 10;

	ST7789_DrawRectangle(x1, y1, x2, y2, color);
	ST7789_DrawRectangleFilled(x3, y3, x4, y4, color);

	ST7789_DrawRectangleFilled(x5, y5, x6, y6, color);
}

