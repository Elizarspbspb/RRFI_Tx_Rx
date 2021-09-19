/** LDPC code header file
 *
 * @authors Andrey Efimov
 * @organization: LCA, ISS Dept, SUAI
 * $Revision: 4 $
 * $Date: 26.07.05 14:29 $
 */

// codename: codeA_066r3(96,24,6)

# ifndef   LDPCCODE_H
#   define LDPCCODE_H

#define LDPCMessageLength      1728        ///< message length
#define LDPCCodewordLength     2304        ///< code length

#define NC    576                          ///< number of checks

#define MNOR  15                           ///< maximum number of ones in rows of parity check matrix
#define MNOC  4                            ///< maximum number of ones in columns of parity check matrix

#define NCE   18                           ///< number of rows in compact representation of parity check matrix

/// LDPC code data
typedef struct {
  int  MessageSet[LDPCMessageLength];                   ///< set of message symbols
  int  CheckSet[LDPCCodewordLength - LDPCMessageLength];///< set of check symbols

  int  RowsLists[NC][MNOR + 1];                         ///< list of "ones" in rows of LDPC parity check matrix
  int  ColsLists[LDPCCodewordLength][MNOC + 1];         ///< list of "ones" in columns of LDPC parity check matrix
  int  ColsListsRev[NC][MNOR];                          ///< reverse map list of "ones" in columns of LDPC parity check matrix

  unsigned int ElHComp[NCE * LDPCCodewordLength];       ///< compact representation of parity check matrix

} ldpc_code;

# endif    LDPCCODE_H
