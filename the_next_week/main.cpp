#include "vec3.h" // TODO should this be moved into util.h ?
#include "util.h"
#include "camera.h"
#include "textures.h"
#include "material.h"
#include "hitable.h"
#include <float.h>
#include <iostream>
#include <thread>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

rgb color(const ray& r, const hitable *world, int depth)
{
	hit_record rec;
	// some of the reflected rays will hit the same object they are bouncing off of at very small values for t because of floating point imprecision
	// using 0.001 as the t_min helps prevent that
	if (world->hit(r, 0.001, FLT_MAX, rec)) // rec is an output of this function
	{
		ray scattered;
		rgb attenuation;
		rgb emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.hit_point); // TODO don't think I have changed every object to set a rec.u and rec.v
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) // attenuation and scattered are outputs
		{
			// recursively call color until the background is hit, a non scattering material is hit, or depth >= 50
			return emitted + attenuation*color(scattered, world, depth+1);
		}
		else
		{
			// ray made it too far without reaching a light source or scatter() returned false
			// return (0,0,0) (black)
			return emitted;
		}
	}
	else // ray didn't hit any objects, return background color
	{
		/*
		point unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0); // maps the y component to scalar between 0 and 1
		// produces rgb value that ranges from (0.5, 0.7, 1.0) to (1, 1, 1)
		return (1.0-t)*rgb(1.0, 1.0, 1.0) + t*rgb(0.5, 0.7, 1.0);
		*/
		return rgb(0,0,0);
	}
}

