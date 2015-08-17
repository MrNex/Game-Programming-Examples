/*
Title: Struct Packing
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

Demonstrates the concept of struct packing and how you can responsibly align your memory
This example was written on a 64 bit intel processor on a Linux machine and compiled using GCC v5.2

Instructions:
To compile this project using visual studio please make a new Empty C++ project, add a file called main.c
And copy and paste the the contents of this file into there.
 */

#include <stdio.h>

//Create defines for colors for cats
#define BROWN 0
#define BLACK 1
#define WHTIE 2
#define ORANGE 3

struct BigCat
{
	unsigned char age;	//Age of cat in years
	char* name;		//Ptr to name of cat
	unsigned char color;	//Color of cat
	int happiness;		//Happyness level of cat, 0 is neutral
	unsigned short mass;	//In grams- average cat is ~ 4500 grams
};

struct SmallCat
{
	unsigned char age;	//Age in years
	unsigned char color;	//COlor of cat
	unsigned short mass;	//Mass in grams
	int happiness;		//Happiness level of cat, 0 is neutral
	char* name;		//Ptr to name
};


//Note the two cats hold the same amount of data- therefore they should take up the same amount of space?
//The answer is no.

//Proof:
int main(void)
{

	printf("Size of BigCat is %u bytes.\nSize of SmallCat is %u bytes.\n", sizeof(struct BigCat), sizeof(struct SmallCat));
	//This will output 32 bytes and 16 bytes for the sizes, respectively.
	//Lets look at the contents of the structs:
	//Type			Size		Total Size
	//-------------------------------------------------
	//unsigned char		1 byte		1 byte
	//unsigned char		1 byte		2 bytes
	//unsigned short	2 bytes		4 bytes
	//int			4 bytes		8 bytes
	//pointer		8 bytes		16 bytes
	//
	//It should be noted that these sizes are specific to an intel 64 bit processor. On an intel 32 bit processor this would
	//be almost identical except the size of the pointer would be only 4 bytes. So then, the test prints out that SmallCat is the correct size!
	//
	//But where is all of that extra space coming from in big cat? It holds the same amount of data but it is double the size on an intel 64 bit processor.
	//Why?
	//
	//The culprit here is data alignment. In a computer data must be placed at certain memory offsets to increase the speed of reading that data.
	//Although there are very specific rules for alignment based on the processor being used, a general rule of thumb is
	//all data must be algined by it's own size. By this I mean, an integer (4 bytes) must begin in a memory address which is divisible by 4.
	//Compilers will add padding to your data to ensure it follows these rules, but by "packing" your structs, as shown here we can avoid the wasted space.
	//
	//So lets take a closer look at SmallCat:
	//We have two characters first, which can be aligned on any byte- so those are placed in the first 2 bytes from where the start of the struct begins.
	//No padding necessary. Then we have a short, which is 2 bytes. This must be 2 byte aligned-- however because our offset is still only 2 bytes, this falls
	//perfectly aligned & is placed right next to the first 2 chars:
	//|	Byte	|	Byte	|	Byte	|	Byte	|
	//---------------------------------------------------------------
	//|	char	|	char	|		short		|	4 Bytes
	//
	//You might be asking yourself, how do we know that the 3rd byte (or 2nd if you 0 index) of our structure is an address that is divisible by 2? Sure it's offset from the start of the structure is 2
	//But the structure's address might not be divisble by 2 breaking this rule! Well, when you declare a struct it is forced to have the alignment of it's largest member.
	//So our cat structs have 8 byte alignment. This means our struct must start on an address divisble by 8 and therefore divisible by 2 as well. This is done to ensure
	//All members of a struct can be easily aligned based off of their offset in the struct. Moving on...
	//
	//Next we have an integer, and the next available byte is at offset 4, so the integer can be placed right next to the short:
	//|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|
	//-------------------------------------------------------------------------------------------------------------------------------
	//|	char	|	char	|		short		|				int				|	8 Bytes
	//
	//Finally, after that we have an 8 byte pointer which falls on a byte at offset 8! So it can follow the int with no padding needed:
	//|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|
	//-------------------------------------------------------------------------------------------------------------------------------
	//|	char	|	char	|		short		|				int				|	8 Bytes
	//|								pointer								|	16 Bytes
	//
	//Now lets take a look at what happened with BigCat:
	//|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|
	//-------------------------------------------------------------------------------------------------------------------------------
	//|	char	|					7 Bytes of Padding							|	8 Bytes
	//|								Pointer								|	16 Bytes
	//|	char	|		3 Bytes of padding		|				int				|	24 bytes
	//|		short		|----------------------------------Unused-------------------------------------------------------|	26 bytes
	//
	//So why does BigCat take up 32 bytes and not 26 like it should?
	//Remember when I said structs have the alignment of the largest member? Well it's a strict rule that the first member of the struct must begin at the first byte in the struct (offset of 0).
	//So what happens if you have an array of these structs? Remember, arrays are contiguous, each index is located right next to the last and right before the next. This would cause every index
	//of the array of structs after the 0th to be misaligned by 2 * index bytes. While this is not the reason this is done, it's an easy way to visualize the problem. To remedy this the compiler
	//introduces what I call tail padding, and will force the end of the structure to follow it's alignment as well (In this case 8 byte alignment). This leaves us with:
	//
	//|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|	Byte	|
	//-------------------------------------------------------------------------------------------------------------------------------
	//|	char	|					7 Bytes of Padding							|	8 Bytes
	//|								Pointer								|	16 Bytes
	//|	char	|		3 Bytes of padding		|				int				|	24 bytes
	//|		short		|					6 Bytes Padding						|	32 bytes
	//
	//This seems negligable, memory is cheap, a few bytes-- who cares? Any good developer would! Look at this abomination, for every byte you are using you are throwing one away.
	//This poorly created (or ignorantly created) structure uses 2x the amount of memory it needs! Do you think a game like Grand Theft Auto V has that kind of memory to throw away with EVERYTHING
	//the game has going on at one time? No way. It wouldn't run! 
	//
	//Pack your structs, kids.
}
