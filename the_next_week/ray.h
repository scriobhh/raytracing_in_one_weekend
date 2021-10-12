#ifndef RAYH
#define RAYH

#include "vec3.h"

class ray
{
public:
	ray() {}
	ray(const point& a, const point& b, float ti = 0.0) { A = a; B = b; _time = ti; }
	point origin() const	{ return A; }
	point direction() const	{ return B; }
	float time() const 		{ return _time; }
	point point_at_parameter(float t) const { return A + t*B; }

	point A;
	point B;
	float _time;
};

#endif
