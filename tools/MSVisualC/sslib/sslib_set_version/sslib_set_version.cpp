/*
** This program creates the file ssvers.h for the Windows build.
**
** On unix, this is done much more easily in a Makefile
*/
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <string.h>

int main()
{
	ifstream *l_input=new ifstream( "..\\..\\..\\..\\src\\safapi\\lib\\saf.h" );
	if(!l_input->is_open())
	{
		cout<<endl<<"Error opening saf.h: sslib_set_version.cpp path is wrong"<<endl<<endl;
		return(-1);
	}

	ofstream *l_output=new ofstream( "..\\..\\ssvers.h" );
	if(!l_output->is_open())
	{
		cout<<endl<<"Error opening ssvers.h: sslib_set_version.cpp path is wrong"<<endl<<endl;
		return(-1);
	}

	int l_numStringsToGet=4;
	char *l_stringsToGet[4] = {
			"#define SAF_VERSION_MAJOR",
			"#define SAF_VERSION_MINOR",
			"#define SAF_VERSION_RELEASE",
			"#define SAF_VERSION_ANNOT"
	};

#define MAX_STR_LEN 1024
	char l_str[MAX_STR_LEN];

	for( int i=0; i<l_numStringsToGet; i++ )
	{
		//cout<<"Doing string: "<<i<<endl;

		bool l_done=false;

		while(!l_done && !l_input->eof())
		{
			l_input->getline( l_str, MAX_STR_LEN-1, '\n' );

			if(!strncmp(l_str,l_stringsToGet[i],strlen(l_stringsToGet[i])))
			{
				//replace the SAF_VERSION with SS_VERS
				l_str[8]=' ';
				l_str[9]=' ';
				l_str[10]=' ';
				l_str[11]=' ';
				l_str[12]='S';
				l_str[13]='S';
				l_str[14]='_';
				l_str[15]='V';
				l_str[16]='E';
				l_str[17]='R';
				l_str[18]='S';

				l_output->write(l_str,strlen(l_str));
				l_output->put('\n');
				l_done=true;

				//cout<<"Wrote string: "<<l_str<<endl;
			}
		}

		l_input->seekg(0,ios::beg);//rewind to begin of file
	}

	l_input->close();
	delete l_input;
	l_output->close();
	delete l_output;
	return(0);
}