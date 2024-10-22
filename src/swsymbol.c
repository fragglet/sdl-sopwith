//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
//
// Sprites
//

#include <string.h>

#include "swsymbol.h"

// Linked list of all symbols, to allow replacement in custom levels:
symset_t *all_symsets = NULL;

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
	"                * * - -         \n"
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

static const char custom_target_sym[] = {
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n",
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

	// TARGET_FACTORY:
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

	// TARGET_OIL_TANK:
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

	// TARGET_TANK:
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

	// TARGET_TRUCK:
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"* # * * # * * # * *             \n"
	"* # * * # * * # * *             \n"
	"* # * * # * * # * * - - -       \n"
	"* # * * # * * # * * - # #       \n"
	"* # * * # * * # * * - # #       \n"
	"* # * * # * * # * * - # #       \n"
	"- - - - - - - - - - * * - - - - \n"
	"* * * * * * * * * * * - * * * * \n"
	"- * * - * * - - - - - * * - * * \n"
	"    - - -               - - -   \n"
	"      -                   -     \n",

	// TARGET_TANKER_TRUCK:
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"  * * * * * * *                 \n"
	"* * * - * - * * * - - - -       \n"
	"* * * - - - * * * - * # #       \n"
	"* * * - * - * * * - * # #       \n"
	"* * * - - - * * * - * # #       \n"
	"  * * - * - * *   - * * * * *   \n"
	"- - - - - - - - - * * * - - - - \n"
	"* * * * * * * * * * * - * * * * \n"
	"- * * - * * - - - - - * * - * * \n"
	"    - - -               - - -   \n"
	"      -                   -     \n",

	// TARGET_FLAG:
	"          -                     \n"
	"          - * * * * * * *       \n"
	"          - * * * * * * *       \n"
	"          - * * * * * * *       \n"
	"          - * * * * * * *       \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"          -                     \n"
	"        - - -                   \n"
	"      - - - - -                 \n",

	// TARGET_TENT:
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"                                \n"
	"          * * * * * *           \n"
	"      * * * - - - - * * *       \n"
	"    * * - - - - - - - - * *     \n"
	"  # * - - - - - - - - - - * #   \n"
	"  # * - - - - - - - - - - * #   \n"
	"#   * - - - - - - - - - - *   # \n"
	"#   * - - - - - - - - - - *   # \n",

	// User-defined target types:
	custom_target_sym,  // TARGET_CUSTOM1
	custom_target_sym,  // TARGET_CUSTOM2
	custom_target_sym,  // TARGET_CUSTOM3
	custom_target_sym,  // TARGET_CUSTOM4
	custom_target_sym,  // TARGET_CUSTOM5
	custom_target_sym,  // TARGET_CUSTOM_PASSIVE1
	custom_target_sym,  // TARGET_CUSTOM_PASSIVE2
	custom_target_sym,  // TARGET_CUSTOM_PASSIVE3
	custom_target_sym,  // TARGET_CUSTOM_PASSIVE4
	custom_target_sym,  // TARGET_CUSTOM_PASSIVE5

};

static const char destroyed_building[] =
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

static const char destroyed_truck[] =
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
	"                                \n"
	"    * *   -       * * -         \n"
	"  * * - * * -   * - - * *       \n"
	"* * - - - * * - * * * * - -     \n"
	"- * * - * * * * * * * * * - - * \n";

static const char destroyed_flag[] =
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
	"                                \n"
	"                                \n"
	"                                \n"
	"        - - -       * * *       \n"
	"      - - - - - - - - - * *     \n";

static const char destroyed_tent[] =
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
	"                                \n"
	"                                \n"
	"                                \n"
	"        * * *         * *       \n"
	"# # * * * * * * * * * * * * #   \n";

static const char destroyed_custom[] =
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
	"                                \n"
	"                                \n"
	"                                \n"
	"  * * * * * * * * * * * * * * * \n"
	"  * * * * * * * * * * * * * * * \n";

