///////////////////////////////////////////////////////////////////////////////////////////////////
//                                             //                                                //
//    FileUtils.c                              //    (c) COPYRIGHT  2003                         //
//    John S. Sadowsky                         //   Intel Corporation                            //
//                                             //                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

// History
// Aug. 10, 2003   JSS added funciton ReadMultiFloat()

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "FileUtils.h"
#include "GeneralUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Local Function Declairations                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FileUtils_ReadLine(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *returnLine
) ;

boolean FileUtils_ConvertStringToInt(
  char           *string ,
  int            *intPtr
) ;

boolean FileUtils_ConvertOctalStringToInt(
  char           *string ,
  int            *intPtr
) ;

boolean FileUtils_ConvertHexStringToInt(
  char           *string ,
  int            *intPtr
) ;

boolean FileUtils_ConvertStringToFloat(
  char           *string ,
  float          *floatPtr
) ;


///////////////////////////////////////////////////////////////////////////////////////////////////
// External Functions                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
// ReadString                                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadString(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *returnString
) {

  char            stringX[MAX_STRING_LENGTH] ;
  char           *token ;
  int             k ;

  FileUtils_ReadLine( fieldName , lineName , fileName , stringX ) ;

  assert( stringX[0] != ' ' ) ;
  assert( stringX[0] != '\0' ) ;

  token = strtok( stringX , PARAMS_FILE_SEPERATOR ) ;

  for ( k = 0 ; k < MAX_FILE_NAME_LEN ; k++ ) if ( token[k] == '\0' ) break ;

  if ( k == MAX_FILE_NAME_LEN ) {

    printf("\n\n** ReadFileName Failure **\n\n" ) ;
    printf("  String Name Too Long\n" ) ;
    printf("    Line: %s\n", lineName ) ;
    printf("   Field: %s\n", fieldName ) ;
    printf("    File: %s\n\n", fileName ) ;
    exit(0) ;

  }

  if ( strcmp( token , "SAME" ) == 0 ) {
    strcpy( returnString , fileName ) ;             // This is used for File Names
  } else {
    strcpy( returnString , token ) ;
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FindNumberedField                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FindNumberedField(
  char           *fieldName ,
  char           *modelName ,
  char           *fileName ,
  char           *returnFieldName
) {

  char            fieldName_X[100], fieldX[100], line[MAX_STRING_LENGTH], numString[3] ;
  char           *token ;
  FILE           *file ;
  int             k ;


  file = fopen( fileName , "r" ) ;

  for ( k = 0 ; k < 100 ; k++ ) {

    // Construct numbered Field Name
    strcpy( fieldName_X , fieldName ) ;
    strcat( fieldName_X , "_" ) ;
    if ( k >= 10 ) {
      numString[0] = (k/10) + 48 ;
      numString[1] = (k%10) + 48 ;
      numString[2] = '\0' ;
    } else {
      numString[0] = k + 48 ;
      numString[1] = '\0' ;
    }
    strcat( fieldName_X , numString ) ;

    strcpy( fieldX , "[" ) ;
    strcat( fieldX , fieldName_X ) ;
    strcat( fieldX , "]" ) ;

    // Find Field
    for ( ; ; ) {

      if ( NULL == fgets( line , MAX_STRING_LENGTH, file ) ) {
        printf("\n\n** FindNumberedField Error **\n\n") ;
        printf("     Field Name: %s\n", fieldName ) ;
        printf("     Model Name: %s\n", modelName ) ;
        printf("      File Name: %s\n", fileName ) ;
        fclose( file ) ;
        exit(0) ;
        return ;
      }

      // Get the token and check if it is the required section
      if ( line[0] == '[' ) {
        token = strtok( line , PARAMS_FILE_SEPERATOR ) ;
        if ( 0 == strcmp( token , fieldX )) break ;
      }

    }  // End of Field Search

    // Find "ModelName" Line
    while ( NULL != fgets(line, MAX_STRING_LENGTH, file ) ) {

      token = strtok( line , PARAMS_FILE_SEPERATOR ) ;
      if ( (token != NULL)
          && ((0 == strcmp( token , "modelName" )) || (0 == strcmp( token , "ModelName" ))) ) {
        token = strtok( NULL, "\n" ) ;
        token = token + strspn( token , " \t" ) ;  // remove spaces
        token = strtok( token , PARAMS_FILE_SEPERATOR ) ;
        if ( strcmp( token , modelName ) == 0 ) {
          fclose( file ) ;
          strcpy( returnFieldName , fieldName_X ) ;
          return ;
        } else {
          break;
      } }

      if ( line[0] == '[' ) {
        printf("\n\n** FindNumberedField Error **\n\n") ;
        printf("     Field Name: %s\n", fieldName ) ;
        printf("     Model Name: %s\n", modelName ) ;
        printf("      File Name: %s\n", fileName ) ;
        fclose( file ) ;
        exit(0) ;
      }

    }
  }

  strcpy( returnFieldName , "NoFindField" ) ;
  fclose( file ) ;
  return ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ReadInt                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadInt(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *Format ,
  int            *IntPtr
) {

  char            stringX[MAX_STRING_LENGTH] ;
  char           *token ;
  boolean         Flag ;

  FileUtils_ReadLine( fieldName , lineName , fileName , stringX ) ;

  token = strtok( stringX , PARAMS_FILE_SEPERATOR ) ;

  if ( (0 == strcmp( Format , "decimal" )) || (0 == strcmp( Format , "Decimal" ))  ) {
    Flag = FileUtils_ConvertStringToInt( token , IntPtr ) ;
  } else if ( (0 == strcmp( Format , "octal" )) || (0 == strcmp( Format , "Octal" )) ) {
    Flag = FileUtils_ConvertOctalStringToInt( token , IntPtr ) ;
  } else if ( (0 == strcmp( Format , "hex" ))  || (0 == strcmp( Format , "hexidecimal" ))
           || (0 == strcmp( Format , "Hex" ))  || (0 == strcmp( Format , "Hexidecimal" )) ) {
    Flag = FileUtils_ConvertHexStringToInt( token , IntPtr ) ;
  } else {
    Flag = F ;
  }

  if ( Flag == F ) {

    printf("\n\n** ReadInt Failure **\n\n" ) ;
    printf("    Line: %s\n", lineName ) ;
    printf("   Field: %s\n", fieldName ) ;
    printf("    File: %s\n\n", fileName ) ;
    exit(0) ;

  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ReadMultiInt                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadMultiInt(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  int             NumInt ,
  char           *Format ,
  int            *IntPtr
) {

  char            stringX[MAX_STRING_LENGTH] ;
  char           *token ;
  int             k ;
  boolean         Flag ;

  FileUtils_ReadLine( fieldName , lineName , fileName , stringX ) ;

  token = strtok( stringX , PARAMS_FILE_SEPERATOR ) ;

  for ( k = 0 ; k < NumInt ; k++ ) {

    if ( 0 == strcmp( Format , "decimal" ) ) {
      Flag = FileUtils_ConvertStringToInt( token , &(IntPtr[k]) ) ;
    } else if ( 0 == strcmp( Format , "octal" ) ) {
      Flag = FileUtils_ConvertOctalStringToInt( token , &(IntPtr[k]) ) ;
    } else {
      Flag = F ;
    }

    if ( Flag == F ) {

      printf("\n\n** ReadInt Failure **\n\n" ) ;
      printf("    Line: %s\n", lineName ) ;
      printf("   Field: %s\n", fieldName ) ;
      printf("    File: %s\n\n",fileName ) ;
      exit(0) ;

    }

    token = strtok( NULL, "\0" ) ;
    token = strtok( token , PARAMS_FILE_SEPERATOR ) ;
//    token = token + strspn( token , " \t" ) ;  // remove spaces

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ReadFloat                                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadFloat(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  float          *FloatPtr
) {

  char            stringX[MAX_STRING_LENGTH] ;
  char           *token ;

  FileUtils_ReadLine( fieldName , lineName , fileName , stringX ) ;

  token = strtok( stringX , PARAMS_FILE_SEPERATOR ) ;

  if ( F == FileUtils_ConvertStringToFloat( token , FloatPtr ) ) {

    printf("\n\n** ReadFloat Failure **\n\n" ) ;
    printf("    Line: %s\n", lineName ) ;
    printf("   Field: %s\n", fieldName ) ;
    printf("    File: %s\n\n",fileName ) ;
    exit(0) ;

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ReadMultiFloat                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadMultiFloat(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  int             numFlt ,
  float          *FloatPtr
) {

  char            stringX[MAX_STRING_LENGTH] ;
  int             k ;
  char           *token ;

  FileUtils_ReadLine( fieldName , lineName , fileName , stringX ) ;

  token = strtok( stringX , PARAMS_FILE_SEPERATOR ) ;

  for ( k = 0 ; k < numFlt ; k++ ) {

    if ( F == FileUtils_ConvertStringToFloat( token , &(FloatPtr[k]) ) ) {

      printf("\n\n** ReadFloat Failure **\n\n" ) ;
      printf("    Line: %s\n", lineName ) ;
      printf("   Field: %s\n", fieldName ) ;
      printf("    File: %s\n\n",fileName ) ;
      exit(0) ;

    }

    token = strtok( NULL, "\0" ) ;
    token = strtok( token , PARAMS_FILE_SEPERATOR ) ;

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Local Functions Definitions                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
// FileUtils_ReadLine                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FileUtils_ReadLine(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *ReturnLine
) {

  char            line[MAX_STRING_LENGTH];
  char            FieldX[MAX_STRING_LENGTH];
  char           *token ;
  FILE           *file ;

  strcpy(ReturnLine, NULL_STRING) ;

  strcpy( FieldX , "[" ) ;
  strcat( FieldX , fieldName ) ;
  strcat( FieldX , "]" ) ;

  file = fopen( fileName , "r" ) ;

  // Find Field
  for ( ; ; ) {

    if ( NULL == fgets( line, MAX_STRING_LENGTH, file ) ) {
      printf("\n\n** ReadLine Failure **\n\n" ) ;
      printf("    Line: %s\n", lineName ) ;
      printf("   Field: %s\n", fieldName ) ;
      printf("    File: %s\n\n",fileName ) ;
      exit(0) ;
    }

    // Get the token and check if it is the required section
    if ( line[0] == '[' ) {
      token = strtok( line , PARAMS_FILE_SEPERATOR ) ;
      if (0 == strcmp( token , FieldX )) break ;
    }

  }

  // Find Line
  while ( NULL != fgets(line, MAX_STRING_LENGTH, file ) ) {
    token = strtok( line , PARAMS_FILE_SEPERATOR ) ;
    if ( (token != NULL) && (0 == strcmp( token , lineName )) ) {
      token = strtok( NULL, "\n" ) ;
      token = token + strspn( token , " \t" ) ;  // remove spaces
      strcpy( ReturnLine , token ) ;
      fclose( file ) ;
      return ;
    }
  }

  printf("\n\n** ReadLine Failure **\n\n" ) ;
  printf("    Line: %s\n",   lineName ) ;
  printf("   Field: %s\n",   fieldName ) ;
  printf("    File: %s\n\n", fileName ) ;
  exit(0) ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FileUtils_ConvertStringToInt                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

boolean FileUtils_ConvertStringToInt(
  char           *string ,
  int            *IntPtr
) {

  int             k, n, ix, IntX, ISign ;

  for ( n = 0 ; n < MAX_STRING_LENGTH ; n++ )
    if ( string[n] == '\0' ) break ;

  if ( n >= MAX_STRING_LENGTH ) return F ;

  IntX = 0 ;
  ix = string[0] - 48 ;
  if ( (ix >= 0) && (ix <= 9)) {
    ISign = 1 ;
    IntX  = ix ;
  } else if ( string[0] == '+' ) {
    ISign = 1 ;
  } else if ( string[0] == '-' ) {
    ISign = -1 ;
  } else {
    return F ;
  }

  for ( k = 1 ; k < n ; k++ ) {
    ix = string[k] - 48 ;
    if ( (ix < 0) || (ix > 9) ) return F ;
    IntX = 10*IntX + ix ;
  }

  *IntPtr = ISign * IntX ;

  return T ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FileUtils_ConvertOctalStringToInt                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

boolean FileUtils_ConvertOctalStringToInt(
  char           *string ,
  int            *IntPtr
) {

  int             k, n, ix, IntX ;

  for ( n = 0 ; n < MAX_STRING_LENGTH ; n++ )
    if ( string[n] == '\0' ) break ;

  if ( n >= MAX_STRING_LENGTH ) return F ;

  IntX = 0 ;
  for ( k = 0 ; k < n ; k++ ) {
    ix = string[k] - 48 ;
    if ( (ix < 0) || (ix > 7) ) return F ;
    IntX = 8*IntX + ix ;
  }

  *IntPtr = IntX ;

  return T ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FileUtils_ConvertHexStringToInt                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

boolean FileUtils_ConvertHexStringToInt(
  char           *string ,
  int            *intPtr
) {

  int             k, n, ix, iz ;

  for ( n = 0 ; n < MAX_STRING_LENGTH ; n++ )
    if ( string[n] == '\0' ) break ;

  if ( n >= MAX_STRING_LENGTH ) return F ;

  if ( string[0] != '0' ) return F ;
  if ( string[1] != 'x' ) return F ;

  iz = 0 ;
  for ( k = 2 ; k < n ; k++ ) {
    ix = string[k] - 48 ;
    if ( (ix >= 0) && (ix <= 9) ) {
      iz = 16*iz + ix ;
    } else if ( (string[k] == 'A') || (string[k] == 'a') ) {
      iz = 16*iz + 10 ;
    } else if ( (string[k] == 'B') || (string[k] == 'b') ) {
      iz = 16*iz + 11 ;
    } else if ( (string[k] == 'C') || (string[k] == 'c') ) {
      iz = 16*iz + 12 ;
    } else if ( (string[k] == 'D') || (string[k] == 'd') ) {
      iz = 16*iz + 13 ;
    } else if ( (string[k] == 'E') || (string[k] == 'e') ) {
      iz = 16*iz + 14 ;
    } else if ( (string[k] == 'F') || (string[k] == 'f') ) {
      iz = 16*iz + 15 ;
    } else {
      return F ;
    }
  }

  *intPtr = iz ;

  return T ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FileUtils_ConvertStringToFloat                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

boolean FileUtils_ConvertStringToFloat(
  char           *string ,
  float          *FloatPtr
) {

  int             k, n, ix, Mantissa, MSign, Radix, Exponent, ESign ;

  for ( n = 0 ; n < MAX_STRING_LENGTH ; n++ )
    if ( string[n] == '\0' ) break ;

  if ( n >= MAX_STRING_LENGTH ) return F ;

  // Construct Mantissa

  Mantissa = 0 ;
  ix = string[0] - 48 ;
  if ( (ix >= 0) && (ix <= 9)) {
    MSign    = 1 ;
    Mantissa = ix ;
  } else if ( string[0] == '+' ) {
    MSign = 1 ;
  } else if ( string[0] == '-' ) {
    MSign = -1 ;
  } else {
    return F ;
  }

  for ( k = 1 ; k < n ; k++ ) {
    ix = string[k] - 48 ;
    if ( (ix < 0) || (ix > 9) ) break ;
    Mantissa = 10*Mantissa + ix ;
  }

  Radix = 0 ;
  if ( string[k++] == '.' ) {
    for ( ; k < n ; k++ ) {
      ix = string[k] - 48 ;
      if ( (ix < 0) || (ix > 9) ) break ;
      Mantissa = 10*Mantissa + ix ;
      Radix++ ;
    }
  }

  Mantissa *= MSign ;

  // Construct Exponent

  Exponent = 0 ;
  ESign    = 1 ;
  if ( string[k] == 'e' ) {

    ix  = string[++k] - 48 ;
    if ( (ix >= 0) && (ix <= 9) ) {
      Exponent = ix ;
    } else if ( string[k] == '+' ) {
    } else if ( string[k] == '-' ) {
      MSign    = -1 ;
    } else {
      return F ;
    }

    for ( k++ ; k < n ; k++ ) {
      ix = string[k] - 48 ;
      if ( (ix < 0) || (ix > 9) ) break ;
      Exponent = 10*Exponent + ix ;
    }

  }

  // Check for proper ending

  if ( k < n ) {
    if ( string[k++] != 'f' ) return F ;
    if ( k < n )              return F ;
  }

  // Construct Float

 *FloatPtr  = (float) (Mantissa * pow( 10.0 , ESign * Exponent - Radix )) ;

  return T ;

}




