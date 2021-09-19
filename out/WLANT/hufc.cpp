#include"hufc.h"

char code[NUM_HUFFMAN_TREES][NUM_MODULATION_TYPES] = {{0,4,5,6,7},{2,0,1,6,7},{0,2,6,14,15},{0,1,2,3},{0,2,6,7},{0,2,3},{0,1}};
char bits[NUM_HUFFMAN_TREES][NUM_MODULATION_TYPES] = {{1,3,3,3,3},{2,2,2,3,3},{1,2,3,4,4},{2,2,2,2},{1,2,3,3},{1,2,2},{1,1}};

char permutations[120][6] = {{4,3,2,1,0},
{4,3,2,0,1},
{4,3,1,2,0},
{4,3,1,0,2},
{4,3,0,1,2},
{4,3,0,2,1},
{4,2,3,1,0},
{4,2,3,0,1},
{4,2,1,3,0},
{4,2,1,0,3},
{4,2,0,1,3},
{4,2,0,3,1},
{4,1,2,3,0},
{4,1,2,0,3},
{4,1,3,2,0},
{4,1,3,0,2},
{4,1,0,3,2},
{4,1,0,2,3},
{4,0,2,1,3},
{4,0,2,3,1},
{4,0,1,2,3},
{4,0,1,3,2},
{4,0,3,1,2},
{4,0,3,2,1},
{3,4,2,1,0},
{3,4,2,0,1},
{3,4,1,2,0},
{3,4,1,0,2},
{3,4,0,1,2},
{3,4,0,2,1},
{3,2,4,1,0},
{3,2,4,0,1},
{3,2,1,4,0},
{3,2,1,0,4},
{3,2,0,1,4},
{3,2,0,4,1},
{3,1,2,4,0},
{3,1,2,0,4},
{3,1,4,2,0},
{3,1,4,0,2},
{3,1,0,4,2},
{3,1,0,2,4},
{3,0,2,1,4},
{3,0,2,4,1},
{3,0,1,2,4},
{3,0,1,4,2},
{3,0,4,1,2},
{3,0,4,2,1},
{2,3,4,1,0},
{2,3,4,0,1},
{2,3,1,4,0},
{2,3,1,0,4},
{2,3,0,1,4},
{2,3,0,4,1},
{2,4,3,1,0},
{2,4,3,0,1},
{2,4,1,3,0},
{2,4,1,0,3},
{2,4,0,1,3},
{2,4,0,3,1},
{2,1,4,3,0},
{2,1,4,0,3},
{2,1,3,4,0},
{2,1,3,0,4},
{2,1,0,3,4},
{2,1,0,4,3},
{2,0,4,1,3},
{2,0,4,3,1},
{2,0,1,4,3},
{2,0,1,3,4},
{2,0,3,1,4},
{2,0,3,4,1},
{1,3,2,4,0},
{1,3,2,0,4},
{1,3,4,2,0},
{1,3,4,0,2},
{1,3,0,4,2},
{1,3,0,2,4},
{1,2,3,4,0},
{1,2,3,0,4},
{1,2,4,3,0},
{1,2,4,0,3},
{1,2,0,4,3},
{1,2,0,3,4},
{1,4,2,3,0},
{1,4,2,0,3},
{1,4,3,2,0},
{1,4,3,0,2},
{1,4,0,3,2},
{1,4,0,2,3},
{1,0,2,4,3},
{1,0,2,3,4},
{1,0,4,2,3},
{1,0,4,3,2},
{1,0,3,4,2},
{1,0,3,2,4},
{0,3,2,1,4},
{0,3,2,4,1},
{0,3,1,2,4},
{0,3,1,4,2},
{0,3,4,1,2},
{0,3,4,2,1},
{0,2,3,1,4},
{0,2,3,4,1},
{0,2,1,3,4},
{0,2,1,4,3},
{0,2,4,1,3},
{0,2,4,3,1},
{0,1,2,3,4},
{0,1,2,4,3},
{0,1,3,2,4},
{0,1,3,4,2},
{0,1,4,3,2},
{0,1,4,2,3},
{0,4,2,1,3},
{0,4,2,3,1},
{0,4,1,2,3},
{0,4,1,3,2},
{0,4,3,1,2},
{0,4,3,2,1}};

