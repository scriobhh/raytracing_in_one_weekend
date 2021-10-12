#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "aabb.h"
#include "assert.h"

// forward declaration
class material;

struct hit_record
{
	float t;
	float u;
	float v;
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
	// bounding_box returns a bool representing if the hitable has a bounding box
	// it constructs an aabb and outputs it to the box argument
	// t0 and t1 are time0 and time1, not t values for rays
	virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};

class hitable_list : public hitable
{
public:
	hitable_list() {}
	hitable_list(hitable **l, int n) { list = l; list_size = n; }
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

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

// TODO NOTE ERROR this code is different from in book, I think there is a bug in the book code so I changed it slightly
bool hitable_list::bounding_box(float t0, float t1, aabb& box) const
{
	if(list_size < 1)
		return false;

	aabb temp_box;
	bool first_box = true;
	for(int i = 0;
		i < list_size;
		i++)
	{
		if(list[i]->bounding_box(t0, t1, temp_box)) // temp_box is an output
		{
			if(first_box)
				box = temp_box;
			else
				box = surrounding_box(box, temp_box);
			first_box = false;
		}
		else
			return false;
	}
	return true;
}

// the bvh tree structure doesn't have a corresponding bvh_tree data type
// instead the head of the tree is just another bvh_node
class bvh_node : public hitable
{
public:
	bvh_node() {}
	bvh_node(hitable **list, int n, float time0, float time1);
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

	// left and right can be any hitable
	// they can be bvh_nodes to continue the tree or other hitables in which case they are leaf nodes
	hitable *left;
	hitable *right;
	aabb box;
};

int box_compare_generic(const void *a, const void *b, const int i)
{
	assert(i < 3);
	aabb box_left, box_right;
	hitable *ah = *(hitable **)a;
	hitable *bh = *(hitable **)b;
	if(!ah->bounding_box(0,0,box_left) || !bh->bounding_box(0,0,box_right))
		std::cerr << "no bounding box in bvh_node constructor\n";
	if(box_left.min()[i] - box_right.min()[i] < 0.0)
		return -1;
	else
		return 1;

}
int box_x_compare(const void *a, const void *b) { return box_compare_generic(a, b, 0); }
int box_y_compare(const void *a, const void *b) { return box_compare_generic(a, b, 1); }
int box_z_compare(const void *a, const void *b) { return box_compare_generic(a, b, 2); }

bvh_node::bvh_node(hitable **list, int n, float time0, float time1)
{
	assert(n > 0);

	// for simplicity's sake the list objects are sorted based on a random axis
	// this sorting will be used to divide up the objects and decide which groupings of objects go into which side of the tree
	int axis = (int)(3*my_rand());
	if(axis == 0)
		qsort(list, n, sizeof(hitable *), box_x_compare);
	else if(axis == 1)
		qsort(list, n, sizeof(hitable *), box_y_compare);
	else if(axis == 2)  // TODO 'if(axis==2)' is my own addition to code, may be a source of bugs since it doesn't match the book code
		qsort(list, n, sizeof(hitable *), box_z_compare);
	
	// n <= 2, the objects are leaf nodes
	if(n == 1)
	{
		// setting left and right to list[0] so that we don't have to check for null pointers anywhere
		left = right = list[0]; 
	}
	else if(n == 2)
	{
		left = list[0];
		right = list[1];
	}
	else // n > 2, the objects are not leaf nodes (this could be optimized to check for n==3 and make one side a leaf node)
	{
		left = new bvh_node(list, n/2, time0, time1);
		right = new bvh_node(list+n/2, n-n/2, time0, time1);
	}
	aabb box_left, box_right;
	if(!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
		std::cerr << "no bounding box in bvh_node constructor\n";
	box = surrounding_box(box_left, box_right);
}

bool bvh_node::bounding_box(float t0, float t1, aabb& b) const
{
	b = box;
	return true;
}

bool bvh_node::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	if(box.hit(r, t_min, t_max))
	{
		// the following 2 calls are recursive
		hit_record left_rec, right_rec;
		bool hit_left = left->hit(r, t_min, t_max, left_rec);
		bool hit_right = right->hit(r, t_min, t_max, right_rec);
		if(hit_left && hit_right)
		{
			if(left_rec.t < right_rec.t)
				rec = left_rec;
			else
				rec = right_rec;
			return true;
		}
		else if(hit_left)
		{
			rec = left_rec;
			return true;
		}
		else if(hit_right)
		{
			rec = right_rec;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

// this is used to get texture co-ordinates from a hitpoint on a sphere
// takes a point on a unit sphere that is centered at the origin (in other words a unit vector...)
// outputs a lattutidue and longitude between 0 and 1 for the point on a unit sphere
// outputs to the u and v arguments
void get_sphere_uv(const point& p, float& u, float& v)
{
	// NOTE atan2 and asin return values in radians
	// 2*pi radians = 360 degress

	// atan2() gives the angle from the positive x axis to a given point
	// the output is in the range (-pi,pi) (note that you can give any point around a circle)
	// we use it to get the longitude (around the y axis)
	float phi = atan2(p.z(), p.x());

	// asin() is the inverse of sin()
	// the output is in the range (-pi/2,pi/2) (note you can only give values that fall on a half circle)
	// since the point is on a unit circle, the inverse sin of p.y() will give us the angle from the sphere's equator to the point p
	float theta = asin(p.y());

	// u and v are normalized co-ordinates (they are between 0 or 1)
	u = 1 - (phi + M_PI) / (2*M_PI); // z=0,x=-1 maps to 0, z=0,x=1 maps to 1
	v = (theta + M_PI/2) / M_PI; // y=1 maps to 1, y=-1 maps to -1
}

class sphere : public hitable
{
public:
	sphere() {}
	sphere(point cen, float r, material *m) : center(cen), radius(r), mtrl(m) {};
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

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
		float temp = (-b - sqrt(discriminant))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			// (rec.hit_point - center) gives vector from origin that points in same direction as center to rec.hit_point
			// (rec.hit_point - center) has a magnitude of radius, dividing it by radius gives a unit vector
			rec.normal = (rec.hit_point - center) / radius;
			rec.mat_ptr = mtrl;
			get_sphere_uv(rec.normal, rec.u, rec.v);  // functions outputs to rec.u and rec.v
			return true;
		}
		temp = (-b + sqrt(discriminant))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			rec.normal = (rec.hit_point - center) / radius;
			rec.mat_ptr = mtrl;
			get_sphere_uv(rec.normal, rec.u, rec.v);  // functions outputs to rec.u and rec.v
			return true;
		}
	}
	return false;
}

bool sphere::bounding_box(float t0, float t1, aabb& box) const
{
	box = aabb(center - point(radius, radius, radius),
			center + point(radius, radius, radius));
	return true;
}

class moving_sphere : public hitable
{
public:
	moving_sphere() {}
	moving_sphere(point cen0, point cen1, float t0, float t1, float r, material *m)
		: center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mtrl(m) {};
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;
	point center(float time) const;

