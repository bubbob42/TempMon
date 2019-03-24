/* 
        cx.c (1990)
        Commodities interface
         
        taken from DevCon '90 commodities example by
        Commodore-Amiga, Inc.   
        
        This module handles all the command messages from commodities.library.
        Commands such as HIDE SHOW ENABLE DISABLE KILL are sent to commidities
        from the Exchange program and are processed here.
 */

#include "local.h"
extern BOOL terminate_loop;

CxObj *broker = NULL;            /* Our broker */

/* a little global for showing the user the hotkey   */
char   hotkeybuff[ 257 ];

struct NewBroker mynb = {
   NB_VERSION,                        /* Library needs to know version */
   COM_NAME,                          /* broker internal name          */
   COM_TITLE,                         /* commodity title               */
   COM_DESCR,                         /* description                   */
   NBU_NOTIFY | NBU_UNIQUE,           /* We want to be the only broker */
                                      /* with this name and we want to */
                                      /* be notified of any attempts   */
                                      /* to add a commodity with the   */
                                      /* same name                     */
   0,                                 /* flags                         */
   0,                                 /* default priority              */
   NULL,                              /* port, will fill in            */
   0                                  /* channel (reserved)            */
};

/****i* Blank.ld/handleCxMsg() ******************************************
*
*   NAME
*        handleCxMsg -- Handle incoming commodities messages.
*
*   SYNOPSIS
*        handleCxMsg(msg)
*
*        VOID handleCxMsg(Struct Message *msg);
*
*   FUNCTION
*        This function handles commodities messages sent to the brokers
*        message port. This is were the standard commodities features
*        such as Enable/Disable Show/Hide Quit and HotKey PopUp are
*        handled. If the  message is not for the standard handler then the
*        function handleCustomCXMsg() is called to handle application
*        specific IEVENTS otherwise handleCustomCXCommand() is called
*        to handle application specific commodities command messages.
*
*   INPUTS
*        msg = A commodities message;
*
*   RESULT
*        Either the standard commodities function is performed or the
*        custom handler is called.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID handleCxMsg(struct Message *msg)
{
   ULONG   msgid;
   ULONG   msgtype;

   msgid   = CxMsgID( (CxMsg *)  msg );
   msgtype = CxMsgType( (CxMsg *) msg );

   #if DEBUG 
        Printf("cx.c: handleCxMsg() enter\n");
   #endif

   ReplyMsg(msg);

   switch(msgtype)
   {
      case CXM_IEVENT:
         #if DEBUG 
                Printf("cx.c: handleCxMsg(CXM_IEVENT)\n");
         #endif       
         handleCustomCXMsg(msgid);
         break;
      case CXM_COMMAND:
         #if DEBUG 
                Printf("cx.c: handleCxMsg(CXM_COMMAND)\n");
         #endif
         switch(msgid)
         {
            case CXCMD_DISABLE:
                #if DEBUG 
                        Printf("cx.c: handleCxMsg(CXCMD_DISABLE)\n");
                #endif                        
                ActivateCxObj(broker,0L);
               break;
            case CXCMD_ENABLE:
                #if DEBUG
                        Printf("cx.c: handleCxMsg(CXCMD_ENABLE)\n");
                #endif
                ActivateCxObj(broker,1L);
                break;
            case CXCMD_UNIQUE:   /* Someone has tried to run us again */
                #if DEBUG 
                        Printf("cx.c: handleCxMsg(CXCMD_APPEAR|CXCMD_UNIQUE)\n");
                #endif
                /* If someone tries to run us a second time the second copy
                 * of the program will fail and we will be sent a
                 * CXCMD_UNIQUE message. 
                */
                terminate_loop = TRUE;
                break;            
            case CXCMD_KILL:
                #if DEBUG 
                        Printf("cx.c: handleCxMsg(CXCMD_KILL)\n");
                #endif
                terminate_loop = TRUE;
                break;
            default:
                #if DEBUG 
                        Printf("cx.c: handleCxMsg(Custom Command)\n");
                #endif
                handleCustomCXCommand(msgid);
                break;
         }     /* end switch(command) */
         break;
   }     /* end switch(msgtype) */
}
/***** setupCX() ******************************************
*
*   NAME
*        setupCX -- Initialize commodities.library specific features.
*
*   SYNOPSIS
*        result = setupCX(ttypes)
*
*        BOOL setupCX(char **ttypes);
*
*   FUNCTION
*        This function performs all the commodities.library specific
*        setup required for a standard commodity. It sets up the brokers
*        message port, and priority. And sets certain flags in the
*        broker structure so the exchange program will know what
*        features this broker supports. If the commodity supports a window
*        the windows hotkey is added to the broker and then the function
*        setupCustomCX() is called to setup the application specific
*        commodities objects. If all this goes well the broker is activated
*        and the function returns TRUE.
*
*   INPUTS
*        ttypes - NULL terminated Argument array containing this
*        applications TOOLTYPES strings.
*
*   RESULT
*        Returns TRUE if all went OK else returns FALSE.
*
*   EXAMPLE
*        if(setupCX(ttypes))
*        {
*           printf("Commodities successfully initialized.\n");
*        } else {
*           printf("Commodities initialization error!\n");
*        }
*
*   NOTES
*        This function can be called at anytime to reinitialize the
*        commodities code from a new set of arguments.
*
*   BUGS
*
*   SEE ALSO
*        setupCustomCX();
*        shutdownCX();
*        shutodwnCustomCX();
*
*****************************************************************************
*
*/
BOOL setupCX(char **ttypes )
{
    LONG   error;

    #if DEBUG 
        Printf("cx.c: setupCX() enter\n"); 
    #endif

    shutdownCX();                          /* remove what was and create */
                                          /* everything from scratch    */

   cxport=CreatePort(mynb.nb_Name,0L);    /* Create Message port        */

   if( ! cxport )
   {
      #if DEBUG 
        Printf("cx.c: setupCX() Could not create message port\n");
      #endif
      return(FALSE);
   }
   #if DEBUG 
        Printf("cx.c: setupCX() cxport=0x%lx\n",cxport); 
   #endif

   cxsigflag = 1L << cxport->mp_SigBit;   /* Create signal mask for Wait*/

   /* Set the brokers priority from the TOOLTYPES or from default if no */
   /* TOOLTYPES are available. Set the brokers Message port.            */
   mynb.nb_Pri  = ArgInt( (CONST_STRPTR *) ttypes, (STRPTR)PRIORITY_TOOL_TYPE, CX_DEFAULT_PRIORITY );
   mynb.nb_Port = cxport;

   #if DEBUG 
        Printf("cx.c: setupCX() mynb.nb_pri=0x%lx\n",mynb.nb_Pri);
   #endif

   /* Attempt to create our broker */
   if ( ! ( broker = CxBroker( &mynb, NULL ) ) )
   {
      #if DEBUG 
        Printf("cx.c: setupCX() could not create broker\n");
      #endif
      shutdownCX();
      return(FALSE);
   }
   #if DEBUG 
        Printf("cx.c: setupCX() broker=0x%lx\n",broker);
   #endif


   /* Setup all application specific commodities objects */
   if( ! setupCustomCX() )
   {
      #if DEBUG 
        Printf("cx.c: setupCX() setupCustomCX failed\n"); 
      #endif
      shutdownCX();
      return(FALSE);
   }

   /* Check for accumulated error */
   if ( (error = CxObjError( broker )) )
   {
      #if DEBUG 
        Printf("cx.c: setupCX() accumulated broker error %ld\n",error);
      #endif
      shutdownCX();
      return (FALSE);
   }

   /* All went well so activate our broker */
   ActivateCxObj(broker,1L);

   #if DEBUG 
        Printf("cx.c: setupCX() returns TRUE");
   #endif
   return (TRUE);
}

