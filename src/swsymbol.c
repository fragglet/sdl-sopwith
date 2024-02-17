//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
// Sprites
//

#include <string.h>

#include "swsymbol.h"

// Order here is counterintuitive: cyan (1) is brighter than magenta (2):
static const char *color_chars = " *-#";

// TODO: Generate the data below from strings, for maintainability.
static const char *swplnsym[] = {
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"* * *                           \n"
	"* * * *         - - - - - - -   \n"
	"* * * * *           -   -       \n"
	"- - - - - * * * * * - * - * * * \n"
	"  * * * * * * * * * - * - * * * \n"
	"    * * * * * * - - - - - - * * \n"
	"    *                 *         \n"
	"                    * * *       \n"
	"                      *         \n"
	"                                \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                      - - -     \n"
	"                  - - -     *   \n"
	"              - - -   - * * * * \n"
	"  *               * - * - * * * \n"
	"* * *   *     * * * - * - -     \n"
	"* * * * - * * * * - - -         \n"
	"* - - - * * * * * -     * *     \n"
	"  * * * * * *         * *       \n"
	"      * *                       \n"
	"      *                         \n"
	"                                \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                                \n"
	"                    -           \n"
	"                    -   *       \n"
	"                - -   * * *     \n"
	"              -     - * * *     \n"
	"            -   - - * - -       \n"
	"                *   - -         \n"
	"              * * * -   * *     \n"
	"            * * * -     * *     \n"
	"  *   * * * * * *               \n"
	"* * * * - * * *                 \n"
	"* * * - * * *                   \n"
	"  * - * * *                     \n"
	"          *                     \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                    *           \n"
	"              - - * * * *       \n"
	"              -   * * *         \n"
	"            -   - - * -         \n"
	"            -   * * - -         \n"
	"            - - - - - * *       \n"
	"          -     * * -   * *     \n"
	"              * * -             \n"
	"              * * -             \n"
	"            * * * *             \n"
	"          * * * *               \n"
	"    * * * - - * *               \n"
	"    * * * - * *                 \n"
	"  * * * * - * *                 \n"
	"    * * -       *               \n"
	"                                \n",
};

static const char *swhitsym[] = {
	"            - - - - - - - - - - \n"
	"              - - - * * - - -   \n"
	"                - - * * - -     \n"
	"                    * *         \n"
	"                    * *         \n"
	"                    * *         \n"
	"                  * * * *       \n"
	"                  * * * *       \n"
	"                  * * * *       \n"
	"                  * * * *       \n"
	"    - - - - - - - - * * - - - - \n"
	"- - - - - - - - - - * * - - -   \n"
	"- - - - - - - - - - * * - - - - \n"
	"- - - - - - - - - - * * - -   - \n"
	"                  * * * *       \n"
	"                  * * * *       \n",
	// -------------------------------
	"                - * * *         \n"
	"              * - * * *         \n"
	"          * * * - * * *         \n"
	"            * * - * *           \n"
	"            * * - *             \n"
	"            * * *               \n"
	"            * * *               \n"
	"            * * *               \n"
	"            - * *   -           \n"
	"            - * *   -           \n"
	"        *   - - - - -           \n"
	"      * * * - * *   -           \n"
	"        *   - - - - -           \n"
	"            - * *   -           \n"
	"            * * *   -           \n"
	"            * * *               \n",
};

static const char *swwinsym[] = {  /*  Win plane pixel array  */
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"              * *               \n"
	"              * *               \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"              * *               \n"
	"            - - - -             \n"
	"            - - - -             \n"
	"              * *               \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"              * *               \n"
	"        - - - * * - - -         \n"
	"              * *               \n"
	"          - - - - - -           \n"
	"            *     *             \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"              * *               \n"
	"              * *               \n"
	"              * *               \n"
	"- - - - - - - * * - - - - - - - \n"
	"              * *               \n"
	"        - - - - - - - -         \n"
	"            * * * *             \n"
	"            * * * *             \n"
	"  - - - - - - - - - - - - - -   \n"
	"          *         *           \n"
	"        *             *         \n"
	"        *             *         \n"
	"                                \n"
	"                                \n",
};


static const char *swbmbsym[] = {

	"                \n"
	"                \n"
	"* *   * * * *   \n"
	"* * * * * * * * \n"
	"* * * * * * * * \n"
	"* *   * * * *   \n"
	"                \n"
	"                \n",
	// ----------------
	"                \n"
	"        * * *   \n"
	"      * * * * * \n"
	"    * * * * * * \n"
	"  *   * * * *   \n"
	"* * * *   *     \n"
	"  * * * *       \n"
	"      *         \n"
};

