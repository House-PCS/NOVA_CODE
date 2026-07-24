
#include "dev_lcd.h"

/*备注：0x800的目的是，由于SDRAM和LCD共用EXMC总线，在LCD操作的过程中，SDRAM是需要预充电的，A10需要拉高*/
#define LCD_REG_ADDR        (ExmcHandle.sram_start_addr | 0x800)
#define LCD_DATA_ADDR       (ExmcHandle.sram_start_addr | 0x80000 | 0x800)

void DevLcdWriteReg(uint16_t reg)
{
    *(__IO uint16_t *)LCD_REG_ADDR = 0x00ff & reg;
}


void DevLcdWriteData(uint16_t data)
{
    *(__IO uint16_t *)LCD_DATA_ADDR = data;
}


uint16_t DevLcdReadData()
{
    return *(__IO uint16_t *)LCD_DATA_ADDR;
}


void DevLcdWriteMultiData(uint16_t * pData, int num)
{
    while(num--)
    {
        *(__IO uint16_t *)LCD_DATA_ADDR = *pData++;
    }
}

void DevLcdReadMultiData(uint16_t * pData, int num)
{
    while(num--)
    {
        *pData++ = *(__IO uint16_t *)LCD_DATA_ADDR;
    }
}


void DevLcdSetCursor(uint16_t x_pos, uint16_t y_pos)
{
    DevLcdWriteReg(0x2A);
    DevLcdWriteData(x_pos >> 8);
    DevLcdWriteData(x_pos);
    DevLcdWriteData(0x01);
    DevLcdWriteData(0xDF);

    DevLcdWriteReg(0x2B);
    DevLcdWriteData(y_pos >> 8);
    DevLcdWriteData(y_pos);
    DevLcdWriteData(0x01);
    DevLcdWriteData(0x3F);
}


void DevLcdWriteRAMCmd()
{
    DevLcdWriteReg(0x2C);
}

void DevLcdReadRAMCmd()
{
    DevLcdWriteReg(0x2E);
}




void DevLcdBLOpen()
{
    GpioBitReset(&LcdBLHandle);
}

void DevLcdBLClose()
{
    GpioBitSet(&LcdBLHandle);
}

void DevLcdRst()
{
    GpioBitReset(&LcdRstnHandle);//LCD nReset控制
    delay_1ms(1);   //复位脉冲最少需要10us
    GpioBitSet(&LcdRstnHandle); //LCD nReset控制
    delay_1ms(120); //复位后的延时不可省略， SleepInMode复位脉冲有效后需等待120ms 其他情况需等待5ms
}


void lcd_test()
{
    int x, y;
    DevLcdSetCursor(0, 0);
    DevLcdWriteRAMCmd();
    for(y = 0; y < 320; y++)
    {
        for(x = 0; x < 480; x++)
        {
//            DevLcdSetCursor(x, y);
//            DevLcdWriteRAMCmd();
            DevLcdWriteData(0x07eF);
//            DevLcdWriteData(0x0000);
        }
    }

}


static void DevIli9486Init2(void)
{
    DevLcdBLClose();
    DevLcdRst();

    DevLcdWriteReg(0xC0);
    DevLcdWriteData(0x17);
    DevLcdWriteData(0x17);

    DevLcdWriteReg(0xC1);
    DevLcdWriteData(0x44);

    DevLcdWriteReg(0xC5);
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x3A);//
    DevLcdWriteData(0x80);

    DevLcdWriteReg(0x36);
    DevLcdWriteData(0x68);

    DevLcdWriteReg(0x3A); //Interface Mode Control
    DevLcdWriteData(0x55);

    DevLcdWriteReg(0xB1);   //Frame rate 60HZ
    DevLcdWriteData(0xA0);

    DevLcdWriteReg(0xB4);
    DevLcdWriteData(0x02);

    DevLcdWriteReg(0xE9);
    DevLcdWriteData(0x00);

    DevLcdWriteReg(0XF7);
    DevLcdWriteData(0xA9);
    DevLcdWriteData(0x51);
    DevLcdWriteData(0x2C);
    DevLcdWriteData(0x82);

    DevLcdWriteReg(0xE0);
    DevLcdWriteData(0x01);
    DevLcdWriteData(0x13);
    DevLcdWriteData(0x1E);
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x0D);
    DevLcdWriteData(0x03);
    DevLcdWriteData(0x3D);
    DevLcdWriteData(0x55);
    DevLcdWriteData(0x4F);
    DevLcdWriteData(0x06);
    DevLcdWriteData(0x10);
    DevLcdWriteData(0x0B);
    DevLcdWriteData(0x2C);
    DevLcdWriteData(0x32);
    DevLcdWriteData(0x0F);

    DevLcdWriteReg(0xE1);
    DevLcdWriteData(0x08);
    DevLcdWriteData(0x10);
    DevLcdWriteData(0x15);
    DevLcdWriteData(0x03);
    DevLcdWriteData(0x0E);
    DevLcdWriteData(0x03);
    DevLcdWriteData(0x32);
    DevLcdWriteData(0x34);
    DevLcdWriteData(0x44);
    DevLcdWriteData(0x07);
    DevLcdWriteData(0x10);
    DevLcdWriteData(0x0E);
    DevLcdWriteData(0x23);
    DevLcdWriteData(0x2E);
    DevLcdWriteData(0x0F);
    /**********set rgb interface mode******************/
    DevLcdWriteReg(0xB6);
    DevLcdWriteData(0x00); //30 set rgb
    DevLcdWriteData(0x22); //GS,SS 02，42，62
    DevLcdWriteData(0x3B);
    /**************************************************/

    DevLcdWriteReg(0x2A); //Frame rate control
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x01);
    DevLcdWriteData(0x3F);

    DevLcdWriteReg(0x2B); //Display function control
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x01);
    DevLcdWriteData(0xDF);

    DevLcdWriteReg(0x21);

    DevLcdWriteReg(0x11);   //Sleep Out
    delay_1ms(100);
    DevLcdWriteReg(0x29);

