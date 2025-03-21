/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file ldpc_encoder_optim.c
 * \brief Defines the optimized LDPC encoder
 * \author Florian Kaltenberger, Raymond Knopp, Kien le Trung (Eurecom)
 * \email openair_tech@eurecom.fr
 * \date 27-03-2018
 * \version 1.0
 * \note
 * \warning
 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "assertions.h"
#include "common/utils/LOG/log.h"
#include "time_meas.h"
#include "openair1/PHY/CODING/nrLDPC_defs.h"
#include "openair1/PHY/CODING/nrLDPC_extern.h"
#include "ldpc_encode_parity_check.c"
#include "ldpc_generate_coefficient.c"

int LDPCencoder(uint8_t **test_input, uint8_t **channel_input, encoder_implemparams_t *impp)
{
  int Zc = impp->Zc;
  int Kb = impp->Kb;
  int block_length = impp->K;
  int BG = impp->BG;
  int nrows=0,ncols=0;
  int rate=3;
  int no_punctured_columns,removed_bit;

  int simd_size;

  //determine number of bits in codeword
   //if (block_length>3840)
   if (BG==1)
     {
       //BG=1;
       nrows=46; //parity check bits
       ncols=22; //info bits
       rate=3;
     }
     //else if (block_length<=3840)
    else if	(BG==2)
     {
       //BG=2;
       nrows=42; //parity check bits
       ncols=10; // info bits
       rate=5;

       }


#ifdef DEBUG_LDPC
  LOG_D(PHY,"ldpc_encoder_optim_8seg: BG %d, Zc %d, Kb %d, block_length %d\n",BG,Zc,Kb,block_length);
  LOG_D(PHY,"ldpc_encoder_optim_8seg: PDU %x %x %x %x\n",test_input[0][0],test_input[0][1],test_input[0][2],test_input[0][3]);
#endif

  if ((Zc&31) > 0) simd_size = 16;
  else simd_size = 32;

  uint8_t c[22*Zc] __attribute__((aligned(32))); //padded input, unpacked, max size
  uint8_t d[46*Zc] __attribute__((aligned(32))); //coded parity part output, unpacked, max size

  // calculate number of punctured bits
  no_punctured_columns=(int)((nrows-2)*Zc+block_length-block_length*rate)/Zc;
  removed_bit = (nrows - no_punctured_columns - 2) * Zc + block_length - block_length * rate;
  // printf("%d\n",no_punctured_columns);
  // printf("%d\n",removed_bit);
  // unpack input
  memset(c, 0, sizeof(c));
  memset(d, 0, sizeof(d));

  if(impp->tinput != NULL) start_meas(impp->tinput);
  for (int i=0; i<block_length; i++) {
    c[i] = (test_input[0][i/8]&(128>>(i&7)))>>(7-(i&7));
      //printf("c(%d,%d)=%d\n",j,i,temp);
    }

  if(impp->tinput != NULL) stop_meas(impp->tinput);

  if ((BG==1 && Zc>=176) || (BG==2 && Zc>=72)) {
    // extend matrix
    if(impp->tprep != NULL) start_meas(impp->tprep);
    if(impp->tprep != NULL) stop_meas(impp->tprep);
    //parity check part
    if(impp->tparity != NULL) start_meas(impp->tparity);
    encode_parity_check_part_optim(c, d, BG, Zc, Kb,simd_size, ncols);
    if(impp->tparity != NULL) stop_meas(impp->tparity);
  }
  else {
    if (encode_parity_check_part_orig(c, d, BG, Zc, Kb, block_length)!=0) {
      printf("Problem with encoder\n");
      return(-1);
    }
  }
  if(impp->toutput != NULL) start_meas(impp->toutput);
  // information part and puncture columns
  memcpy(&channel_input[0][0], &c[2*Zc], (block_length-2*Zc)*sizeof(unsigned char));
  memcpy(&channel_input[0][block_length-2*Zc], &d[0], ((nrows-no_punctured_columns) * Zc-removed_bit)*sizeof(unsigned char));

  if(impp->toutput != NULL) stop_meas(impp->toutput);
  return 0;
}