static const char *swtrgsym[] = {

	"                      -         \n"
	"                      - * * * * \n"
	"                      - * * * * \n"
	"                      -         \n"
	"                      -         \n"
	"                      -         \n"
	"                      -         \n"
	"* * * * * * * * * * * * * * * * \n"
	"* * * * * * * * * * * * * * * * \n"
	"* * - - - - - - - - - - - - * * \n"
	"* * - * * * * * * * * * * - * * \n"
	"* * - * * * * * * * * * * - * * \n"
	"* * - * * * * * * * * * * - * * \n"
	"* * - * * * * * * * * * * - * * \n"
	"* * - * * * * * * * * * * - * * \n"
	"* * - * * * * * * * * * * - * * \n",
	// -------------------------------
	"                    - -     - - \n"
	"                    - -     - - \n"
	"                    - -     - - \n"
	"* * * * * * * * * * - -     - - \n"
	"* * * * * * * * * * - -     - - \n"
	"* * * - * - * - * * - -     - - \n"
	"* * * * * * * * * * * * * * - - \n"
	"* * * - * - * - * * * * * * - - \n"
	"* * * * * * * * * * * * * * - - \n"
	"* * * - * - * - * - * - * * - - \n"
	"* * * * * * * * * * * * * * - - \n"
	"* * * - * - * - * - * - * * - - \n"
	"* * * * * * * * * * * * * * - - \n"
	"* * * - * - * - * - * - * * - - \n"
	"* * * * * * * * * * * * * * - - \n"
	"* * * * * * * * * * * * * * - - \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"      * * * * * * * * * *       \n"
	"  * * * * * * * * * * * * * *   \n"
	"* * * * * * - * - * * * * * * * \n"
	"* * * * * * - - - * * * * * * * \n"
	"* * * * * * - * - * * * * * * * \n"
	"* * * * * * - - - * * * * * * * \n"
	"  * * * * * - * - * * * * * *   \n"
	"    * * * * - - - * * * * *     \n"
	"    - -     -   -       - -     \n"
	"    - -     - - -       - -     \n"
	"    - -     -   -       - -     \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"          * * * * * *           \n"
	"          * * * * * * * * * * * \n"
	"          * * * * * *           \n"
	"* * * * * * * * * * * * * * * * \n"
	"* * * * * * * * * * * * * * * * \n"
	"* - - - - - - - - - - - - - - * \n"
	"- * * * * * * * * * * * * * * - \n"
	"- * * * * * * * * * * * * * * - \n"
	"  - - - - - - - - - - - - - -   \n",
};

static const char *swhtrsym =
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                                \n"
"                            *   \n"
"*                           * * \n"
"* *       *           -   * * * \n"
"* *   * * *     * *   - * * * * \n"
"* * * * - - * * * * - * * * * * \n"
"* * * * - - * * * - * * * * * * \n";

static const char *swexpsym[] = {
	"* * *   -       \n"
	"* * *   -       \n"
	"* * * -         \n"
	"    - - - * * * \n"
	"- -   - * * * * \n"
	"      * *       \n"
	"      * *       \n"
	"      * *       \n",
	// ----------------
	"      * *       \n"
	"  * * * * * *   \n"
	"* * * - - * * * \n"
	"* * - - - - * * \n"
	"* * - - - - * * \n"
	"* * * - - * * * \n"
	"  * * * * * *   \n"
	"      * *       \n",
	// ----------------
	"          -     \n"
	"        - - -   \n"
	"      - - - - - \n"
	"    - - - - -   \n"
	"  - - - - -     \n"
	"- - - - -       \n"
	"  - - -         \n"
	"    -           \n",
	// ----------------
	"      * *       \n"
	"      * * *     \n"
	"  * * * * * *   \n"
	"* * * * * * * * \n"
	"* * * * * * * * \n"
	"  * * * *   *   \n"
	"    * * *       \n"
	"      * *       \n",
	// ----------------
	"  * - - -       \n"
	"  * * * * *     \n"
	"    - - * - *   \n"
	"- * * - - - *   \n"
	"            * * \n"
	"  * -       * - \n"
	"    * *     - * \n"
	"    * -       * \n",
	// ----------------
	"* *             \n"
	"* *     * -     \n"
	"        - *     \n"
	"                \n"
	"          - -   \n"
	"  - -     - -   \n"
	"  - -           \n"
	"                \n",
	// ----------------
	"                \n"
	"            * * \n"
	"            * * \n"
	"      * *       \n"
	"* -   * *       \n"
	"- *             \n"
	"          - -   \n"
	"          - -   \n",
	// ----------------
	"                \n"
	"  *       *     \n"
	"      *         \n"
	"            *   \n"
	"*   *           \n"
	"        *     * \n"
	"    *           \n"
	"            *   \n",
};

