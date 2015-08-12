/*
Title: Buffer Overflow
File Name: main.c
Copyright © 2015
Original authors: Nicholas Gallagher

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
Demonstrates the concept of a Buffer Overflow and how it can lead to 
security issues in your program.

This example was written on a 64 bit intel processor on a Linux machine 
using GCC and recompiled in GCC using MinGW for the Windows platform on a 
64 bit intel processor.
*/

#include <stdio.h>
#include <string.h>

//Disclaimer: Code was tested on 64 bit machines, I'm unsure of behavior on 32 bit machines.

unsigned char authenticate(char* input)
{
	//Authenticate will set up a new stack frame
	//The stack grows downward in memory addresses
	//
	//Here is a glimpse of what the stack might look like
	//	PREVIOUS STACK FRAME
	//		...
	//	CURRENT STACK FRAME
	//		parameters			4 bytes on 32 bit machine 8 bytes on 64 bit machine
	//		Function return address		4 bytes on 32 bit machine 8 bytes on 64 bit machine
	//		Pointer to previous stack frame 4 bytes on 32 bit machine 8 bytes on 64 bit machine
	//		Exception handler frame		0 bytes (This is C-- no exceptions!)
	//		Local Variables			16 bytes on 64 bit machine, could possibly be 12 bytes on 32 bit machine? Unsure.
	//		
	//		
	//The first thing on the stack is the parameters, input.
	//In the debugging of this program this was located at address
	//0x7fffffffffe820
	//
	//The function return address would then be located at
	//0x7fffffffffe818
	//
	//Followed by the pointer to the previous stack frame
	//0x7fffffffffe810
	//
	//Then comes the local variables. First authenticate would be put on the stack
	//Authenticate is only 1 byte, so it is put at address:
	//0x7fffffffffe80f
	//
	//Finally we have our buffer. On a 64 bit intel machine (What I am running on) 8 bytes of memory must be DWORD (8 byte) aligned
	//Meaning the start of the buffer must be located at a memory address which is evenly divisible by 8.
	//On a 32 bit machine, I believe the data would be WORD (4 byte) aligned.
	//So on a 64 bit machine, this would put the beginning of our buffer at address:
	//0x7fffffffffe800
	//
	//And the end of our buffer, the last characters address would be located at:
	//0x7fffffffffe807
	//
	//This leaves the memory range 0x7fffffffe808 - 0x7fffffffe8e unoccupied (on a 64 bit intel machine).
	//
	char buffer[8];			//At memory address 0x7fffffffe800 - 0x7fffffffe807
	unsigned char authenticate = 0;	//At memory address 0x7fffffffe80f

	//Now we copy input into buffer until we read a null terminator.
	//Notice, in input, there are 16 bytes before reaching the first null terminator!
	//Also notice the number of bytes between authenticate and the start of our buffer:
	//	0x7fffffffe80f
	//    - 0x7fffffffe800
	//---------------------
	//      0x00000000000f
	//Which in decimal is 16!
	//
	//This tells us that the 16th character ('k' = 107 in ascii) of our input string will overwrite
	//our authenticate variable setting it to 107 or 'k' in ascii instead of the initial value of 0!
	//It should be noted that this causes stack corruption, but we will get to that later.
	strcpy(buffer, input);

	//Now, we still don't have the right password.. So this will never return true- but we don't need it to
	if(strcmp(buffer, "Password") == 0)
		authenticate = 1;

	//We return authenticate, which is a value of 107. This works fine because in C any non-zero value is considered true!
	return authenticate;
}

int main(void)
{
	//Create a buffer for user input
	char input[128] = { "\0" };

	//Ask the user for a password
	printf("Please enter a password.\n\nJust kidding,\nfor the purpose of this program I will force a password to be entered\n\n");
	//scanf("%s", input);
	//I am going to force a hack here to bypass password requirement
	//Copy the incorrect password into the input buffer
	strcpy(input, "LookMom,I'mAHack");	//16 bytes
	printf("Password:\t%s\n", input);

	//Call authenticate
	if(authenticate(input))
	{
		printf("\nValid!\nWelcome to my secure files!\n");
	}
	else
		printf("\nAccess denied!\n");
	
	//At this point, we have accessed the secure files and revealed data we should not have revealed without the
	//Proper password. The program is still going to crash with a segmentation fault because we corrupted the stack
	//However, we still were able to access the information which we wanted and to my knowledge the program
	//Will continue running without crashing until the main function ends.
	printf("\nPress enter to exit\n");
	getchar();
	
	return 0;
}