	point center0, center1;
	float time0, time1;
	float radius;
	material *mtrl;
};

point moving_sphere::center(float time) const
{
	return center0 + ((time-time0) / (time1-time0)) * (center1-center0);
}

bool moving_sphere::bounding_box(float t0, float t1, aabb& box) const
{
	aabb t0_box = aabb(center(t0) - point(radius,radius,radius),
				center(t0) + point(radius,radius,radius));
	aabb t1_box = aabb(center(t1) - point(radius,radius,radius),
				center(t1) + point(radius,radius,radius));
	box = surrounding_box(t0_box, t1_box);
	return true;
}

bool moving_sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	point oc = r.origin() - center(r.time());
	float a = dot(r.direction(), r.direction());
	float b = 2.0*dot(oc, r.direction());
	float c = dot(oc, oc) - radius*radius;
	float discriminant = b*b - 4*a*c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			// (rec.hit_point - center) gives vector from origin that points in same direction as center to rec.hit_point
			// (rec.hit_point - center) has a magnitude of radius, dividing it by radius gives a unit vector
			rec.normal = (rec.hit_point - center(r.time())) / radius;
			rec.mat_ptr = mtrl;
			return true;
		}
		temp = (-b + sqrt(discriminant))/(2.0*a);
		if (temp < t_max && temp > t_min)
		{
			rec.t = temp;
			rec.hit_point = r.point_at_parameter(rec.t);
			rec.normal = (rec.hit_point - center(r.time())) / radius;
			rec.mat_ptr = mtrl;
			return true;
		}
	}
	return false;
	
}