void encode(unsigned char* source, int srcLen, unsigned char* dest, int* destLenBytes, int* destLenBits, int compressionMode) {
  int i,j,temp;
  int num_nodes;
  int bit_position;
	int tree_mode;
	int code_length;
  unsigned char src_statistics[NUM_MODULATION_TYPES];
  unsigned char sorted_order[NUM_MODULATION_TYPES];
  unsigned char anti_order[NUM_MODULATION_TYPES];
  unsigned char src[NUM_SYMBOLS];
  int HuffmanEstimateBits=11; // 11 bits of header
  int BitsLength=2;
  int MinimalBitsLength = BitsLength;
  int MinimalBits;

	memcpy(src,source,srcLen);
  for(i=0; i<srcLen; i++)
    switch(src[i]) {
      case 0: break;
      case 1: break;
      case 2: break;
      case 4: src[i] = 3; break;
      case 6: src[i] = 4; break;
      default: printf("Wrong source type in encode, exit"); return;
    }
  
  memset(dest,0,srcLen);
  memset(src_statistics,0,NUM_MODULATION_TYPES);
  for(i=0; i<srcLen; i++)
    src_statistics[src[i]]++;    
  for(i=0; i<NUM_MODULATION_TYPES; i++)
    sorted_order[i] = i;

  for(i=0; i<NUM_MODULATION_TYPES-1; i++)
    for(j=0; j<NUM_MODULATION_TYPES-i-1; j++)
      if(src_statistics[j]<src_statistics[j+1]) {
        temp = src_statistics[j]; src_statistics[j] = src_statistics[j+1]; src_statistics[j+1] = temp;
        temp = sorted_order[j]; sorted_order[j] = sorted_order[j+1]; sorted_order[j+1] = temp;
      }

  num_nodes = NUM_MODULATION_TYPES;
  while(src_statistics[num_nodes-1]==0)
    num_nodes--;

  switch(num_nodes) {
    case 5:
      if(src_statistics[1]<=(src_statistics[3]+src_statistics[4]) && src_statistics[2]<=(src_statistics[3]+src_statistics[4]))
        if(src_statistics[0]>=(src_statistics[1]+src_statistics[2]))
          tree_mode = 0;
        else tree_mode = 1;
      else if((src_statistics[3]+src_statistics[4])<=src_statistics[1] && (src_statistics[2]+src_statistics[3]+src_statistics[4])<=src_statistics[0])
        tree_mode = 2;
      else if(src_statistics[2]<=(src_statistics[3]+src_statistics[4]) && src_statistics[1]<=(src_statistics[2]+src_statistics[3]+src_statistics[4]) && src_statistics[0]<=(src_statistics[2]+src_statistics[3]+src_statistics[4]))
        tree_mode = 1;
      else { printf("Doctor lazhanulsya s 5 uzlami"); return; }
      break;
    case 4:
      if((src_statistics[2]+src_statistics[3])<src_statistics[0]) tree_mode = 4;
      else tree_mode = 3; 
      break;
    case 3: tree_mode = 5; break;
		case 2: tree_mode = 6; break;
    case 1: tree_mode = 7; break;
    default: printf("Doctor lazhanulsya s derev'yami"); return;
  }

	if(tree_mode==7) {
//    printf("\n--- RLE is selected since tree_mode==7 ----------\n");
		// RLE flag, equal to [0]
		dest[0] = 0;
		// BitsValue, equal to 6 = [110]
		dest[1] = 1; dest[2] = 1; dest[3] = 0;
		encodeRLE(src,srcLen,dest+4,destLenBytes,destLenBits,3,6); // RLE mode
		*destLenBits += 4;
		*destLenBytes = (*destLenBits >> 3)+1;
		return;
	}

// optional RLE attempt ----------------------------------------------------
  if(compressionMode==2 || compressionMode==0) {
    for(i=0; i<num_nodes; i++) HuffmanEstimateBits += src_statistics[i]*bits[tree_mode][i];
	  printf("\nHuffman Estimated Bits: %d\n",HuffmanEstimateBits);

	  encodeRLE(src,srcLen,dest,&temp,&MinimalBits,3,BitsLength); 
	  for(BitsLength=BitsLength+1; BitsLength<=5; BitsLength++) {
		  encodeRLE(src,srcLen,dest,&temp,&i,3,BitsLength); 
		  if(i<MinimalBits) {
			  MinimalBits = i;
			  MinimalBitsLength = BitsLength;
		  }
	  }
	  if(((MinimalBits+4)<HuffmanEstimateBits && compressionMode==0) || compressionMode==2) {
		  printf("---------- RLE is selected ------------\n");
		  // RLE flag, equal to [0]
		  dest[0] = 0;
		  // BitsValue
		  for(i=0; i<3; i++)
			  dest[i+1] = (MinimalBitsLength >> (3-i-1))&1;
		  encodeRLE(src,srcLen,dest+4,destLenBytes,destLenBits,3,MinimalBitsLength); 
		  *destLenBits += 4;
		  *destLenBytes = (*destLenBits >> 3)+1;
		  return;
	  }
  }

// end of optional RLE attempt ----------------------------------------------------
  
//  printf("\n----------Huffman encoding ---\n");
  for(i=0; i<NUM_MODULATION_TYPES; i++)
    anti_order[sorted_order[i]]=i;
  
	bit_position = 0;
	// Huffman flag, equal to [1]
	dest[bit_position++] = 1;
	// tree code, 3 bits
	for(i=0; i<3; i++)
		dest[bit_position++] = (tree_mode >> (3-i-1)) & 1;
	// sequence code, 7 bits
	for(i=0; i<120; i++) {
		for(j=0; j<5; j++) 
			if(permutations[i][j]!=sorted_order[j]) break;
		if(j==5) 
			break;
	}
	for(j=0; j<7; j++)
		dest[bit_position++] = (i >> (7-j-1)) & 1;

  for(i=0; i<srcLen; i++) {
    code_length = bits[tree_mode][anti_order[src[i]]];
    for(j=0; j<code_length; j++)
      dest[bit_position++] = (code[tree_mode][anti_order[src[i]]] >> (code_length-j-1)) & 1;
  }

  *destLenBytes = (bit_position >> 3)+1;
  *destLenBits = bit_position;
}

