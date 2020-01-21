#include <stdio.h>
#include <unistd.h> //for the getopt function
#include <stdlib.h> //for atoi
#include <iostream>
using namespace std;

bool parseParams(int argc, char *argv[ ], int& cache_capacity, int& cache_blocksize, int& cache_associativity, int& cache_spilt, int& write_back, int& write_through, int &write_allocate, int &write_no_allocate)
{
  //needed for the parsing of command line options
  int c;
  bool c_flag, b_flag, a_flag, s_flag, wb_flag, wt_flag, wa_flag, wn_flag ;
  bool errflg = false;

  c_flag = b_flag = a_flag = s_flag = wb_flag = wt_flag = wa_flag = wn_flag = errflg = false;

  //the following variables are used by getopt and
  //are defined elsewhere so we just make them
  //extern here
  extern char *optarg;
  extern int optind, optopt;

  //start the parsing of the command line options.
  //end is indicated by getopt returning -1
  //each option has a case statement
  //the corresponding flags are set if the option exists on
  //the command line
  //the : int he getopt indicates that the option preceeding the
  //: requires a argument to be specified
  while ((c = getopt(argc, argv, "c:b:a:sw:w:w:w:")) != -1) 
	{
		switch (c) 
		{
			case 'c':
			cache_capacity = atoi(optarg);
			c_flag = true;
			break;
			case 'b':
			cache_blocksize = atoi(optarg);
			b_flag = true;
			break;
			case 'a':
			cache_associativity = atoi(optarg);
			a_flag = true;
			break;
			case 's':
			cache_spilt = 1;
			s_flag = true;
			break;
			case 'w':
			//char *a = optarg;
			if (optarg[0] == 'b')
			{
				write_back = 1;
				wb_flag = true;
			}
			else if (optarg[0] == 't')
			{
				write_through = 1;
				wt_flag = true;
			}
			else if (optarg[0] == 'a')
			{
				write_allocate = 1;
				wa_flag = true;
			}
			else if (optarg[0] == 'n')
			{
				write_no_allocate = 1;
				wn_flag = true;
			}
			break;
			case ':':       /* -c without operand */
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			errflg++;
			break;
			case '?':
			fprintf(stderr, "Unrecognised option: -%c\n", optopt);
			errflg=true;
		}
  }

  //check if we have all the options and have no illegal options
  if(errflg || !c_flag || !b_flag || !a_flag) 
	{
		fprintf(stderr, "usage: %s -c<capacity> -b<blocksize> -a<associativity>\n", argv[0]);
		return false;
  }
	if(wb_flag && wt_flag)
	{
		cout<<"Write back and write through cannot be used at the same time."<<endl;
		return false;
	}
	if(wa_flag && wn_flag)
	{
		cout<<"Write allocate and write no-allocate cannot be used at the same time."<<endl;
		return false;
	}

  return true;
}