// a rectangle is defined by a plane
// for an xy plane, the equation of the plane is z=k
// the boundaries of the rectangle are defined by 4 lines on the plane z=k
// the axis-aligned planes x=x0, x=x1, y=y0 and y=y1 are used to define the boundaries of the rectangle (the lines of the rectangle are the intersections of the planes x=x0, x=x1, y=y0, y=y1 with the plane z=k)
// in this rectangle implementation case all planes are always axis-aligned (rotation will be handled by instancing)
class xy_rect : public hitable
{
public:
	xy_rect() {}
	xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, material *mat)
		: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mat_ptr(mat) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		// the k-0.0001 and k+0.0001 is to create a small amount of padding for the aabb
		box = aabb(point(x0, y0, k-0.0001), point(x1, y1, k+0.0001));
		return true;
	}
	material *mat_ptr;
	float k;				// the plane of the rectangle
	float x0, x1, y0, y1;	// the planes that define the boundaries of the rectangle
};

bool xy_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	// the ray equation is r = A + t*B
	// finding where the ray intersects the z=k plane is done with the z components of the A and B vectors
	// rz = Az + t*Bz where rz, Az and Bz are the z components of r, A and B
	// since we are looking for the point where r intersects with z=k, we can replace rz with k 
	// k = Az + t*Bz
	// since we have Az, Bz and k we can re-arrange this to find the value of t
	// t = (k-Az) / Bz
	// since we already have t, we can use the above to find the x and y values at the point t on ray r
	// x = Ax + t*Bx
	// y = Ay + t*By
	// we then check that the x component is within the bounds of x0, x1 and the y component is within y0 and y1
	float t = (k-r.origin().z()) / r.direction().z();
	if(t<t_min || t>t_max) return false;
	float x = r.origin().x() + t*r.direction().x();
	float y = r.origin().y() + t*r.direction().y();
	if(x<x0 || x>x1 || y<y0 || y>y1) return false;
	// u and v are texture co-ordinates
	rec.u = (x-x0)/(x1-x0);
	rec.v = (y-y0)/(y1-y0);
	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.hit_point = r.point_at_parameter(t);
	rec.normal = point(0,0,1);
	return true;
}

class xz_rect : public hitable
{
public:
	xz_rect() {}
	xz_rect(float _x0, float _x1, float _z0, float _z1, float _k, material *mat)
		: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mat_ptr(mat) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		// the k-0.0001 and k+0.0001 is to create a small amount of padding for the aabb
		box = aabb(point(x0, k-0.0001, z0), point(x1, k+0.0001, z1));
		return true;
	}
	material *mat_ptr;
	float k;				// the plane of the rectangle
	float x0, x1, z0, z1;	// the planes that define the boundaries of the rectangle
};

bool xz_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = (k-r.origin().y()) / r.direction().y();
	if(t<t_min || t>t_max) return false;
	float x = r.origin().x() + t*r.direction().x();
	float z = r.origin().z() + t*r.direction().z();
	if(x<x0 || x>x1 || z<z0 || z>z1) return false;
	// u and v are texture co-ordinates
	rec.u = (x-x0)/(x1-x0);
	rec.v = (z-z0)/(z1-z0);
	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.hit_point = r.point_at_parameter(t);
	rec.normal = point(0,1,0);
	return true;
}

class yz_rect : public hitable
{
public:
	yz_rect() {}
	yz_rect(float _y0, float _y1, float _z0, float _z1, float _k, material *mat)
		: y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mat_ptr(mat) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		// the k-0.0001 and k+0.0001 is to create a small amount of padding for the aabb
		box = aabb(point(k-0.0001, y0, z0), point(k+0.0001, y1, z1));
		return true;
	}
	material *mat_ptr;
	float k;				// the plane of the rectangle
	float y0, y1, z0, z1;	// the planes that define the boundaries of the rectangle
};

bool yz_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = (k-r.origin().x()) / r.direction().x();
	if(t<t_min || t>t_max) return false;
	float y = r.origin().y() + t*r.direction().y();
	float z = r.origin().z() + t*r.direction().z();
	if(y<y0 || y>y1 || z<z0 || z>z1) return false;
	// u and v are texture co-ordinates
	rec.u = (y-y0)/(y1-y0);
	rec.v = (z-z0)/(z1-z0);
	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.hit_point = r.point_at_parameter(t);
	rec.normal = point(1,0,0);
	return true;
}

