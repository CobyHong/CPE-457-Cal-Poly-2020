/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
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
		if (shapes[i].mesh.positions[3 * v + 0] < minX) minX = shapes[i].mesh.positions[3 * v + 0];
		if (shapes[i].mesh.positions[3 * v + 0] > maxX) maxX = shapes[i].mesh.positions[3 * v + 0];

		if (shapes[i].mesh.positions[3 * v + 1] < minY) minY = shapes[i].mesh.positions[3 * v + 1];
		if (shapes[i].mesh.positions[3 * v + 1] > maxY) maxY = shapes[i].mesh.positions[3 * v + 1];

		if (shapes[i].mesh.positions[3 * v + 2] < minZ) minZ = shapes[i].mesh.positions[3 * v + 2];
		if (shapes[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shapes[i].mesh.positions[3 * v + 2];
	}
}

//From min and max compute necessary scale and shift for each dimension
float maxExtent, xExtent, yExtent, zExtent;
xExtent = maxX - minX;
yExtent = maxY - minY;
zExtent = maxZ - minZ;
if (xExtent >= yExtent && xExtent >= zExtent) {
	maxExtent = xExtent;
}
if (yExtent >= xExtent && yExtent >= zExtent) {
	maxExtent = yExtent;
}
if (zExtent >= xExtent && zExtent >= yExtent) {
	maxExtent = zExtent;
}
scaleX = 2.0 / maxExtent;
shiftX = minX + (xExtent / 2.0);
scaleY = 2.0 / maxExtent;
shiftY = minY + (yExtent / 2.0);
scaleZ = 2.0 / maxExtent;
shiftZ = minZ + (zExtent) / 2.0;

//Go through all verticies shift and scale them
for (size_t i = 0; i < shapes.size(); i++) {
	for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
		shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
		assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
		assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
		shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
		assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
		assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
		shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
		assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
		assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
	}
}
}

class Triangle {
public:
	int x1;
	int y1;
	float z1;
	int r1;
	int g1;
	int b1;

	int x2;
	int y2;
	float z2;
	int r2;
	int g2;
	int b2;

	int x3;
	int y3;
	float z3;
	int r3;
	int g3;
	int b3;
};

Triangle genTri(int x1, int y1, int z1, int r1, int g1, int b1, int x2, int y2, int z2, int r2, int g2, int b2, int x3, int y3, int z3, int r3, int g3, int b3) {
	Triangle tri;

	tri.x1 = x1;	// Vertex 1 x-coord
	tri.y1 = y1;	// Vertex 1 y-coord
	tri.z1 = z1;	// Vertex 1 z-coord
	tri.r1 = r1;	// vertex 1 RGB values
	tri.g1 = g1;
	tri.b1 = b1;

	tri.x2 = x2;	// Vertex 2 x-coord
	tri.y2 = y2;	// Vertex 2 y-coord
	tri.z2 = z2;	// Vertex 2 z-coord
	tri.r2 = r2;	// vertex 2 RGB values
	tri.g2 = g2;
	tri.b2 = b2;

	tri.x3 = x3;	// Vertex 3 x-coord
	tri.y3 = y3;	// Vertex 3 y-coord
	tri.z3 = z3;	// Vertex 3 z-coord
	tri.r3 = r3;	// vertex 3 RGB values
	tri.g3 = g3;
	tri.b3 = b3;

	return tri;
}

float calcArea(float ax, float ay, float bx, float by, float cx, float cy) {
	return 0.5*(((bx - ax)*(cy - ay)) - ((cx - ax)*(by - ay)));
}

float calcScale(float subArea, float totalArea) {
	return subArea / totalArea;
}

int largestInt(int x, int y, int z) {
	int temp = x;

	if (y > temp)
		temp = y;
	if (z > temp)
		temp = z;
	return temp;
}

