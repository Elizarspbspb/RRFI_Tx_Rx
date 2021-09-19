
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                             //                                                //
//    FileUtils.h                              //    (c) COPYRIGHT  2003                         //
//    John S. Sadowsky                         //   Intel Corporation                            //
//                                             //                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#define     PARAMS_FILE_ENDLINE         "\n"
#define     NULL_STRING                 ""
#define     PARAMS_FILE_SEPERATOR       " ,\t\n\r"

// String limit defintions
#define MAX_STRING_LENGTH 2048
#define MAX_FILE_NAME_LEN 200



#define OPEN_FILE(dst,name,flags) {if(((dst) = fopen((name),(flags))) == NULL){ \
fprintf(stderr,"ERROR: could not open file %s\n ",(name)); ERROR("Exit with ERROR"); }}

#define CLOSE_FILE(file) {if(fclose(file) != 0) {extern int errno; perror("Could not close file"); ERROR("Exit with error ");}}


void ReadString(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *returnString
) ;

void FindNumberedField(
  char           *fieldName ,
  char           *modelName ,
  char           *fileName ,
  char           *returnFieldName
) ;

void ReadInt(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  char           *format ,
  int            *intPtr
) ;

void ReadMultiInt(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  int             numInt ,
  char           *format ,
  int            *intPtr
) ;

void ReadFloat(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  float          *floatPtr
) ;

void ReadMultiFloat(
  char           *fieldName ,
  char           *lineName ,
  char           *fileName ,
  int             numFlt ,
  float          *FloatPtr
) ;

#endif