static const char *swflksym[] = {

	"  #                             \n"
	"#   #                           \n"
	"              #   #         #   \n"
	"      #         #         #   # \n"
	"    #   #                       \n"
	"                                \n"
	"#   #     #   #         #       \n"
	"  #         #         #   #     \n"
	"                                \n"
	"    #   #     #         #   #   \n"
	"      #     #   #         #     \n"
	"                                \n"
	"      #             #           \n"
	"    #   #         #   #         \n"
	"            #   #               \n"
	"              #                 \n",
	// -------------------------------
	"#   #                           \n"
	"  #                             \n"
	"                #         #   # \n"
	"    #   #     #   #         #   \n"
	"      #                         \n"
	"                                \n"
	"  #         #         #   #     \n"
	"#   #     #   #         #       \n"
	"                                \n"
	"      #     #   #         #     \n"
	"    #   #     #         #   #   \n"
	"                                \n"
	"    #   #         #   #         \n"
	"      #             #           \n"
	"              #                 \n"
	"            #   #               \n"
};

static const char *swbrdsym[] = {
	"  #     \n"
	"#   #   \n",
	//---------
	"#   #   \n"
	"  #     \n",
};

static const char *swoxsym[] = {
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                    #     #     \n"
	"                    # - - -     \n"
	"                    - # * # #   \n"
	"      - - - - - - # - - # # #   \n"
	"  - - - - - - - - # - - # # - # \n"
	"# - - - - - - - - # - - - # #   \n"
	"# - - - - - - - - - # #         \n"
	"# - - - - - - - - - - -         \n"
	"# - -   - -     - -   - -       \n"
	"  - -   - -     - -   - -       \n"
	"  # #   # #     # #   # #       \n",
	// -------------------------------
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                        #       \n"
	"    - - - - - - - -   - - -     \n"
	"# - # # - - - - # # - - - # #   \n"
	"# - - - - - - - - - - - - # #   \n"
	"# - - # # - - # # -   - - #     \n"
};

static const char *swshtsym =
"                            *   \n"
"      *                 * *     \n"
"      *           * * *         \n"
"        *       *               \n"
"        *     *     * *         \n"
"          * * *   *     *       \n"
"* * * *   * * - *         * *   \n"
"        * * * - - *           * \n"
"        * * * - * *             \n"
"          * - - *   * *         \n"
"        *   * *         *       \n"
"      *       *         *       \n"
"    *         *         *       \n"
"    *     * * *       *         \n"
"    *   *               *       \n"
"          * *             *     \n";

static const char *swsplsym =
"                                  - - -                         \n"
"                                  - * * - -                     \n"
"      *                             - * * * -             *     \n"
"    * * *                             - * * * -           *     \n"
"      * *           *                 - * * * * -       * *     \n"
"      * *         * * *               - * * * * * -             \n"
"                    *         *       - * * * * * * -           \n"
"                                      - * * * * * * * -         \n"
"        *                           - * * * * * * * * -         \n"
"                                  - * * * * * * * * * -         \n"
"                          *     - * * * * * * * * * * -         \n"
"    *                         - * * * * * * * * * * * -         \n"
"              - - - - - -   - * * * * * * * * * * * -           \n"
"            - * * * * * * - * * * * * * * * * * * -             \n"
"          - * - * * * - * * - * * * * * * * * * -         * *   \n"
"        - * -   - * -   - * * - * * * * * * * -                 \n"
"        - * -   - * -   - * * - - - * * - - -       *           \n"
"        - * * - * * * - * * * - * * - -                     *   \n"
"        - * * * * * * * * * * - * * * -             *   *       \n"
"        - * - - * - * * * * * - * * * * -                       \n"
"          - - * - * * * * * - - * * * * * -               *     \n"
"        - - * - * * * * * -   - * * * * * * - - - - -           \n"
"      - - * - - - - - - -       - * * * * * * * * * * -         \n"
"    - - * -             - -       - * * * * * * * * * * -       \n"
"    - * -               -   -       - * * * * * * * * * * -     \n"
"                        -     -       - * * * * * * * * * * -   \n"
"                        -       -       - * * * * * * * * * * - \n"
"                      -           -       - * * * * * * * * * - \n"
"      *         - - - - -         -         - - - - - * * * * - \n"
"  * *       *       -       - - - - - -               - * * * - \n"
"* * *     * * *     -           -         *             - * -   \n"
"* *         *                 -                 *         -     \n";

