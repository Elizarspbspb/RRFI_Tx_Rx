#ifndef RSWRAPPER_H
#define RSWRAPPER_H

//extern "C" {

typedef struct rs_handle
{
  void* RS_handle;
  void* GF_handle;
	int *B;
	int *T;
	int iN;
	int iK;
	int iExt;
	int iMinDist;
} RSHANDLE;

extern RSHANDLE RS_handle;

RSHANDLE createRSCode( int iExt, int iMinDist, int iNShortened, int iOffset);

void destroyRSCode( RSHANDLE handle );

void RSEncode( RSHANDLE handle, 
							 const unsigned char *pMessage,
							 unsigned char *pCodeword );

void RSDecode( RSHANDLE handle,
							 unsigned char *pVector, 
							 unsigned char *pMessage,
							 int iNumOfErasures, 
							 int *pErasureLocations );

//}

#endif //RSWRAPPER_H
