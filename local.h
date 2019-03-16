/*
    local.h (1990, minor changes 02.06.2014)
    
    taken from DevCon '90 commodities example by
    Commodore-Amiga, Inc.   
    
    Definitions used by standard modules 
*/
#define DEBUG    0

#include <clib/alib_protos.h>

#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/commodities.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>

#include "app.h"
#include "TempMon.h"

/**********/
/* main.c */
/**********/
extern struct IntuitionBase *IntuitionBase;
extern struct DosLibrary    *DOSBase;
extern struct Library       *CxBase;
extern struct Library        *WorkbenchBase;
extern struct Library         *IconBase;

extern struct MsgPort *cxport;      /* commodities messages here      */
extern ULONG          cxsigflag;    /* signal for above               */

extern struct MsgPort *iport;       /* Intuition IDCMP messages here   */
extern ULONG          isigflag;     /* signal for above                */

/********/
/* cx.c */
/********/
extern char   hotkeybuff[];   /* holds the string describing the popup */
                              /* hotkey. Used for the window title     */

VOID handleCxMsg(struct Message *);
BOOL setupCX(char **);
VOID shutdownCX(VOID);

#define PRIORITY_TOOL_TYPE     "CX_PRIORITY"
//#define POP_ON_START_TOOL_TYPE "CX_POPUP"
//#define POPKEY_TOOL_TYPE       "CX_POPKEY"
