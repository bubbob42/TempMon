/*      -------------------------------------------------
        TempMon.c (01.06.2015) Marcus Gerards

        Temperature Monitor's core
        -------------------------------------------------
*/

#include "app.h"
#include "local.h"
#include "TempMon.h"
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <intuition/intuition.h>
#include <proto/i2c.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "i2cdisplay.h"
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
      HotKey( ArgString((CONST_STRPTR *) ttypes, (STRPTR)"HIDETEMP", (STRPTR)"ctrl shift alt t"), cxport, HIDETEMP) );
   AttachCxObj(broker,
      HotKey( ArgString((CONST_STRPTR *) ttypes, (STRPTR)"DISPLAYTEMP", (STRPTR)"ctrl alt t"), cxport, DISPLAYTEMP) );

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


    if (windowflag == DISPLAYTEMP) {
        if (!(TWindow)) {
            /* Open Temp Window */

            if ((ascreen = LockPubScreen (NULL))) {
                ascreen_width = ascreen->Width;

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
                             WA_Left, ascreen_width - 250,
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
    char temp[40] = {"\0"};

    UBYTE LTC_Mode[]    = { 0x01, 0x58 };
    UBYTE LTC_Trigger[] = { 0x02, 0x01 };
    UBYTE LTC_Data[17]  = { "\0" };

    if (TWindow) {

        if (I2C_Base) {
            (void) SendI2C   (0x98,  2, LTC_Mode);
            (void) ReceiveI2C(0x98, 16, LTC_Data);
            (void) SendI2C   (0x98,  2, LTC_Trigger);
        } else {
            memset(LTC_Data, 0, 17);
        }

        sprintf(temp, "Zorro: %5.2f°C  V1: %5.2fV  V2: %5.2fV",
                (float) ((float)(((LTC_Data[8] & 0x7F) << 8) + (float)LTC_Data[9]) / 16),

                (float) ((((LTC_Data[4] & 0x7F) << 8) + LTC_Data[5]) * 0.61 / 1000),
                (float) ((((LTC_Data[6] & 0x7F) << 8) + LTC_Data[7]) * 1.22 / 1000)
                );

        myText.IText = (STRPTR) &temp;

        SetRast(TWindow->RPort,2);
        PrintIText(TWindow->RPort, &myText, 0, 0);

        sprintf(temp, "Zorro: %5.2f\xDF C",
                (float) ((float)(((LTC_Data[8] & 0x7F) << 8) + (float)LTC_Data[9]) / 16));
        setpos_2_display( 0,0 );
        print_2_display(temp);

        sprintf(temp, "%5.2fV    %5.2fV",
                (float) ((((LTC_Data[4] & 0x7F) << 8) + LTC_Data[5]) * 0.61 / 1000),
                (float) ((((LTC_Data[6] & 0x7F) << 8) + LTC_Data[7]) * 1.22 / 1000)
                );
        setpos_2_display( 0,1 );
        print_2_display(temp);

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

