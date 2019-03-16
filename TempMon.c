/*      -------------------------------------------------
        TempMon.c (01.06.2015) Marcus Gerards
        
        Temperature Monitor's core        
        -------------------------------------------------
*/

#include "app.h"
#include "local.h"
#include "TempMon.h"
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <i2c_library.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define HIDETEMP 0L
#define DISPLAYTEMP 1L                 

/* Globals */
struct Window *TWindow;

/* Timer port & stuff */
extern struct MsgPort *TimePort;
extern struct timerequest *TimeRequest;
extern struct Device *TimerBase;
extern struct Library *I2C_Base;


/* Text structure for the window */
struct IntuiText myText;
struct TextAttr  myTextAttr;            // TextAttr for Window Texts (not labels!)

ULONG myTEXTPEN;
ULONG myBACKGROUNDPEN;

/*
        ---------------------------
        Attaches hotkeys to brokers
        ---------------------------
*/
BOOL setupIHelp()
{
   LONG         error;
   
   AttachCxObj(broker,
      HotKey( ArgString(ttypes, (STRPTR)"HIDETEMP", (STRPTR)"ctrl shift alt t"), cxport, HIDETEMP) );
   AttachCxObj(broker,
      HotKey( ArgString(ttypes, (STRPTR)"DISPLAYTEMP", (STRPTR)"ctrl alt t"), cxport, DISPLAYTEMP) );
         
   if ((error = CxObjError(broker)))
   {
      // accumulated broker error 
      return(0);
   }
   return(1);
}

/*
        --------------------------------
        Wrapper to redirect broker calls
        to the commodity's functions
        --------------------------------
*/
VOID MyHandleCXMsg(id)
ULONG id;
{
   switch(id)
   {
      case HIDETEMP:
            TempWindow(HIDETEMP);
            break;
      case DISPLAYTEMP:
            TempWindow(DISPLAYTEMP);
            break;
    }			  
}