//    DevLcdBLOpen();

//    lcd_test();
}


void DevIli9486Init()
{
    DevLcdBLClose();
    DevLcdRst();

    DevLcdWriteReg(0XF7);// A d j u s t C o n t r o l
    DevLcdWriteData(0xA9);
    DevLcdWriteData(0x51);
    DevLcdWriteData(0x2C);
    DevLcdWriteData(0x82);// D 7 s t r e a m , l o o s e

    DevLcdWriteReg(0xC0);//Power Control 1
    DevLcdWriteData(0x17);//Vreg1out 5V
    DevLcdWriteData(0x17);//Verg2out 5V

    DevLcdWriteReg(0xC1);//Power Control 2
    DevLcdWriteData(0x44);//VGH,VGL

    DevLcdWriteReg(0XC5);//Power Control 3
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x3A); //Vcom
    DevLcdWriteData(0x80);

    DevLcdWriteReg(0xB1);//Frame rate
    DevLcdWriteData(0xA0);//60Hz

    DevLcdWriteReg(0xB4);//Display Inversion Control
    DevLcdWriteData(0x02);

    DevLcdWriteReg(0xB6);//RGB/MCU Interface Control
    DevLcdWriteData(0x02);
    DevLcdWriteData(0x22);//Source,Gate scan dieection
    DevLcdWriteData(0x3B);

    DevLcdWriteReg(0xE9);// Set Image Function
    DevLcdWriteData(0x00);//disable 24 bit data input

    /*关键配置，配置不生效*/
    DevLcdWriteReg(0x36);//Memory Access
    DevLcdWriteData(0x78);
//    DevLcdWriteData(0x18);

    DevLcdWriteReg(0x3A);// Interface Pixel Format
    DevLcdWriteData(0x55);

    DevLcdWriteReg(0x21);//Normal Black
    DevLcdWriteReg(0xB0);// Interface Mode Control
    DevLcdWriteData(0x00);
    DevLcdWriteReg(0xE0);//P-Gamma
    //配置1
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x10);
    DevLcdWriteData(0x14);
    DevLcdWriteData(0x03);
    DevLcdWriteData(0x0E);
    DevLcdWriteData(0x04);
    DevLcdWriteData(0x36);
    DevLcdWriteData(0x56);
    DevLcdWriteData(0x4B);
    DevLcdWriteData(0x04);
    DevLcdWriteData(0x0C);
    DevLcdWriteData(0x0A);
    DevLcdWriteData(0x30);
    DevLcdWriteData(0x34);
    DevLcdWriteData(0x0F);
    //配置2
//    DevLcdWriteData(0x00);
//    DevLcdWriteData(0x07);
//    DevLcdWriteData(0x0C);
//    DevLcdWriteData(0x05);
//    DevLcdWriteData(0x13);
//    DevLcdWriteData(0x09);
//    DevLcdWriteData(0x36);
//    DevLcdWriteData(0xAA);
//    DevLcdWriteData(0x46);
//    DevLcdWriteData(0x09);
//    DevLcdWriteData(0x10);
//    DevLcdWriteData(0x0D);
//    DevLcdWriteData(0x1A);
//    DevLcdWriteData(0x1E);
//    DevLcdWriteData(0x0F);


    DevLcdWriteReg(0XE1);//N-Gamma
    //配置1
    DevLcdWriteData(0x00);
    DevLcdWriteData(0x0E);
    DevLcdWriteData(0x13);
    DevLcdWriteData(0x03);
    DevLcdWriteData(0x10);
    DevLcdWriteData(0x06);
    DevLcdWriteData(0x3E);
    DevLcdWriteData(0x34);
    DevLcdWriteData(0x55);
    DevLcdWriteData(0x05);
    DevLcdWriteData(0x0F);
    DevLcdWriteData(0x0E);
    DevLcdWriteData(0x3A);
    DevLcdWriteData(0x3E);
    DevLcdWriteData(0x0F);
    //配置2
//    DevLcdWriteData(0x00);
//    DevLcdWriteData(0x20);
//    DevLcdWriteData(0x23);
//    DevLcdWriteData(0x04);
//    DevLcdWriteData(0x10);
//    DevLcdWriteData(0x06);
//    DevLcdWriteData(0x37);
//    DevLcdWriteData(0x56);
//    DevLcdWriteData(0x49);
//    DevLcdWriteData(0x04);
//    DevLcdWriteData(0x0C);
//    DevLcdWriteData(0x0A);
//    DevLcdWriteData(0x33);
//    DevLcdWriteData(0x37);
//    DevLcdWriteData(0x0F);


    DevLcdWriteReg(0x11);
    delay_1ms(120);
    DevLcdWriteReg(0x29);

//    DevLcdBLOpen();

    lcd_test();
}




void DevLcdInit()
{
    LcdCtlBspInit();
//    SramBspInit();
    DevIli9486Init();
}













