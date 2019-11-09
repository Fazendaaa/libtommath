#include "tommath_private.h"
#ifdef S_MP_MUL_COMBA_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/* Fast (comba) multiplier
 *
 * This is the fast column-array [comba] multiplier.  It is
 * designed to compute the columns of the product first
 * then handle the carries afterwards.  This has the effect
 * of making the nested loops that compute the columns very
 * simple and schedulable on super-scalar processors.
 *
 * This has been modified to produce a variable number of
 * digits of output so if say only a half-product is required
 * you don't have to compute the upper half (a feature
 * required for fast Barrett reduction).
 *
 * Based on Algorithm 14.12 on pp.595 of HAC.
 *
 */
mp_err s_mp_mul_comba(const mp_int *a, const mp_int *b, mp_int *c, int digs)
{
   int      oldused, pa, ix;
   mp_err   err;
   mp_digit c0, c1, c2;
   mp_int   tmp, *c_;

   /* prepare the destination */
   err = (MP_ALIAS(a, c) || MP_ALIAS(b, c))
         ? mp_init_size((c_ = &tmp), digs)
         : mp_grow((c_ = c), digs);
   if (err != MP_OKAY) {
      return err;
   }

   /* number of output digits to produce */
   pa = MP_MIN(digs, a->used + b->used);

   /* clear the carry */
   c0 = c1 = c2 = 0;
   for (ix = 0; ix < pa; ix++) {
      int tx, ty, iy, iz;

      /* get offsets into the two bignums */
      ty = MP_MIN(b->used-1, ix);
      tx = ix - ty;

      /* this is the number of times the loop will iterrate, essentially
         while (tx++ < a->used && ty-- >= 0) { ... }
       */
      iy = MP_MIN(a->used-tx, ty+1);

      /* execute loop
       *
       * Give the autovectorizer a hint! this might not be necessary.
       * I don't think the generated code will be particularily good here,
       * if we will use full width digits the masks will go away.
       */
      for (iz = 0; iz + 3 < iy;) {
         mp_word w = (mp_word)c0 + ((mp_word)a->dp[tx + iz] * (mp_word)b->dp[ty - iz]);
         c0 = (mp_digit)(w & MP_MASK);
         w = (mp_word)c1 + (w >> MP_DIGIT_BIT);
         c1 = (mp_digit)(w & MP_MASK);
         c2 += (mp_digit)(w >> MP_DIGIT_BIT);
         ++iz;

         w = (mp_word)c0 + ((mp_word)a->dp[tx + iz] * (mp_word)b->dp[ty - iz]);
         c0 = (mp_digit)(w & MP_MASK);
         w = (mp_word)c1 + (w >> MP_DIGIT_BIT);
         c1 = (mp_digit)(w & MP_MASK);
         c2 += (mp_digit)(w >> MP_DIGIT_BIT);
         ++iz;

         w = (mp_word)c0 + ((mp_word)a->dp[tx + iz] * (mp_word)b->dp[ty - iz]);
         c0 = (mp_digit)(w & MP_MASK);
         w = (mp_word)c1 + (w >> MP_DIGIT_BIT);
         c1 = (mp_digit)(w & MP_MASK);
         c2 += (mp_digit)(w >> MP_DIGIT_BIT);
         ++iz;

         w = (mp_word)c0 + ((mp_word)a->dp[tx + iz] * (mp_word)b->dp[ty - iz]);
         c0 = (mp_digit)(w & MP_MASK);
         w = (mp_word)c1 + (w >> MP_DIGIT_BIT);
         c1 = (mp_digit)(w & MP_MASK);
         c2 += (mp_digit)(w >> MP_DIGIT_BIT);
         ++iz;
      }

      /* execute rest of loop */
      for (; iz < iy;) {
         mp_word w = (mp_word)c0 + ((mp_word)a->dp[tx + iz] * (mp_word)b->dp[ty - iz]);
         c0 = (mp_digit)(w & MP_MASK);
         w = (mp_word)c1 + (w >> MP_DIGIT_BIT);
         c1 = (mp_digit)(w & MP_MASK);
         c2 += (mp_digit)(w >> MP_DIGIT_BIT);
         ++iz;
      }

      /* store term */
      c_->dp[ix] = c0;

      /* make next carry */
      c0 = c1;
      c1 = c2;
      c2 = 0;
   }

   /* setup dest */
   oldused  = c_->used;
   c_->used = pa;

   /* clear unused digits [that existed in the old copy of c] */
   s_mp_zero_digs(c_->dp + c_->used, oldused - c_->used);

   mp_clamp(c_);

   if (c_ == &tmp) {
      mp_clear(c);
      *c = *c_;
   }

   return MP_OKAY;
}
#endif