// There is one entry in this array for each in swtrgsym, so that different
// targets can have different "destroyed" symbols:
static const char *swhtrsym[] = {
	destroyed_building,  // TARGET_HANGAR
	destroyed_building,  // TARGET_FACTORY
	destroyed_building,  // TARGET_OIL_TANK
	destroyed_building,  // TARGET_TANK
	destroyed_truck,  // TARGET_TRUCK
	destroyed_truck,  // TARGET_TANKER_TRUCK
	destroyed_flag,  // TARGET_FLAG
	destroyed_tent,  // TARGET_TENT
	destroyed_custom,  // TARGET_CUSTOM1
	destroyed_custom,  // TARGET_CUSTOM2
	destroyed_custom,  // TARGET_CUSTOM3
	destroyed_custom,  // TARGET_CUSTOM4
	destroyed_custom,  // TARGET_CUSTOM5
	destroyed_custom,  // TARGET_CUSTOM_PASSIVE1
	destroyed_custom,  // TARGET_CUSTOM_PASSIVE2
	destroyed_custom,  // TARGET_CUSTOM_PASSIVE3
	destroyed_custom,  // TARGET_CUSTOM_PASSIVE4
	destroyed_custom,  // TARGET_CUSTOM_PASSIVE5
};

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

static const char *swshtsym[] = {
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
	"          * *             *     \n"
};

static const char *swsplsym[] = {
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
"* *         *                 -                 *         -     \n"
};

static const char *swmscsym[] = {

	"                \n"
	"                \n"
	"  * *           \n"
	"  * * * * * *   \n"
	"  * * * * * *   \n"
	"  * *           \n"
	"                \n"
	"                \n",
	//-----------------
	"                \n"
	"                \n"
	"          * *   \n"
	"      * * * *   \n"
	"* * * * *       \n"
	"  * * *         \n"
	"    * *         \n"
	"                \n",
	//-----------------
	"                \n"
	"          * *   \n"
	"        * * *   \n"
	"      * * *     \n"
	"  * * * *       \n"
	"    * *         \n"
	"      *         \n"
	"                \n",
	//-----------------
	"                \n"
	"        * *     \n"
	"        * *     \n"
	"      * *       \n"
	"      * *       \n"
	"  * * *         \n"
	"    * * *       \n"
	"                \n",
};

static const char *swbstsym[] = {
	"        *       \n"
	"  *     *   *   \n"
	"    *   * *     \n"
	"* * * * *       \n"
	"      * * * * * \n"
	"    * *   *     \n"
	"  *   *     *   \n"
	"      *         \n",
	//-----------------
	"      *     *   \n"
	"*     *   *     \n"
	"  *   * *       \n"
	"    * * * * * * \n"
	"* * * * * *     \n"
	"      * *   *   \n"
	"    *   *     * \n"
	"  *     *       \n",
};

static const char *swmedalsym[] = {
	"    # # # #     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"      * *       \n"
	"  - -     - -   \n"
	"- # # - - - - - \n"
	"- # - - - - - - \n"
	"  - - - - - -   \n"
	"    - - - -     \n"
	"      - -       \n",
	//-----------------
	"    # # # #     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"    * * * *     \n"
	"      * *       \n"
	"      * *       \n"
	"                \n"
	"      # #       \n"
	"    # - # #     \n"
	"    # - - #     \n"
	"      # #       \n",
	//-----------------
	"    # # # #     \n"
	"    - - - -     \n"
	"    - - - -     \n"
	"    - - - -     \n"
	"      - -       \n"
	"                \n"
	"      # #       \n"
	"      * *       \n"
	"  # * # * * #   \n"
	"  # * * * * #   \n"
	"      * *       \n"
	"      # #       \n",
};

