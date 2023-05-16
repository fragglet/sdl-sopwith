//
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
