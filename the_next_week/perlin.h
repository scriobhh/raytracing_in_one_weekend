#ifndef PERLINH
#define PERLINH

// perlin noise is like blurry white noise
// a key advantage of perlin noise is that it is repeatable
// it is also simple to implement and fast
// NOTE there are more modern noise algorithms like simplex noise which seemlingly should be preferred (idk)
// perlin works by having a grid, where each corner on the grid has a random unit vector called a 'gradient vector' associated with it
// when you check the color value for a point:
//   you determine which grid section the point falls into
//   you find the 'gradient vector' for each corner of the grid section
//   for each grid corner, you get the vector from the point to the grid corner, then get the dot product of that with the grid corner's gradient vector
//   once you have all dot products, you use linear interpolation to create a weighted average of the dot products
class perlin
{
public:
	float noise(const point& p) const
	{
		// u, v, w is position of point in the grid section
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
		// i, j, k is the base corner of the grid section
		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		// holds the 2 x positions, the 2 y positions and the 2 z positions of the grid section's corners
		point c[2][2][2];
		// generates the random unit vectors for each corner of each grid section called 'gradient vectors' 
		for(int di = 0; di < 2; di++)
		{
			for(int dj = 0; dj < 2; dj++)
			{
				for(int dk = 0; dk < 2; dk++)
				{
					// the '& 255' means the value will wrap around to 0 when it goes above a multiple of 255
					// the 'a ^ b ^ c' makes it so that a value between 0 and 255 will be returned 
					// e.g.
					// 0^0 = 0
					// 0^1 = 1
					// -------
					// 1^0 = 1
					// 1^0 = 0
					c[di][dj][dk] = ranvec[perm_x[(i+di) & (ARR_SIZE-1)] ^
										   perm_y[(j+dj) & (ARR_SIZE-1)] ^
										   perm_z[(k+dk) & (ARR_SIZE-1)]];
				}
			}
		}
		// make a vector from point to corner, dot product that with the corner's gradient vector, do this for all corners,
		// interpolate the results of these dot products with an interpolation that is weighted more towards the corner it is closest to
		return perlin_interp(c, u, v, w);
	}
	static const int ARR_SIZE = 256;
	static vec3 *ranvec;
	static int *perm_x;
	static int *perm_y;
	static int *perm_z;

	static vec3 *perlin_generate()
	{
		vec3 *p = new vec3[ARR_SIZE];
		for(int i = 0;
			i < ARR_SIZE;
			++i)
		{
			// -1 + 2*my_rand() gives random number between -1 and 1
			p[i] = unit_vector(vec3(-1 + 2*my_rand(),
									-1 + 2*my_rand(),
									-1 + 2*my_rand()));
		}
		return p;
	}

	static void permute(int *p, int n)
	{
		for(int i = n-1;
			i > 0;
			i--)
		{
			int target = (int)(my_rand()*(i+1));
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	static int *perlin_generate_perm()
	{
		int *p = new int[ARR_SIZE];
		for(int i = 0;
			i < ARR_SIZE;
			i++)
		{
			p[i] = i;
		}
		permute(p, ARR_SIZE);
		return p;
	}

	// NOTE u, v and w are offsets (between 0 and 1) from the corner in the x, y and z axes
	static float perlin_interp(point c[2][2][2], float u, float v, float w)
	{
		// this is based on the equation y=(3x^2)-(2x^3) to make an 'smoothstep' mapping (look up smoothstep if you don't know)
		// this is almost like a linear equation (y=x) except values are biased towards 0 or 1
		// (e.g. values that are close to 0 come out closer to 0 than you would expect, values close to 1 come out closer to 1 than you would expect)
		// this (apparantly) makes the noise look better
		float uu = u*u*(3-2*u);
		float vv = v*v*(3-2*v);
		float ww = w*w*(3-2*w);
		// get vector of the point to the corner (weight_v), dot product that with the corner's gradient vector
		// the values are weighted towards the gradient vectors of the corners closer to the point
		// (this is what the ((i*uu) + (1-i)*(1-uu)) lines are about)
		// accumulate the result
		float accum = 0.0f;
		// i, j and k represent x, y and z for the 2 values on each axes that make up the cubes corners
		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				for(int k = 0; k < 2; k++)
				{
					// i,j, k are 0 or 1
					// uu,vv, ww are between 0 and 1
					vec3 weight_v(u-i, v-j, w-k);
					accum += (i*uu + (1-i)*(1-uu)) *
							 (j*vv + (1-j)*(1-vv)) *
							 (k*ww + (1-k)*(1-ww)) *
							 dot(c[i][j][k], weight_v);
				}
			}
		}
		return accum;
	}

	// a composite noise with multiple frequencies is often used
	// this is usually called 'turbulence'
	float turbulence(const point& p, int depth=7) const
	{
		float accum = 0.0f;
		point temp_p = p;
		float weight = 1.0f;
		for(int i = 0;
			i < depth;
			i++)
		{
			accum += weight*noise(temp_p);
			weight *= 0.5;
			// temp_p is multiplied by 2 so that the exact same output isn't given every time by noise(temp_p)
			temp_p *= 2;
		}
		return fabs(accum);
	}
};

vec3 *perlin::ranvec = perlin::perlin_generate();
int *perlin::perm_x = perlin::perlin_generate_perm();
int *perlin::perm_y = perlin::perlin_generate_perm();
int *perlin::perm_z = perlin::perlin_generate_perm();

#endif
