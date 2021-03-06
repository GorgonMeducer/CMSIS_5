/*
 * Copyright (C) 2010-2019 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_elementwise_mul_s8
 * Description:  Element wise multiplication
 *
 * $Date:        August 2019
 * $Revision:    V.1.0.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"

/**
 *  @ingroup groupNN
 */

/**
 * @addtogroup BasicMath
 * @{
 */

/**
   * @brief s8 element wise multiplication of two vectors
   *
   * @note   Refer header file for details.
   *
   */

arm_status
arm_elementwise_mul_s8(const int8_t *input_1_vect,
                       const int8_t *input_2_vect,
                       const int32_t input_1_offset,
                       const int32_t input_2_offset,
                       int8_t *output,
                       const int32_t out_offset,
                       const int32_t out_mult,
                       const int32_t out_shift,
                       const int32_t out_activation_min,
                       const int32_t out_activation_max,
                       const uint32_t block_size)
{

  uint32_t loop_count;
  int32_t input_1;
  int32_t input_2;
  int32_t mul_res;

#if defined(ARM_MATH_LOOPUNROLL) && defined(ARM_MATH_DSP)
  int32_t a_1, b_1, a_2, b_2;

  int32_t offset_1_packed, offset_2_packed;

  int8_t r1, r2, r3, r4;

  offset_1_packed = (input_1_offset << 16U) | (input_1_offset & 0x0FFFFL);
  offset_2_packed = (input_2_offset << 16U) | (input_2_offset & 0x0FFFFL);

  loop_count = block_size >> 2;

  while (loop_count > 0U)
  {
    /* 4 outputs are calculated in one loop. The order of calculation is follows the order of output sign extension
       intrinsic */
    input_1_vect = read_and_pad_reordered(input_1_vect, &b_1, &a_1);
    input_2_vect = read_and_pad_reordered(input_2_vect, &b_2, &a_2);

    a_1 = __SADD16(a_1, offset_1_packed);
    b_1 = __SADD16(b_1, offset_1_packed);

    a_2 = __SADD16(a_2, offset_2_packed);
    b_2 = __SADD16(b_2, offset_2_packed);

    /* Mul 1 */
    input_1 = (int16_t)(b_1 & 0x0FFFFL);
    input_2 = (int16_t)(b_2 & 0x0FFFFL);

    mul_res = input_1 * input_2;
    mul_res = arm_nn_divide_by_power_of_two(arm_nn_sat_doubling_high_mult(mul_res, out_mult), -out_shift) + out_offset;

    mul_res = MAX(mul_res, out_activation_min);
    mul_res = MIN(mul_res, out_activation_max);
    r1 = (q7_t)mul_res;

    /* Mul 3 */
    input_1 = (int16_t)((b_1 >> 16U) & 0x0FFFFL);
    input_2 = (int16_t)((b_2 >> 16U) & 0x0FFFFL);

    mul_res = input_1 * input_2;
    mul_res = arm_nn_divide_by_power_of_two(arm_nn_sat_doubling_high_mult(mul_res, out_mult), -out_shift) + out_offset;
    mul_res = MAX(mul_res, out_activation_min);
    mul_res = MIN(mul_res, out_activation_max);
    r3 = (q7_t)mul_res;

    /* Mul 2 */
    input_1 = (int16_t)(a_1 & 0x0FFFFL);
    input_2 = (int16_t)(a_2 & 0x0FFFFL);

    mul_res = input_1 * input_2;
    mul_res = arm_nn_divide_by_power_of_two(arm_nn_sat_doubling_high_mult(mul_res, out_mult), -out_shift) + out_offset;
    mul_res = MAX(mul_res, out_activation_min);
    mul_res = MIN(mul_res, out_activation_max);
    r2 = (q7_t)mul_res;

    /* Mul 4 */
    input_1 = (int16_t)((a_1 >> 16U) & 0x0FFFFL);
    input_2 = (int16_t)((a_2 >> 16U) & 0x0FFFFL);

    mul_res = input_1 * input_2;
    mul_res = arm_nn_divide_by_power_of_two(arm_nn_sat_doubling_high_mult(mul_res, out_mult), -out_shift) + out_offset;
    mul_res = MAX(mul_res, out_activation_min);
    mul_res = MIN(mul_res, out_activation_max);
    r4 = (q7_t)mul_res;

    write_q7x4_ia(&output, __PACKq7(r1, r2, r3, r4));

    loop_count--;
  }

  loop_count = block_size & 0x3;
#else
  loop_count = block_size;
#endif

  while (loop_count > 0U)
  {
    /* C = A * B */

    input_1 = *input_1_vect++ + input_1_offset;
    input_2 = *input_2_vect++ + input_2_offset;

    mul_res = input_1 * input_2;
    mul_res = arm_nn_divide_by_power_of_two(arm_nn_sat_doubling_high_mult(mul_res, out_mult), -out_shift) + out_offset;

    mul_res = MAX(mul_res, out_activation_min);
    mul_res = MIN(mul_res, out_activation_max);

    *output++ = (q7_t)mul_res;

    /* Decrement loop counter */
    loop_count--;
  }

  return (ARM_MATH_SUCCESS);
}

/**
 * @} end of BasicMath group
 */
