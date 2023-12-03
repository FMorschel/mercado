void LCD5110_init(void);

void LCD5110_write_char(unsigned char c);

void LCD5110_write_char_reg(unsigned char c);

void LCD5110_clear(void);

void LCD5110_set_XY(unsigned char X, unsigned char Y);

void LCD5110_write_string(char *s);

void LCD5110_write_Dec(unsigned int buffer);

void LCD5110_LCD_write_byte(unsigned char dat,unsigned char mode);

void Nokia_BackLight(int temp);

void LCD5110_drawBitmap(int x, int y, uint8_t bitmap, int sx, int sy);

void enableSleep();
void disableSleep();
void update();
void clrScr();
void fillScr();
void invert(int mode);
void setPixel(uint16_t x, uint16_t y);
void clrPixel(uint16_t x, uint16_t y);
void invPixel(uint16_t x, uint16_t y);
void invertText(int mode);
void print(char *st, int x, int y);
//void print(String st, int x, int y);
//void printNumI(long num, int x, int y, int length=0, char filler=' ');
//void printNumF(double num, byte dec, int x, int y, char divider='.', int length=0, char filler=' ');
void setFont(uint8_t* font);
void drawBitmap(int x, int y, uint8_t* bitmap, int sx, int sy);
void drawLine(int x1, int y1, int x2, int y2);
void clrLine(int x1, int y1, int x2, int y2);
void drawRect(int x1, int y1, int x2, int y2);
void clrRect(int x1, int y1, int x2, int y2);
void drawRoundRect(int x1, int y1, int x2, int y2);
void clrRoundRect(int x1, int y1, int x2, int y2);
void drawCircle(int x, int y, int radius);
void clrCircle(int x, int y, int radius);
void LCD5110_drawLine(int x1, int y1, int x2, int y2);
