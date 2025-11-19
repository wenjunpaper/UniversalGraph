#ifndef _GSUM_H
#define _GSUM_H
#include "DetectorAbstract.h"
#include <cmath>
#include <math.h>

using g_func = double (*)(double);
using g2_func = double (*)(double,double);
double Cardinality(double x) {
	return x > 0 ? 1 : 0;
}
double Sum(double x)
{
	return x >= 0 ? x : 0;
}
double Entropy(double x) {
	return (x > 0) ? x * log2(x) : 0;
}
double Sqr(double x)
{
	return x > 0 ? x * x : 0;
}

#endif