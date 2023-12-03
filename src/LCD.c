#include "stm32f4xx.h"
#include "LCD.h"
#include "font.h"
//#include "grafic.h"



#define LCD_CS 2 //CE
#define LCD_RST 3 //RST
#define LCD_MO 1 //DIN
#define LCD_SCK 8 //CLK
#define LCD_DC 0 //DO
#define LCD_BL 9 //BL (backLight)
#define PORT GPIOC //GPIO onde está o display



#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define false 0
#define true 1

 int scrbuf[504];


//Define the LCD Operation function
void LCD5110_LCD_write_byte(unsigned char dat,unsigned char LCD5110_MOde);
void LCD5110_LCD_delay_ms(unsigned int t);

//Define the hardware operation function
void LCD5110_GPIO_Config(void);
void LCD5110_SCK(unsigned char temp);
void LCD5110_MO(unsigned char temp);
void LCD5110_CS(unsigned char temp);
void LCD5110_RST(unsigned char temp);
void LCD5110_DC(unsigned char temp);




void LCD5110_init()
{

	LCD5110_GPIO_Config();

	LCD5110_DC(1);//LCD_DC = 1;
	LCD5110_MO(1);//SPI_MO = 1;
	LCD5110_SCK(1);//SPI_SCK = 1;
	LCD5110_CS(1);//SPI_CS = 1;
	
	LCD5110_RST(0);//LCD_RST = 0;
	LCD5110_LCD_delay_ms(10);
	LCD5110_RST(1);//LCD_RST = 1;

	LCD5110_LCD_write_byte(0x21,0); // function set h=0
	LCD5110_LCD_write_byte(0xc3,0); // contrast (0x90 to 0xff)
	LCD5110_LCD_write_byte(0x20,0);  // function set h=1
	LCD5110_clear();
	LCD5110_LCD_write_byte(0x0c,0); // display control
}

void LCD5110_GPIO_Config()
{

	PORT->MODER&=~(3<<(LCD_CS*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_CS*2));	//seta os bits

	PORT->MODER&=~(3<<(LCD_RST*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_RST*2));	//seta os bits

	PORT->MODER&=~(3<<(LCD_MO*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_MO*2));	//seta os bits

	PORT->MODER&=~(3<<(LCD_SCK*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_SCK*2));	//seta os bits

	PORT->MODER&=~(3<<(LCD_DC*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_DC*2));	//seta os bits

	PORT->MODER&=~(3<<(LCD_BL*2));	//limpa os bits
	PORT->MODER|=(1<<(LCD_BL*2));	//seta os bits
}

void Nokia_BackLight(int temp)
{
	if(temp) PORT->ODR |= (1<<LCD_BL);
	else PORT->ODR &=~ (1<<LCD_BL);
}

void LCD5110_clear()
{
	unsigned char i,j;
	for(i=0;i<6;i++)
		for(j=0;j<84;j++)
			LCD5110_LCD_write_byte(0,1);	
}

void LCD5110_set_XY(unsigned char X,unsigned char Y)
{
	unsigned char x;
	x = 6*X;

	LCD5110_LCD_write_byte(0x40|Y,0);
	LCD5110_LCD_write_byte(0x80|x,0);
}

void LCD5110_LCD_delay_ms(unsigned int nCount)
{
  unsigned long t;
	t = nCount * 40000;
	while(t--);
}



void LCD5110_CS(unsigned char temp)
{
	if (temp) PORT->ODR|=1<<LCD_CS;
	else PORT->ODR&=~(1<<LCD_CS);


}

void LCD5110_RST(unsigned char temp)
{
	if (temp) PORT->ODR|=1<<LCD_RST;
	else PORT->ODR&=~(1<<LCD_RST);

}

void LCD5110_DC(unsigned char temp)
{
	if (temp) PORT->ODR|=1<<LCD_DC;
	else PORT->ODR&=~(1<<LCD_DC);

}

void LCD5110_MO(unsigned char temp)
{
	if (temp) PORT->ODR|=1<<LCD_MO;
	else PORT->ODR&=~(1<<LCD_MO);

}

void LCD5110_SCK(unsigned char temp)
{
	if (temp) PORT->ODR|=1<<LCD_SCK;
	else PORT->ODR&=~(1<<LCD_SCK);
}

/* Para usar a função printf */
int _write(int file, char *ptr, int len)
{
  int i=0;
  for(i=0 ; i<len ; i++)
	  LCD5110_write_char(*ptr++);
  return len;
}

void LCD5110_clrScr()
{
   for (int c=0; c<504; c++)
      scrbuf[c]=0;
}

void LCD5110_fillScr()
{
   for (int c=0; c<504; c++)
      scrbuf[c]=255;
}

void LCD5110_LCD_write_byte(unsigned char dat,unsigned char mode)
{
	unsigned char i;

	LCD5110_CS(0);//SPI_CS = 0;

	if (0 == mode)
		LCD5110_DC(0);//LCD_DC = 0;
	else
		LCD5110_DC(1);//LCD_DC = 1;

	for(i=0;i<8;i++)
	{
		LCD5110_MO(dat & 0x80);//SPI_MO = dat & 0x80;
		dat = dat<<1;
		LCD5110_SCK(0);//SPI_SCK = 0;
		LCD5110_SCK(1);//SPI_SCK = 1;
	}

	LCD5110_CS(1);//SPI_CS = 1;

}

void LCD5110_write_char(unsigned char c)
{
	unsigned char line;
	unsigned char ch = 0;

	c = c - 32;

	for(line=0;line<6;line++)
	{
		ch = font6_8[c][line];
		LCD5110_LCD_write_byte(ch,1);

	}
}
void LCD5110_write_char_reg(unsigned char c)
{
	unsigned char line;
	unsigned char ch = 0;

	c = c - 32;

	for(line=0;line<6;line++)
	{
		ch = ~font6_8[c][line];
		LCD5110_LCD_write_byte(ch,1);

	}
}

void LCD5110_write_string(char *s)
{
	unsigned char ch;
  	while(*s!='\0')
	{
		ch = *s;
		LCD5110_write_char(ch);
		s++;
	}
}

void setPixel(uint16_t x, uint16_t y)
{
   int by, bi;

   if ((x>=0) && (x<84) && (y>=0) && (y<48))
   {
      by=((y/8)*84)+x;
      bi=y % 8;

      scrbuf[by]=scrbuf[by] | (1<<bi);
   }
}

void LCD5110_clrPixel(uint16_t x, uint16_t y)
{
   int by, bi;

   if ((x>=0) && (x<84) && (y>=0) && (y<48))
   {
      by=((y/8)*84)+x;
      bi=y % 8;

      scrbuf[by]=scrbuf[by] & ~(1<<bi);
   }
}
/*
void LCD5110_drawBitmap(int x, int y, uint8_t* bitmap, int sx, int sy)
{
   int bit;
   char data;

   for (int cy=0; cy<sy; cy++)
   {
      bit= cy % 8;
      for(int cx=0; cx<sx; cx++)
      {
         data=bitmap[cx+((cy/8)*sx)];
         if ((data & (1<<bit))>0)
        	 setPixel(x+cx, y+cy);
         else
        	 LCD5110_clrPixel(x+cx, y+cy);
      }
   }
}//*/
