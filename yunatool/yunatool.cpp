// ConsoleApplication3.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#define FIRSTBLOCK  0x0173d000 //yuna 2
//#define FIRSTBLOCK  0x042f9000 //yuna 1
#define BLOCKLENGTH 0x4000
#define OUTPUTFILE "E:/yuna/block_00.bin"
#define OUTPUTTXT "E:/yuna/block_00.txt"
#define OUTPUTTBL "E:/yuna/block_00.tbl"
#define OUTPUTTSV "E:/yuna/block_00.tsv"
#define NUMBEROFBLOCKS 2 //30 //yuna 1
#define READ 0
#define WRITE 1

char data[BLOCKLENGTH]; //one block
char data_buff[BLOCKLENGTH]; //set data buffer
char filename[sizeof(OUTPUTFILE)] = OUTPUTFILE;
char txtname[sizeof(OUTPUTTXT)] = OUTPUTTXT;
char tblname[sizeof(OUTPUTTBL)] = OUTPUTTBL;
char tsvname[sizeof(OUTPUTTSV)] = OUTPUTTSV;
int filepospointer = FIRSTBLOCK;
int out_file_num = 0;
unsigned int i;
unsigned int dataptr;
const int tens_off = sizeof(OUTPUTFILE)-7;
const int ones_off = sizeof(OUTPUTFILE)-6;
const unsigned int command_list[11] = { 0x1016, 0x2016, 0x3016, 0x4016, 0x101a, 0x101f, 0x3011, 0x1004, 0x1005, 0x1022, 0x1001 };
const char out_file_char[11] = "0123456789";
const char hexadecimal[17] = "0123456789ABCDEF";

fstream iso; //input stream for iso
fstream scr; //output stream for scripts
fstream tsv; //tsv output stream

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

