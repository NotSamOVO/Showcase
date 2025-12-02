#include "Object.h"
#include "Camera.h"
#include "Light.h"
#include "Sphere.h"
#include "Plane.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "read_json.h"
#include "write_ppm.h"
#include "viewing_ray.h"
#include "raycolor.h"
#include "text_overlay.h"
#include <Eigen/Core>
#include <vector>
#include <iostream>
#include <memory>
#include <limits>
#include <functional>
#include <random>

int main(int argc, char * argv[])
{
  // --- SCENE SETUP ---
  Camera camera;
  camera.e = Eigen::Vector3d(0, 5, 16); // Moved camera closer
  
  // Calculate camera basis vectors
  Eigen::Vector3d target(0, 1, 0); // Look at the pile
  Eigen::Vector3d up(0, 1, 0);
  Eigen::Vector3d gaze = (target - camera.e).normalized();
  camera.w = -gaze;
  camera.u = up.cross(camera.w).normalized();
  camera.v = camera.w.cross(camera.u);

  camera.d = 1.5;
  // Set image plane dimensions based on aspect ratio
  double aspect_ratio = 640.0 / 360.0;
  camera.height = 1.0; // Physical height of image plane
  camera.width = aspect_ratio * camera.height;

  // Pixel resolution
  int width =  640;
  int height = 360;

  std::vector< std::shared_ptr<Object> > objects;
  std::vector< std::shared_ptr<Light> > lights;

  // 1. Mirror Floor
  auto mirror_mat = std::make_shared<Material>();
  mirror_mat->ka = Eigen::Vector3d(0.0, 0.0, 0.0);
  mirror_mat->kd = Eigen::Vector3d(0.1, 0.1, 0.1);
  mirror_mat->ks = Eigen::Vector3d(0.8, 0.8, 0.8);
  mirror_mat->km = Eigen::Vector3d(0.9, 0.9, 0.9); // Highly reflective
  mirror_mat->phong_exponent = 1000;

  auto floor = std::make_shared<Plane>();
  floor->point = Eigen::Vector3d(0, -1, 0);
  floor->normal = Eigen::Vector3d(0, 1, 0);
  floor->material = mirror_mat;
  objects.push_back(floor);

  // 1b. Mirror Box Walls & Ceiling
  auto back_wall = std::make_shared<Plane>();
  back_wall->point = Eigen::Vector3d(0, 0, -12);
  back_wall->normal = Eigen::Vector3d(0, 0, 1);
  back_wall->material = mirror_mat;
  objects.push_back(back_wall);

  auto front_wall = std::make_shared<Plane>();
  front_wall->point = Eigen::Vector3d(0, 0, 25); // Moved front wall behind camera
  front_wall->normal = Eigen::Vector3d(0, 0, -1);
  front_wall->material = mirror_mat;
  objects.push_back(front_wall);

  auto left_wall = std::make_shared<Plane>();
  left_wall->point = Eigen::Vector3d(-8, 0, 0);
  left_wall->normal = Eigen::Vector3d(1, 0, 0);
  left_wall->material = mirror_mat;
  objects.push_back(left_wall);

  auto right_wall = std::make_shared<Plane>();
  right_wall->point = Eigen::Vector3d(8, 0, 0);
  right_wall->normal = Eigen::Vector3d(-1, 0, 0);
  right_wall->material = mirror_mat;
  objects.push_back(right_wall);

  auto ceiling = std::make_shared<Plane>();
  ceiling->point = Eigen::Vector3d(0, 10, 0);
  ceiling->normal = Eigen::Vector3d(0, -1, 0);
  ceiling->material = mirror_mat;
  objects.push_back(ceiling);

  // 2. Truffle Pile
  // Materials
  auto dark_choc = std::make_shared<Material>();
  dark_choc->ka = Eigen::Vector3d(0.05, 0.02, 0.01);
  dark_choc->kd = Eigen::Vector3d(0.2, 0.1, 0.05); // Dark Brown
  dark_choc->ks = Eigen::Vector3d(0.3, 0.3, 0.3);
  dark_choc->km = Eigen::Vector3d(0.05, 0.05, 0.05);
  dark_choc->phong_exponent = 60;

  auto cocoa_dusted = std::make_shared<Material>();
  cocoa_dusted->ka = Eigen::Vector3d(0.1, 0.05, 0.02);
  cocoa_dusted->kd = Eigen::Vector3d(0.5, 0.3, 0.15); // Light Brown
  cocoa_dusted->ks = Eigen::Vector3d(0.0, 0.0, 0.0); // Matte
  cocoa_dusted->km = Eigen::Vector3d(0.0, 0.0, 0.0);
  cocoa_dusted->phong_exponent = 1;
  cocoa_dusted->is_noise = true; // Texture

  auto milk_choc = std::make_shared<Material>();
  milk_choc->ka = Eigen::Vector3d(0.05, 0.03, 0.01);
  milk_choc->kd = Eigen::Vector3d(0.4, 0.2, 0.1); // Medium Brown
  milk_choc->ks = Eigen::Vector3d(0.1, 0.1, 0.1);
  milk_choc->km = Eigen::Vector3d(0.02, 0.02, 0.02);
  milk_choc->phong_exponent = 30;

  std::vector<std::shared_ptr<Material>> truffle_mats = {dark_choc, cocoa_dusted, milk_choc};

  double r = 0.7;
  int levels = 5;
  double y_start = -1.0 + r; // Floor is at -1.0

  // Random generator for materials and slight position jitter
  std::default_random_engine rng;
  std::uniform_int_distribution<int> mat_dist(0, 2);
  std::uniform_real_distribution<double> jitter_dist(-0.1, 0.1);

  for (int l = 0; l < levels; ++l) {
      int side = levels - l; 
      double y = y_start + l * (r * 1.3); // Stack height
      double offset = (side - 1) * r; // Center offset
      
      for (int x = 0; x < side; ++x) {
          for (int z = 0; z < side; ++z) {
              // Skip corners on larger levels to make it rounder
              if (l < 2 && (x == 0 || x == side-1) && (z == 0 || z == side-1)) continue;

              auto sphere = std::make_shared<Sphere>();
              sphere->radius = r;
              sphere->center = Eigen::Vector3d(
                  (x * 2 * r) - offset + jitter_dist(rng),
                  y + jitter_dist(rng) * 0.2,
                  (z * 2 * r) - offset + jitter_dist(rng)
              );
              sphere->material = truffle_mats[mat_dist(rng)];
              objects.push_back(sphere);
          }
      }
  }

  // 3. (Removed Star)

  // Lights
  auto point_light = std::make_shared<PointLight>();
  point_light->p = Eigen::Vector3d(0, 9, 10); // Single light on top front
  point_light->I = Eigen::Vector3d(1.5, 1.5, 1.5);
  lights.push_back(point_light);


  std::vector<unsigned char> rgb_image(3*width*height);

  // Random number generator for AA
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(-0.5, 0.5);
  int num_samples = 1; // Keep 1 for now until viewing_ray is fixed

  // For each pixel (i,j)
  for(unsigned i=0; i<height; ++i) 
  {
    for(unsigned j=0; j<width; ++j)
    {
      // Set background color
      Eigen::Vector3d rgb(0,0,0);

      // Compute viewing ray
      Ray ray;
      viewing_ray(camera,i,j,width,height,ray);
      
      // Shoot ray and collect color
      raycolor(ray,1.0,objects,lights,0,rgb);

      // Write double precision color into image
      auto clamp = [](double s){ return std::max(std::min(s,1.0),0.0);};
      rgb_image[0+3*(j+width*i)] = 255.0*clamp(rgb(0));
      rgb_image[1+3*(j+width*i)] = 255.0*clamp(rgb(1));
      rgb_image[2+3*(j+width*i)] = 255.0*clamp(rgb(2));

    }
  }

  // Add overlay text
  std::vector<unsigned char> white = {255, 255, 255};
  std::vector<unsigned char> yellow = {255, 255, 0};
  draw_text(rgb_image, width, height, "Truffle Pile in Mirror Box", 10, 10, white, 2);
  draw_text(rgb_image, width, height, "Infinite Reflections", 10, 30, yellow, 1);
  draw_text(rgb_image, width, height, "CSC317 Fall 2025 - Tianle Xu", 10, height - 20, white, 1);

  write_ppm("piece.ppm",rgb_image,width,height,3);
}