int smallestInt(int x, int y, int z) {
	int temp = x;

	if (y < temp)
		temp = y;
	if (z < temp)
		temp = z;
	return temp;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		cout << "Usage: raster meshfile imagefile" << endl;
		return 0;
	}
	string meshName(argv[1]); // OBJ filename
	string imgName(argv[2]);

	g_width = atoi(argv[3]); //set g_width and g_height
	g_height = atoi(argv[4]);

	int modeFlag = atoi(argv[5]);

	auto image = make_shared<Image>(g_width, g_height); //create an image

	vector<unsigned int> triBuf;		// triangle buffer
	vector<float> posBuf;				// position buffer
	vector<vector<float>> zBuf;			//image pixel buffer
	for (int i = 0; i < g_width; i++) {	//init pixel buffer to 0
		vector<float> xBuf(g_height, 0);
		zBuf.push_back(xBuf);
	}

	// Some obj files contain material information.
	// We'll ignore them for this assignment.

	vector<tinyobj::shape_t> shapes;			// geometry
	vector<tinyobj::material_t> objMaterials;	// material
	string errStr;

	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if (!rc) {
		cerr << errStr << endl;
	}
	else {
		//keep this code to resize your object to be within -1 -> 1
		resize_obj(shapes);
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << posBuf.size() / 3 << endl;	//display number of vertices and trianlges in .obj file
	cout << "Number of triangles: " << triBuf.size() / 3 << endl;

	Triangle tri;

	//calculate aspect ratio
	float l, r, b, t;
	if (g_width >= g_height) {
		l = -1 * g_width / g_height;
		r = g_width / g_height;
		b = -1;
		t = 1;
	}
	else {
		l = -1;
		r = 1;
		b = -1 * g_height / g_width;
		t = g_height / g_width;
	}

	float c = g_width / (r - l);
	float d = -1 * c * l;
	float e = g_height / (t - b);
	float f = -1 * e * b;

	for (int i = 0; i < triBuf.size(); i+=3){

		//Set triangle vertices
		tri.x1 = (int)c * posBuf[3*triBuf[i]] + d;
		tri.y1 = (int)e * posBuf[3*triBuf[i] + 1] + f;
		tri.z1 = posBuf[3*triBuf[i] + 2];

		tri.x2 = (int)c * posBuf[3*triBuf[i+1]] + d;
		tri.y2 = (int)e * posBuf[3*triBuf[i+1] + 1] + f;
		tri.z2 = posBuf[3*triBuf[i+1] + 2];

		tri.x3 = (int)c * posBuf[3*triBuf[i+2]] + d;
		tri.y3 = (int)e * posBuf[3*triBuf[i+2] + 1] + f;
		tri.z3 = posBuf[3*triBuf[i+2] + 2];

		// calculate squarebounds
		int minX = smallestInt(tri.x1, tri.x2, tri.x3);
		int maxX = largestInt(tri.x1, tri.x2, tri.x3);
		int minY = smallestInt(tri.y1, tri.y2, tri.y3);
		int maxY = largestInt(tri.y1, tri.y2, tri.y3);

		float colorInc = 255.0 / g_height;

		// Draw a rectangle
		for (int y = minY; y < maxY; y++) {
			for (int x = minX; x < maxX; x++) {
				
				//calculate sub areas
				float a1 = calcArea(tri.x1, tri.y1, tri.x2, tri.y2, x, y);
				float a2 = calcArea(tri.x2, tri.y2, tri.x3, tri.y3, x, y);
				float a3 = calcArea(tri.x3, tri.y3, tri.x1, tri.y1, x, y);

				//calculate alpha, beta, and gamma
				float aT = a1 + a2 + a3;
				float s1 = calcScale(a1, aT);
				float s2 = calcScale(a2, aT);
				float s3 = calcScale(a3, aT);

				if (s1 >= 0 && s1 <= 1 && s2 >= 0 && s2 <= 1 && s3 >= 0 && s3 <= 1) {	//check if within triangle
					//calculate z buffer values
					float zVal = (s1*(tri.z3) + s2*(tri.z1) + s3*(tri.z2)) + 1;
					if (zVal > zBuf[x][y]) {
						zBuf[x][y] = zVal;
					}
					if (modeFlag == 1)	// in mode 1
						image->setPixel(x, y, (255 / 2)*zBuf[x][y], 0, 0);
					else				// in mode 2
						image->setPixel(x, y, 255 - (colorInc * y), 0, colorInc * y);
				}
			}
		}
	}
	image->writeToFile(imgName);	//write out the image
		return 0;
}
