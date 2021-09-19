
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                             //                                                //
//    GeneralUtils.h                           //   (c) COPYRIGHT  2003                          //
//    John S. Sadowsky                         //   Intel Corporation                            //
//                                             //                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//    This file contains general macros and constants, mostly for memory allocation.             //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _GENERAL_UTILS_H_
#define _GENERAL_UTILS_H_


#define       PI     (3.14159265358979)
#define   TWO_PI     (6.28318530717959)
#define   PI_TWO     (1.57079632679490)

typedef enum {
  F     = 0 ,
  T     = 1
} boolean ;


#define ERROR(message) { \
 fprintf(stderr,"ERROR in file %s, in line %d \n %s \n",__FILE__,__LINE__,(message)) ;\
 abort();  /* aboart and not exit for debugging porpose */  \
  }

#define WARNING(MES)        printf("Warning in %s(%d): %s\n", __FILE__, __LINE__, MES)


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Vector Allocation
///////////////////////////////////////////////////////////////////////////////////////////////////

#define ALLOCATE_VECTOR(ptr,size,type) {\
 if ((size) <= 0 ) ERROR("ALLOCATE_VECTOR, size must be a positive number"); \
 if ((ptr = (type *) malloc((size) * sizeof(type))) == NULL) {\
    ERROR("Could not allocate memory"); \
 }};

#define FREE_VECTOR(ptr) {if(ptr!=NULL) free(ptr) ; ptr = NULL;}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Matrix Allocation
///////////////////////////////////////////////////////////////////////////////////////////////////

#define ALLOCATE_MATRIX(ptr,nRow,nCol,type) { \
 int macro_matrix_i; \
 if (((nRow) <= 0) || ((nCol) <= 0)) {\
		ERROR("ALLOCATE_MATRIX(ptr,rows,columns,type) \n Number of rows or columns is not > 0") ; \
  } \
  ALLOCATE_VECTOR(ptr,(nRow),type *); \
  ALLOCATE_VECTOR(ptr[0],(nRow*nCol),type) ; \
  for ( macro_matrix_i = 1 ; macro_matrix_i < (nRow) ; macro_matrix_i++ ) {\
    ptr[macro_matrix_i] = ptr[0] + (macro_matrix_i*(nCol)) ; \
 }}

#define FREE_MATRIX(ptr) {if(ptr!=NULL){FREE_VECTOR(ptr[0]); FREE_VECTOR(ptr);} }


#define ALLOCATE_3D_MATRIX(ptr,n1,n2,n3,type) { \
  int macro_k1, macro_k2 ; \
  if (((n1) <= 0) || ((n2) <= 0) || ((n3) <= 0)) {\
    ERROR("ALLOCATE_3D_MATRIX \n Dimension not > 0") ; \
  } \
  ALLOCATE_MATRIX(ptr,(n1),(n2),type *) ; \
  ALLOCATE_VECTOR(ptr[0][0] , (n1*n2*n3) , type ) ; \
  for (   macro_k1 = 0 ; macro_k1 < (n1) ; macro_k1++ ) { \
    for ( macro_k2 = 0 ; macro_k2 < (n2) ; macro_k2++ ) { \
      ptr[macro_k1][macro_k2] = ptr[0][0] + (macro_k1*(n2*n3) + macro_k2*(n3)) ; \
  } } \
 }

#define FREE_3D_MATRIX(ptr) {if(ptr!=NULL){FREE_VECTOR( ptr[0][0] ); FREE_MATRIX( ptr );}}


/* compiler dependend optimization option */

#if (defined(__GNUC__) && defined(NDEBUG))
#define INLINE __inline__
#else
#define INLINE
#endif



#endif
