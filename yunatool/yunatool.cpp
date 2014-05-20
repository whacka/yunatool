// ConsoleApplication3.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

//#define FIRSTBLOCK  0x0173d000 //yuna 2
#define FIRSTBLOCK  0x042f9000 //yuna 1
#define BLOCKLENGTH 0x4000
#define OUTPUTFILE "E:/yuna/block_00.bin"
#define OUTPUTTXT "E:/yuna/block_00.txt"
#define OUTPUTTBL "E:/yuna/block_00.tbl"
#define NUMBEROFBLOCKS 30
#define READ 0
#define WRITE 1

char data[BLOCKLENGTH]; //one block
char data_buff[BLOCKLENGTH]; //set data buffer
char filename[sizeof(OUTPUTFILE)] = OUTPUTFILE;
char txtname[sizeof(OUTPUTTXT)] = OUTPUTTXT;
char tblname[sizeof(OUTPUTTBL)] = OUTPUTTBL;
int filepospointer = FIRSTBLOCK;
int out_file_num = 0;
unsigned int i;
unsigned int dataptr;
const int tens_off = sizeof(OUTPUTFILE)-7;
const int ones_off = sizeof(OUTPUTFILE)-6;
const char out_file_char[11] = "0123456789";

fstream iso; //input stream for iso
fstream scr; //output stream for scripts

void block_rw(bool operation, char* path)
{
	if (operation == WRITE){
		iso.open(path, fstream::binary|fstream::out|fstream::in); //open file given by path
		iso.seekp((streampos)filepospointer);
		iso.write(data, BLOCKLENGTH);
	}
	else{
		iso.open(path, fstream::binary | fstream::in); //open file given by path
		iso.seekg((streampos)filepospointer);
		iso.read(data, BLOCKLENGTH);
	}
	filepospointer += BLOCKLENGTH;
	iso.close();
	return;
}

void output_bin_rw(bool operation)
{
	if (operation == WRITE){
		scr.open(filename, fstream::binary | fstream::out); //open file given by path
		scr.write(data, BLOCKLENGTH);
	}
	else{
		scr.open(filename, fstream::binary | fstream::in); //open file given by path
		scr.read(data, BLOCKLENGTH);
	}
	scr.close();
	cout << filename << endl;
	return;
}

void output_tbl_rw(bool operation, int length)
{
	if (operation == WRITE){
		scr.open(tblname, fstream::binary | fstream::out); //open file given by path
		scr.write(data, length);
	}
	else{
		scr.open(tblname, fstream::binary | fstream::in); //open file given by path
		scr.read(data, length);
	}
	scr.close();
	cout << tblname << endl;
	return;
}

void output_txt_rw(bool operation, int length)
{
	if (operation == WRITE){
		scr.open(txtname, fstream::binary | fstream::out); //open file given by path
		scr.write(data, length);
	}
	else{
		scr.open(txtname, fstream::binary | fstream::in); //open file given by path
		scr.read(data, length);
	}
	scr.close();
	cout << txtname << endl;
	return;
}

void find_script_head(){
	dataptr = ((unsigned int)data_buff[1]) << 8;
	i = 0xFF & (unsigned int)data_buff[0];
	dataptr += i;
	return;
}

void find_ptr_tbl_head(){
	i = 0;
	while (++i) // find first "pointer value 0x000?"
	{
		if (data_buff[i + dataptr + 6] == '\0'){
			if (data_buff[i + dataptr + 7] == '\0'){
				if (data_buff[i + dataptr + 8] == '\0') break;
			}
		}
	}
	dataptr = dataptr + 7 + i; //now points to head of ptr table
	return;
}

void bin_2txt()
{
	memset(data_buff, 0, sizeof(data_buff)); //make sure data buffer is empty
	copy(data + 0, data + sizeof(data), data_buff); //copy data to buffer

	find_script_head();
	cout << hex << dataptr << endl; //now points to head of script

	find_ptr_tbl_head();

	//extract pointer table into int array
	unsigned int txtptr = 0; //offset pointer
	i = 0;
	while (txtptr < 0x4000)
	{
		txtptr = (0xFF & ((unsigned int)data[dataptr + i + 1])) << 8;
		txtptr += 0xFF & (unsigned int)data[dataptr + i];
		i += 2;
	}

	//write tble
	copy(data_buff + dataptr, data_buff + sizeof(data_buff), data);
	output_tbl_rw(WRITE, i - 2);

	//insert control codes into text
	dataptr = dataptr + i - 2; //now points to head of text
	i = 0;
	unsigned int n = 0; //offset for control codes
	while (1)
	{
		if (data_buff[dataptr + i] == '\0'){
			if (data_buff[dataptr + i + 1] == '\0'){
				if (data_buff[dataptr + i + 2] == '\0') break;
			}
		}
		if (data_buff[dataptr + i] == '\0'){
			data[i + n] = '\n';
		}
		else if (data_buff[dataptr + i] == 0x0A){
			data[i + n] = '/';
		}
		else {
			data[i + n] = data_buff[dataptr + i];
		}
		i++;
	}

	output_txt_rw(WRITE, i);
	return;
}

