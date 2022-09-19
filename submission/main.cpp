#include "lodepng.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Rasterizer.h"
using namespace std;

int main(int argc, char *argv[]) {
  //NOTE: this sample will overwrite the file or test.png without warning!
  const char* input_filename;
  if (argc == 2)
  {
    input_filename = argv[1];
    cout << "Input filename:" << input_filename << endl;
  }
  else cout << "error in filename for reading" << endl;

  Rasterizer my_rasterizer(input_filename);
  my_rasterizer.Draw();

  return 0;
}