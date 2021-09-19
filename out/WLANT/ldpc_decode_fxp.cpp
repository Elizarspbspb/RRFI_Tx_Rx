/** LDPC decoders implementation file
 *
 * @authors Andrey Efimov
 * @organization: LCA, ISS Dept, SUAI
 * $Revision: $
 * $Date: $
 */

#include "ldpc_code.h"
#include "ldpc_decode_fxp.h"
#include "commonfunc.h"
#include "model.h"

#include <memory.h>
#include <stdio.h>

extern ldpc_code ldpc;

// --------------------------------------------------------------------------

/**
 * Decodes vector
 * @param pVector pointer to vector received, size [LDPCCodewordLength]
 * @param pMessage pointer to output message, size [LDPCMessageLength]
 * @param pCodeword pointer to output codeword, size [LDPCCodewordLength]
 * @param pReliabilities pointer to output reliabilitites vector, size [LDPCCodewordLength]
 * @return 
 *  0  no error,
 *  -1 unrecoverable error
 */
int ldpc_decode_fxp(int *pVector, char *pMessage, char *pCodeword, int *pReliabilities, int precision) {
  int i, rt;

  #ifdef LOGGING
  logging( "ldpc_decode_fxp...\n", 1 );
  #endif

  //[ decode vector
#if (DECODER == LDPC_DECODER_MTH)
  rt = ldpc_correct_errors_mth_fxp(pVector, pCodeword, pReliabilities, precision);
#elif (DECODER == LDPC_DECODER_SFF)
  rt = ldpc_correct_errors_sff_fxp(pVector, pCodeword, pReliabilities, precision);
#else
#pragma message("Unknown decoder type")  
#endif
  //]

  //[ extract message
  for (i = 0; i < LDPCMessageLength; i++) {
    pMessage[i] = pCodeword[ldpc.MessageSet[i]];
  }
  //]

  #ifdef LOGGING
  logging( "ldpc_decode_fxp finished\n", -1 );
  #endif

  return rt;
}

// --------------------------------------------------------------------------

/**
 * Decodes input vector using multithreshold parity check reliabilities voting with fixed-point values
 * @param pVector pointer to vector received, size [LDPCCodewordLength]
 * @param pCodeword pointer to output codeword , size [LDPCCodewordLength]
 * @param pReliabilities pointer to output reliabilities vector, size [LDPCCodewordLength]
 * @return 
 *  0  no error,
 *  -1 unrecoverable error
 */
