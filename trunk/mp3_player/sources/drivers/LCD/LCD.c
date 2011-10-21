//*****************************************************************************
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "LCD.h"
void enable_lcd(void);
void delay_us(unsigned long int microsecond);
//===================================
void lcd_out_data4(unsigned char val)
{  
      GPIOPinWrite(LCD_Port,LCD_DATA,0) ; 
      GPIOPinWrite(LCD_Port,LCD_DATA,val) ;
}

/****************************/
/* Write Data 1 Byte to LCD */
/****************************/
void lcd_write_byte(unsigned char val)
{  		
    lcd_out_data4((val>>4)&0x0F);	
    enable_lcd();			
    lcd_out_data4(val&0x0F);		
    enable_lcd();			
    delay_us(1500);
   // while(busy_lcd());     
}
/****************************/
/* Write Instruction to LCD */
/****************************/
void lcd_write_control(unsigned char val)
{ 
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RS,0);	
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RW,0);	
    lcd_write_byte(val);
}

/****************************/
/* Write Data(ASCII) to LCD */
/****************************/
void lcd_write_ascii(unsigned char c)
{  
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RS,(unsigned char)LCD_RS);		
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RW,0);
    lcd_write_byte(c);	 
}
/*******************************/
/* Initial 4-Bit LCD Interface */
/*******************************/
void lcd_init(void)
{
  
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); 
	GPIOPinTypeGPIOOutput(LCD_Port,(unsigned char)LCD_IOALL);
    delay_us(15000);	
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_IOALL,0);
    GPIOPinWrite(LCD_Port,(unsigned char)(LCD_D5|LCD_D4),(unsigned char)(LCD_D5|LCD_D4));
    enable_lcd();	
    delay_us(500);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_IOALL,0);
    GPIOPinWrite(LCD_Port,(unsigned char)(LCD_D5|LCD_D4),(unsigned char)(LCD_D5|LCD_D4));
    enable_lcd();
    delay_us(150);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_IOALL,0);
    GPIOPinWrite(LCD_Port,(unsigned char)(LCD_D5|LCD_D4),(unsigned char)(LCD_D5|LCD_D4));
    enable_lcd();	
    delay_us(150);
    //while(busy_lcd());
    delay_us(1500);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_IOALL,0);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_D5,(unsigned char)LCD_D5);
    enable_lcd();	
    delay_us(1500);
    //while(busy_lcd()); 
    lcd_write_control(0x28);  	// Function Set (DL=0 4-Bit,N=1 2 Line,F=0 5X7)
    lcd_write_control(0x0C);  	// Display on/off Control (Entry Display,Cursor off,Cursor not Blink)
    lcd_write_control(0x06);  	// Entry Mode Set (I/D=1 Increment,S=0 Cursor Shift)
    lcd_write_control(0x01);  	// Clear Display  (Clear Display,Set DD RAM Address=0) 
     delay_us(5000);
}

/***************************/
/* Set LCD Position Cursor */
/***************************/
void gotoxy(unsigned char row,unsigned char collum)
{	   unsigned char i;
  if (row ==0)	 
      i=collum;
 else 
	  i=(collum)|0x40;
      i =i|0x80;	
      lcd_write_control(i);  
	
}

/************************************/
/* Print Display Data(ASCII) to LCD */
/************************************/
void lcd_print(unsigned char* str)
{
  int i;

  for (i=0;i<16 && str[i]!=0;i++)  	// 16 Character Print
  {
      lcd_write_ascii(str[i]);		// Print Byte to LCD
  }
}

/******************/
/* Wait LCD Ready */
/******************/

char busy_lcd(void)
{
  unsigned long busy_status;	
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RS,0);	
    GPIOPinTypeGPIOInput(LCD_Port,(unsigned char)LCD_DATA);				
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RS,0);	 				
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RS,(unsigned char)LCD_RS);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_RW,(unsigned char)LCD_RW); 		
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_EN,(unsigned char)LCD_EN);	
    enable_lcd();
    delay_us(1000);
    busy_status = (GPIOPinRead(LCD_Port,(unsigned char)LCD_D7) & 0x08);

  if(busy_status == 0x08) 
  {
        GPIOPinWrite(LCD_Port,(unsigned char)LCD_EN,0);		
	GPIOPinWrite(LCD_Port,(unsigned char)LCD_RW,0);			
	GPIOPinTypeGPIOOutput(LCD_Port,(unsigned char)LCD_IOALL);	
    return 1;			
  }
  else
  {
       GPIOPinWrite(LCD_Port,(unsigned char)LCD_EN,0);			
	GPIOPinWrite(LCD_Port,(unsigned char)LCD_RW,0);				
	 GPIOPinTypeGPIOOutput(LCD_Port,(unsigned char)LCD_IOALL);		
    return 0;						
  }
}


/***********************/
/* Enable Pulse to LCD */
/***********************/
void enable_lcd(void)	 			
{
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_EN,(unsigned char)LCD_EN);					// Enable ON
    delay_us(100);
    GPIOPinWrite(LCD_Port,(unsigned char)LCD_EN,0);
}

/***********************/
/* milisecond delay funtion*/

/***********************/
void delay_us(unsigned long int microsecond)
{    
        SysCtlDelay(((unsigned long )microsecond*(SysCtlClockGet() / (3*1000000))));
}

void lcd_print_number(unsigned long int num)
{
	unsigned long int so_ascii ;
     so_ascii=num + 48;
    lcd_write_ascii(so_ascii);
}
void lcd_print_number4(unsigned long int num)
{unsigned long int so_bi_chia,so_ascii,tam1,tam2;
    so_bi_chia=num;
    tam1=so_bi_chia/1000;		
    tam2=so_bi_chia-(tam1*1000);
    so_ascii=tam1 + 48;
    if (so_ascii==48)
 	   lcd_write_ascii(32);
    else
 	lcd_write_ascii(so_ascii);
    tam1=tam2/100;			
    tam2=tam2-tam1*100;
    so_ascii=tam1+48;
    lcd_write_ascii(so_ascii);
    tam1=tam2/10;			
    tam2=tam2-tam1*10;
    so_ascii=tam1+48;
    lcd_write_ascii(so_ascii);
    so_ascii=tam2+48;	
    lcd_write_ascii(so_ascii);
 }