static unsigned char swmscsym[][16] = {

/*  bomb symbols based on the following template file:


08                08                08                08
. . . . . . . .   . . . . . . . .   . . . . . . . .   . . . . . . . .
. . . . . . . .   . . . . . . . .   . . . . . 1 1 .   . . . . 1 1 . .
. 1 1 . . . . .   . . . . . 1 1 .   . . . . 1 1 1 .   . . . . 1 1 . .
. 1 1 1 1 1 1 .   . . . 1 1 1 1 .   . . . 1 1 1 . .   . . . 1 1 . . .
. 1 1 1 1 1 1 .   1 1 1 1 1 . . .   . 1 1 1 1 . . .   . . . 1 1 . . .
. 1 1 . . . . .   . 1 1 1 . . . .   . . 1 1 . . . .   . 1 1 1 . . . .
. . . . . . . .   . . 1 1 . . . .   . . . 1 . . . .   . . 1 1 1 . . .
. . . . . . . .   . . . . . . . .   . . . . . . . .   . . . . . . . .
*/
	{
		0x0 , 0x0 , 0x0 , 0x0 , 0x14, 0x0 , 0x15, 0x54, 0x15, 0x54,
		0x14, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	},
	{
		0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x14, 0x1 , 0x54, 0x55, 0x40,
		0x15, 0x0 , 0x5 , 0x0 , 0x0 , 0x0 ,
	},
	{
		0x0 , 0x0 , 0x0 , 0x14, 0x0 , 0x54, 0x1 , 0x50, 0x15, 0x40,
		0x5 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 ,
	},
	{
		0x0 , 0x0 , 0x0 , 0x50, 0x0 , 0x50, 0x1 , 0x40, 0x1 , 0x40,
		0x15, 0x0 , 0x5 , 0x40, 0x0 , 0x0 ,
	},
};

static unsigned char swbstsym[][16] = {


/*  starburst symbols based on the following template file:

08                 08
. . . . 1 . . .    . . . 1 . . 1 .
. 1 . . 1 . 1 .    1 . . 1 . 1 . .
. . 1 . 1 1 . .    . 1 . 1 1 . . .
1 1 1 1 1 . . .    . . 1 1 1 1 1 1
. . . 1 1 1 1 1    1 1 1 1 1 1 . .
. . 1 1 . 1 . .    . . . 1 1 . 1 .
. 1 . 1 . . 1 .    . . 1 . 1 . . 1
. . . 1 . . . .    . 1 . . 1 . . .
*/
	{
		0x0 , 0x40, 0x10, 0x44, 0x4 , 0x50, 0x55, 0x40, 0x1 , 0x55,
		0x5 , 0x10, 0x11, 0x4 , 0x1 , 0x0 ,
	},
	{		
		0x1 , 0x4 , 0x41, 0x10, 0x11, 0x40, 0x5 , 0x55, 0x55, 0x50,
		0x1 , 0x44, 0x4 , 0x41, 0x10, 0x40
	}
};

