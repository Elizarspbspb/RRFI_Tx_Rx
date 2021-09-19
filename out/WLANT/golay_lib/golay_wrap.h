#ifndef GOLAYWRAPPER_H
#define GOLAYWRAPPER_H

//extern "C" {      //В оригинале extern "C"

typedef struct golay_handle {
  void* GOLAY_handle;
} GOLAYHANDLE;

extern GOLAYHANDLE golay_handle;

GOLAYHANDLE createGolayCode(void); //error

void destroyGolayCode( GOLAYHANDLE handle );

void GolayEncode( GOLAYHANDLE handle, 
							 const unsigned char *pMessage,
							 unsigned char *pCodeword );

void GolayDecode( GOLAYHANDLE handle,
							 int *pVector, 
							 unsigned char *pMessage );

//}

#endif //GOLAYWRAPPER_H
