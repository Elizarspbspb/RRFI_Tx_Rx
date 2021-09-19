#pragma once

//#include <conio.h> // только винда
//#include <ncurses.h> //аналог в линукс
#include <ncurses.h>
#include <vector>
#include <deque>
#include <iostream>
#include <string>
#include <math.h>
#include <map>
#include <complex>
#include <fstream>
//#include <Windows.h>
#include <inttypes.h> //стало в линукс для типов данных

using namespace std;

enum ALUcfg{ acReAddImAdd, acReSubImSub, acReSubImAdd, acReAddImSub };
enum ComplexPart{ cpRe, cpIm };

struct gsl_complex
{
	double dat[2];
};

#define SAFE_DELETE(p)  {if(p) { delete (p);     (p)=NULL; }}
#define SAFE_DELETEA(p) {if(p) { delete [] (p);  (p)=NULL; }}

#define ISDISPLAY 0
#define ISDISPLAYX 0
#define ISLIB 1

#if ISDISPLAY
#define DISPLAY(m) {cout << m ;}
#define CONSOLECOLOR(c) {HANDLE  hStdout = GetStdHandle(STD_OUTPUT_HANDLE); SetConsoleTextAttribute(hStdout, c); }
#define CONSOLESIZE(sx, sy) {HANDLE  hStdout = GetStdHandle(STD_OUTPUT_HANDLE); COORD coor; coor.X = sx; coor.Y = sy; SetConsoleScreenBufferSize(hStdout, coor); }
#else
#define DISPLAY(m)
#define CONSOLECOLOR(c)
#define CONSOLESIZE(sx, sy)
#endif

#if ISDISPLAYX
#define DISPLAYX(m) {cout << m ;}
#else
#define DISPLAYX(m)
#endif


typedef unsigned char byte;
//typedef __int64 int64; //было в винде
typedef uint64_t int64; //стало в линуксе
typedef unsigned long int int32;
typedef unsigned int int16;