VOID TempWindow(BOOL windowflag) {
    
    struct DrawInfo *drawinfo = NULL;
    struct Screen *ascreen;
    WORD ascreen_width;
    WORD ascreen_height;
    WORD ascreen_tb_height;
    UWORD depth_gadget_width;
    
    ULONG mySHADOWPEN = 1;  /* set default values for pens */
    ULONG mySHINEPEN  = 2;  /* in case can't get info...   */

    
    if (windowflag == DISPLAYTEMP) {
        if (!(TWindow)) {
            /* Open Temp Window */
            
            if ((ascreen = LockPubScreen (NULL))) {
                ascreen_width = ascreen->Width;
                ascreen_height = ascreen->Height;
                ascreen_tb_height = ascreen->BarHeight;
                
                myTextAttr.ta_Name = ascreen->Font->ta_Name;
                myTextAttr.ta_YSize = ascreen->Font->ta_YSize;


                if( ! drawinfo) {
                    if(drawinfo=GetScreenDrawInfo(ascreen)) {
                        myTEXTPEN = drawinfo->dri_Pens[TEXTPEN];
                        myBACKGROUNDPEN  = drawinfo->dri_Pens[BACKGROUNDPEN];
                        
                        /* create a TextAttr that matches the specified font. */
                        /*myTextAttr.ta_Name  = drawinfo->dri_Font->tf_Message.mn_Node.ln_Name;
                        myTextAttr.ta_YSize = drawinfo->dri_Font->tf_YSize;
                        myTextAttr.ta_Style = drawinfo->dri_Font->tf_Style;
                        myTextAttr.ta_Flags = drawinfo->dri_Font->tf_Flags;
                        */
                        myTextAttr.ta_Name = ascreen->Font->ta_Name;
                        myTextAttr.ta_YSize = ascreen->Font->ta_YSize;
                        
                        //Printf("FontSize: %ld", myTextAttr.ta_YSize);
                        FreeScreenDrawInfo(ascreen,drawinfo);
                        drawinfo=NULL;
                    }
                }
                UnlockPubScreen (NULL,ascreen);
                
                TWindow = OpenWindowTags(NULL,
                             WA_Left, ascreen_width - 450, 
                             //WA_Left, ascreen_width - 220, 
                             WA_Top, 1,
                             WA_Width, 410, 
                             //WA_Width, 180, 
                             WA_Height, myTextAttr.ta_YSize,
                             WA_BlockPen,2,
                             WA_DragBar, FALSE,
                             WA_Borderless, TRUE,
                             WA_GimmeZeroZero, FALSE,
                             WA_Backdrop, FALSE,
                             //WA_NoCareRefresh, TRUE,
                             WA_BackFill, LAYERS_NOBACKFILL,
                             WA_Flags, WFLG_SMART_REFRESH,
                             WA_WBenchWindow, TRUE,
                             TAG_DONE);
                             
                if (TWindow) {
                    
                     /* set IDCMP flag */
                    //ModifyIDCMP(TWindow, IDCMP_REFRESHWINDOW);
                    
                    /* activate the MsgPort for our little window */
                    //iport = TWindow->UserPort;
                    //isigflag = 1L << iport->  mp_SigBit;
                    
                    /* copy WB screentitle text attr to our text's attributes */
                    myText.ITextFont = &myTextAttr;
                    myText.FrontPen = 1;
                    //WindowToFront(TWindow);
                }
            }
            else return;
        }
        else return;
    }
    else {
        /* Close Temp Window */
        if (TWindow) {
            CloseWindowSafely(TWindow);
            TWindow = NULL;
        }
    }
}

    
/*
        print/refresh text
*/ 
VOID refreshWindow()
{
    LONG h;
    LONG x;
    SHORT topborder;
    ULONG i2c_error;
    char temp[255] = {"\0"};
    UBYTE Dallas_TempMode[] = { 0xAA };
    UBYTE Dallas_TempData[2] = { 0x00 };

    UBYTE MCP9808_TempInit[] = {0x01 }; 
    UBYTE MCP9808_TempMode[] = {0x05 };     
    UBYTE MCP9808_TempData[2] = { 0x00 };
    UBYTE MCP9808_TempData2[2] = { 0x00 };
    int Temp2 = 0;
    int Temp3 = 0;
    int Temp4 = 0;
    int Temp5 = 0;

    UBYTE LTC_Mode[] = { 0x01, 0x58 };
    UBYTE LTC_Trigger[] = { 0x02, 0x01 };
    UBYTE LTC_Data[17] = { "\0" };
    struct Screen    *screen;
    
    if (TWindow) {

        if (I2C_Base) {
                      
            /*
            i2c_error = SendI2C(0x9E, 1, Dallas_TempMode);
            i2c_error = ReceiveI2C(0x9E, 2, Dallas_TempData);
            */

            i2c_error = SendI2C(0x30, 1, MCP9808_TempInit);
            i2c_error = SendI2C(0x30, 1, MCP9808_TempMode);
            i2c_error = ReceiveI2C(0x30, 2, MCP9808_TempData);
            
            /*
            i2c_error = SendI2C(0x32, 1, MCP9808_TempInit);
            i2c_error = SendI2C(0x32, 1, MCP9808_TempMode);
            i2c_error = ReceiveI2C(0x32, 2, MCP9808_TempData2);  
            */
            i2c_error = SendI2C(0x98, 2, LTC_Mode);
            i2c_error = ReceiveI2C(0x98, 16, LTC_Data);
	    i2c_error = SendI2C(0x98, 2, LTC_Trigger); 
    
            /* MCP9808  
                0x01 senden
                0x05 senden
                2 Byte lesen
                if T > 0
                        T = ((upperByte & 0x1f) * 16 + lowerByte /16)
            */
            
            //stcl_d(temp, Dallas_TempData[0]);
        }
        else {
            stcl_d(temp, 20);
        }
        
        Temp2 = (int) ( ( (UBYTE) MCP9808_TempData[0] & 0x1f) * 16 + ( (UBYTE) MCP9808_TempData[1] /16 ) );      

//        Temp3 = (int) ( ( (UBYTE) MCP9808_TempData2[0] & 0x1f) * 16 + ( (UBYTE) MCP9808_TempData2[1] /16 ) );
        
        Temp4 = (int) ( ((UBYTE) MCP9808_TempData[1] % 16) * 10 / 16);
//        Temp5 = (int) ( ((UBYTE) MCP9808_TempData2[1] % 16) * 10 / 16 );

/*
        printf("Temp1: %.2f°C  Temp2: %.2f°C.  V1: %.2fV  V2: %.2fV  VCC: %.2fV",
    	(float) ((float)(((LTC_Data[8] & 0x7F) << 8) + (float)LTC_Data[9]) / 16),
       	(float) ((float)(((LTC_Data[2] & 0x7F) << 8) + LTC_Data[3]) / 16),
    	(float) ((((LTC_Data[4] & 0x7F) << 8) + LTC_Data[5]) * 0.61 / 1000),
    	(float) ((((LTC_Data[6] & 0x7F) << 8) + LTC_Data[7]) * 1.22 / 1000),
    	(float) (2.5 + (305.18 * (float) ((((LTC_Data[12] & 0x7F) << 8) + LTC_Data[13])) /1000000))
    	);             
	return;
*/
        //myText.IText = strcat(temp, "°C\0");       
        /* original ICY V1 A4000D string pattern */ 
        /*sprintf(temp, "Zorro: %ld °C    PSU: %ld.%ld °C    CPU: %ld.%ld °C", Dallas_TempData[0], Temp2 , Temp4, Temp3, Temp5);*/

     sprintf(temp, "Zorro: %.2f°C   CPU: %ld.%ld °C   V1: %.2fV   V2: %.2fV ", 
                (float) ((float)(((LTC_Data[8] & 0x7F) << 8) + (float)LTC_Data[9]) / 16),
                Temp2,Temp4,
                (float) ((((LTC_Data[4] & 0x7F) << 8) + LTC_Data[5]) * 0.61 / 1000),
            	(float) ((((LTC_Data[6] & 0x7F) << 8) + LTC_Data[7]) * 1.22 / 1000)
                );

/*
     sprintf(temp, "%.2f°C   %.2fV   %.2fV ", 
                (float) ((float)(((LTC_Data[8] & 0x7F) << 8) + (float)LTC_Data[9]) / 16),
                
                (float) ((((LTC_Data[4] & 0x7F) << 8) + LTC_Data[5]) * 0.61 / 1000),
            	(float) ((((LTC_Data[6] & 0x7F) << 8) + LTC_Data[7]) * 1.22 / 1000)
                );
*/

        myText.IText = (STRPTR) &temp; 
        
        //Printf("test: %s\n", myText.IText);
                    
            //x = TWindow->Width/2 + IntuiTextLength(&SettingsWindow_ResizeOpts_Msg)/2;
            
        //BeginRefresh( TWindow );
        //Move(TWindow->RPort, 0, 0);
        SetRast(TWindow->RPort,2);
        //ClearScreen(TWindow->RPort);
        PrintIText(TWindow->RPort, &myText, 0, 0);
        //EndRefresh( TWindow, TRUE );
    }
    return;
}

BOOL SetupTimerRequest(VOID) {
    
    TimePort = CreateMsgPort();
    
    if(TimePort == NULL) {
        Printf("Could not create timer message port.\n");
        return FALSE;
    } else {
        TimeRequest = (struct timerequest *)CreateIORequest(TimePort,sizeof(*TimeRequest));
        
	if(TimeRequest == NULL) {
            Printf("Could not create timer I/O request.\n");
            return FALSE;
	} else {
            if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)TimeRequest,0) != 0) {
                Printf("Could not open 'timer.device'.\n");
                return FALSE;
            } else {
                TimerBase = TimeRequest->tr_node.io_Device;
            }
        }
    }
    return TRUE;
}

