#include "vec3.h"
#include "math.h"

float cosine_of_two_vectors(vec3 *v1, vec3* v2)
{
	float dot = v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
	float cosine = dot / v1.length()*v2.length();
	return cosine;
}= 

float angle_between_two_vectors(vec3 *v1, vec3 *v2)
{
	float cosine = cosine_of_two_vectors(v1, v2);
	return acos(cosine);
}