// this class wraps another hitable object in order to flip the normal of the hit_record
// not sure if there is a better way to do this with this architecture... using a class for this just feels wrong :'(
class flip_normals : public hitable
{
public:
	flip_normals(hitable *p) : ptr(p) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const
	{
		if(ptr->hit(r, t_min, t_max, rec))
		{
			rec.normal = -rec.normal;
			return true;
		}
		else
			return false;
	}
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		return ptr->bounding_box(t0, t1, box);
	}
	hitable *ptr;
};

class box : public hitable
{
public:
	box() {}
	box(const point& p0, const point& p1, material *mat_ptr);
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		box = aabb(p_min, p_max);
		return true;
	}
	point p_min, p_max;
	hitable *list_ptr;
};

box::box(const point& p0, const point& p1, material *mat_ptr)
{
	p_min = p0;
	p_max = p1;
	hitable **list = new hitable*[6];
	// xy planes (front/back)
	list[0] = 					new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mat_ptr);
	list[1] = new flip_normals(	new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mat_ptr));
	// xz planes (top/bottom)
	list[2] = 					new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), mat_ptr);
	list[3] = new flip_normals(	new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), mat_ptr));
	// yz planes (left/right)
	list[4] = 					new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), mat_ptr);
	list[5] = new flip_normals(	new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), mat_ptr));
	list_ptr = new hitable_list(list, 6);
}

bool box::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	return list_ptr->hit(r, t_min, t_max, rec);
}

// instances: in ray tracing, an 'instance' is a geometric primitive that has been moved or rotated
// it is typically done by moving the rays that his the geometric primitive, instead of moving the geometric primitive itself

// in this architecture we will handle translating an object by using a wrapper class
// this wraper class moves the ray that hits the object
// you could also change the points of the wrapped object to translate it instead, but according to the book this method of moving the rays instead of moving the object is almost always used for ray tracers
class translate : public hitable
{
public:
	translate(hitable *p, const point& displacement) : ptr(p), offset(displacement) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;
	hitable *ptr;
	point offset;
};

bool translate::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	// moved_r is a new temporary ray where the original ray's origin gets moved in the opposite direction of the offset
	// e.g. if a sphere is at (1,0,0) and it is translated by (2,0,0) to a new 'position' of (3,0,0)
	// this means that a ray that is sent to (3,0,0) will hit the moved sphere
	// so the ray should be be moved by (-2,0,0) to compensate for the fact that the sphere is still actually at (1,0,0)
	ray moved_r(r.origin() - offset, r.direction(), r.time());
	if(ptr->hit(moved_r, t_min, t_max, rec))
	{
		rec.hit_point += offset;
		return true;
	}
	else
		return false;
}

bool translate::bounding_box(float t0, float t1, aabb& box) const
{
	if(ptr->bounding_box(t0, t1, box))
	{
		box = aabb(box.min()+offset, box.max()+offset);
		return true;
	}
	else
		return false;
}

// in this architecture, rotation of an object is done with a wrapper class
// this works just like the translate class, it moves the incoming ray instead of moving the underlying object
// the maths for rotating points about the y axis is:
// x' =  cos(theta)*x + sin(theta)*z
// z' = -sin(theta)*x + cos(theta)*z
// where x' is the new x position, z' is the new z position and theta is the angle to rotate by
// to reverse the direction of rotation:
// cos(-theta) = cos(theta)
// sin(-theta) = -sin(theta)
// so that gives us:
// x' = cos(theta)*x - sin(theta)*z
// z' = sin(theta)*x + cos(theta)*z
class rotate_y : public hitable
{
public:
	// angle is in degrees
	rotate_y(hitable *p, float angle);
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		// TODO should change this so it only sets box after checking has_box ?
		box = b_box;
		return has_box;
	}
	hitable *ptr;
	float sin_theta;
	float cos_theta;
	bool has_box;
	aabb b_box;
};

