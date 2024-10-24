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

#ifndef __SWGAMES_H__
#define __SWGAMES_H__

#include "sw.h"

extern const GAMES original_level;
extern GAMES custom_level;
extern bool have_custom_level;

void LoadCustomLevel(const char *filename);

#endif
