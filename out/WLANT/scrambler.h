# ifndef   SCRAMBLER_H
#   define SCRAMBLER_H

typedef struct ScramblerState
{
  unsigned char reg1;
  unsigned char reg2;
} Scrambler_state;

extern Scrambler_state tx_scrambler_state;

extern Scrambler_state descrambler_table[128];

void scrambler_11a( unsigned char* inf_bits, 
            		    unsigned num_of_bits,
  		              Scrambler_state* scrambler_state,
                    char descrambler_flag );

#endif // SCRAMBLER_H