#include "RS_wrapper.h"
#include "CyclicCode.h"
#include "GaloisFieldInterface.h"

RSHANDLE createRSCode( int iExt, int iMinDist, int iNShortened, int iOffset )
{
	RSHANDLE rs;
  CCyclicCode* bc = new CCyclicCode;
  IGaloisField *pGF = createGF2s( iExt );

	rs.RS_handle = (void*)bc;
	rs.GF_handle = (void*)pGF;
	rs.iExt = iExt;

  bc->initRSCode( pGF, iMinDist, iNShortened, iOffset ); 
	rs.iN = bc->getCodewordLength();
	rs.iK = bc->getMessageLength();
	rs.iMinDist = bc->getMinDistance();

	rs.B = new int[rs.iN];
	rs.T = new int[rs.iN];

	return rs;
}

void destroyRSCode( RSHANDLE handle ) 
{
  delete (CCyclicCode*)handle.RS_handle;
	delete (IGaloisField*)handle.GF_handle;
	delete[] handle.B;
	delete[] handle.T;
}

void RSEncode( RSHANDLE handle, 
							 const unsigned char *pMessage,
							 unsigned char *pCodeword )
{
	((CCyclicCode*)handle.RS_handle)->encode( pMessage, pCodeword );
}

void RSDecode( RSHANDLE handle,
							 unsigned char *pVector, 
							 unsigned char *pMessage,
							 int iNumOfErasures, 
							 int *pErasureLocations ) 
{
  for ( int i = 0; i < handle.iN; i++) 
	{
    handle.B[i] = 0;
    for ( int j = 0; j < ((IGaloisField*)handle.GF_handle)->getExtension(); j++) 
		{
      handle.B[i] |= (int)pVector[i * ((IGaloisField*)handle.GF_handle)->getExtension() + j] << j;
    }
  }

	((CCyclicCode*)handle.RS_handle)->decode( handle.B, handle.T, 0, iNumOfErasures, pErasureLocations );

  for ( int i = 0; i < handle.iK; i++) {
    for ( int j = 0; j < ((IGaloisField*)handle.GF_handle)->getExtension(); j++) 
		{
      pMessage[i * ((IGaloisField*)handle.GF_handle)->getExtension() + j] = 
				(handle.T[i] >> j) & 1;
    }
  }
}