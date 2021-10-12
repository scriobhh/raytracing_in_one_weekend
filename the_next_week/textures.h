#ifndef TEXTURESH
#define TEXTURESH

#include "vec3.h"
#include "perlin.h"

#pragma warning(push, 0) // disable compiler warnings
#define  STB_IMAGE_IMPLEMENTATION
#include "3rd_party/stb_image.h"
#pragma warning(pop) // restore compiler warnings

// a texture is implemented as a function that returns an rgb value
class texture
{
public:
	virtual rgb value(float u, float v, const point& hit_point) const = 0;
};

class constant_texture : public texture
{
public:
	constant_texture() {}
	constant_texture(rgb& c) : color(c) {}
	virtual rgb value(float u, float v, const point& hit_point) const
	{
		return color;
	}

	rgb color;
};

class checker_texture : public texture
{
public:
	checker_texture() {}
	checker_texture(texture *t0, texture*t1) : even(t0), odd(t1) {}
	virtual rgb value(float u, float v, const point& hit_point) const
	{
		// float sines = sin(10*hit_point.x()) * sin(10*hit_point.y()) * sin(10*hit_point.z());
		float sines = sin(10*hit_point.x()) * sin(10*hit_point.z());
		return (sines < 0) ? odd->value(u, v, hit_point) : even->value(u, v, hit_point);
	}

	texture *odd;
	texture *even;
};

class noise_texture : public texture
{
public:
	noise_texture() {}
	noise_texture(float sc) : scale(sc) {}
	virtual rgb value(float u, float v, const point& p) const
	{
		//return rgb(1.0,1.0,1.0)*noise.noise(scale * p);

		// the 0.5*(1+sin(...)) part maps the sine output (-1 to 1) to the range (0,1)
		// (this is used to scale the rgb values)
		// the (scale*p.z() + 10*noise.turbulence(p)) maps to the range (-1,1) 
		//   scale*p.z() will cause the sin value to rise or fall as the points move along the z axis
		//   10*noise.turbulence(p) will cause the sin value to randomly fluctuate based on the return value of turbulence(p)
		return rgb(1,1,1)*0.5*(1+sin(scale*p.z() + 10*noise.turbulence(p)));
	}
	perlin noise;
	float scale;
};

class image_texture : public texture
{
public:
	image_texture() {}
	//image_texture(unsigned char *pixels, int A, int B) : data(pixels), width(A), height(B) {}
	image_texture(const char *image_filename)
	{
		int bytes_per_pixel;
		// creates an array of unsigned chars with the image data
		// the format is: for each pixel there is 1 byte for r, 1 byte for g, 1 byte for b
		// indexes 0 to width is the first row of pixels in the image, width+1 to 2*width is the second row, etc.
		data = stbi_load(image_filename, &width, &height, &bytes_per_pixel, 3);
		assert(data != NULL);
	}
	~image_texture()
	{
		stbi_image_free(data);
	}
	virtual rgb value(float u, float v, const point& p) const;
	unsigned char *data;
	int width, height;
};

// u and v are 'texture co-ordinates'
// these are floating point values between 0 and 1 that represent positions on the image
// these are converted to pixel values by multiplying by the number of pixels in the width or height
// these are used to give a common interface for all textures,
// if pixel values were used in the API instead of texture co-ordinates, then the calling code would have to know the dimensions of each and every texture it uses
// also you would have to change the calling code anytime you change the texture to something with a different size
rgb image_texture::value(float u, float v, const point& p) const
{
	int i = (u)*width;
	int j = (1-v)*height-0.001; // the 1-v reverses the way that get_sphere_uv outputs v=1 for hte top of the sphere and v=0 for the bottom of the sphere
	// clamping i and j
	if(i<0) i=0;
	if(j<0) j=0;
	if(i>width-1) i=width-1;
	if(j>height-1) j=height-1;
	// the 3s are because there are 3 bytes for each pixel (each byte represents an r, g or b value)
	float r = (float)(data[3*i + 3*width*j]) / 255.0f;
	float g = (float)(data[3*i + 3*width*j+1]) / 255.0f;
	float b = (float)(data[3*i + 3*width*j+2]) / 255.0f;
	return rgb(r, g, b);
}

#endif