static unsigned char swmedalsym[][24] = {
/* Medal symbols based on the following templates:
08
. . 3 3 3 3 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . . 1 1 . . .
. 2 2 . . 2 2 .
2 3 3 2 2 2 2 2
2 3 2 2 2 2 2 2
. 2 2 2 2 2 2 .
. . 2 2 2 2 . .
. . . 2 2 . . .

08
. . 3 3 3 3 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . 1 1 1 1 . .
. . . 1 1 . . .
. . . 1 1 . . .
. . . . . . . .
. . . 3 3 . . .
. . 3 2 3 3 . .
. . 3 2 2 3 . .
. . . 3 3 . . .

08
. . 3 3 3 3 . .
. . 2 2 2 2 . .
. . 2 2 2 2 . .
. . 2 2 2 2 . .
. . . 2 2 . . .
. . . . . . . .
. . . 3 3 . . .
. . . 1 1 . . .
. 3 1 3 1 1 3 .
. 3 1 1 1 1 3 .
. . . 1 1 . . .
. . . 3 3 . . .

*/
	{
	  0x0f, 0xf0,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x01, 0x40,
	  0x28, 0x28,
	  0xbe, 0xaa,
	  0xba, 0xaa,
	  0x2a, 0xa8,
	  0x0a, 0xa0,
	  0x02, 0x80
	},
	{
	  0x0f, 0xf0,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x05, 0x50,
	  0x01, 0x40,
	  0x01, 0x40,
	  0x00, 0x00,
	  0x03, 0xc0,
	  0x0e, 0xf0,
	  0x0e, 0xb0,
	  0x03, 0xc0
	},
	{
	  0x0f, 0xf0,
	  0x0a, 0xa0,
	  0x0a, 0xa0,
	  0x0a, 0xa0,
	  0x02, 0x80,
	  0x00, 0x00,
	  0x03, 0xc0,
	  0x01, 0x40,
	  0x37, 0x5c,
	  0x35, 0x5c,
	  0x01, 0x40,
	  0x03, 0xc0
	}
};

static unsigned char swribbonsym[][4] = {
	/* Actual width: 7 pixels */
	{ 0x57, 0x54,
	  0x57, 0x54 }, /* CCCWCCC : ACE */
	{ 0x5d, 0xd4,
	  0x5d, 0xd4 }, /* CCWCWCC : TOPACE */
	{ 0xef, 0xec,
	  0xef, 0xec }, /* WMWWWMW : PERFECT */
	{ 0xd5, 0x5c,
	  0xd5, 0x5c }, /* WCCCCCW : SERVICE */
	{ 0xda, 0x9c,
	  0xda, 0x9c }, /* WCMMMCW : COMPETENCE2 */
	{ 0xaf, 0xe8,
	  0xaf, 0xe8 }	/* MMWWWMM : PREVALOUR */
};

static void rotate(int *x, int *y, int w, int h, int rotations, bool mirror)
{
	int i, tmp;

	for (i = 0; i < rotations; i++) {
		tmp = *x; *x = *y; *y = w - 1 - tmp;
		tmp = w; w = h; h = tmp;
	}

	if (mirror) {
		*y = h - 1 - *y;
	}
}

// sdh 27/6/2002: create a sopsym_t structure from the original
// raw sprite data.
// the data in sopsym_t's is in a simpler one-byte-per-pixel format
// rather than packing 4 pixels into one byte as in the original data.
// this simplifies various stuff such as collision detection.
// 'rotations' specified the number of 90 degree rotations to perform
// on the input data, eg. 3=270 degrees.
static void sopsym_from_data(sopsym_t *sym, unsigned char *data, int w, int h,
                             int rotations, bool mirror)
{
	unsigned char *d;
	int x, y, dx, dy;

	if ((rotations & 1) == 0) {
		sym->w = w;
		sym->h = h;
	} else {
		sym->w = h;
		sym->h = w;
	}
	sym->data = malloc(w * h);

	// decode the symbol data
	d = data;
	for (y=0; y<h; ++y) {
		for (x=0; x<w; x += 4, ++d) {
			dx = x; dy = y;
			rotate(&dx, &dy, w, h, rotations, mirror);
			sym->data[dy * sym->w + dx] = (*d >> 6) & 0x03;

			dx = x + 1; dy = y;
			rotate(&dx, &dy, w, h, rotations, mirror);
			sym->data[dy * sym->w + dx] = (*d >> 4) & 0x03;

			dx = x + 2; dy = y;
			rotate(&dx, &dy, w, h, rotations, mirror);
			sym->data[dy * sym->w + dx] = (*d >> 2) & 0x03;

			dx = x + 3; dy = y;
			rotate(&dx, &dy, w, h, rotations, mirror);
			sym->data[dy * sym->w + dx] = *d & 0x03;
		}
	}
}

static symset_t *symset_from_data(unsigned char *data, int w, int h)
{
	symset_t *s = malloc(sizeof(*s));
	int r;

	for (r = 0; r < 4; r++)
	{
		sopsym_from_data(&s->sym[r], data, w, h, r, false);
		sopsym_from_data(&s->sym[r + 4], data, w, h, r, true);
	}

	return s;
}

