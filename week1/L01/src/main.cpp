#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

struct BoundingBox {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
} boundingbox;

int smallest(int arr[], int n){
   int temp = arr[0];
   for(int i=0; i<n; i++) {
      if(temp>arr[i]) {
         temp=arr[i];
      }
   }
   return temp;
}

int largest(int arr[], int n){
   int temp = arr[0];
   for(int i=0; i<n; i++) {
      if(temp<arr[i]) {
         temp=arr[i];
      }
   }
   return temp;
}

int main(int argc, char **argv)
{
	if(argc < 10) {
		cout << "Usage: Lab01 filename width height vax vay vbx vby vcx vcy" << endl;
		return 0;
	}
	// Output filename
	string filename(argv[1]);
	// Width of image
	int width = atoi(argv[2]);
	// Height of image
	int height = atoi(argv[3]);
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	auto image = make_shared<Image>(width, height);

	//Creating list.
	int x_parts[3];
	int y_parts[3];

	x_parts[0] = atoi(argv[4]);
	x_parts[1] = atoi(argv[6]);
	x_parts[2] = atoi(argv[8]);

	y_parts[0] = atoi(argv[5]);
	y_parts[1] = atoi(argv[7]);
	y_parts[2] = atoi(argv[9]);

	//Creating box structure.
	struct BoundingBox box;

	box.xmin = smallest(x_parts, 3);
	box.xmax = largest(x_parts, 3);

	box.ymin = smallest(y_parts, 3);
	box.ymax = largest(y_parts, 3);

	// Draw a rectangle
	for(int y = box.ymin; y < box.ymax; ++y) {
		for(int x = box.xmin; x < box.xmax; ++x) {
			image->setPixel(x, y, 0, 255, 0);
		}
	}

	//Display vertices
	image->setPixel(atoi(argv[4]), atoi(argv[5]), 255, 0, 0);
	image->setPixel(atoi(argv[6]), atoi(argv[7]), 255, 0, 0);
	image->setPixel(atoi(argv[8]), atoi(argv[9]), 255, 0, 0);
	
	// Write image to file
	image->writeToFile(filename);
	return 0;
}
