# ifndef   CYCLICPREFIX_H
#   define CYCLICPREFIX_H

#include "commonfunc.h"

void add_cyclic_prefix_11a( fxp_complex* time_signal,
   												  fxp_complex* time_sym,
                            unsigned num_of_OFDM_symbols );

#endif // CYCLICPREFIX_H
  