static void sopsym_from_text(sopsym_t *sym, const char *text, int w, int h,
                             int rotations, bool mirror)
{
	const char *p, *p2;
	int x, y, dx, dy, c;

	if ((rotations & 1) == 0) {
		sym->w = w;
		sym->h = h;
	} else {
		sym->w = h;
		sym->h = w;
	}
	sym->data = malloc(w * h);

	x = 0; y = 0;
	for (p = text; *p != '\0'; p++) {
		if (*p == '\n') {
			x = 0;
			y++;
		} else {
			p2 = strchr(color_chars, *p);
			if (p2 != NULL) {
				c = p2 - color_chars;
			} else {
				c = 0;
			}

			if (x < w * 2 && y < h && (x % 2) == 0) {
				dx = x / 2; dy = y;
				rotate(&dx, &dy, w, h, rotations, mirror);
				sym->data[dy * sym->w + dx] = c;
			}
			x++;
		}
	}
}

static symset_t *symset_from_text(const char *text, int w, int h)
{
	symset_t *s = malloc(sizeof(*s));
	int r;

	for (r = 0; r < 4; r++)
	{
		sopsym_from_text(&s->sym[r], text, w, h, r, false);
		sopsym_from_text(&s->sym[r + 4], text, w, h, r, true);
	}

	return s;
}

// converted symbols:

symset_t *symbol_bomb[2];                 // swbmbsym
symset_t *symbol_targets[4];              // swtrgsym
symset_t *symbol_target_hit;              // swhtrsym
symset_t *symbol_debris[8];               // swexpsym
symset_t *symbol_flock[2];                // swflksym
symset_t *symbol_bird[2];                 // swbrdsym
symset_t *symbol_ox[2];                   // swoxsym
symset_t *symbol_shotwin;                 // swshtsym
symset_t *symbol_birdsplat;               // swsplsym
symset_t *symbol_missile[4];              // swmscsym
symset_t *symbol_burst[2];                // swbstsym
symset_t *symbol_plane[4];                // swplnsym
symset_t *symbol_plane_hit[2];            // swhitsym
symset_t *symbol_plane_win[4];            // swwinsym
symset_t *symbol_medal[3];                // swmedalsym
symset_t *symbol_ribbon[6];               // swribbonsym

// special symbol for single pixel (bullets etc)

static unsigned char pixel_data[] = { 3 };

sopsym_t symbol_pixel = {
	pixel_data,
	1,
	1
};

// generate symbols from data

#define symsets_from_data(data, w, h, out)                        \
        { int _i;                                                 \
          for (_i=0; _i<sizeof(out)/sizeof(*(out)); ++_i)         \
             (out)[_i] = symset_from_data((data)[_i], (w), (h));  \
        }

#define symsets_from_text(text, w, h, out)                        \
        { int _i;                                                 \
          for (_i=0; _i<sizeof(out)/sizeof(*(out)); ++_i)         \
             (out)[_i] = symset_from_text((text)[_i], (w), (h));  \
        }

void symbol_generate(void)
{
	symsets_from_text(swbmbsym, 8, 8, symbol_bomb);
	symsets_from_text(swtrgsym, 16, 16, symbol_targets);
	symsets_from_text(swexpsym, 8, 8, symbol_debris);
	symsets_from_text(swflksym, 16, 16, symbol_flock);
	symsets_from_text(swbrdsym, 4, 2, symbol_bird);
	symsets_from_text(swoxsym, 16, 16, symbol_ox);
	symsets_from_data(swmscsym, 8, 8, symbol_missile);
	symsets_from_data(swbstsym, 8, 8, symbol_burst);
	symsets_from_text(swplnsym, 16, 16, symbol_plane);
	symsets_from_text(swhitsym, 16, 16, symbol_plane_hit);
	symsets_from_text(swwinsym, 16, 16, symbol_plane_win);
	symsets_from_data(swmedalsym, 8, 12, symbol_medal);
	symsets_from_data(swribbonsym, 8, 2, symbol_ribbon);

	symbol_target_hit = symset_from_text(swhtrsym, 16, 16);
	symbol_shotwin = symset_from_text(swshtsym, 16, 16);
	symbol_birdsplat = symset_from_text(swsplsym, 32, 32);
}

//
// 2003-02-14: Code was checked into version control; no further entries
// will be added to this log.
//
// sdh 14/2/2003: change license header to GPL
// sdh 28/06/2002: move plane sprites here, drop swplanes.c
//                 original sprite data (char sw*) made static
// sdh 27/06/2002: add code to generate new sopsym_t objects from the
//                 original sprite data
// sdh 21/10/2001: reformatted headers, added cvs tags
// sdh 20/10/2001: added some missing {}'s to shut up compiler
//