void output_rw(bool operation)
{
	if (operation == WRITE){
		//scr.open(tsvname, fstream::out); //open file given by path
		//scr.write();
	}
	else{
		//scr.open(tsvname, fstream::in); //open file given by path
		//scr.read(data, 0);
	}
	//scr.close();
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

void yuna2_tsv_out()
{
	int headptr;
	int endptr;
	int relptr;
	int relctr = 0;

	headptr = (data[0] & 0xFF);
	headptr += (data[1] & 0xFF) << 8;
	headptr += 6;
	endptr = (data[2] & 0xFF);
	endptr += (data[3] & 0xFF) << 8;
	endptr += headptr;

	string tsvstring;
	string::iterator tsviterator;
	tsv.open(tsvname, fstream::out | fstream::binary);

	/*tsvstring = data + headptr;
	while (tsvstring.length() != 0)
	{
		tsv << "," << tsvstring << endl;
		headptr = headptr + tsvstring.length();
		headptr++;
		tsvstring = data + headptr; 
		cout << tsvstring.length() << endl;
	}*/

	/*do{
		relptr = (data[relctr] & 0xFF);
		relctr++;
		relptr += (data[relctr] & 0xFF) << 8;
		relctr++;
		if (relptr < (endptr - headptr)){
			tsvstring = data + relptr + headptr;
			if (tsvstring[0] != 's'){
				tsv << "[" << hex << relptr << "]," << tsvstring << endl;
			} else {
				tsv << "[" << hex << relptr << "]";
			}
		} else {
			tsv << "[" << hex << relptr << "]";
		}
	} while (relctr < headptr);*/

	/*do{
		relptr = (data[relctr] & 0xFF);
		relctr++;
		relptr += (data[relctr] & 0xFF) << 8;
		relctr++;
		if (relptr < (endptr - headptr)){
			tsvstring = data + relptr + headptr;
			if (tsvstring[0] != 's'){
				tsv << "[" << hex << relptr << "]," << tsvstring << endl;
			}
			else {
				tsv << "[" << hex << relptr << "]";
			}
		}
		else {
			tsv << "[" << hex << relptr << "]";
		}
	} while (relctr < headptr);*/

	do{
		relptr = (data[relctr] & 0xFF);
		relctr++;
		relptr += (data[relctr] & 0xFF) << 8;
		relctr++;
		tsv << hex << relptr << '-';
		for (int i = 0; i < (sizeof(command_list) / sizeof(unsigned int)); i++){
			if (relptr == command_list[i]){
				int u = relptr & 0xFF00;
				u = u >> 12;
				relptr = (data[relctr] & 0xFF);
				relctr++;
				relptr += (data[relctr] & 0xFF) << 8;
				relctr++;
				tsv << hex << relptr << '-';
				if (relptr == 0){
					if (i == 6){
						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsv << hex << relptr << '-';

						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsvstring = data + relptr + headptr;
						tsv << "\t" << tsvstring << endl;

						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						relptr -= relctr;
						tsv << hex << relptr << '-';
						break;
					}
					if (i == 10){
						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						relptr -= relctr;
						tsv << hex << relptr << '-';
						break;
					}
					if (i <= 3){
						tsv << "\t";
						i = 0xff;
					} //else tsv << "\a";
					do{
						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsvstring = data + relptr + headptr;
						tsv << tsvstring;
						u--;
					} while (u);
					if (i == 0xff) tsv << endl;
					else tsv << '-';
					break;
				}
			}
		}
	} while (relctr < headptr);

	tsv.close();

	return;
}

void yuna2_tsv_in()
{
	int headptr = 0;
	int endptr;
	int relptr;
	int txtptr = 0;
	int relctr = 0;
	int rel_offset = 0;
	int relptrbuffer[BLOCKLENGTH];
	memset(relptrbuffer, 0, sizeof(relptrbuffer));

	string tsvstring;
	stringstream tsvstringstream;
	string::iterator tsviterator;
	tsv.open(tsvname, ios::in);

	memset(data, 0, sizeof(data)); //make sure data is empty

	tsv.getline(data_buff, unsigned(-1), '-');
	int i = 0;
	while (data_buff[i] != '\0')
	{
		if (data_buff[i] < 0x40){
			headptr = headptr << 4;
			headptr += (data_buff[i] & 0xF);
		}
		else{
			headptr = headptr << 4;
			headptr += (data_buff[i] & 0xF) + 9;
		}
		i++;
	}
	memset(data_buff, 0, sizeof(data_buff));
	relptr = headptr;
	headptr += 6;
	txtptr = 0;
	bool flag = false;
	i = 0;

	while (relctr != headptr){
		i = 0;
		while (data_buff[i] != '\0')
		{
			if (data_buff[i] < 0x40){
				relptr = relptr << 4;
				relptr += (data_buff[i] & 0xF);
			}
			else{
				relptr = relptr << 4;
				relptr += (data_buff[i] & 0xF) + 9;
			}
			i++;
		}
		data[relctr] = (relptr & 0xFF);
		relctr++;
		data[relctr] = (relptr & 0xFF00) >> 8;
		relctr++;
			switch (relptr){
			case 0x4016:
				relptrbuffer[relctr]--;
			case 0x3016:
				relptrbuffer[relctr]--;
			case 0x2016:
				relptrbuffer[relctr]--;
			case 0x1016:
				relctr -= 2; //no new line code yet
				relptr = 0x1016;
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				relptr = 0;	//fully parse next command
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				tsv.ignore(unsigned(-1), '\t'); // read line of text
				tsv.getline(data_buff, unsigned(-1), '\n');

				i = 0; // copy text into data, formatting code may be added here as needed
				while (data_buff[i] != '\0')
				{
					data[i + txtptr + headptr] = data_buff[i];
					i++;
				}
				i++;

				data[relctr] = (txtptr & 0xFF); // generate text pointer
				relctr++;
				data[relctr] = (txtptr & 0xFF00) >> 8;
				relctr++;
				txtptr += i;
				break;
			case 0x1001:
				relptr = 0;	//fully parse next command
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				relptr = 0;	//fully parse next command (i.e. pointer)
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				relptrbuffer[relctr] = relptr; // store pointer in buffer
				relptr += relctr + 2;

				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;
				break;
			case 0x3011:
				relptr = 0;	//fully parse next command
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				relptr = 0;	//fully parse next command
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				tsv.ignore(unsigned(-1), '\t'); // read line of text
				tsv.getline(data_buff, unsigned(-1), '\n');

				i = 0; // copy text into data, formatting code may be added here as needed
				while (data_buff[i] != '\0')
				{
					data[i + txtptr + headptr] = data_buff[i];
					i++;
				}
				i++;

				data[relctr] = (txtptr & 0xFF); // generate text pointer
				relctr++;
				data[relctr] = (txtptr & 0xFF00) >> 8;
				relctr++;
				txtptr += i;

				relptr = 0;	//fully parse next command (i.e. pointer)
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				relptrbuffer[relctr] = relptr; // store pointer in buffer
				relptr += relctr + 2;

				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;
				break;
			case 0x101a:
			case 0x101f:
			case 0x1004:
			case 0x1005:
			case 0x1022:
				relptr = 0;	//fully parse next command
				memset(data_buff, 0, sizeof(data_buff));
				tsv.getline(data_buff, unsigned(-1), '-');
				i = 0;
				while (data_buff[i] != '\0')
				{
					if (data_buff[i] < 0x40){
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF);
					}
					else{
						relptr = relptr << 4;
						relptr += (data_buff[i] & 0xF) + 9;
					}
					i++;
				}
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;

				tsv.getline(data_buff, unsigned(-1), '-');

				i = 0; // copy text into data, formatting code may be added here as needed
				while (data_buff[i] != '\0')
				{
					data[i + txtptr + headptr] = data_buff[i];
					i++;
				}
				i++;

				data[relctr] = (txtptr & 0xFF); // generate text pointer
				relctr++;
				data[relctr] = (txtptr & 0xFF00) >> 8;
				relctr++;
				txtptr += i;
				break;
			}
			relptr = 0;
			memset(data_buff, 0, sizeof(data_buff));
			tsv.getline(data_buff, unsigned(-1), '-');
		}


		relctr = 0;

		do{
			switch (relptrbuffer[relctr])
			{
			default:
				rel_offset = 2;
				while (relptrbuffer[relctr] != 0){
					switch(relptrbuffer[relctr + rel_offset]){
					case -1:
					case -2:
					case -3:	
					case 1:
					case 2:
					case 3:
						if (relptrbuffer[relctr] > 0){
							relptrbuffer[relctr] += (relptrbuffer[relctr + rel_offset]) * 2;
						}
						else
						{
							relptrbuffer[relctr] -= (relptrbuffer[relctr + rel_offset]) * 2;
						}
					default:
						if (relptrbuffer[relctr] > 0){
							relptrbuffer[relctr] -= 2;
							rel_offset += 2;
						}
						else
						{
							relptrbuffer[relctr] += 2;
							rel_offset -= 2;
						}
					}
				}
				data[relctr] = (rel_offset + relctr) & 0xFF;
				data[relctr + 1] = ((rel_offset + relctr) & 0xFF00) >> 8;
			case 0:
			case -1:
			case -2:
			case -3:
				relctr += 2;
				break;
			}
		} while (relctr < headptr);
	/*do{
		switch (relptr){
		case 0x4016:
		case 0x3016:
		case 0x2016:
		case 0x1016:{
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\0');
			tsv >> hex >> relptr;
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\t');
			tsv >> tsvstring;
			txtptr += tsvstring.length();
			tsv.getline((data + txtptr + headptr), unsigned(-1), '\0');
			data[relctr] = (txtptr & 0xFF);
			relctr++;
			data[relctr] = (txtptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\n');
			break;
		}
		case 0x3011:{
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\0');
			tsv >> hex >> relptr;
		}
		case 0x101a:{
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\0');
			tsv >> hex >> relptr;
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			tsv.ignore(unsigned(-1), '\0');
			tsv >> tsvstring;
			txtptr += tsvstring.length();
			tsv.getline((data + txtptr + headptr), unsigned(-1), '\0');
			//tsv >> tsvstring;
			//txtptr += tsvstring.length();
			//(data + txtptr) = tsvstring;
			data[relctr] = (txtptr & 0xFF);
			relctr++;
			data[relctr] = (txtptr & 0xFF00) >> 8;
			relctr++;
			//txtptr += tsvstring.length();
			//tsv.ignore(-1, '\0');
			break;
		}
		default:{
			data[relctr] = (relptr & 0xFF);
			relctr++;
			data[relctr] = (relptr & 0xFF00) >> 8;
			relctr++;
			//getline(tsv, tsvstring, '\t');
			//cout << tsvstring;
			//relptr = tsvstring;
			tsv.ignore(unsigned(-1), '\0');
		}
		}
		tsv >> hex >> relptr;
		} while (relctr != headptr);*/

		/*for (int i = 0; i < (sizeof(command_list) / sizeof(unsigned int)); i++){
			if (relptr == command_list[i]){
				data[relctr] = (relptr & 0xFF);
				relctr++;
				data[relctr] = (relptr & 0xFF00) >> 8;
				relctr++;
				tsv.ignore(unsigned(-1), '\0');
				tsv >> hex >> relptr;
				if (relptr == 0){
					if (i == 7){
						data[relctr] = (relptr & 0xFF);
						relctr++;
						data[relctr] = (relptr & 0xFF00) >> 8;
						relctr++;
						tsv.ignore(unsigned(-1), '\0');
						tsv >> hex >> relptr;
					}
					else {
						data[relctr] = 0;
						relctr++;
						data[relctr] = 0;
						relctr++;
						data[relctr] = (txtptr + headptr) & 0xFF;
						relctr++;
						data[relctr] = (txtptr + headptr) & 0xFF00;
						relctr++;
						tsv >> tsvstring;
						tsv >> (data + txtptr + headptr);
						txtptr += tsvstring.length();
						tsv.ignore(unsigned(-1), '\0');
					}
				}
			}
		}
	} while (relctr < headptr);*/

	/*do{
		relptr = (data[relctr] & 0xFF);
		relctr++;
		relptr += (data[relctr] & 0xFF) << 8;
		relctr++;
		tsv << "[" << hex << relptr << "]";
		for (int i = 0; i < (sizeof(command_list) / sizeof(unsigned int)); i++){
			if (relptr == command_list[i]){
				int u = relptr & 0xFF00;
				u = u >> 12;
				relptr = (data[relctr] & 0xFF);
				relctr++;
				relptr += (data[relctr] & 0xFF) << 8;
				relctr++;
				tsv << "[" << hex << relptr << "]";
				if (relptr == 0){
					if (i == 6){
						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsv << "[" << hex << relptr << "]";

						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsvstring = data + relptr + headptr;
						tsv << "\t" << tsvstring << endl;
						break;
					}
					if (i <= 3){
						tsv << "\t";
						i = 0xff;
					} //else tsv << "\a";
					do{
						relptr = (data[relctr] & 0xFF);
						relctr++;
						relptr += (data[relctr] & 0xFF) << 8;
						relctr++;
						tsvstring = data + relptr + headptr;
						tsv << tsvstring;
						u--;
					} while (u);
					if (i == 0xff) tsv << endl;
					break;
				}
			}
		}
	} while (relctr < headptr);*/

	tsv.close();

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
	tsvname[tens_off] = out_file_char[out_file_num / 10];
	tsvname[ones_off] = out_file_char[out_file_num % 10];
	return;
}

int main(int argc, char* argv[])
{
	memset(data, 0, sizeof(data)); //make sure data is empty
	switch (argc)
	{
		case 2:
		{
			cout << "ENTER;" << endl << 
				"1 TO DUMP ISO TO BINARY SCRIPT" << endl << 
				"2 YUNA1 TABLE/TXT FILES FROM BINARY" << endl << 
				"3 YUNA1 REGENERATE DAT FILES" << endl << 
				"4 TO REINSERT SCRIPTS" << endl <<
				"5 YUNA2 CSV SCRIPT EXTRACTOR" << endl <<
				"6 YUNA2 DAT REGENERATOR" << endl;
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
			case ('5') : //insert
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					output_bin_rw(READ);
					yuna2_tsv_out();
					output_rw(WRITE);
					output_file_inc();
				}
				break;
			}
			case ('6') : //insert
			{
				while (out_file_num != NUMBEROFBLOCKS)
				{
					output_rw(READ);
					yuna2_tsv_in();
					output_bin_rw(WRITE);
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
