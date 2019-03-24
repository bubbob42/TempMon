/*
        TempMon by Marcus Gerards

        22.02.2015

        main(); based on commodities example source
        (C) 1990  Commodore-Amiga, Inc.  All Rights Reserved.

*/


#include "local.h"
#include "TempMon_rev.h"
#include "TempMon.h"
#include "app.h"
#include <proto/gadtools.h>
#include <devices/timer.h>
#include <proto/i2c.h>
#include "i2cdisplay.h"

struct IntuitionBase    *IntuitionBase = NULL;
struct Library          *ExpansionBase = NULL;
struct Library          *CxBase        = NULL;
struct Library          *IconBase      = NULL;
struct Library          *WorkbenchBase = NULL;
struct Library          *GadToolsBase  = NULL;
struct GfxBase          *GfxBase       = NULL;
struct Library          *I2C_Base      = NULL;
struct Device           *TimerBase     = NULL;

/* Tooltype stuff */
char **ttypes;

/* CLI */
BPTR output = 0; //NULL;
BPTR old_output = 0; //NULL;

/* Board identification flags */
BOOL    i2c_found = FALSE;

/* Timer port & stuff */
struct MsgPort *TimePort = NULL;
struct timerequest *TimeRequest = NULL;

/*
        these globals are the connection between the main program
        loop and the two message handling routines
*/

struct MsgPort *cxport   = NULL; /* commodities messages here      */
ULONG          cxsigflag = 0;    /* signal for above               */

struct MsgPort *iport = NULL; /* Intuition IDCMP messages for the window here   */
ULONG          isigflag  = 0;    /* signal for above                */

BOOL terminate_loop = FALSE;

int main(int argc,char **argv)
{
    struct Message *msg;

    DOSBase      =(struct DosLibrary *)   OpenLibrary("dos.library",0);
    IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39);
    GfxBase      =(struct GfxBase *)      OpenLibrary("graphics.library",0);
    GadToolsBase =                        OpenLibrary("gadtools.library",37);
    ExpansionBase=                        OpenLibrary("expansion.library",0);
    CxBase       =                        OpenLibrary("commodities.library",5);
    IconBase     =                        OpenLibrary("icon.library",37);
    WorkbenchBase=                        OpenLibrary("workbench.library",37);
    I2C_Base     =                        OpenLibrary("i2c.library",40);

    if ( ! ( IntuitionBase && CxBase && DOSBase && IconBase && GfxBase && GadToolsBase && WorkbenchBase && ExpansionBase) ) {
        //Failed to open one or more libraries
        terminate();
    }

    if (I2C_Base) i2c_found = TRUE;

    if (argc == 0) {             // start from Workbench?

        #if DEBUG
        if (output = Open("CON:0/0//200/TempMon DEBUG/WAIT/CLOSE",MODE_NEWFILE)) {
            old_output = SelectOutput(output);
            Printf("%s Debug output enabled!\n\n", ABOUTVER);
        }
        #endif

        /* commodities support library function to find argv or tooltypes   */
        ttypes = (char**) ArgArrayInit( argc, (CONST_STRPTR *) argv );

        /*
        // Fill ToolType-Settings struct *Opt
        if ((tt_value = FindToolType(ttypes, "RESPECTSCREENTITLE")) != NULL)
                if (MatchToolValue(tt_value, "Yes")) WZ_Opt->RespectScreenTitle = TRUE;
        */
    }
    else {
        output = Output();

        #if DEBUG
                Printf("%s Debug output enabled!\n\n", ABOUTVER);
        #else
                Printf("Sorry, no shell startup yet - please start from Workbench!\n\n");
                //terminate();
        #endif
    }

    if (!setupCX(ttypes)) {
        terminate();
    }

    /* fire up the timer */
    if (SetupTimerRequest()) {

        ULONG timer_mask;
        ULONG signal_mask;
        ULONG signals;
        ULONG td_Interval = 1000;

        setup_displayinterface ( I2C_1602DISPLAY_ADDRESS, 2,1,0,4,5,6,7,3);
        init_display();

        TempWindow(DISPLAYTEMP);
        refreshWindow();

        timer_mask = 1L << TimePort->mp_SigBit;

        /* we want to get a signal at ctrl-e or every second */
        signal_mask = SIGBREAKF_CTRL_E | timer_mask | cxsigflag | isigflag;
        signals = 0;

        TimeRequest->tr_node.io_Command = TR_ADDREQUEST;
        TimeRequest->tr_time.tv_secs    = 0;
        /* user input is measured in 1/1000 s */
        TimeRequest->tr_time.tv_micro   = td_Interval;

        SendIO((struct IORequest *)TimeRequest);


        /* this is the main loop */
        while (!(terminate_loop)) {           /* exit by setting terminate   */
            if (signals == 0) {
                /* wait for our signals */
                signals = Wait(signal_mask);
            }
            else {
                signals |= SetSignal(0,signal_mask) & signal_mask;
            }

            /* we got a signal from timer.device */
            if (signals & timer_mask) {
                if (GetMsg(TimePort) != NULL) {
                    WaitIO((struct IORequest *)TimeRequest);

                    /* call our little main routine */
                    refreshWindow();

                    /* and renew the timer request */
                    TimeRequest->tr_node.io_Command     = TR_ADDREQUEST;
                    TimeRequest->tr_time.tv_secs        = 0;
                    TimeRequest->tr_time.tv_micro       = td_Interval * 4000;

                    SendIO((struct IORequest *)TimeRequest);
                }
                signals &= ~timer_mask;
            }
            /* we are a nice commodity and exit by ctrl-e */
            if(signals & SIGBREAKF_CTRL_E) {
                terminate_loop = TRUE;
                signals &= ~SIGBREAKF_CTRL_E;
            }

            while(cxport && (msg=GetMsg(cxport))) handleCxMsg(msg);
            /*
            while(iport && (msg=(struct Message *)GT_GetIMsg(iport))) {
                #if DEBUG
                Printf("Received IntuiMsg\n");
                #endif
                handleIMsg((struct IntuiMessage *)msg, 0);
            }
            */
        }
        /* abort and wait for pending device requests */
        if (CheckIO((struct IORequest *)TimeRequest) == NULL) {
            AbortIO((struct IORequest *)TimeRequest);
        }
        WaitIO((struct IORequest *)TimeRequest);

    }
    terminate();
}

