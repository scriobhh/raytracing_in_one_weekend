#ifndef HITABLEH
#define HITABLEH

#include "ray.h"

class material;

struct hit_record
{
	float t;
	point hit_point;
	point normal;
	material *mat_ptr;
};

class hitable
{
public:
	// tmin and tmax are to put boundaries on the min and max distance from the origin
	// of a ray that the hit will count
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};

class hitable_list : public hitable
{
public:
	hitable_list() {}
	hitable_list(hitable **l, int n) { list = l; list_size = n; }
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	hitable **list;
	int list_size;
};

bool hitable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;  // prevents rendering anything behind the closest object
	for (int i = 0;
		i < list_size;
		i++)
	{
		if (list[i]->hit(r, t_min, closest_so_far, temp_rec))
		{
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

class sphere : public hitable
{
public:
	sphere() {}
	sphere(point cen, float r, material *m) : center(cen), radius(r), mtrl(m) {};
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	point center;
	float radius;
	material *mtrl;
};

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	// equation for a sphere at (0, 0, 0) with radius R is:
	// x*x + y*y + z*z = R*R
	// in English:
	// "for any (x, y, z) if x*x + y*y + z*Z = R*R then (x, y, z) is on the sphere"
	// equation for a sphere with center c and radius R is:
	// (x-cx)*(x-cx) + (y-cy)*(y-cy) + (z-cz)*(z-cz) = R*R

	// the vector of distance from center C=(cx,cy,cz) to point p=(x,y,z) is (p - C)
	// dot((p-C),(p-C)) = (x-cx)*(x-cx) + (y-cy)*(y-cy) + (z-cz)*(z-cz)
	// the equation of a sphere in vector form is:
	// dot((p-C),(p-C)) = R*R

	// a ray p(t) is represented by the equation (A + t*B)
	// we want to see if the ray satisfies the sphere equation anywhere on the ray
	// the equation for this is:
	// dot((p(t)-C),(p(t)-C)) = R*R

	// quadratic equations are of the form:
	// ax^2 + bx + c = 0

	// if you expand p(t) to (A + t*B) and move everything to the left hand side you get this quadratic equation:
	// t*t*dot(B,B) + 2*t*dot(B,A-C) + dot(A-C,A-C) - R*R = 0
	// where:
	// x = t
	// a = dot(B,B)
	// b = 2*dot(B,A-C)
	// c = dot(A-C,A-C) - R*R

	// you can figure out the number of roots by using the discriminant of the quadratic equation
	// discriminant of a quadratic:
	// b*b - 4*a*c

	// the meaning of solving the quadratic of a ray intersecting a sphere:
	// 0 roots: the ray doesn't intersect the sphere
	// 1 root:  the ray intersects the sphere once
	// 2 roots: the ray intersects the sphere twice

	// NOTE: in the book some redundant 2's that cancel each other out are removed from the maths below

	point oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = 2.0*dot(oc, r.direction());
	float c = dot(oc, oc) - radius*radius;
	float discriminant = b*b - 4*a*c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(b*b-4*a*c))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			// (rec.hit_point - center) gives vector from origin that points in same direction as center to rec.hit_point
			// (rec.hit_point - center) has a magnitude of radius, dividing it by radius gives a unit vector
			rec.normal = (rec.hit_point - center) / radius;
			rec.mat_ptr = mtrl;
			return true;
		}
		temp = (-b + sqrt(b*b-4*a*c))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			rec.normal = (rec.hit_point - center) / radius;
			rec.mat_ptr = mtrl;
			return true;
		}
	}
	return false;
}

#endif
