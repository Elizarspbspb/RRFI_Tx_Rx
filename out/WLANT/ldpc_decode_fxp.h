/** LDPC decoders header file
 *
 * @authors Andrey Efimov
 * @organization: LCA, ISS Dept, SUAI
 * $Revision: 1 $
 * $Date: 21.10.04 13:31 $
 */

# ifndef   LDPCDECODE_FXP_H
#   define LDPCDECODE_FXP_H

#include "ldpc_code.h"

// Decoder types
#define LDPC_DECODER_MTH  0x01            ///< multi-threshold decoder
#define LDPC_DECODER_SFF  0x02            ///< min-sum decoder

#define DECODER   LDPC_DECODER_SFF        ///< used decoder

#define NUM_OF_ITERATIONS 30            ///< maximum number of decoder iterations

// Decodes vector
int ldpc_decode_fxp(int *pVector, char *pMessage, char *pCodeword, int *pReliabilities, int precision);

// Decodes input vector using multithreshold parity check reliabilities voting
int ldpc_correct_errors_mth_fxp(int *pVector, char *pCodeword, int *pReliabilities, int precision);

// Decodes input vector using SFF (min-sum) decoder
int ldpc_correct_errors_sff_fxp(int *pVector, char *pCodeword, int *pReliabilities, int precision);

// Tests a vector to be a codeword
int ldpc_is_codeword(char *pVector);

# endif    LDPCDECODE_FXP_H