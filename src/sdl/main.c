//
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
//
//
// Program entry point
//
// SDL sets up some routines which wrap the main function; however
// the main sopwith files do not include the SDL headers.
//

#include <SDL.h>

#include "swmain.h"

int main(int argc, char *argv[])
{
	return swmain(argc, argv);
}
