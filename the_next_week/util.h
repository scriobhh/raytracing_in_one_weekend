#ifndef UTILH
#define UTILH

#include <stdlib.h>

// returns random doubles in the range: 0 <= val < 1
inline double my_rand()
{
	int r = rand() % 99;
	return static_cast<double>(r) / 100.0;
}

point random_in_unit_sphere()
{
	point p;
	do
	{
		// produces (x, y, z) values that range from almost -1 to almost 1
		p = 2.0*point(my_rand(), my_rand(), my_rand()) - point(1,1,1);
	} while (p.squared_length() >= 1.0);
	return p;
}

#endif
