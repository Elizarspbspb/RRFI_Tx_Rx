/** LDPC encode used parity check matrix header file
 *
 * @authors Andrey Efimov
 * @organization: LCA, ISS Dept, SUAI
 * $Revision: 1 $
 * $Date: 21.10.04 13:31 $
 */

# ifndef   LDPCENCODE_H
#   define LDPCENCODE_H

#include "ldpc_code.h"

// Encodes message vector
int ldpc_encode(const char *pMessage, char *pCodeword);

# endif    LDPCENCODE_H