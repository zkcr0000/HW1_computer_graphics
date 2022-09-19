#include "Rasterizer.h"
using namespace Eigen;
using namespace std;

static vector<string> split_by_delimiter(string str, string delimiter=" ");
static void fill_color(vector<unsigned char> &image, int width, int x, int y, Vector4f pixel_color);
static int convert_index(int ind, int len);

Rasterizer::Rasterizer(const char* input_filename) 
{ 
  default_color << 255.0f,255.0f,255.0f,255.0f;
  ifstream fin;
  fin.open(input_filename);
  string str; // the string of a line
  vector<string> words; // words of a line after splitting by space

  if(getline(fin, str))
  {
    // define a function separate string by space
    // https://www.delftstack.com/howto/cpp/cpp-split-string-by-space/
    // return a vector of string
    words = split_by_delimiter(str);
  }

  // read in first line "png width height filename"
  // conversion between number and string
  stringstream ss; 
  ss.clear();
  ss << words[1];
  ss >> width;
  ss.clear();
  ss << words[2];
  ss >> height;
  ss.clear();
  output_filename = words[3];
  cout << "width:" << width << endl;
  cout << "height:" << height << endl;
  cout << "output filename:" << output_filename << endl;

  //generate some image (default is transparent black)
  frame_buffer.resize(width * height * 4);
  for(int y = 0; y < height; y++)
  {
    for(int x = 0; x < width; x++) 
    {
        fill_color(frame_buffer, width, x, y, Vector4f(0.0f,0.0f,0.0f,0.0f));
    } 
  }

  float x,y,z,w; // read in coordinates
  Vector4f vertex3D; // read in vertex 3D
  Vector4f color_temp; // read in color
  int R,G,B; // read in color
  int a,b,c; // read in triangle
  Triangle_ind triangle_temp; // read in triangle


  while (!fin.eof())
  {
    if(getline(fin, str) && str.size() > 0)
    {
      words = split_by_delimiter(str);
      if (words[0] == "xyzw")
      {
        cout << "xyzw point" << endl;
        ss.clear();
        ss << words[1];
        ss >> x;
        ss.clear();
        ss << words[2];
        ss >> y;
        ss.clear();
        ss << words[3];
        ss >> z;
        ss.clear();
        ss << words[4];
        ss >> w;
        ss.clear();
        vertex3D << x, y, z, w;
        cout << "position" << endl << vertex3D << endl;
        Point3D_list.push_back(vertex3D);
        Color_list.push_back(default_color);
      }
      else if(words[0] == "tri")
      {
        cout << "read in triangle" << endl;
        ss << words[1];
        ss >> a;
        ss.clear();
        ss << words[2];
        ss >> b;
        ss.clear();
        ss << words[3];
        ss >> c;
        ss.clear();
        a = convert_index(a, Point3D_list.size());
        b = convert_index(b, Point3D_list.size());
        c = convert_index(c, Point3D_list.size());
        triangle_temp.ind[0] = a;
        triangle_temp.ind[1] = b;
        triangle_temp.ind[2] = c;
        Triangle_ind_list.push_back(triangle_temp);
        cout << a << "," << b << "," << c << endl;
      }
      else if(words[0] == "rgb")
      {
        cout << "read in color" << endl;
        ss << words[1];
        ss >> R;
        ss.clear();
        ss << words[2];
        ss >> G;
        ss.clear();
        ss << words[3];
        ss >> B;
        ss.clear();
        default_color << R,G,B,255.0f;
        cout << default_color << endl;
      }
      else if(words[0] == "depth")
      {
        cout << "using depth buffer" << endl;
        depth = true;
        depth_buffer.resize(width,height);
      }
      else
      {
           cout << "no matching format, ignore this line " << endl;
           cout << "content:" << str << endl;
           continue;
      }
    }
  }
  fin.close();
}

Vector3f Rasterizer::Map_3D_2D_single(Vector4f Point)
{
  Vector3f Point2D;
  // (x/w+1)*width/2, (y/w+1)*height/2, z/w
  Point2D << (Point[0]/Point[3] + 1) * width / 2, (Point[1]/Point[3] + 1) * height / 2, Point[2]/Point[3]; 
  return Point2D;
}

