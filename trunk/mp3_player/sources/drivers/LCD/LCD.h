/*****************************************************************************
 *
 *   LCD.h:  LCD header file PANDORA MP3 
 *
******************************************************************************/
 // Character 16x2 4-Bit Mode Interface 
// D4 = PH0					   
// D5 = PH1					   
// D6 = PH2					   
// D7 = PH3						   
// RS = PH4
// RW = PH5
// EN = PH6
//SLEEP = PH7
#ifndef __LCD_H
#define __LCD_H
#include "driverlib/gpio.h"
// Define Port for LCD
#define LCD_Port    GPIO_PORTH_BASE
// Define PINs for LCD
#define  LCD_D4     GPIO_PIN_0				
#define  LCD_D5     GPIO_PIN_1   
#define  LCD_D6     GPIO_PIN_2  					
#define  LCD_D7     GPIO_PIN_3  						
#define  LCD_RS     GPIO_PIN_4    						
#define  LCD_RW     GPIO_PIN_5    					
#define  LCD_EN	    GPIO_PIN_6 	
#define  LCD_SLEEP  GPIO_PIN_7					
#define  LCD_DATA   (LCD_D4|LCD_D5|LCD_D6|LCD_D7)
#define  LCD_IOALL  (LCD_D4|LCD_D5|LCD_D6|LCD_D7|LCD_EN|LCD_RS|LCD_RW|LCD_SLEEP)

//#define  lcd_clear()          lcd_write_control(0x01);	// Clear Display
//#define  lcd_cursor_home()    lcd_write_control(0x02);	// Set Cursor = 0
//#define  lcd_display_on()     lcd_write_control(0x0E);	// LCD Display Enable
//#define  lcd_display_off()    lcd_write_control(0x08);	// LCD Display Disable
//#define  lcd_cursor_blink()   lcd_write_control(0x0F);	// Set Cursor = Blink
//#define  lcd_cursor_on()      lcd_write_control(0x0E);	// Enable LCD Cursor
//#define  lcd_cursor_off()     lcd_write_control(0x0C);	// Disable LCD Cursor
//#define  lcd_cursor_left()    lcd_write_control(0x10);	// Shift Left Cursor
//#define  lcd_cursor_right()   lcd_write_control(0x14);	// Shift Right Cursor
//#define  lcd_display_sleft()  lcd_write_control(0x18);	// Shift Left Display
//#define  lcd_display_sright() lcd_write_control(0x1C);	// Shift Right Display
//**********************************
/* pototype  section */
void lcd_init(void);										// Initial LCD
void gotoxy(unsigned char,unsigned char);						// Set Position Cursor LCD
void lcd_print(unsigned char*);							// Print Display to LCD
void lcd_print_number(unsigned long int num);
void lcd_print_number4(unsigned long int num);		
#endif	//end __LCD_H
