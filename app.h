/* 
        app.h (22.02.2015)
        #defines and prototypes for app.c
        
        based on DevCon '90 commodities example by
        Commodore-Amiga, Inc.   
*/

#ifndef APP_H
#define APP_H
#endif
#include <exec/types.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <libraries/commodities.h>

/**********************************************************************/
/* Prototypes for functions declared in app.c and called from the  */
/* standard modules.                                                  */
/**********************************************************************/
BOOL setupCustomCX(VOID);
VOID shutdownCustomCX(VOID);
VOID handleCustomCXMsg(ULONG);
VOID handleCustomCXCommand(ULONG);
VOID handleCustomSignal(VOID);

/**********************************************************************/
/* Prototypes for functions declared in the standard modules and      */
/* called by app.c                                                    */
/**********************************************************************/
VOID terminate(VOID);

/**********************************************************************/
/* Prototypes for functions declared in application modules and       */
/* called by app.c                                                    */
/**********************************************************************/
BOOL setupIHelp(VOID);
VOID MyHandleCXMsg(ULONG);
VOID   handleIMsg(struct IntuiMessage *, UBYTE);

/**********************************************************************/
/* definitions for global variables declared in the standard modules  */
/* referenced by app.c                                                */
/**********************************************************************/
extern CxObj                  *broker;
extern SHORT                  topborder;
extern VOID                   *vi;
extern char                   **ttypes;
extern struct MsgPort         *cxport;
extern struct IntuitionBase   *IntuitionBase;
extern BOOL                   IDCMPRefresh;
CONST_STRPTR GetString(enum AppStringsID id);

/**********************************************************************/
/* definitions for global variables declared in app.c and             */
/* referenced by the standard modules.                                */
/**********************************************************************/

// none left


/**********************************************************************/
/* Commodities specific definitions.                                  */
/*                                                                    */
/* COM_NAME  - used for the scrolling display in the Exchange program */
/* COM_TITLE - used for the window title bar and the long description */
/*             in the Exchange program                                */
/* COM_DESC  - Commodity description used by the Exchange program     */
/* CX_DEFAULT_PRIORITY - default priority for this commodities broker */
/*                       can be overidden by using icon TOOL TYPES    */
/**********************************************************************/
#define COM_NAME  "TempMon"
#define COM_TITLE "TempMon"
#define COM_DESCR "Display your Amiga's temperature"
#define CX_DEFAULT_PRIORITY 0
//#define CX_DEFAULT_POP_KEY ("lamiga f9")
#define CX_DEFAULT_POP_ON_START ("NO")

/**********************************************************************/
/* Custom Signal control                                              */
/*                                                                    */
/* If CSIGNAL = 0 then this commodity will NOT have a custom signal   */
/* If CSIGNAL = 1 this commodity will support a custom signal         */
/**********************************************************************/
#define CSIGNAL 0

/**********************************************************************/
/* Window control                                                     */
/*                                                                    */
/* If WINDOW = 0 then this commodity will NOT have a popup window     */
/* If WINDOW = 1 this commodity will support a popup window with the  */
/*               attributes defined below.                            */
/**********************************************************************/
#define WINDOW 0

/*#define POP_KEY_ID     (86L) */        /* pop up identifier           */


/***********************************************************************/
/*      TempMon functionality - 
        supported commands' table
        ------------------------- 
************************************************************************/
#define HIDETEMP              0L
#define DISPLAYTEMP       1L
