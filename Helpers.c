/*      -------------------------------------------------
        Helpers.c (29.10.2014) Marcus Gerards

        Utility and debug functions for WinZoom

*/

#include "app.h"
#include "local.h"

#ifndef __amigaos4__
VOID StripIntuiMessages(struct MsgPort *mp, struct Window *win)
{
    struct IntuiMessage *msg;
    struct Node *succ;

    msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

    while ((succ = msg->ExecMessage.mn_Node.ln_Succ)) {
        if (msg->IDCMPWindow == win)
        {
            /* Intuition is about to free this message.
                Make sure that we have politely sent it back.
            */
            Remove((struct Node *) msg);
            ReplyMsg((struct Message *) msg);
        }
        msg = (struct IntuiMessage *)succ;
    }
}
#endif

VOID CloseWindowSafely(struct Window *win)
{
    if (win->UserPort) {

        /* we forbid here to keep out of race conditions with Intuition */
        Forbid();

        /* send back any messages for this window  that have not yet been
        ** processed
        */
        StripIntuiMessages(win->UserPort, win);

        /* clear UserPort so Intuition will not free it */
        win->UserPort = NULL;

        /* tell Intuition to stop sending more messages */
        ModifyIDCMP(win, 0L);

        /* turn multitasking back on */
        Permit();
    }
    /* Now it's safe to really close the window */
    CloseWindow(win);
}