void encodeRLE(unsigned char* src, int srcLen, unsigned char* dest, int* destLenBytes, int* destLenBits, int BitsValue, int BitsLength) {
  int bit_position = 0;
  int counter=0;
  int i,j;

  for(i=0; i<srcLen; i++) {
    for(j=0; j<BitsValue; j++)
      dest[bit_position++] = (src[i] >> (BitsValue-j-1)) & 1;
    for(counter=1; counter<(1<<BitsLength)-1 && src[i+counter]==src[i] && i+counter < srcLen; counter++)
      ;
    for(j=0; j<BitsLength; j++)  
			dest[bit_position++] = (counter >> (BitsLength-j-1)) & 1;
		i += counter-1;
  }
	*destLenBytes = (bit_position >> 3)+1;
	*destLenBits = bit_position;
}

int decode(unsigned char* src, int srcLen, unsigned char* dest) {
	int destLen = 0;
	int i,j,k,temp;
	int tree_mode = 0;
	int src_values = 0;
	int bit_position=1;
	int num_node;

	if(!src[0]) {
		return decodeRLE(src,srcLen,dest,3);
	}

	for(i=0; i<3; i++)
		tree_mode ^= (src[bit_position++] << (3-i-1));
	for(i=0; i<7; i++)
		src_values ^= (src[bit_position++] << (7-i-1));
	switch(tree_mode) {
		case 0: case 1: case 2: num_node = 5; break;
		case 3: case 4: num_node = 4; break;
		case 5: num_node = 3; break;
		case 6: num_node = 2; break;
		default: printf("Tree detection error in Huffman decoding"); return -1;
	}

	while(bit_position < srcLen) {
		for(j=0; j<num_node; j++) {
			for(k=0,temp=0; k<bits[tree_mode][j]; k++)
				temp ^= (src[bit_position+k] << (bits[tree_mode][j]-k-1));
			if(temp==code[tree_mode][j]) {
				dest[destLen++] = permutations[src_values][j];
				bit_position += bits[tree_mode][j];
				break;
			}
		}
		if(j==num_node) {
			printf("Huffman decoding error occurs");
			return -1;
		}
	}
	for(i=0; i<destLen; i++)
		switch(dest[i]) {
			case 0: break;
			case 1: break;
			case 2: break;
			case 3: dest[i] = 4; break;
			case 4: dest[i] = 6; break;
			default: printf("Symbols restoration error"); return -1;
		}
	return destLen;
}

int decodeRLE(unsigned char* src, int srcLen, unsigned char* dest, int BitsValue) {
	int destLen = 0;
	int i;
	int bit_position = 1;
	int BitsLength = 0;
	unsigned char symbol;
	int RunLength;

  if(src[0]) {
		printf("This is not RLE encoded sequence");
		return -1;
	}

	for(i=0; i<3; i++)
		BitsLength ^= (src[bit_position++] << (3-i-1));
	
	while(bit_position<srcLen) {
		for(symbol = 0, i=0; i<BitsValue; i++)
			symbol ^= (src[bit_position++] << (BitsValue-i-1));
		for(RunLength = 0, i=0; i<BitsLength; i++)
			RunLength ^= (src[bit_position++] << (BitsLength-i-1));
    for(i=0; i<RunLength; i++)
			dest[destLen++] = symbol;
	}
	for(i=0; i<destLen; i++)
		switch(dest[i]) {
			case 0: break;
			case 1: break;
			case 2: break;
			case 3: dest[i] = 4; break;
			case 4: dest[i] = 6; break;
			default: printf("Symbols restoration error"); return -1;
		}
  return destLen;
}





