void Rasterizer::Map_3D_2D()
{
  for (int i = 0; i < Point3D_list.size(); i++)
  {
    Point2D_list.push_back(Map_3D_2D_single(Point3D_list[i]));
  }  
  // for (int i = 0; i < Point2D_list.size(); i++)
  // {
  //   cout << Point2D_list[i] << endl;
  // }
}

void Rasterizer::Store_triangle2D()
{
  Triangle2D triangle_temp;
  int index;
  for( int i = 0; i < Triangle_ind_list.size(); i++)
  {
    index = Triangle_ind_list[i].ind[0];
    triangle_temp.v[0] = Point2D_list[index];
    triangle_temp.color[0] = Color_list[index];
    index = Triangle_ind_list[i].ind[1];
    triangle_temp.v[1] = Point2D_list[index];
    triangle_temp.color[1] = Color_list[index];
    index = Triangle_ind_list[i].ind[2];
    triangle_temp.v[2] = Point2D_list[index];
    triangle_temp.color[2] = Color_list[index];
    Triangle2D_list.push_back(triangle_temp);
  }
}

void Rasterizer::Draw_triangle(const Triangle2D &triangle)
{
  cout << "Draw triangle" << endl;
  // DDA algorith and scanline to draw triangle with linear interpolation of color
  // We will scan in y dimension for DDA algorithm

  // rearange the point such that A,B,C such that for the y value A < B < C

  vector<int> idx(3);
  iota(idx.begin(), idx.end(), 0);
  sort(idx.begin(), idx.end(), [&triangle](int i1, int i2) {return triangle.v[i1][1] < triangle.v[i2][1];});
  Color_Point A = {triangle.v[idx[0]], triangle.color[idx[0]]};
  Color_Point B = {triangle.v[idx[1]], triangle.color[idx[1]]};
  Color_Point C = {triangle.v[idx[2]], triangle.color[idx[2]]};

  // Find the s vector of each edge https://cs418.cs.illinois.edu/website/text/fixed-functionality.html#depth-buffer
  Vector3f s_ba = (B.v - A.v) / (B.v[1] - A.v[1]);
  Vector3f s_ca = (C.v - A.v) / (C.v[1] - A.v[1]);
  Vector3f s_cb = (C.v - B.v) / (C.v[1] - B.v[1]);
  // Find the integer y value we want to scan
  int y_a = ceil(A.v[1]);
  int y_b = ceil(B.v[1]);
  int y_c = ceil(C.v[1]);
  // starting point for A, B, C
  Vector3f p_ca = s_ca * (y_a - A.v[1]) + A.v;
  Vector3f p_ba = s_ba * (y_a - A.v[1]) + A.v;
  Vector3f p_cb = s_cb * (y_b - B.v[1]) + B.v;
  // cout << "my points" << endl;
  // cout << p_ca << endl;
  // cout << p_ba << endl;
  // cout << p_cb << endl;
  // cout << "end" << endl;
  // cout << "my color" << endl;
  // cout << A.color << endl;
  // cout << B.color << endl;
  // cout << C.color << endl;
  // cout << "end" << endl;

  for (int y = y_a; y < y_b; y++)
  {
    // fix y value can in x betweeen edge CA and BA

    // fix y, left most and right most point on edge CA and BA
    Vector3f point_ca = p_ca + (y - y_a)* s_ca;
    Vector3f point_ba = p_ba + (y - y_a)* s_ba;
    // fix y, left most and right most x coordiante on edge CA and BA
    float x1 = point_ca[0];
    float x2 = point_ba[0];
    // cout << "y value:" << y << ",x1:" << x1 << ",x2:" << x2 << endl;
    // fix y, left most and right most color on edge CA and BA
    Vector4f color_ca = (C.v[1] - y) / (C.v[1]-A.v[1]) * A.color + (y - A.v[1])/(C.v[1]-A.v[1]) * C.color;
    Vector4f color_ba = (B.v[1] - y) / (B.v[1]-A.v[1]) * A.color + (y - A.v[1]) / (B.v[1]-A.v[1]) * B.color;
    // cout << "ca color: " << color_ca << endl;
    // cout << "ba color: " << color_ba << endl;
    if (x1 < x2)
    {
      for (int x = ceil(x1); x <= x2; x++)
      {
        Vector4f pixel_color = (x-x1)/(x2-x1) * color_ba + (x2-x)/(x2-x1) * color_ca;
        if (x < width && x >= 0 && y < height && y >= 0)
        {
          fill_color(frame_buffer, width, x, y, pixel_color);
        }
      }
    }
    else
    {
      for (int x = ceil(x2); x <= x1; x++)
      {
        Vector4f pixel_color = (x1-x)/(x1-x2) * color_ba + (x-x2)/(x1-x2) * color_ca;
        if (x < width && x >= 0 && y < height && y >= 0)
        {
          fill_color(frame_buffer, width, x, y, pixel_color);
        }
      }    
    }
  }
  for (int y = y_b; y < y_c; y++)
  {
    // fix y value can in x betweeen edge CA and CB

    // fix y, left most and right most point on edge CA and BA
    Vector3f point_ca = p_ca + (y - y_a)* s_ca;
    Vector3f point_cb = p_cb + (y - y_b)* s_cb;
    // fix y, left most and right most x coordiante on edge CA and BA
    float x1 = point_ca[0];
    float x2 = point_cb[0];
    // cout << "y value:" << y << ",x1:" << x1 << ",x2:" << x2 << endl;
    // fix y, left most and right most color on edge CA and BA
    Vector4f color_ca = (C.v[1] - y) / (C.v[1] - A.v[1]) * A.color + (y - A.v[1]) / (C.v[1] - A.v[1]) * C.color;
    Vector4f color_cb = (C.v[1] - y) / (C.v[1] - B.v[1]) * B.color + (y - B.v[1]) / (C.v[1] - B.v[1]) * C.color;
    // cout << "ca color: " << color_ca << endl;
    // cout << "cb color: " << color_cb << endl;
    if (x1 < x2)
    {
      // left edge is ca, right edge is cb
      for (int x = ceil(x1); x <= x2; x++)
      {
        Vector4f pixel_color = (x-x1)/(x2-x1) * color_cb + (x2-x)/(x2-x1) * color_ca;
        if (x < width && x >= 0 && y < height && y >= 0)
        {
          fill_color(frame_buffer, width, x, y, pixel_color);
        }
      }
    }
    else
    {
      // left is cb, right is ca
      for (int x = ceil(x2); x <= x1; x++)
      {
        Vector4f pixel_color = (x1-x)/(x1-x2)* color_cb + (x-x2)/(x1-x2) * color_ca;
        if (x < width && x >= 0 && y < height && y >= 0)
        {
          fill_color(frame_buffer, width, x, y, pixel_color);
        }
      }    
    }
  }
}