void txt_2bin()
{
	output_bin_rw(READ);
	memset(data_buff, 0, sizeof(data_buff)); //make sure data buffer is empty
	copy(data + 0, data + sizeof(data), data_buff); //copy data to buffer
	memset(data, 0, sizeof(data)); //make sure data is empty
	find_script_head();
	find_ptr_tbl_head();

	output_txt_rw(READ, BLOCKLENGTH);
	i = 0;
	unsigned int table_length = 2;
	unsigned int text_length = 0;
	unsigned int table[BLOCKLENGTH];
	memset(table, 0, sizeof(table)); //make sure table is empty
	unsigned char current_char;
	memset(data_buff, 0, sizeof(data_buff)); //make sure data buffer is empty
	copy(data + 0, data + sizeof(data), data_buff); //copy data to buffer
	memset(data, 0, sizeof(data)); //make sure data is empty
	
	while (1) //find table length and write
	{
		current_char = data_buff[i];
		//cout << hex << (unsigned int)current_char << endl;
		if (current_char == '\0') break;
		/*if (current_char == '\0'){	// possibly better code? may break
			if (data_buff[i + 1] == '\0'){
				if (data_buff[i + 2] == '\0') break;
			}
		} //end of file*/
		else if (current_char == '\n'){ //line end
			data[i] = '\0';
			table[table_length] = (i + 1) & 0xFF;
			table[table_length + 1] = (i + 1) >> 8;
			table_length+=2;
		}
		else if (current_char == '/'){ //new line
			data[i] = 0x0A;
		}
		else{
			data[i] = current_char;
		}
		i++;
	}
	text_length = i;
	output_txt_rw(WRITE, text_length); //write new table
	i = 0;
	while (i < table_length){
		data[i] = (unsigned char)table[i];
		i++;
	}
	output_tbl_rw(WRITE, table_length);

	output_bin_rw(READ);
	copy(data + 0, data + sizeof(data), data_buff); //copy data to buffer
	output_tbl_rw(READ, table_length);
	copy(data + 0, data + (table_length), data_buff + dataptr); //copy table to buffer
	output_txt_rw(READ, text_length);
	copy(data + 0, data + text_length, data_buff + (dataptr + table_length)); //copy text to buffer
	copy(data_buff + 0, data_buff + sizeof(data_buff), data); //copy buffer to data
	output_bin_rw(WRITE);
	return;
}
void output_file_inc()
{
	out_file_num++;
	filename[tens_off] = out_file_char[out_file_num / 10];
	filename[ones_off] = out_file_char[out_file_num % 10];
	txtname[tens_off] = out_file_char[out_file_num / 10];
	txtname[ones_off] = out_file_char[out_file_num % 10];
	tblname[tens_off] = out_file_char[out_file_num / 10];
	tblname[ones_off] = out_file_char[out_file_num % 10];
	return;
}

int main(int argc, char* argv[])
{
	memset(data, 0, sizeof(data)); //make sure data is empty
	switch (argc)
	{
		case 2:
		{
			cout << "ENTER;" << endl << "1 TO DUMP ISO TO BINARY SCRIPT" << endl << "2 TO GENERATE TABLE/TXT FILES FROM BINARY" << endl << "3 TO REGENERATE DAT FILES" << endl << "4 TO REINSERT SCRIPTS" << endl;
			char input;
			cin >> input;
			switch (input)
			{
			case ('1') : //dump
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					block_rw(READ, argv[1]);
					output_bin_rw(WRITE);
					output_file_inc();
				}
				break;
			}
			case ('2') : //bin2txt
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					output_bin_rw(READ);
					bin_2txt();
					output_file_inc();
				}
				break;
			}
			case ('3') : //txt2bin
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					txt_2bin();
					output_file_inc();
				}
				break;
			}
			case ('4') : //insert
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					output_bin_rw(READ);
					block_rw(WRITE, argv[1]);
					output_file_inc();
				}
				break;
			}
			default:
			{
				cout << "ERROR READING INPUT" << endl;
			}
			}
			break;
		}
		default:
		{
			cout << "NO PATH TO ISO";
		}
	}
	return 0;
}