/***** shutdownCX() ******************************************
*
*   NAME
*        shutdownCX -- Cleanup all commodities brokers and handlers.
*
*   SYNOPSIS
*        shutdownCX()
*
*        VOID shutdownCX(VOID);
*
*   FUNCTION
*        Shuts down and cleans up all variables and data used for supporting
*        the commodities.library side of this commodity. This function
*        MUST be set up so that it can be called regardless of the current
*        state of the program. This function handles all the standard
*        cleanup and calls shutdownCustomCX(); to cleanup the application
*        specific code.
*
*   INPUTS
*        None.
*
*   RESULT
*        The commodities specific code is cleaned up and made ready for
*        a terminate(); or a call to setupCX();.
*
*   EXAMPLE
*
*   NOTES
*        The first thing that setupCX() does is call this routine. Therefore
*        this function must work even before your commodity has been
*        initialized.
*
*   BUGS
*
*   SEE ALSO
*        setupCX();
*        setupCustomCX();
*        shutdownCustomCX();
*
*****************************************************************************
*
*/
VOID shutdownCX()
{
   struct Message   *msg;

   #if DEBUG 
        Printf("cx.c: shutdownCX() enter broker now: %lx\n", broker );
   #endif
   shutdownCustomCX();

   if(cxport)
   {
      #if DEBUG 
        Printf("cx.c: shutdownCX() Deleting all objects\n");
      #endif
      DeleteCxObjAll(broker);      /* safe, even if NULL   */

      /* now that messages are shut off, clear port   */
      while((msg=GetMsg(cxport))) 
      	ReplyMsg(msg);
      
      
      DeletePort(cxport);
	  	  
      cxport    = NULL;
      cxsigflag = 0;
      broker    = NULL;
   }
   #if DEBUG 
        Printf("cx.c: shutdownCX() returns\n");
   #endif
}

