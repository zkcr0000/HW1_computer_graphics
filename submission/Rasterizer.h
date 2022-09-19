#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "eigen-3.4.0_test/Eigen/Dense"
#include "lodepng.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric> // std::iota
#include <algorithm> // std::sort, std::stable_sort

class Rasterizer{
  private:
    int width, height;
    // depth buffer
    bool depth = false;
    Eigen::MatrixXd depth_buffer;

    struct Color_Point
    {
      Eigen::Vector3f v;
      Eigen::Vector4f color;
    };
    struct Triangle_ind
    {
      Eigen::Vector3i ind; // 3 vertex index of triangle
    };
    struct Triangle2D
    {
      Eigen::Vector3f v[3]; // 3 vertices of triangle
      Eigen::Vector4f color[3]; // 3 vertex colors of triangle
    };
    std::vector<Eigen::Vector4f> Point3D_list;
    std::vector<Eigen::Vector3f> Point2D_list;
    std::vector<Eigen::Vector4f> Color_list;
    std::vector<Triangle_ind> Triangle_ind_list;
    std::vector<Triangle2D> Triangle2D_list;
    std::string output_filename;

    Eigen::Vector4f default_color; // the default color when reading a vertex

    std::vector<unsigned char> frame_buffer;

  public:
    // constructor takes in filename
    Rasterizer(const char* input_filename);
    Eigen::Vector3f Map_3D_2D_single(Eigen::Vector4f Point); // project a single 3D point to 2D point
    void Map_3D_2D(); // project all 3D points in Point3D_list to Point2D_list
    void Store_triangle2D(); // store triangles into Triangle2D_list;
    void Draw_triangle(const Triangle2D &triangle);
    void Draw();
    void encodeOneStep();


};

#endif