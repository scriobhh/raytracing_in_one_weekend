#ifndef MATERIALH
#define MATERIALH

#include "hitable.h"
#include "util.h"
#include "textures.h"

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const = 0;
	// emitted is used by light sources to 'emit' light, should be overridden by light sources
	virtual rgb emitted(float u, float v, const point& p) const { return rgb(0.0, 0.0, 0.0); }
};

point reflect(const point& v, const point& normal)
{
	// the normal is the midpoint of the angle of reflection
	// dot(v,normal) gives the magnitude of the vector mapped onto the normal
	// dot(v,normal)*normal gives a vector in the direction of the normal with the magnitude of dot(v,normal)
	// v hits at point on the surface and continues below the surface
	// v - dot(v,normal)*normal gives a vector that is parallel to the surface
	// v - 2*dot(v,normal)*normal gives a vector that is reflected off the surface
	return v - 2*dot(v,normal)*normal;
}

// snell's law is used to determine the direction of the refracted ray
static bool refract(const point& v, const point& normal, float ni_over_nt, point& refracted)
{
	// https://viclw17.github.io/2018/08/05/raytracting-dielectric-materials/
	// the following maths is figured out by taking the equation R = A + B where:
	// R is the refracted vector
	// A is the component vector of R that is perpendicular to the normal
	// B is the component vector of R that is parallel to the normal
	// the equation is then re-arranged and substituted until the equation is in terms of the incoming vector, the normal and the refraction index
	point uv = unit_vector(v);
	float dt = dot(uv, normal);
	// dt represents the steepness of the angle of the incoming vector
	// the disciminant is larger for larger values of dt and smaller refractive indexes
	// a refractive index of 1 or lower means refraction is guaranteed
	float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
	// discriminant values:
	//   >0 means the ray is refracted
	//    0 means the ray is parallel to the surface
	//   <0 means the ray is not refracted
	if (discriminant > 0)
	{
		refracted = ni_over_nt*(uv - normal*dt) - normal*sqrt(discriminant);
		return true;
	}
	else
		return false;
}

// this is called Schlick's approximation
// it is a simple approximation of the reflection coefficient of the boundary of 2 mediums
// the reflection coefficient is used with the boundary of 2 mediums to calculate the amount of light that is reflected off the boundary
// of the incoming light that bounces off the boundary, a certain amount is reflected and the rest is refracted
// the reflectance of the boundary and angle is the ratio of the imcoing light that is reflected
// the ratio of incoming light that is refracted can be calculated by (1 - reflectance)
// e.g. if 80% of the energy of the incoming light is reflected, then schlick() should approximately return 0.8
//   in that case the amount of light that is refracted would be 20%, and the ratio of incoming light to refracted light would be 0.2
// note that the reflection coefficient changes with angle (cosine)
static float schlick(float cosine, float ref_idx)
{
	float r0 = (1-ref_idx) / (1+ref_idx);
	r0 = r0*r0;
	return r0 + (1-r0)*pow((1-cosine), 5);
}

// diffuse materials cause rays to bounce in random directions, called 'diffuse reflection'
class lambertian : public material
{
public:
	lambertian(texture *a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const
	{
		// target is the point the ray will bounce to, it is calculated by:
		// take the hitpoint
		// add the tangent (which is a unit vector) to get a point that is 1 magnitude away from the surface of the sphere and perpendicular to the surface
		// add a random point on a unit circle
		point target = rec.hit_point + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.hit_point, target-rec.hit_point, r_in.time());
		attenuation = albedo->value(rec.u,rec.v,rec.hit_point);
		return true;
	}

	texture *albedo;
};

// metal materials reflect rays in a predictable way called 'specular reflection'
class metal : public material
{
public:
	metal(const rgb& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }

	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const
	{
		point reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		// fuzz is used to add a certain amount of randomness to the direction of the reflection, giving the appearance of a rough or fuzzy surface
		// CHANGED
		scattered = ray(rec.hit_point, reflected + fuzz*random_in_unit_sphere(), r_in.time());
		attenuation = albedo;
		// returns false if the reflected ray is parallel to the surface or goes below the surface
		return (dot(scattered.direction(), rec.normal) > 0);
	}

	rgb albedo;
	float fuzz;
};

// dielectric materials are transparent materials that reflect and refract rays
// in real life they produce both a reflected and refracted ray at the same time
// a certain amount of the light is reflected and the remaining amount is refracted
// the amount that is reflected vs refracted depends on the viewing angle, and there are angles where no refraction or no reflection take place
// see: snell's law, refractive index, critical angle, fresnel equations, reflection coefficient, schlick's approximation, brewster's angle, total internal reflection
// a design decision was made here to only account for one of the rays and randomly decide if the reflected ray or refracted ray should be returned
class dielectric : public material
{
public:
	dielectric(float ri) : ref_idx(ri) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const
	{
		point outward_normal;
		point reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = rgb(1.0, 1.0, 1.0); // the glass surface absorbs nothing
		point refracted;
		float reflect_prob;
		float cosine;
		if(dot(r_in.direction(), rec.normal) > 0) // ray comes from inside the object
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
			// NOTE cosine is calculated slightly differently (see the commented out line below this) in the book, the * ref_idx part doesn't make any sense to me because it can result in cosine values that are >1 which doesn't make any sense, I changed it to not include the * ref_idx and the picture is almost the same, just that glass is slightly more reflective (I also changed the way the maths is written to make it slightly easier to read) 
			// see the no_ref_idx and with_ref_idx files in the build/ file
			//cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = dot(unit_vector(r_in.direction()), rec.normal);
		}
		else // ray comes from outside the object
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		if(refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		{
			reflect_prob = schlick(cosine, ref_idx); // schlick() returns the ratio of incoming light that is reflected off the surface, e.g. if 80% of the energy of the incoming light is reflected, then schlick should approximately return 0.8
		}
		else
		{
			//scattered = ray(rec.hit_point, reflected);
			reflect_prob = 1.0;
		}

		if(my_rand() < reflect_prob)
		{
			// CHANGED
			scattered = ray(rec.hit_point, reflected, r_in.time());
		}
		else
		{
			// CHANGED
			scattered = ray(rec.hit_point, refracted, r_in.time());
		}
		return true;
	}

	float ref_idx;
};

// materials are used as light sources since this allows us to easily make any object (sphere, etc.) a light source
// uses the emitted() method to return an rgb value representing light that was emitted by the light source
// NOTE the texture for 'emit' should have rgb values higher than (1,1,1) to make the light bright enough to light other objects
class diffuse_light : public material
{
public:
	diffuse_light(texture *e) : emit(e) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const
	{
		return false;
	}
	virtual rgb emitted(float u, float v, const point& p) const
	{
		return emit->value(u, v, p);
	}

	texture *emit;
};

class isotropic : public material
{
public:
	isotropic(texture *a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, rgb& attenuation, ray& scattered) const
	{
		scattered = ray(rec.hit_point, random_in_unit_sphere());
		attenuation = albedo->value(rec.u, rec.v, rec.hit_point);
		return true;
	}

	texture *albedo;
};

#endif
