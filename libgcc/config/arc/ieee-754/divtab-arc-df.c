/* Copyright (C) 2008-2025 Free Software Foundation, Inc.
   Contributor: Joern Rennecke <joern.rennecke@embecosm.com>
		on behalf of Synopsys Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* We use a polynom similar to a Tchebycheff polynom to get an initial
   seed, and then use a newton-raphson iteration step to get an
   approximate result
   If this result can't be rounded to the exact result with confidence, we
   round to the value between the two closest representable values, and
   test if the correctly rounded value is above or below this value.

   Because of the Newton-raphson iteration step, an error in the seed at X
   is amplified by X.  Therefore, we don't want a Tchebycheff polynom
   or a polynom that is close to optimal according to the maximum norm
   on the errro of the seed value; we want one that is close to optimal
   according to the maximum norm on the error of the result, i.e. we
   want the maxima of the polynom to increase linearily.
   Given an interval [X0,X2) over which to approximate,
   with X1 := (X0+X2)/2,  D := X1-X0, F := 1/D, and S := D/X1 we have,
   like for Tchebycheff polynoms:
   P(0) := 1
   but then we have:
   P(1) := X + S*D
   P(2) := 2 * X^2 + S*D * X - D^2
   Then again:
   P(n+1) := 2 * X * P(n) - D^2 * P (n-1)
 */

static long double merr = 42.;

double
err (long double a0, long double a1, long double x)
{
  long double y0 = a0 + (x-1)*a1;

  long double approx = 2. * y0 - y0 * x * y0;
  long double true = 1./x;
  long double err = approx - true;

  if (err <= -1./65536./16384.)
    printf ("ERROR EXCEEDS 1 ULP %.15f %.15f %.15f\n",
	    (double)x, (double)approx, (double)true);
  if (merr > err)
    merr = err;
  return err;
}

int
main (void)
{
  long double T[5]; /* Taylor polynom */
  long double P[5][5];
  int i, j;
  long double X0, X1, X2, S;
  long double inc = 1./64;
  long double D = inc*0.5;
  long i0, i1, i2, io;

  memset (P, 0, sizeof (P));
  P[0][0] = 1.;
  for (i = 1; i < 5; i++)
    P[i][i] = 1 << i-1;
  P[2][0] = -D*D;
  for (X0 = 1.; X0 < 2.; X0 += inc)
    {
      X1 = X0 + inc * 0.5;
      X2 = X0 + inc;
      S = D / X1;
      T[0] = 1./X1;
      for (i = 1; i < 5; i++)
	T[i] = T[i-1] * -T[0];
#if 0
      printf ("T %1.8f %f %f %f %f\n", (double)T[0], (double)T[1], (double)T[2],
(double)T[3], (double)T[4]);
#endif
      P[1][0] = S*D;
      P[2][1] = S*D;
      for (i = 3; i < 5; i++)
	{
	  P[i][0] = -D*D*P[i-2][0];
	  for (j = 1; j < i; j++)
	    P[i][j] = 2*P[i-1][j-1]-D*D*P[i-2][j];
	}
#if 0
      printf ("P3 %1.8f %f %f %f %f\n", (double)P[3][0], (double)P[3][1], (double)P[3][2],
(double)P[3][3], (double)P[3][4]);
      printf ("P4 %1.8f %f %f %f %f\n", (double)P[4][0], (double)P[4][1], (double)P[4][2],
(double)P[4][3], (double)P[4][4]);
#endif
      for (i = 4; i > 1; i--)
	{
	  long double a = T[i]/P[i][i];

	  for (j = 0; j < i; j++)
	    T[j] -= a * P[i][j];
	}
#if 0
      printf ("A %1.8f %f %f\n", (double)T[0], (double)T[1], (double)T[2]);
#endif
#if 0
      i2 = T[2]*1024;
      long double a = (T[2]-i/1024.)/P[2][2];
      for (j = 0; j < 2; j++)
	T[j] -= a * P[2][j];
#else
      i2 = 0;
#endif
	  long double T0, Ti1;
      for (i = 0, i0 = 0; i < 4; i++)
	{

	  i1 = T[1]*4096. + i0 / (long double)(1 << 20) - 0.5;
	  i1 = - (-i1 & 0x0fff);
	  Ti1 = ((unsigned)(-i1 << 20) | i0) /-(long double)(1LL<<32LL);
	  T0 = T[0] - (T[1]-Ti1)/P[1][1] * P[1][0] - (X1 - 1) * Ti1;
	  i0 = T0 * 1024 * 1024 + 0.5;
	  i0 &= 0xfffff;
	}
#if 0
      printf ("A %1.8f %f %f\n", (double)T[0], (double)T[1], (double)T[2]);
#endif
      io = (unsigned)(-i1 << 20) | i0;
      long double A1 = (unsigned)io/-65536./65536.;
      long double A0 =  (unsigned)(io << 12)/65536./65536.;
      long double Xm0 = 1./sqrt (-A1);
      long double Xm1 = 0.5+0.5*-A0/A1;
#if 0
      printf ("%f %f %f %f\n", (double)A0, (double)A1, (double) Ti1, (double)X0);
      printf ("%.12f %.12f %.12f\n",
	      err (A0, A1, X0), err (A0, A1, X1), err (A0, A1, X2));
      printf ("%.12f %.12f\n", (double)Xm0, (double)Xm1);
      printf ("%.12f %.12f\n", err (A0, A1, Xm0), err (A0, A1, Xm1));
#endif
      printf ("\t.long 0x%x\n", io);
   }
#if 0
  printf ("maximum error: %.15f %x %f\n", (double)merr, (unsigned)(long long)(-merr * 65536 * 65536), (double)log(-merr)/log(2));
#endif
  return 0;
}