int ldpc_correct_errors_mth_fxp(int *pVector, char *pCodeword, int *pReliabilities, int precision) {
  int i,ii,j,m,iter;
  char localS, chOverFlow;
  int sum ,Z_n, minZ_n;

  char X_hard[LDPCCodewordLength];
  int R_n[LDPCCodewordLength];

  char X_mn[LDPCCodewordLength * MNOC];
  char Sigma_mn[LDPCCodewordLength * MNOC];
  int Y_mn[LDPCCodewordLength * MNOC];
  int Y_mn_min[LDPCCodewordLength * MNOC];
  
  int Thresholds[NUM_OF_ITERATIONS];
  
    //[ initialization
  memset(Y_mn, 0, sizeof(int) * LDPCCodewordLength*MNOC);
  memset(X_mn, 0, LDPCCodewordLength*MNOC);
  for(i=0;i<LDPCCodewordLength;i++) {
    if (pVector[i] >= 0) {
      X_hard[i] = 1;
      R_n[i] = pVector[i];
    }
    else {
      X_hard[i] = 0;

      #ifdef LOGGING
      sprintf( overflow_comment,
      "Overflow:  R_n[i] = fxp_not\n" );
      #endif

      R_n[i] = fxp_not(pVector[i], precision, &chOverFlow, overflow_comment);
    }
    
    for(m = ldpc.ColsLists[i][j=0]; m != -1; m = ldpc.ColsLists[i][++j]) {
      X_mn[j*LDPCCodewordLength+i]=X_hard[i];
      Y_mn[j*LDPCCodewordLength+i]=R_n[i];
    }
  }
  //] 

  iter = 0;
  
  do {
    //[ one decoding iteration
    iter++;
    
    //[ parity node updates
    memset(Sigma_mn, 0, LDPCCodewordLength*MNOC);
    for(j=0;j<NC;j++) {
      int min1, min2;
      int min1Index=-1;

      SET_FXP_MAX(min1, precision);
      SET_FXP_MAX(min2, precision);
      
      localS = 0;

      for(i = ldpc.RowsLists[j][ii = 0]; i != -1; i = ldpc.RowsLists[j][++ii]) {
        
        localS ^= X_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
        if(Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]<min1){
          min2=min1; 
          min1=Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
          min1Index=i;
         } else if(Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]<min2){
          min2=Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
        }
        
      }
      

      for(i = ldpc.RowsLists[j][ii = 0]; i != -1; i = ldpc.RowsLists[j][++ii]) {
        if(i==min1Index){
          Y_mn_min[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]=min2;
        } else {
          Y_mn_min[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]=min1;
        }
        Sigma_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i] = localS ^ X_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i] ^ X_hard[i];
      }

    }
    //]
    
    if(iter == 1) {
      //[ set up of thresholds

      minZ_n = 0;
      // find minimum for thresholds
      for (j = 0; j < LDPCCodewordLength; j++) {
        Z_n=R_n[j];
        for(m = ldpc.ColsLists[j][i = 0]; m != -1; m = ldpc.ColsLists[j][++i]) {
          #ifdef LOGGING
          sprintf( overflow_comment,
          "Overflow: Z_n = fxp_sub\n" );
          #endif

          if (Sigma_mn[i*LDPCCodewordLength + j]) 
            Z_n = fxp_sub(Z_n, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
          else
            Z_n = fxp_add(Z_n, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);

          //Z_n += Y_mn_min[i*LDPCCodewordLength+j] * (Sigma_mn[i*LDPCCodewordLength + j]? -1:1) ;
        }
        if(Z_n<minZ_n) {
          minZ_n = Z_n;
        }
      }
   
      // set up thresholds values
      memset(Thresholds, 0, sizeof(int) * NUM_OF_ITERATIONS);
      for(i=0; i <= NUM_OF_ITERATIONS / 2; i++) 
        Thresholds[i] = minZ_n;
      //]
    }

    //[ bit node updates
    for (j = 0; j < LDPCCodewordLength; j++) {
      Z_n=sum=R_n[j];
      for(m = ldpc.ColsLists[j][i = 0]; m != -1; m = ldpc.ColsLists[j][++i]) {
        #ifdef LOGGING
        sprintf( overflow_comment,
        "Overflow: sum = fxp_sub(\n" );
        #endif

        if (Sigma_mn[i*LDPCCodewordLength + j]) 
            sum = fxp_sub(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
        else
            sum = fxp_add(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);

        //sum+= Y_mn_min[i*LDPCCodewordLength+j] * (Sigma_mn[i*LDPCCodewordLength + j] ? -1:1);
      }
      Z_n=sum;
      
      for(m = ldpc.ColsLists[j][i = 0]; m != -1; m = ldpc.ColsLists[j][++i]) {
        int s = sum;  //??
        #ifdef LOGGING
        sprintf( overflow_comment,
        "Overflow: s = fxp_add\n" );
        #endif

        if (Sigma_mn[i*LDPCCodewordLength + j]) 
          s = fxp_add(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
        else
          s = fxp_sub(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);

        //double s=sum-(Y_mn_min[i*LDPCCodewordLength+j] * (Sigma_mn[i*LDPCCodewordLength + j] ? -1:1) );
  

        if (s > 0) {
          Y_mn[i*LDPCCodewordLength+j] = s;
          X_mn[i*LDPCCodewordLength+j] = X_hard[j];
        }
        else {
          if(s>Thresholds[iter-1]) {
            Y_mn[i*LDPCCodewordLength+j] = 0;
            X_mn[i*LDPCCodewordLength+j] = X_hard[j];
          }
          else {
            Y_mn[i*LDPCCodewordLength+j] = fxp_not(s, precision, &chOverFlow, overflow_comment);


            X_mn[i*LDPCCodewordLength+j] = X_hard[j] ^ 1;
          }
        }
      }
      
      // make codeword
      pCodeword[j] = (pReliabilities[j] = Z_n) > 0 ? X_hard[j] : X_hard[j]^1;
    }
    //]

    //[ test codeword
    if(ldpc_is_codeword(pCodeword)) {
      return 0;
    }
    //]
    
    //]
  } while (iter < NUM_OF_ITERATIONS);
  
  return -1;
}

// --------------------------------------------------------------------------

/**
 * Decodes input vector using SFF (min-sum) decoder with fixed-point values
 * @param pVector pointer to vector received, size [LDPCCodewordLength]
 * @param pCodeword pointer to output codeword , size [LDPCCodewordLength]
 * @param pReliabilities pointer to output reliabilities vector, size [LDPCCodewordLength]
 * @return 
 *  0  no error,
 *  -1 unrecoverable error
 */
int ldpc_correct_errors_sff_fxp(int *pVector, char *pCodeword, int *pReliabilities, int precision) {
  int i,ii,j,m,iter;
  char localS;
  int sum, Z_n;
  char chOverFlow = 0;

  char X_hard[LDPCCodewordLength];
  int R_n[LDPCCodewordLength];

  char X_mn[LDPCCodewordLength * MNOC];
  char Sigma_mn[LDPCCodewordLength * MNOC];
  int Y_mn[LDPCCodewordLength * MNOC];
  int Y_mn_min[LDPCCodewordLength * MNOC];
  
  //[ initialization
  memset(Y_mn, 0, sizeof(int) * LDPCCodewordLength*MNOC);
  memset(X_mn, 0, LDPCCodewordLength*MNOC);
  for(i=0;i<LDPCCodewordLength;i++) {
    if (pVector[i] >= 0) {
      X_hard[i] = 1;
      R_n[i] = pVector[i];
    }
    else {
      X_hard[i] = 0;

      #ifdef LOGGING
      sprintf( overflow_comment,
      "Overflow: R_n[i] = fxp_not\n" );
      #endif

      R_n[i] = fxp_not(pVector[i], precision, &chOverFlow, overflow_comment);
    }
    
    for(m = ldpc.ColsLists[i][j=0]; m != -1; m = ldpc.ColsLists[i][++j]) {
      X_mn[j*LDPCCodewordLength+i]=X_hard[i];
      Y_mn[j*LDPCCodewordLength+i]=R_n[i];
    }
  }
  //] 
  
  iter = 0;
  do {
    //[ one decoding iteration
    iter++;
    
    //[ parity node updates
    memset(Sigma_mn, 0, LDPCCodewordLength*MNOC);
    for(j=0;j<NC;j++) {
      int min1, min2;
      int min1Index=-1;

      SET_FXP_MAX(min1, precision);
      SET_FXP_MAX(min2, precision);
  
      
      localS = 0;
      for(i = ldpc.RowsLists[j][ii = 0]; i != -1; i = ldpc.RowsLists[j][++ii]) {
        
        localS ^= X_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
        if(Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]<=min1){
          min2=min1; 
          min1=Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
          min1Index=i;
        } else if(Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]<=min2){
          min2=Y_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i];
        }
        
      }
      for(i = ldpc.RowsLists[j][ii = 0]; i != -1; i = ldpc.RowsLists[j][++ii]) {
        if(i==min1Index){
          Y_mn_min[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]=min2;
        } else {
          Y_mn_min[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i]=min1;
        }
        Sigma_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i] = localS ^ X_mn[ldpc.ColsListsRev[j][ii]*LDPCCodewordLength+i] ^ X_hard[i];
      }

    }
    //]

    //[ bit nodes updates
    for (j = 0; j < LDPCCodewordLength; j++) {
      Z_n=sum=R_n[j];
      for(m = ldpc.ColsLists[j][i = 0]; m != -1; m = ldpc.ColsLists[j][++i]) {
        #ifdef LOGGING
        sprintf( overflow_comment,
        "Overflow: sum = fxp_sub\n" );
        #endif

        if (Sigma_mn[i*LDPCCodewordLength + j]) 
          sum = fxp_sub(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
        else
          sum = fxp_add(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
      }
      Z_n=sum;
      
      for(m = ldpc.ColsLists[j][i = 0]; m != -1; m = ldpc.ColsLists[j][++i]) {
        int s = sum;
        #ifdef LOGGING
        sprintf( overflow_comment,
        "Overflow: s = fxp_add\n" );
        #endif

        if (Sigma_mn[i*LDPCCodewordLength + j]) 
          s = fxp_add(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
        else
          s = fxp_sub(sum, Y_mn_min[i*LDPCCodewordLength+j], precision, &chOverFlow, overflow_comment);
        
        //double s=sum-(Y_mn_min[i*LDPCCodewordLength+j] * (Sigma_mn[i*LDPCCodewordLength + j] ? -1:1) );
  

        if(s>0) {
          Y_mn[i*LDPCCodewordLength+j] = s;
          X_mn[i*LDPCCodewordLength+j] = X_hard[j];
        }
        else {
          #ifdef LOGGING
          sprintf( overflow_comment,
          "Overflow: Y_mn[i*LDPCCodewordLength+j] = fxp_not\n" );
          #endif


          Y_mn[i*LDPCCodewordLength+j] = fxp_not(s, precision, &chOverFlow, overflow_comment);
          X_mn[i*LDPCCodewordLength+j] = X_hard[j] ^ 1;
        }
      }
      
      // make codeword
      pCodeword[j] = (pReliabilities[j] = Z_n) > 0 ? X_hard[j] : X_hard[j]^1;
    }
    //]

    //[ test codeword
    if(ldpc_is_codeword(pCodeword)) {
      return 0;
    }
    //]
    
    //]
  } while (iter < NUM_OF_ITERATIONS);
  
  return -1;
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

/**
 * Tests vector to be codeword
 * @param pVector pointer to vector will be tested, size [LDPCCodewordLength]
 * @return 
 *  1 : vector is codeword,
 *  0 : otherwise
 */
int ldpc_is_codeword(char *pVector) {
  int i, j;
  unsigned int syndrome_item;

  for (i = 0; i < NCE; i++) {
    syndrome_item = 0;
    for (j = 0; j < LDPCCodewordLength; j++) {
      if (pVector[j]) {
        syndrome_item ^= ldpc.ElHComp[i * LDPCCodewordLength + j];
      }
    }
    if (syndrome_item > 0) {
      return 0;
    }
  }
  return 1;
}

// --------------------------------------------------------------------------


