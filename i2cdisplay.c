// *******************************
// * i2cdisplay.c
// * - routines to access HD44780 display via I2C
// *
// *******************************

#include "i2cdisplay.h"
#include <proto/i2c.h>
#include <libraries/i2c.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/***************************************************************/
/* variables to store pin-assignments:
 * - default-values according to "LCM1602 IIC V1" - PCB
*/
uint8_t _DisplayAdr = 0x4e;     // I2C Address of the IO expander (PCF8574)
uint8_t _Bl         = 0x03;     // LCD expander word for backlight pin
uint8_t _En         = 0x02;     // LCD expander word for enable pin
uint8_t _Rw         = 0x01;     // LCD expander word for R/W pin
uint8_t _Rs         = 0x00;     // LCD expander word for Register Select pin
uint8_t _data[4]    = {0x04,0x05,0x06,0x07};    // LCD data lines

//-------
uint8_t _Backlight  = 0;   // Backlight is turned off by default

/***************************
 * setup_displayinterface
 * - to assign all used display-pins to output of PCF8574-chip
 */
void setup_displayinterface(uint8_t displayI2CAdr, uint8_t En, uint8_t Rw, uint8_t Rs,
                            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t Bl )
{
    // store I2C-address shifted by 1
    _DisplayAdr = displayI2CAdr << 1;

    _En = ( 1 << En );
    _Rw = ( 1 << Rw );
    _Rs = ( 1 << Rs );
    _Bl = ( 1 << Bl );

    _data[0] = ( 1 << d4 );
    _data[1] = ( 1 << d5 );
    _data[2] = ( 1 << d6 );
    _data[3] = ( 1 << d7 );

    _Backlight = 0;
}

/***************************
 * send_2_pcf8574
 * - send the data via I2C to the PCF8574 to be passed
 *   to the display and use the right pin-assignments
 */
void send_2_pcf8574(uint8_t senddata, uint8_t mode)
{
    uint8_t pinMapValue = 0;

    for ( uint8_t i = 0; i < 4; ++i )
    {
        if ( 1 == ( senddata & 0x1 ) )
        {
            pinMapValue |= _data[i];
        }
        senddata = ( senddata >> 1 );
    }

    // Is it a command or data
    if ( MODE_DATA == mode )
    {
        pinMapValue |= _Rs;
    }
    pinMapValue |= _Backlight;
    pinMapValue |= _En ;
    (void) SendI2C( _DisplayAdr, 1, &pinMapValue);
    pinMapValue &= (~_En) ;
    (void) SendI2C( _DisplayAdr, 1, &pinMapValue);
}

/***************************
 * send_2_display
 * - this will send commands directly and split data in 2*4bit transmissions
 */
void send_2_display(uint8_t senddata, uint8_t mode)
{
    if ( MODE_4BIT == mode )
    {
        send_2_pcf8574((senddata & 0x0F), MODE_CMD );
    } else {
        send_2_pcf8574(((senddata >> 4) & 0x0F), mode);
        send_2_pcf8574( (senddata       & 0x0F), mode);
    }
}

/***************************
 * print_2_display
 * - rudimentary routine to send a char-array bytewise to the display
 */
void print_2_display(char* chardata)
{
    while (0 != (*chardata))
    {
        send_2_display(*chardata, MODE_DATA);
        ++chardata;
    }
}

/***************************
 * setpos_2_display
 * - change the display-address pointer to new position
 */
void setpos_2_display(uint8_t xpos, uint8_t ypos)
{
    uint8_t yposarray[] = { 0, 0x40, 0x10, 0x50 }; // 1604 - displays
    send_2_display ( CMD_SETDDRADDR | xpos | yposarray[ypos], MODE_CMD );
    usleep(500);   // wait 0.5ms
}

/***************************
 * init_display
 * - initialize a display behind the PCF8574
 * - use "2LINE"-LCDs and write from left to right
 * - clear the screen and turn on the backlight
 */
void init_display()
{
    send_2_display ( PCF8574_4BitMODE, MODE_CMD );
    usleep(4500);   // wait 4.5ms
    send_2_display ( PCF8574_4BitMODE, MODE_CMD );
    usleep(4500);   // wait 4.5ms
    send_2_display ( PCF8574_4BitMODE, MODE_CMD );
    usleep(150);
    send_2_display ( PCF8574_4BitInterface, MODE_CMD );
    usleep(150);

    send_2_display ( CMD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, MODE_CMD );
    usleep(4500);   // wait 4.5ms
    send_2_display ( CMD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, MODE_CMD );
    usleep(150);
    send_2_display ( CMD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, MODE_CMD );

    usleep(150);
    send_2_display ( CMD_DISPLAYCTL | LCD_DISPLAYON , MODE_CMD );
    send_2_display ( CMD_CLEARDISPLAY , MODE_CMD );
    send_2_display ( CMD_ENTRYMODE | LCD_ENTRYLEFT , MODE_CMD );

    _Backlight = _Bl;
    send_2_display ( CMD_RETURNHOME , MODE_CMD );
    usleep(4500);   // wait 4.5ms

    setpos_2_display( 0, 0 );
}