static const char *swribbonsym[] = {
	"* * * # * * *   \n"
	"* * * # * * *   \n", /* CCCWCCC : ACE */

	"* * # * # * *   \n"
	"* * # * # * *   \n", /* CCWCWCC : TOPACE */

	"# - # # # - #   \n"
	"# - # # # - #   \n", /* WMWWWMW : PERFECT */

	"# * * * * * #   \n"
	"# * * * * * #   \n", /* WCCCCCW : SERVICE */

	"# * - - - * #   \n"
	"# * - - - * #   \n", /* WCMMMCW : COMPETENCE2 */

	"- - # # # - -   \n"
	"- - # # # - -   \n", /* MMWWWMM : PREVALOUR */
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

void symset_from_text(symset_t *s, const char *text, int w, int h)
{
	int r;

	for (r = 0; r < 4; r++)
	{
		sopsym_from_text(&s->sym[r], text, w, h, r, false);
		sopsym_from_text(&s->sym[r + 4], text, w, h, r, true);
	}
}

symset_t *lookup_symset(const char *name, int frame)
{
	symset_t *s;

	for (s = all_symsets; s != NULL; s = s->next) {
		if (!strcmp(name, s->name) && frame == s->frame) {
			return s;
		}
	}

	return NULL;
}

static void init_symset(symset_t *s, const char *name, int frame)
{
	s->name = name;
	s->frame = frame;
	s->next = all_symsets;
	all_symsets = s;
}

// converted symbols:

symset_t symbol_bomb[2];                 // swbmbsym
symset_t symbol_targets[NUM_TARGET_TYPES];    // swtrgsym
symset_t symbol_target_hit[NUM_TARGET_TYPES]; // swhtrsym
symset_t symbol_debris[8];               // swexpsym
symset_t symbol_flock[2];                // swflksym
symset_t symbol_bird[2];                 // swbrdsym
symset_t symbol_ox[2];                   // swoxsym
symset_t symbol_shotwin[1];              // swshtsym
symset_t symbol_birdsplat[1];            // swsplsym
symset_t symbol_missile[4];              // swmscsym
symset_t symbol_burst[2];                // swbstsym
symset_t symbol_plane[4];                // swplnsym
symset_t symbol_plane_hit[2];            // swhitsym
symset_t symbol_plane_win[4];            // swwinsym
symset_t symbol_medal[3];                // swmedalsym
symset_t symbol_ribbon[6];               // swribbonsym

// special symbol for single pixel (bullets etc)

static unsigned char pixel_data[] = { 3 };

sopsym_t symbol_pixel = {
	pixel_data,
	1,
	1
};

// generate array of symset_t structs from array of strings:
#define symsets_from_text(text, w, h, out)                        \
        { int _i;                                                 \
          for (_i=0; _i<sizeof(out)/sizeof(*(out)); ++_i) {       \
             init_symset(&(out)[_i], #text, _i);                  \
             symset_from_text(&(out)[_i], (text)[_i], (w), (h));  \
          }                                                       \
        }

void symbol_generate(void)
{
	symsets_from_text(swbmbsym, 8, 8, symbol_bomb);
	symsets_from_text(swtrgsym, 16, 16, symbol_targets);
	symsets_from_text(swexpsym, 8, 8, symbol_debris);
	symsets_from_text(swflksym, 16, 16, symbol_flock);
	symsets_from_text(swbrdsym, 4, 2, symbol_bird);
	symsets_from_text(swoxsym, 16, 16, symbol_ox);
	symsets_from_text(swmscsym, 8, 8, symbol_missile);
	symsets_from_text(swbstsym, 8, 8, symbol_burst);
	symsets_from_text(swplnsym, 16, 16, symbol_plane);
	symsets_from_text(swhitsym, 16, 16, symbol_plane_hit);
	symsets_from_text(swwinsym, 16, 16, symbol_plane_win);
	symsets_from_text(swmedalsym, 8, 12, symbol_medal);
	symsets_from_text(swribbonsym, 8, 2, symbol_ribbon);
	symsets_from_text(swhtrsym, 16, 16, symbol_target_hit);
	symsets_from_text(swshtsym, 16, 16, symbol_shotwin);
	symsets_from_text(swsplsym, 32, 32, symbol_birdsplat);
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
