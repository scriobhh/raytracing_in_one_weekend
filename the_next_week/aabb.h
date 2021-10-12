#ifndef AABBH
#define AABBH

// stands for 'axis aligned bounding box'
// it is a box made up of axis-aligned planes which acts as a boundary or container for the objects you want to render
// while rendering: when you check if a ray hits an object, for each ray you have to check every individual object in the scene to see if the ray hit it
// if you put a group of objects inside an aabb, then when checking if a ray hits an object, you first check if the ray hits the objects' surrounding aab, that way you can skip entire groupings of objects when looping over objects in the hit detection code
// ----
// an aabb is made up of 3 groups where each group has 2 parallel planes which are aligned with a pair of axes
// the 3 groups of planes are defined by the equations:
// x = x0, x = x1
// y = y0, y = y1
// z = z0, z = z1
// the area where all 3 of these pairs of planes overlap is our box (our aabb)
// the reason these planes are aligned with the axes is because this simplifies and speeds up the hit detection math
// ----
// aabbs are typically used in ray tracers to create a bvh (bounding volume hierarchy) which is a tree structure of aabbs that encompass other aabbs until the leaf nodes which are scene objects
// the use of a tree structure with aab hit detection allows us to turn the rendering object hit detection loop into logarithmic time
class aabb
{
public:
	aabb() {}
	// _min and _max are opposite corners of the aabb
	aabb(const point& a, const point& b) { _min = a; _max = b; }

	point min() const { return _min; }
	point max() const { return _max; }

	bool hit(const ray& r, float t_min, float t_max) const
	{
		for(int i = 0;
			i < 3;
			i++)
		{
			// a ray is p(t) = A + t*B where A is the ray origin, B is the direction, t is the distance along the ray
			// taking the ray equation and the equations above the aabb class definition (x = x0, x = x1, etc.) we can solve the intersection point with each plane with a 1 dimensional equation of the line
			// e.g. for the x = x0 plane
			// x0 = Ax + t0*Bx
			// where Ax is the x component of A, Bx is the x component of B, t0 is the t value for the point of intersection with the plane
			// re-arranging this gives:
			// t0 = (x0-Ax) / Bx

			// we do the same for t1 with x1

			// t0 is the intersection point closer to the origin, t1 is the intersection point further from the origin

			// for our ray to intersect the aabb, the t0 and t1 for each pair of planes need to overlap
			// the method for checking this:
			// t0 and t1 are the distance along the ray that the intersection happens
			// we keep track of the largest t0 as t_min and the smallest t1 as t_max across all 3 axes
			// if the largest t_min is smaller than the smallest t_max then the ray intersects the aabb

			// there are some issues with the line equation:
			// if Bx is negative (negative direction for x component):
			//   -t0 and t1 can end up backwards e.g. (7,3) instead of (3,7)
			//   -to handle this we can do t0 = fmin(t0, t1); t1=fmax(t0,t1); (except replace t0,t1 in the arguments with the actual calculation for t0 and t1
			//   -to handle this we can also check if the direction is negative or check if t0>t1 and swap t0 and t1 if it is (this is more readable and probably better performing)
			// if Bx is 0 we get problems:
			//   -this means there is no x component to the ray, so the ray is parallel to the planes and never intersects them
			//   -floating point will give us back -infinity or +infinity depending on the signs of (x0-Ax) and (x1-Ax)
			//   -we can use the signs of (x0-Ax) and (x1-Ax) to tell if the ray is between the planes or not (if the ray is between the planes then one will be + and one will be -, otherwise they will both be the same sign)
			// if Bx is 0 and either (x0-Ax)=0 or (x1-Ax)=0, then floating point gives us NaN:
			//   -this means the ray origin is on one of the planes and the x component of the ray is 0
			//   -we don't handle this in this code with the assumption that it won't happen very often

			float inverse_dir = 1.0f / r.direction()[i];
			float t0 = (min()[i] - r.origin()[i]) * inverse_dir;
			float t1 = (max()[i] - r.origin()[i]) * inverse_dir;
			if(inverse_dir < 0.0f)
				std::swap(t0, t1);
			t_min = t0 > t_min ? t0 : t_min;
			t_max = t1 < t_max ? t1 : t_max;
			if(t_max <= t_min)  // this accounts for normal cases and for cases where r.direction()[i] = 0
				return false;
		}
		return true;
	}

	point _min;
	point _max;
};

// create a large aabb that encompasses two smaller aabbs
aabb surrounding_box(aabb box0, aabb box1)
{
	point small(fmin(box0.min().x(), box1.min().x()),
				fmin(box0.min().y(), box1.min().y()),
				fmin(box0.min().z(), box1.min().z()));
	point big(fmax(box0.max().x(), box1.max().x()),
			  fmax(box0.max().y(), box1.max().y()),
			  fmax(box0.max().z(), box1.max().z()));
	return aabb(small, big);
}

#endif
