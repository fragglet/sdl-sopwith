/*
        swmiscjr -      SW miscellaneous

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-07-23        Development
                        87-03-09        Microsoft compiler.
*/
#include        "sw.h"




puts( sp )
char    *sp;
{
register char   *s;

        s = sp;
        while( *s )
                swputc( *s++ );
}