// cam is an output
hitable *create_scene(int scene_num, camera& cam, int total_nx, int total_ny)
{
	switch(scene_num)
	{
	case(0):
	{
		point lookfrom(13.0,2.0,3.0);
		point lookat(0.0,0.0,0.0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0.0,1.0,0.0),
				   20,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		int n = 500;
		hitable **list = new hitable*[n+1];
		texture *temp1 = new constant_texture(rgb(0.2,0.3,0.1));
		texture *temp2 = new constant_texture(rgb(0.9,0.9,0.9));
		texture *checker = new checker_texture(temp1, temp2);  
		list[0] = new sphere(point(0.0,-1000.0,0.0), 1000, new lambertian(checker));
		int i = 1;
		for(int a = -10;
			a < 10;
			a++)
		{
			for(int b = -10;
				b < 10;
				b++)
			{
				float choose_mat = my_rand();
				point center(a+0.9*my_rand(),0.2,b+0.9*my_rand());
				if((center-point(4.0,0.2,0.0)).length() > 0.9)
				{
					if(choose_mat < 0.8)
					{
						constant_texture *tex = new constant_texture(rgb(my_rand()*my_rand(),
																		 my_rand()*my_rand(),
																		 my_rand()*my_rand())); 
						list[i++] = new moving_sphere(center,
													  center+point(0.0,0.5*my_rand(),0.0),
													  0.0, 1.0,
													  0.2,
													  new lambertian(tex));
					}
					else if(choose_mat < 0.95)
					{
						list[i++] = new sphere(center, 0.2, new metal(rgb(0.5*(1+my_rand()),
																	0.5*(1+my_rand()),
																	0.5*(1+my_rand())),
																	0.5*my_rand()));
					}
					else
					{
						list[i++] = new sphere(center, 0.2, new dielectric(1.5));
					}
				}
			}
		}

		list[i++] = new sphere(point(0.0,1.0,0.0), 1.0, new dielectric(1.5));
		list[i++] = new sphere(point(-4.0,1.0,0.0), 1.0, new lambertian(new constant_texture(rgb(0.4,0.2,0.1))));
		list[i++] = new sphere(point(4.0,1.0,0.0), 1.0, new metal(rgb(0.7,0.6,0.5), 0.0));

		return new hitable_list(list, i);
	} break;
	
	case(1):
	{
		point lookfrom(13.0,2.0,3.0);
		point lookat(0.0,0.0,0.0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0.0,1.0,0.0),
				   20,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		texture *temp1 = new constant_texture(rgb(0.2,0.3,0.1));
		texture *temp2 = new constant_texture(rgb(0.9,0.9,0.9));
		texture *checker = new checker_texture(temp1, temp2);  
		hitable **list = new hitable*[2];
		list[0] = new sphere(point(0.0,-10.0,0.0), 10, new lambertian(checker));
		list[1] = new sphere(point(0.0,10.0,0.0), 10, new metal(rgb(0.8,0.3,0.3), 0.02));

		return new hitable_list(list, 2);
	} break;

	case(2):
	{
		point lookfrom(13.0,2.0,3.0);
		point lookat(0.0,0.0,0.0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0.0,1.0,0.0),
				   20,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		texture *pertex = new noise_texture(4);
		hitable **list = new hitable*[2];
		list[0] = new sphere(point(0.0,-1000.0,0.0), 1000, new lambertian(pertex));
		list[1] = new sphere(point(0.0,2.0,0.0), 2, new lambertian(pertex));
		return new hitable_list(list, 2);
	} break;

	case(3):
	{
		point lookfrom(13.0,2.0,3.0);
		point lookat(0.0,0.0,0.0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0.0,1.0,0.0),
				   20,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		// working directory when the program is ran by run.bat is r:\\the_next_week\images
		texture *txtre = new image_texture("..\\assets\\earthmap.jpg");
		hitable **list = new hitable*[1];
		list[0] = new sphere(point(0,0,0), 1, new lambertian(txtre));
		return new hitable_list(list, 1);
	} break;

	case(4):
	{
		point lookfrom(13.0,2.0,3.0);
		point lookat(0.0,0.0,0.0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0.0,1.0,0.0),
				   60,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		const int COUNT = 4;
		texture *pertex = new noise_texture(4);
		hitable **list = new hitable*[COUNT];
		list[0] = new sphere(point(0,-1000,0), 1000, new lambertian(pertex));
		list[1] = new sphere(point(0,2,0), 2, new lambertian(pertex));
		// note that the rgb value for diffuse_light is above (1,1,1)
		list[2] = new sphere(point(0,7,0), 2, new diffuse_light(new constant_texture(rgb(4,4,4))));
		list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(rgb(4,4,4))));
		return new hitable_list(list, COUNT);
	} break;

	case(5):
	{
		point lookfrom(278, 278, -800);
		point lookat(278, 278, 0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0,1,0),
				   40,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		material *red   = new lambertian(new constant_texture(rgb(0.65, 0.05, 0.05)));
		material *white = new lambertian(new constant_texture(rgb(0.73, 0.73, 0.73)));
		material *green = new lambertian(new constant_texture(rgb(0.12, 0.45, 0.15)));
		// note that the rgb value for diffuse_light is above (1,1,1)
		material *light = new diffuse_light(new constant_texture(rgb(15, 15, 15)));

		hitable **list = new hitable*[6];
		int i = 0;

		// background walls
		list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
		list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
		list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
		list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
		list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
		list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
/*
		// foreground boxes
		list[i++] = new box(point(130, 0, 65), point(295, 165, 230), white);
		list[i++] = new box(point(265, 0, 295), point(430, 330, 460), white);
*/
		// foreground boxes
		list[i++] = new translate(
								new rotate_y(
											new box(point(0, 0, 0),
													point(165, 165, 165),
													white),
											-18),
								point(130,0,65));

		list[i++] = new translate(
								new rotate_y(
											new box(point(0, 0, 0),
													point(165, 330, 165),
													white),
											15),
								point(265,0,295));

		return new hitable_list(list, i);
	} break;

	case(6):
	{
		point lookfrom(278, 278, -800);
		point lookat(278, 278, 0);
		float dist_to_focus = 10.0; //(lookfrom-lookat).length();
		float aperture = 0.0; // controls how much of the image is in focus, lower number = more of the image is in focus
		cam = camera(lookfrom, lookat,
				   point(0,1,0),
				   40,
				   (float)total_nx/(float)total_ny,
				   aperture,
				   dist_to_focus,
				   0.0, 1.0);

		material *red   = new lambertian(new constant_texture(rgb(0.65, 0.05, 0.05)));
		material *white = new lambertian(new constant_texture(rgb(0.73, 0.73, 0.73)));
		material *green = new lambertian(new constant_texture(rgb(0.12, 0.45, 0.15)));
		// note that the rgb value for diffuse_light is above (1,1,1)
		//material *light = new diffuse_light(new constant_texture(rgb(7, 7, 7)));
		// TODO
		material *light = new diffuse_light(new constant_texture(rgb(4, 4, 4)));

		hitable **list = new hitable*[8];
		int i = 0;

		// background walls
		list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
		list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
		list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
		list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
		list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
		list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

		hitable *b1 = new translate(new rotate_y(new box(point(0,0,0),
														 point(165,165,165),
														 white),
												 -18),
									point(130,0,65));

		hitable *b2 = new translate(new rotate_y(new box(point(0,0,0),
														 point(165,330,165),
														 white),
												 15),
									point(265,0,295));

		list[i++] = new constant_medium(b1, 0.01, new isotropic(new constant_texture(rgb(0.4, 0.4, 1))));
		list[i++] = new constant_medium(b2, 0.01, new isotropic(new constant_texture(rgb(0,0,0))));
		return new hitable_list(list, i);
	} break;

	default:
	{
		assert(1 == 0);
	} break;
	}
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

void render_ppm_section(image_section sec, int ns, const hitable *world, const camera& cam, const char *file_name)
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
	const int total_nx = 800;  // resolution width
	const int total_ny = 400;  // resolution height
	const int ns = 100;  // number of samples per pixel
	const int THREAD_COUNT = 8;

	camera cam;
	hitable *world = create_scene(6, cam, total_nx, total_ny);

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

