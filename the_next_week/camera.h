#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
// thie define is necessary to use M_PI from math.h
#define _USE_MATH_DEFINES
#include <math.h>
#include "util.h"

point random_in_unit_disk()
{
	point p;
	do
	{
		// produces a random point in the range (1,1,0) to (-1,-1,0)
		p = 2.0*point(my_rand(), my_rand(), 0) - point(1.0,1.0,0.0);
	} while(dot(p,p) >= 1.0); // selects points that fall within a sphere of radius 1, see the equation of a sphere if this is confusing
	return p;
}

class camera
{
public:
	camera() {}
	// view_fov is top to bottom fov in degrees
	// viewup's relative position from (0,0,0) is used to calculate the roll (as in pitch, yaw and roll) of the camera
	// aperture determines how big the 'hole' that lets light through into the camera is (e.g. how wide the lens is), a wider hole lets in more unfocused light which means a less focused image
	// aspect is image width / image height
	camera(point lookfrom, point lookat, point view_up, float view_fov, float aspect, float aperture, float focus_dist, float t0, float t1)
	{
		time0 = t0;
		time1 = t1;
		lens_radius = aperture / 2;
		float theta = view_fov*M_PI/180;  // convert degress into radians
		// if you cast a ray from lookfrom to lookto, half_height = (distance from lookto to the top of the image / distance from lookfrom to lookto)
		// half_height and half_width are ratios used below to calculate the vectors that define the boundaries of the image
		float half_height = tan(theta/2);
		float half_width = aspect * half_height;
		origin = lookfrom;
		// u and v are perpendicular vectors that are the right and up directions for the orientation of the camera
		// they define the plane of the camera and thus the orientation (the relative upwards and right directions) of the camera 
		// this is so the camera can pitch, yaw and roll
		// note that they are both unit vectors (this is important for multiplying by scalars like half_width or half_height)
		// w is along a line that intersects lookfrom and lookto
		// the camera points towards -w
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(view_up, w)); // "right" for the camera orientation
		v = cross(w, u); // "up" for the camera orientation

		// these 3 vectors define the plane and boundaries of the image
		lower_left_corner = origin - half_width*focus_dist*u - half_height*focus_dist*v - focus_dist*w;
		horizontal = 2*half_width*focus_dist*u; // right edge (offset to lower_left_corner)
		vertical = 2*half_height*focus_dist*v; // top edge (offset to lower_left_corner)
	}

	// sends a ray from a random point on the camera lens to a position on the image plane
	// these rays will converge on the same point at focus_dist
	// this means they will be more spread out the further they are from focus_dist
	// this means objects at focus_dist will be clear and objects far from focus_dist will be blurry, simluating camera focus
	ray get_ray(float s, float t) const
	{
		point rd = lens_radius*random_in_unit_disk();
		point offset = u*rd.x() + v*rd.y();
		float time = time0 + my_rand() * (time1-time0);
		// the -origin-offset turns it into a direction relative to origin+offset
		point dir = lower_left_corner + s*horizontal + t*vertical - origin - offset;
		return ray((origin + offset), dir, time);
	}

	point origin;
	point lower_left_corner;
	point horizontal;
	point vertical;
	point u, v, w;
	float time0, time1;
	float lens_radius;
};

#endif
