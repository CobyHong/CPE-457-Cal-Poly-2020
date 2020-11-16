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

struct Point {
	int x;
	int y;
	int r;
	int g;
	int b;
} point;

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

double getArea(double x1, double y1, double x2, double y2, double x3, double y3)
{
	return 0.5 * ( ( (x2 - x1) * (y3 - y1) ) - ( (x3 - x1) * (y2 - y1) ) );
}

double getScale(double subArea, double mainArea)
{
	return subArea / mainArea;
}

int main(int argc, char **argv)
{
	if(argc < 19) {
		cout << "Usage: Lab02 filename width height x0 y0 r0 g0 b0 x1 y1 r1 g1 b1 x2 y2 r2 g2 b2" << endl;
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

	//Saving points.
	struct Point p1;
	p1.x = atoi(argv[4]);
	p1.y = atoi(argv[5]);
	p1.r = atoi(argv[6]);
	p1.g = atoi(argv[7]);
	p1.b = atoi(argv[8]);

	struct Point p2;
	p2.x = atoi(argv[9]);
	p2.y = atoi(argv[10]);
	p2.r = atoi(argv[11]);
	p2.g = atoi(argv[12]);
	p2.b = atoi(argv[13]);

	struct Point p3;
	p3.x = atoi(argv[14]);
	p3.y = atoi(argv[15]);
	p3.r = atoi(argv[16]);
	p3.g = atoi(argv[17]);
	p3.b = atoi(argv[18]);

	//Creating list.
	int x_parts[3];
	int y_parts[3];

	x_parts[0] = p1.x;
	x_parts[1] = p2.x;
	x_parts[2] = p3.x;

	y_parts[0] = p1.y;
	y_parts[1] = p2.y;
	y_parts[2] = p3.y;

	//Creating box structure.
	struct BoundingBox box;

	box.xmin = smallest(x_parts, 3);
	box.xmax = largest(x_parts, 3);

	box.ymin = smallest(y_parts, 3);
	box.ymax = largest(y_parts, 3);

	// Draw a rectangle
	for(int y = box.ymin; y <= box.ymax; ++y) {
		for(int x = box.xmin; x <= box.xmax; ++x) {

			//Get area of the sub-triangles in main triangle.
			double a1 = getArea(p1.x, p1.y, p2.x, p2.y, x, y);
			double a2 = getArea(p2.x, p2.y, p3.x, p3.y, x, y);
			double a3 = getArea(p3.x, p3.y, p1.x, p1.y, x, y);

			//Area of main triangle.
			double aM = a1 + a2 + a3;

			//Getting alpha, beta, gamma.
			double alpha = getScale(a1, aM);
			double beta = getScale(a2, aM);
			double gamma = getScale(a3, aM);

			//If parts in 0->1 area. Point is inside triangle so draw pixel!
			if( (alpha > 0 && alpha <= 1) && (beta > 0 && beta <= 1) && (gamma > 0 && gamma <= 1) )
			{
				//RGB values for that point.
				double  r = (alpha*p1.r) + (beta*p2.r) + (gamma*p3.r);
				double  g = (alpha*p1.g) + (beta*p2.g) + (gamma*p3.g);
				double  b = (alpha*p1.b) + (beta*p2.b) + (gamma*p3.b);

				image->setPixel(x, y, r, g, b);
			}
		}
	}

	//Display vertices
	image->setPixel(p1.x, p1.y, 255, 0, 0);
	image->setPixel(p2.x, p2.y, 255, 0, 0);
	image->setPixel(p3.x, p3.y, 255, 0, 0);
	
	// Write image to file
	image->writeToFile(filename);
	return 0;
}
