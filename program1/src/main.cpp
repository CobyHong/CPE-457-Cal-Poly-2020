/* Release code for program 1 CPE 471 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>

#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

//=============================================================================//
//BOUNDING BOX HELPERS.

//A triangle only consist of 3 points.
//So when computing bounding box, just comparing 3 values.
int largest(int a, int b, int c)
{
	int result = a;

	if (b > result)
		result = b;
	if (c > result)
		result = c;
	return result;
}

//A triangle only consist of 3 points.
//So when computing bounding box, just comparing 3 values.
int smallest(int a, int b, int c)
{
	int result = a;

	if (b < result)
		result = b;
	if (c < result)
		result = c;
	return result;
}

//=============================================================================//
//STRUCTURES.

//Basic point structure but with RGB as well.
struct Point {
	int x;
	int y;
   float z;
	int r;
	int g;
	int b;
} point;

//contains bounding box information needed for barycentric calculations.
struct BoundingBox {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
} boundingbox;

//contains 3 points which also have rgb values and bounding box info.
struct Triangle {
   Point p1;
   Point p2;
   Point p3;
} triangle;

//=============================================================================//
//STRUCT HELPERS.

BoundingBox createBoundingBox(Triangle triangle)
{
   BoundingBox box;

   int xmin = smallest(triangle.p1.x, triangle.p2.x, triangle.p3.x);
   int xmax = largest(triangle.p1.x, triangle.p2.x, triangle.p3.x);

   int ymin = smallest(triangle.p1.y, triangle.p2.y, triangle.p3.y);
   int ymax = largest(triangle.p1.y, triangle.p2.y, triangle.p3.y);

   box.xmin = xmin;
   box.xmax = xmax;

   box.ymin = ymin;
   box.ymax = ymax;

   return box;
}

Triangle createTriangle(Point p1, Point p2, Point p3)
{
   Triangle triangle;
   
   triangle.p1 = p1;
   triangle.p2 = p2;
   triangle.p3 = p3;

   return triangle;
}

//=============================================================================//
//TRIANGLE HELPERS.

//Get area of current triangle against point.
double getArea(double x1, double y1, double x2, double y2, double x3, double y3)
{
	return 0.5 * ( ( (x2 - x1) * (y3 - y1) ) - ( (x3 - x1) * (y2 - y1) ) );
}

//subArea be the smaller triangles. mainArea the main triangle being the sum of the smaller triangles.
float getScale(float subArea, float mainArea)
{
	return subArea / mainArea;
}

//=============================================================================//

int main(int argc, char **argv)
{
	if(argc < 3) {
      cout << "Usage: raster meshfile imagefile" << endl;
      return 0;
   }
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);

	//set g_width and g_height appropriately! [ADDED]
	g_width = atoi(argv[3]);
   g_height = atoi(argv[4]);

   //the mode chosen by the user. 1 or 2. [ADDED]
   int mode = atoi(argv[5]);

   //create an image
	auto image = make_shared<Image>(g_width, g_height);

	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;

   //Generating zBuf (2D Array) filled with -infinity. [ADDED]
   vector<vector<float>> zBuf;
	for (int i = 0; i < g_width; i++)
   {
		vector<float> xBuf(g_height, 0);
		zBuf.push_back(xBuf);
	}

	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) {
		cerr << errStr << endl;
	} else {
 		//keep this code to resize your object to be within -1 -> 1
   	resize_obj(shapes); 
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	cout << "Number of triangles: " << triBuf.size()/3 << endl;

	//TODO add code to iterate through each triangle and rasterize it
   //Mainting the aspect ration. Given in notes. [ADDED]
   float l, r, b, t;
   if(g_width >= g_height)
   {
      l = -1 * (g_width / g_height);
      r = (g_width / g_height);
      b = -1;
      t = 1;
   }
   else
   {
      l = -1;
      r = 1;
      b = -1 * (g_height / g_width);
      t = (g_height / g_width);
   }

   //Applying ratio changes to triangle values. Given in notes. [ADDED]
   float c, d, e, f;
   c = (g_width) / (r - l);
   d = -1 * (c * l);
   e = (g_height) / (t - b);
   f = -1 * (e * b);

   //Getting triangles and drawing pixels. [ADDED]
   for(int i=0; i< triBuf.size(); i+=3)
   {
      Triangle triangle;
      BoundingBox box;
      Point p1;
      Point p2;
      Point p3;

      //Xp = c * Xw + d.
      //Yp = e * Yw + f.
      //Zp = Zw.
      p1.x = int(c) * posBuf[3*triBuf[i + 0] + 0] + d; //x.
      p1.y = int(e) * posBuf[3*triBuf[i + 0] + 1] + f; //y.
      p1.z = posBuf[3*triBuf[i + 0] + 2];              //z.

      p2.x = int(c) * posBuf[3*triBuf[i + 1] + 0] + d; //x.
      p2.y = int(e) * posBuf[3*triBuf[i + 1] + 1] + f; //y.
      p2.z = posBuf[3*triBuf[i + 1] + 2];              //z.

      p3.x = int(c) * posBuf[3*triBuf[i + 2] + 0] + d; //x.
      p3.y = int(e) * posBuf[3*triBuf[i + 2] + 1] + f; //y.
      p3.z = posBuf[3*triBuf[i + 2] + 2];              //z.
      
      triangle = createTriangle(p1,p2,p3);
      box = createBoundingBox(triangle);

      for(int y = box.ymin; y < box.ymax; y++)
      {
         for(int x = box.xmin; x < box.xmax; x++)
         {
			   //Get area of the sub-triangles in main triangle.
			   float a1 = getArea(p1.x, p1.y, p2.x, p2.y, x, y);
			   float a2 = getArea(p2.x, p2.y, p3.x, p3.y, x, y);
			   float a3 = getArea(p3.x, p3.y, p1.x, p1.y, x, y);

			   //Area of main triangle.
			   float aM = a1 + a2 + a3;

			   //Getting alpha, beta, gamma.
			   float alpha = getScale(a1, aM);
			   float beta = getScale(a2, aM);
			   float gamma = getScale(a3, aM);

			   //If parts in 0->1 area. Point is inside triangle so draw pixel!
			   if( (alpha >= 0 && alpha <= 1) && (beta >= 0 && beta <= 1) && (gamma >= 0 && gamma <= 1) )
			   {
               //Depth creation.
               float current_z = (alpha*triangle.p3.z) + (beta*triangle.p1.z) + (gamma*triangle.p2.z) + 1;
               if(current_z > zBuf[x][y])
               {
                  zBuf[x][y] = current_z;
               }
               if(mode == 1)
               {
                  image->setPixel(x, y, (255/2) * zBuf[x][y], 0, 0);
               }
               else
               {
                  if( (alpha >= 0 && alpha <= 0.05) || (beta >= 0 && beta <= 0.05) || (gamma >= 0 && gamma <= 0.05) )
                  {
                     image->setPixel(x, y, (255/2) * zBuf[x][y], 0, 0);
                  }
               }
			   }
         }
      }
   }

	//write out the image
   image->writeToFile(imgName);

	return 0;
}
