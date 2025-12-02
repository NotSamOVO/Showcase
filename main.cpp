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
  camera.e = Eigen::Vector3d(0, 4, 14); // Moved camera up and back
  
  // Calculate camera basis vectors
  Eigen::Vector3d target(0, 2, 0); // Look at the middle of the stack
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

  // 1. Checkerboard Floor
  auto floor = std::make_shared<Plane>();
  floor->point = Eigen::Vector3d(0, -1, 0);
  floor->normal = Eigen::Vector3d(0, 1, 0);
  floor->material = std::make_shared<Material>();
  floor->material->ka = Eigen::Vector3d(0.1, 0.1, 0.1);
  floor->material->kd = Eigen::Vector3d(0.5, 0.5, 0.5); // Will be overridden by checkerboard
  floor->material->ks = Eigen::Vector3d(0.1, 0.1, 0.1);
  floor->material->km = Eigen::Vector3d(0.2, 0.2, 0.2); // Slight reflection
  floor->material->phong_exponent = 10;
  floor->material->is_checkerboard = true;
  objects.push_back(floor);

  // 2. Base Sphere (Marble)
  auto base_sphere = std::make_shared<Sphere>();
  base_sphere->center = Eigen::Vector3d(0, 0.5, 0); // y = -1 (floor) + 1.5 (radius)
  base_sphere->radius = 1.5;
  base_sphere->material = std::make_shared<Material>();
  base_sphere->material->ka = Eigen::Vector3d(0.1, 0.0, 0.1);
  base_sphere->material->kd = Eigen::Vector3d(0.8, 0.2, 0.8); // Purple base
  base_sphere->material->ks = Eigen::Vector3d(0.2, 0.2, 0.2);
  base_sphere->material->km = Eigen::Vector3d(0.1, 0.1, 0.1);
  base_sphere->material->phong_exponent = 50;
  base_sphere->material->is_noise = true;
  objects.push_back(base_sphere);

  // 3. Middle Sphere (Gold)
  auto middle_sphere = std::make_shared<Sphere>();
  middle_sphere->center = Eigen::Vector3d(0, 2.9, 0); // y = 0.5 (base center) + 1.5 (base radius) + 0.9 (radius)
  middle_sphere->radius = 0.9;
  middle_sphere->material = std::make_shared<Material>();
  middle_sphere->material->ka = Eigen::Vector3d(0.1, 0.1, 0.0);
  middle_sphere->material->kd = Eigen::Vector3d(0.2, 0.2, 0.0);
  middle_sphere->material->ks = Eigen::Vector3d(0.8, 0.7, 0.2); // Gold specular
  middle_sphere->material->km = Eigen::Vector3d(0.1, 0.1, 0.1);
  middle_sphere->material->phong_exponent = 200;
  objects.push_back(middle_sphere);

  // 4. Top Sphere (Mirror)
  auto top_sphere = std::make_shared<Sphere>();
  top_sphere->center = Eigen::Vector3d(0, 4.4, 0); // y = 2.9 (mid center) + 0.9 (mid radius) + 0.6 (radius)
  top_sphere->radius = 0.6;
  top_sphere->material = std::make_shared<Material>();
  top_sphere->material->ka = Eigen::Vector3d(0.0, 0.0, 0.0);
  top_sphere->material->kd = Eigen::Vector3d(0.1, 0.1, 0.1);
  top_sphere->material->ks = Eigen::Vector3d(0.8, 0.8, 0.8);
  top_sphere->material->km = Eigen::Vector3d(0.9, 0.9, 0.9); // Highly reflective
  top_sphere->material->phong_exponent = 1000;
  objects.push_back(top_sphere);

  // 6. Ring of larger red spheres
  int num_ring_spheres = 8;
  double ring_radius = 4.5;
  for(int i = 0; i < num_ring_spheres; ++i) {
      double angle = 2.0 * 3.14159 * i / num_ring_spheres;
      auto ring_sphere = std::make_shared<Sphere>();
      // Floor is at -1.0. Radius is 0.8. Center y should be -0.2.
      ring_sphere->center = Eigen::Vector3d(ring_radius * cos(angle), -0.2, ring_radius * sin(angle)); 
      ring_sphere->radius = 0.8;
      ring_sphere->material = std::make_shared<Material>();
      ring_sphere->material->ka = Eigen::Vector3d(0.1, 0.0, 0.0);
      ring_sphere->material->kd = Eigen::Vector3d(0.2, 0.0, 0.0); // Darker red for metallic look
      ring_sphere->material->ks = Eigen::Vector3d(0.9, 0.9, 0.9); // Bright specular
      ring_sphere->material->km = Eigen::Vector3d(0.8, 0.5, 0.5); // Strong reddish reflection
      ring_sphere->material->phong_exponent = 200;
      objects.push_back(ring_sphere);
  }

  // 7. Scattered Metallic Pebbles
  std::vector<Eigen::Vector3d> pebble_pos = {
      Eigen::Vector3d(-3.5, -0.8, 2.0),
      Eigen::Vector3d(3.5, -0.8, -1.0),
      Eigen::Vector3d(-1.0, -0.8, 4.0),
      Eigen::Vector3d(2.5, -0.8, 3.0)
  };
  
  for(const auto& pos : pebble_pos) {
      auto pebble = std::make_shared<Sphere>();
      pebble->center = pos;
      pebble->radius = 0.2;
      pebble->material = std::make_shared<Material>();
      pebble->material->ka = Eigen::Vector3d(0.1, 0.1, 0.1);
      pebble->material->kd = Eigen::Vector3d(0.7, 0.7, 0.7); // Silver
      pebble->material->ks = Eigen::Vector3d(0.9, 0.9, 0.9);
      pebble->material->km = Eigen::Vector3d(0.5, 0.5, 0.5);
      pebble->material->phong_exponent = 100;
      objects.push_back(pebble);
  }

  // 8. Inner Ring of Blue Spheres (Non-reflective)
  int num_blue_spheres = 12;
  double blue_ring_radius = 3.0;
  for(int i = 0; i < num_blue_spheres; ++i) {
      double angle = 2.0 * 3.14159 * i / num_blue_spheres;
      auto blue_sphere = std::make_shared<Sphere>();
      // Floor is at -1.0. Radius is 0.6. Center y should be -0.4.
      blue_sphere->center = Eigen::Vector3d(blue_ring_radius * cos(angle), -0.4, blue_ring_radius * sin(angle)); 
      blue_sphere->radius = 0.6;
      blue_sphere->material = std::make_shared<Material>();
      blue_sphere->material->ka = Eigen::Vector3d(0.0, 0.0, 0.1);
      blue_sphere->material->kd = Eigen::Vector3d(0.1, 0.1, 0.9); // Blue
      blue_sphere->material->ks = Eigen::Vector3d(0.5, 0.5, 0.5); // Moderate specular
      blue_sphere->material->km = Eigen::Vector3d(0.0, 0.0, 0.0); // No reflection
      blue_sphere->material->phong_exponent = 50;
      objects.push_back(blue_sphere);
  }

  // Lights
  auto point_light = std::make_shared<PointLight>();
  point_light->p = Eigen::Vector3d(3, 5, 5);
  point_light->I = Eigen::Vector3d(1.0, 1.0, 1.0);
  lights.push_back(point_light);

  auto point_light2 = std::make_shared<PointLight>();
  point_light2->p = Eigen::Vector3d(-3, 5, 5);
  point_light2->I = Eigen::Vector3d(0.5, 0.5, 1.0); // Blueish light
  lights.push_back(point_light2);

  auto dir_light = std::make_shared<DirectionalLight>();
  dir_light->d = Eigen::Vector3d(-1, -1, -1).normalized();
  dir_light->I = Eigen::Vector3d(0.2, 0.2, 0.2);
  lights.push_back(dir_light);


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
  draw_text(rgb_image, width, height, "Reflective Red & Matte Blue Rings", 10, 10, white, 2);
  draw_text(rgb_image, width, height, "Procedural Noise & Reflections", 10, 30, yellow, 1);
  draw_text(rgb_image, width, height, "CSC317 Fall 2025 - Tianle Xu(Sam)", 10, height - 20, white, 1);

  write_ppm("piece.ppm",rgb_image,width,height,3);
}
