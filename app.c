/*
        app.c (22.02.2015)
        Custom commodity functions not
        directly related to TempMon-functionality

        based on app.c from DevCon '90 example
        by Commodore-Amiga, Inc.
*/

#include "app.h"
#include "TempMon_rev.h"
#include "TempMon.h"
#include <proto/dos.h>
#include <ctype.h>
#include <proto/gadtools.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>

//#include <stdlib.h>

#define V(x) ((VOID *)x)
#define DEBUG 1

BOOL IDCMPRefresh = FALSE;


BOOL setupCustomCX()
{
   return(setupIHelp());
}
VOID shutdownCustomCX()
{
}
VOID handleCustomCXMsg(id)
ULONG id;
{
   switch(id)
   {
      case 0:
      default:
            MyHandleCXMsg(id);
            break;
   }
}
VOID handleCustomCXCommand(id)
ULONG id;
{
   switch(id)
   {
      case 0:
      default:
            break;
   }
}
#if CSIGNAL
VOID handleCustomSignal(VOID)
{
}
#endif

/****handleIMsg() ******************************************
*
*   NAME
*        handleIMsg -- Handle window IDCMP messages.
*
*   SYNOPSIS
*        handleIMsg(msg);
*
*        VOID handleIMsg(struct IntuiMessage *msg);
*
*   FUNCTION
*        This function handles all the IntuiMessages for standard
*        commodities functions. If the message is for an application
*        Gadget or Menu the appropriate application function,
*        handleCustomMenu() or HandleGadget(), is called.
*
*   INPUTS
*        msg = The current IntuiMessage.
*
*   RESULT
*        The appropriate action for the message is performed.
*
*****************************************************************************
*
*/
VOID handleIMsg(struct IntuiMessage *msg, UBYTE port)
{
   ULONG   msg_Class;
   UWORD   code;

   msg_Class    = msg->Class;
   code     = msg->Code;

   #if DEBUG
   Printf("Received a message, class %ld, code %ld\n", msg_Class, code);
   #endif

   /*  handleIMsg() */
   GT_ReplyIMsg( (struct IntuiMessage *) msg );

   switch ( msg_Class )
   {

      case IDCMP_REFRESHWINDOW:
         Printf("Refreshing Window...\n");
         IDCMPRefresh=TRUE;
         refreshWindow();
         IDCMPRefresh=FALSE;
         break;

      case IDCMP_MENUPICK:
         break;

      case IDCMP_VANILLAKEY:
         break;

      case IDCMP_RAWKEY:
         //handleRawKey(code);
         break;

      case IDCMP_MOUSEBUTTONS:
         //terminate();
         break;
   }
}

