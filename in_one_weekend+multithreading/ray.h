#ifndef RAYH
#define RAYH

#include "vec3.h"

class ray
{
public:
	ray() {}
	ray(const point& a, const point& b) { A = a; B = b; }
	point origin() const		{ return A; }
	point direction() const	{ return B; }
	point point_at_parameter(float t) const { return A + t*B; }

	point A;
	point B;
};

#endif