void Rasterizer::Draw()
{
  // Convert all 3D points to 2D points
  Map_3D_2D();
  // Store all triangles into Triangle_2D_list
  Store_triangle2D();
  // Draw each triangle using DDA algorithm and Scanline algorithm
  for( int i = 0; i < Triangle2D_list.size(); i++)
  // for( int i = 0; i < 1; i++)
  {
    Draw_triangle(Triangle2D_list[i]);
  }
  // save picture
  encodeOneStep();
}

static vector<string> split_by_delimiter(string str, string delimiter)
{
    vector<string> words;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != string::npos) {
        words.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
        while(str[0] == ' ' && str.size() > 0) str.erase(0,1);
    }
    if(str.size() > 0) words.push_back(str);
    return words;
}

static void fill_color(vector<unsigned char> &image, int width, int x, int y, Vector4f pixel_color)
{
    image[4 * width * y + 4 * x + 0] = pixel_color[0];
    image[4 * width * y + 4 * x + 1] = pixel_color[1];
    image[4 * width * y + 4 * x + 2] = pixel_color[2];
    image[4 * width * y + 4 * x + 3] = pixel_color[3];
}

static int convert_index(int ind, int len)
{
  // convert negative index to the correct index
  if (ind >=0 ) return ind - 1;
  else return len + ind;
}

void Rasterizer::encodeOneStep() {
  //Encode the image
  unsigned error = lodepng::encode(output_filename.c_str(), frame_buffer, width, height);

  //if there's an error, display it
  if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}