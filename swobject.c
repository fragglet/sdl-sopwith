/*
        swobject -      SW object allocation and deallocation

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-07        Development
                        84-06-12        PCjr Speed-up
                        84-10-31        Atari
                        87-03-09        Microsoft compiler.
*/
#include        "sw.h"



extern  OBJECTS *nobjects;              /* Objects list.                    */
extern  OBJECTS *objbot, *objtop,       /* Top and bottom of object list    */
                *objfree,               /* Free list                        */
                *deltop, *delbot;       /* Newly deallocated objects        */
extern  OBJECTS *objsmax;               /* Maximum allocated object         */



OBJECTS *allocobj()
{
register OBJECTS *ob;

        if ( !objfree )
                return( NULL );

        ob = objfree;
        objfree = ob->ob_next;

        ob->ob_next = NULL;
        ob->ob_prev = objbot;

        if ( objbot )
                objbot->ob_next = ob;
        else
                objtop = ob;

        ob->ob_sound = NULL;
        ob->ob_drwflg = ob->ob_delflg = 0;
        if ( ob > objsmax )
                objsmax = ob;
        return( objbot = ob );
}



deallobj( obp )
OBJECTS *obp;
{
register OBJECTS *ob, *obb;

        ob =obp;
        if ( obb = ob->ob_prev )
                obb->ob_next = ob->ob_next;
        else
                objtop = ob->ob_next;

        if ( obb = ob->ob_next )
                obb->ob_prev = ob->ob_prev;
        else
                objbot = ob->ob_prev;

        ob->ob_next = 0;
        if ( delbot )
                delbot->ob_next = ob;
        else
                deltop = ob;

        delbot = ob;


}
