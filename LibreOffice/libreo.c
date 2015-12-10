/*
// Compiles with TCC compiler http://bellard.org/tcc/
./tcc libreo.c

Uku-Kaarel Jõesaar
ukjoesaar@gmail.com
v1.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _UNICODE
#include <windows.h>

//#include <tchar.h>
//#include <wchar.h>

char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}


int main(int argc, char **argv)
{
 char *s0 = "libreo.cmd";
 char *s1 = " ";
 char *s2 = "\"";
 char *s;
 int c, n;
    UINT oldCodePage;
    char buf[1024];

    oldCodePage = GetConsoleOutputCP();
    if (!SetConsoleOutputCP(65001)) {
        printf("error\n");
    }

	
 /* wchar_t buf[1024];
 HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); */
 
 s = concat(9,s0,s1,s2,argv[1],s2,s1,s2,argv[2],s2); 
 
 /* n = MultiByteToWideChar(CP_UTF8, 0,
            lpcsTest, strlen(lpcsTest),
            buf, sizeof(buf));

			
 WriteConsole(hConsole, buf, n, &n, NULL); */
 // printf("%s", s);
 
 
 
 system(s);
 free(s);//deallocate the string
 SetConsoleOutputCP(oldCodePage);
  return 0;
}