/****** terminate() ******************************************
*
*   NAME
*        terminate -- Cleanup all resources and exit the program.
*
*   SYNOPSIS
*        terminate()
*
*        VOID terminate(VOID);
*
*   FUNCTION
*        This function performs all the necessary cleanup for the
*        commodity and calls exit() to return control to the OS.
*        No matter how the program exits this should be the last function
*        called.
*
*   INPUTS
*        None.
*
*   RESULT
*        All resources are freed and the program exits.
*
*   EXAMPLE
*        if(!AllocWindow())
*           terminate();
*
*   NOTES
*        This function must be set up so that it can be called at any
*        time regardless of the current state of the program.
*
*   BUGS
*
*   SEE ALSO
*        shutdownCX();
*        shutdownWindow();
*
*****************************************************************************
*
*/
VOID terminate() {

    /* abort and wait for pending device requests */
    /*if (CheckIO((struct IORequest *)TimeRequest) == NULL) {
        AbortIO((struct IORequest *)TimeRequest);
    }
    WaitIO((struct IORequest *)TimeRequest);
*/
    if (TimeRequest != NULL) {
        if (TimeRequest->tr_node.io_Device != NULL) {
            CloseDevice((struct IORequest *)TimeRequest);
        }

        DeleteIORequest((struct IORequest *)TimeRequest);
        TimeRequest = NULL;
    }
    if (TimePort != NULL) {
        DeleteMsgPort(TimePort);
        TimePort = NULL;
    }


    shutdownCX();
    TempWindow(HIDETEMP);

    ArgArrayDone();    /* cx_supp.lib function   */

#if DEBUG
    /* close DEBUG con - BAD CHECKING*/
//    if (old_output && output) {
        SelectOutput(old_output);
        Close(output);
//     }
#endif

    /* Close Libraries */

    if(CxBase)          CloseLibrary(CxBase);
    if(IntuitionBase)   CloseLibrary((struct Library *)IntuitionBase);
    if(GfxBase)         CloseLibrary((struct Library *)GfxBase);
    if(GadToolsBase)    CloseLibrary(GadToolsBase);
    if(DOSBase)         CloseLibrary((struct Library *)DOSBase);
    if(IconBase)        CloseLibrary(IconBase);
    if(ExpansionBase)   CloseLibrary(ExpansionBase);
    if(WorkbenchBase)   CloseLibrary(WorkbenchBase);
    if(I2C_Base)        CloseLibrary(I2C_Base);

    exit(0);
}
