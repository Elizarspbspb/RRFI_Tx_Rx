#include "golay_wrap.h"
#include "Golay.h"

GOLAYHANDLE golay_handle;

GOLAYHANDLE createGolayCode(void) {
	GOLAYHANDLE gh;
	CGolayCodec *GCode = new CGolayCodec(); //error
	gh.GOLAY_handle = (void*) GCode;
	return gh;
}

void destroyGolayCode( GOLAYHANDLE handle ) {
	delete (CGolayCodec*)handle.GOLAY_handle;
}

void GolayEncode( GOLAYHANDLE handle, 
							 const unsigned char *pMessage,
							 unsigned char *pCodeword ) {
	((CGolayCodec*)handle.GOLAY_handle)->encode((char*) pMessage, (char*) pCodeword);
							 
}


void GolayDecode( GOLAYHANDLE handle,
							 int *pVector, 
							 unsigned char *pMessage ) {
	double dVector[ ((CGolayCodec*)handle.GOLAY_handle)->LENGTHCWORD ];
	int i;
	for(i=0;i<((CGolayCodec*)handle.GOLAY_handle)->LENGTHCWORD; i++) dVector[i]=pVector[i];
	((CGolayCodec*)handle.GOLAY_handle)->decode(dVector,(char*)pMessage);
}