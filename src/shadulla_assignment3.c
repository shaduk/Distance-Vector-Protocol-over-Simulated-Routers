/**
 * @shadulla_assignment3
 * @author  Shad Ullah Khan <shadulla@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
#include "../include/global.h"
#include "../include/connection_manager.h"


int main(int argc, char **argv)
{
	/*Start Here*/
	 sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
	 		printf("%" SCNu16 , CONTROL_PORT);
    	init(); // Initialize connection manager; This will block
	return 0;
}