rotate_y::rotate_y(hitable *p, float angle) : ptr(p)
{
	// the '180.' is short for '180.0'
	float radians = (M_PI / 180.) * angle;
	sin_theta = sin(radians);
	cos_theta = cos(radians);
	// TODO should this be changed to ptr->bounding_box(t0, t1, b_box); ?
	has_box = ptr->bounding_box(0, 1, b_box);

	// find the bounding box:
	// min and max store the min and max corners of the bounding box (they are updated with actual values below)
	point min(FLT_MAX, FLT_MAX, FLT_MAX);
	point max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			for(int k = 0; k < 2; k++)
			{
				// i,j,k are only going to be 0 or 1
				// so its' guaranteed that one of the i* and (1-i)* will be 1 and one of them will be 0
				float x = i*b_box.max().x() + (1-i)*b_box.min().x();
				float y = j*b_box.max().y() + (1-j)*b_box.min().y();
				float z = k*b_box.max().z() + (1-k)*b_box.min().z();
				// rotate x and z about the y axis
				float new_x = cos_theta*x + sin_theta*z;
				float new_z = -sin_theta*x + cos_theta*z;
				// update the min and max points
				point tester(new_x, y, new_z);
				for(int c = 0;
					c < 3;
					c++)
				{
					if(tester[c] > max[c])
						max[c] = tester[c];
					if(tester[c] < min[c])
						min[c] = tester[c];
				}
			}
		}
	}
	b_box = aabb(min, max);
}

bool rotate_y::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	// the origin and direction of the ray are rotated about the y axis and used to create a new temporary ray
	// the new temporary ray is used to check if it hits the object
	// note that the rotated ray is rotated in the opposite direction than the object is supposed to be rotated
	// rotating an object by 20 degrees is equivalent to rotating the camera by -20 degrees
	point new_origin = r.origin();
	point new_direction = r.direction();
	// x and z co-ordinates are rotated about the y axis
	new_origin[0] = cos_theta*r.origin()[0] - sin_theta*r.origin()[2];
	new_origin[2] = sin_theta*r.origin()[0] + cos_theta*r.origin()[2];
	new_direction[0] = cos_theta*r.direction()[0] - sin_theta*r.direction()[2];
	new_direction[2] = sin_theta*r.direction()[0] + cos_theta*r.direction()[2];

	ray rotated_r(new_origin, new_direction, r.time());
	if(ptr->hit(rotated_r, t_min, t_max, rec))
	{
		// the hit_point and normal for the hit_record are changed to their positions on the rotated object
		// note that these are rotated in the opposite direction than the camera was
		point new_hit_point = rec.hit_point;
		point new_normal = rec.normal;
		new_hit_point[0] = cos_theta*rec.hit_point[0] + sin_theta*rec.hit_point[2];
		new_hit_point[2] = -sin_theta*rec.hit_point[0] + cos_theta*rec.hit_point[2];
		new_normal[0] = cos_theta*rec.normal[0] + sin_theta*rec.normal[2];
		new_normal[2] = -sin_theta*rec.normal[0] + cos_theta*rec.normal[2];
		rec.hit_point = new_hit_point;
		rec.normal = new_normal;
		return true;
	}
	else
		return false;
}

// the maths for rotating about the z axis is:
// x' = cos(theta)*x - sin(theta)*y
// y' = sin(theta)*x + cos(theta)*y
// where x' is the new x position, y' is the new y position and theta is the angle to rotate by
// to reverse the direction of rotation:
// cos(-theta) = cos(theta)
// sin(-theta) = -sin(theta)

// the maths for rotating about the x axis is:
// y' = cos(theta)*y - sin(theta)*z
// z' = sin(theta)*y + cos(theta)*z
// where y' is the new y position, z' is the new z position and theta is the angle to rotate by
// to reverse the direction of rotation:
// cos(-theta) = cos(theta)
// sin(-theta) = -sin(theta)

// volumes (fog/smoke object)
class constant_medium : public hitable
{
public:
	constant_medium(hitable *b, float d, material *i)
		: boundary(b), density(d), phase_function(i) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		return boundary->bounding_box(t0, t1, box);
	}

	hitable *boundary;
	float density;
	material *phase_function;
};

bool constant_medium::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	hit_record rec1, rec2;
	if(boundary->hit(r, -FLT_MAX, FLT_MAX, rec1))
	{
		if(boundary->hit(r, rec1.t+0.0001, FLT_MAX, rec2))
		{
			if(rec1.t < t_min) rec1.t = t_min;
			if(rec2.t > t_max) rec2.t = t_max;
			if(rec1.t >= rec2.t) return false;
			if(rec1.t < 0) rec1.t = 0;
			float distance_inside_boundary = (rec2.t - rec1.t)*r.direction().length();
			float hit_distance = -(1/density)*log(my_rand());
			if(hit_distance < distance_inside_boundary)
			{
				rec.normal = point(1,0,0); // this is arbitrary (its' from the book)
				rec.mat_ptr = phase_function;
				return true;
			}
		}
	}
	return false;
}

#endif
