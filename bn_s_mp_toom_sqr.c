#include "tommath_private.h"
#ifdef BN_S_MP_TOOM_SQR_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/* squaring using Toom-Cook 3-way algorithm */
mp_err s_mp_toom_sqr(const mp_int *a, mp_int *b)
{
   mp_int w0, w1, w2, w3, w4, tmp1, a0, a1, a2;
   int B;
   mp_err err;

   /* init temps */
   if ((err = mp_init_multi(&w0, &w1, &w2, &w3, &w4, &a0, &a1, &a2, &tmp1, NULL)) != MP_OKAY) {
      return err;
   }

   /* B */
   B = a->used / 3;

   /* a = a2 * B**2 + a1 * B + a0 */
   if ((err = mp_mod_2d(a, MP_DIGIT_BIT * B, &a0)) != MP_OKAY) {
      goto LBL_ERR;
   }

   if ((err = mp_copy(a, &a1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   mp_rshd(&a1, B);
   if ((err = mp_mod_2d(&a1, MP_DIGIT_BIT * B, &a1)) != MP_OKAY) {
      goto LBL_ERR;
   }

   if ((err = mp_copy(a, &a2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   mp_rshd(&a2, B*2);

   /* w0 = a0*a0 */
   if ((err = mp_sqr(&a0, &w0)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* w4 = a2 * a2 */
   if ((err = mp_sqr(&a2, &w4)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* w1 = (a2 + 2(a1 + 2a0))**2 */
   if ((err = mp_mul_2(&a0, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, &a1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_mul_2(&tmp1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, &a2, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }

   if ((err = mp_sqr(&tmp1, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* w3 = (a0 + 2(a1 + 2a2))**2 */
   if ((err = mp_mul_2(&a2, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, &a1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_mul_2(&tmp1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, &a0, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }

   if ((err = mp_sqr(&tmp1, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }


   /* w2 = (a2 + a1 + a0)**2 */
   if ((err = mp_add(&a2, &a1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, &a0, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sqr(&tmp1, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* now solve the matrix

      0  0  0  0  1
      1  2  4  8  16
      1  1  1  1  1
      16 8  4  2  1
      1  0  0  0  0

      using 12 subtractions, 4 shifts, 2 small divisions and 1 small multiplication.
    */

   /* r1 - r4 */
   if ((err = mp_sub(&w1, &w4, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3 - r0 */
   if ((err = mp_sub(&w3, &w0, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r1/2 */
   if ((err = mp_div_2(&w1, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3/2 */
   if ((err = mp_div_2(&w3, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r2 - r0 - r4 */
   if ((err = mp_sub(&w2, &w0, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sub(&w2, &w4, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r1 - r2 */
   if ((err = mp_sub(&w1, &w2, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3 - r2 */
   if ((err = mp_sub(&w3, &w2, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r1 - 8r0 */
   if ((err = mp_mul_2d(&w0, 3, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sub(&w1, &tmp1, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3 - 8r4 */
   if ((err = mp_mul_2d(&w4, 3, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sub(&w3, &tmp1, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* 3r2 - r1 - r3 */
   if ((err = mp_mul_d(&w2, 3uL, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sub(&w2, &w1, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_sub(&w2, &w3, &w2)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r1 - r2 */
   if ((err = mp_sub(&w1, &w2, &w1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3 - r2 */
   if ((err = mp_sub(&w3, &w2, &w3)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r1/3 */
   if ((err = mp_div_3(&w1, &w1, NULL)) != MP_OKAY) {
      goto LBL_ERR;
   }
   /* r3/3 */
   if ((err = mp_div_3(&w3, &w3, NULL)) != MP_OKAY) {
      goto LBL_ERR;
   }

   /* at this point shift W[n] by B*n */
   if ((err = mp_lshd(&w1, 1*B)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_lshd(&w2, 2*B)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_lshd(&w3, 3*B)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_lshd(&w4, 4*B)) != MP_OKAY) {
      goto LBL_ERR;
   }

   if ((err = mp_add(&w0, &w1, b)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&w2, &w3, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&w4, &tmp1, &tmp1)) != MP_OKAY) {
      goto LBL_ERR;
   }
   if ((err = mp_add(&tmp1, b, b)) != MP_OKAY) {
      goto LBL_ERR;
   }

LBL_ERR:
   mp_clear_multi(&w0, &w1, &w2, &w3, &w4, &a0, &a1, &a2, &tmp1, NULL);
   return err;
}

#endif