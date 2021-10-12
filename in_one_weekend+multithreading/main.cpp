#include <iostream>
#include "float.h"
#include "camera.h"
#include "util.h"
#include "material.h"
#include <thread>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

rgb color(const ray& r, const hitable const *world, int depth)
{
	hit_record rec;
	// some of the reflected rays will hit the same object they are bouncing off of at very small values for t because of floating point imprecision
	// using 0.001 as the t_min helps prevent that
	if (world->hit(r, 0.001, FLT_MAX, rec)) // rec is an output of this function
	{
		ray scattered;
		rgb attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) // attenuation and scattered are outputs
		{
			// recursively call color until the background is hit or depth >= 50
			return attenuation*color(scattered, world, depth+1);
		}
		else
		{
			// ray made it too far without reaching a light source or scatter() returned false
			// return (0,0,0) (black)
			return rgb(0.0, 0.0, 0.0);
		}
	}
	else // ray didn't hit any objects, return background color
	{
		point unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0); // maps the y component to scalar between 0 and 1
		// produces rgb value that ranges from (0.5, 0.7, 1.0) to (1, 1, 1)
		return (1.0-t)*rgb(1.0, 1.0, 1.0) + t*rgb(0.5, 0.7, 1.0);
	}
}

hitable *random_scene()
{
	int n = 500;
	hitable **list = new hitable*[n+1];
	list[0] = new sphere(point(0.0,-1000.0,0.0), 1000, new lambertian(rgb(0.5,0.5,0.5)));
	int i = 1;
	for(int a = -11;
		a < 11;
		a++)
	{
		for(int b = -11;
			b < 11;
			b++)
		{
			float choose_mat = my_rand();
			point center(a+0.9*my_rand(),0.2,b+0.9*my_rand());
			if((center-point(4.0,0.2,0.0)).length() > 0.9)
			{
				if(choose_mat < 0.8)
				{
					list[i++] = new sphere(center, 0.2, new lambertian(rgb(my_rand()*my_rand(), my_rand()*my_rand(), my_rand()*my_rand()))); 
				}
				else if(choose_mat < 0.95)
				{
					list[i++] = new sphere(center, 0.2, new metal(rgb(0.5*(1+my_rand()), 0.5*(1+my_rand()), 0.5*(1+my_rand())), 0.5*my_rand()));
				}
				else
				{
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(point(0.0,1.0,0.0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(point(-4.0,1.0,0.0), 1.0, new lambertian(rgb(0.4,0.2,0.1)));
	list[i++] = new sphere(point(4.0,1.0,0.0), 1.0, new metal(rgb(0.7,0.6,0.5), 0.0));

	return new hitable_list(list, i);
}

struct image_section
{
	int start_nx;
	int end_nx;
	int total_nx;

	int start_ny;
	int end_ny;
	int total_ny;
};

void render_ppm_section(image_section sec, int ns, const hitable const *world, const camera& cam, const char *file_name)
{
	printf("file: %s\ns_ny: %d | e_ny: %d | t_ny: %d\ns_nx: %d | e_nx: %d | t_nx: %d\n\n", file_name, sec.start_ny, sec.end_ny, sec.total_ny, sec.start_nx, sec.end_nx, sec.total_nx);

	FILE *f = fopen(file_name, "w");
	if(!f)
	{
		printf("failed to open file %s in render_ppm_section", file_name);
	}
	if(strcmp(file_name, "temp1") == 0)
	{
		fprintf(f, "P3\n%d %d\n255\n", sec.total_nx, sec.total_ny);
	}
	for (int j = sec.end_ny-1;
		j >= sec.start_ny;
		j--)
	{
		for (int i = sec.start_nx;
			i < sec.end_nx;
			i++)
		{
			// sampling
			rgb pixel(0, 0, 0);
			for (int s = 0;
				s < ns;
				s++)
			{
				// i+my_rand() gives random values in the range: i <= val < (i+1)
				float u = (float)(i+my_rand()) / (float)sec.total_nx;
				float v = (float)(j+my_rand()) / (float)sec.total_ny;
				// u and v are used as randomized points on the image plane that always fall within the boundaries of the pixel
				// this is for anti-aliasing to smooth out pixelated edges and sharp color boundaries in the final image
				ray r = cam.get_ray(u, v);
				pixel += color(r, world, 0);
			}
			pixel /= (float)(ns);  // average of the color values of all the samples
			// we must apply 'gamma correction' to the output to make sure dark/light shades look ok on monitors
			// we are using 'gamma 2', which means rgb values need to be to the power of 1/gamma, which with gamma=2 means square root
			pixel = rgb(sqrt(pixel[0]), sqrt(pixel[1]), sqrt(pixel[2]));
			int ir = (int)(255.99*pixel[0]);
			int ig = (int)(255.99*pixel[1]);
			int ib = (int)(255.99*pixel[2]);
			//std::cout << ir << " " << ig << " " << ib << "\n";
			fprintf(f, "%d %d %d\n", ir, ig, ib);
			fflush(f);
		}
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
	const int total_nx = 200;  // resolution width
	const int total_ny = 100;  // resolution height
	const int ns = 100;  // number of samples per pixel
	const int THREAD_COUNT = 8;

	hitable *world = random_scene();

	point lookfrom(6.0,2.0,1.5);
	point lookat(0.0,0.0,-1.0);
	float dist_to_focus = (lookfrom-lookat).length();
	// float aperture = 0.2; // controls how much of the image is in focus, lower number = more of the image is in focus
	float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
	camera cam(lookfrom, lookat, point(0.5,0.5,0.0), 30, (float)total_nx/(float)total_ny, aperture, dist_to_focus);

	int ny_sections[THREAD_COUNT+1];
	ny_sections[THREAD_COUNT] = total_ny;
	for(int i = 0;
		i < THREAD_COUNT;
		i++)
	{
		ny_sections[i] = (total_ny/(THREAD_COUNT))*i;
	}

	char *file_names[THREAD_COUNT];
	for(int i = 0;
		i < THREAD_COUNT;
		i++)
	{
		file_names[i] = (char *)malloc(10);
		char f_n[10] = "temp";
		char num_str[5];
		itoa(i+1, num_str, 10);
		strcat(f_n, num_str);
		strcpy(file_names[i], f_n);
	}

	std::thread threads[THREAD_COUNT];
	int count = 0;
	for(int y = THREAD_COUNT-1;
		y >= 0;
		y--)
	{
		std::cout << " y: " << y << " count: " << count << " file_name: " << file_names[count] << "\n";
		image_section section;
		section.start_nx = 0;
		section.end_nx = total_nx;
		section.total_nx = total_nx;
		section.start_ny = ny_sections[y];
		section.end_ny = ny_sections[y+1];
		section.total_ny = total_ny;
		threads[count] = std::thread(render_ppm_section,
									section,
									ns, 
									world, 
									cam, 
									file_names[count]);
		++count;
	}

	for(int i = 0;
		i < THREAD_COUNT;
		i++)
	{
		threads[i].join();
	}

	time_t rawt;
	time(&rawt);
	tm *timeinfo;
	timeinfo = localtime(&rawt);
	char file_name[100];
	strftime(file_name, 100, "%d-%m-%Y__%H'%M'%S", timeinfo);
	strcat(file_name, ".ppm");
	FILE *output = fopen(file_name, "w");
	if(output)
	{
		for(int i = 0;
			i < THREAD_COUNT;
			i++)
		{
			printf("READING FROM FILE: %s\n", file_names[i]);
			FILE *input = fopen(file_names[i], "r");
			if(input)
			{
				char buf[200];
				while(fgets(buf, 200, input) != NULL)
				{
					fputs(buf, output);
				}
			}
			else
			{
				printf("opening %s failed\n", file_names[i]);
			}
			fclose(input);
		}
		fclose(output);
	}
	else
	{
		printf("opening test2.ppm failed\n");
	}
	
	for(int i = 0;
		i < THREAD_COUNT;
		i++)
	{
		remove(file_names[i]);
	}
}

