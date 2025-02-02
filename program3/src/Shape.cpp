#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
	texOff = true;
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;
}

void Shape::measure() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
   max.x = maxX;
   max.y = maxY;
   max.z = maxZ;
}

void Shape::init()
{
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		cout << "We in Here" << endl;

		//norBufID = 0;
		norBuf.clear();

		for (int i=0; i<posBuf.size(); i++)
			norBuf.push_back(0.0);

		for (int i=0; i<eleBuf.size(); i+=3)
		{
			float v1x = posBuf[3 * eleBuf[(i + 1)] + 0] - posBuf[3 * eleBuf[i] + 0];
			float v1y = posBuf[3 * eleBuf[(i + 1)] + 1] - posBuf[3 * eleBuf[i] + 1];
      		float v1z = posBuf[3 * eleBuf[(i + 1)] + 2] - posBuf[3 * eleBuf[i] + 2];

      		float v2x = posBuf[3 * eleBuf[(i + 2)] + 0] - posBuf[3 * eleBuf[i] + 0];
      		float v2y = posBuf[3 * eleBuf[(i + 2)] + 1] - posBuf[3 * eleBuf[i] + 1];
      		float v2z = posBuf[3 * eleBuf[(i + 2)] + 2] - posBuf[3 * eleBuf[i] + 2];

      		float x = (v1y * v2z) - (v1z * v2y);
      		float y = (v1z * v2x) - (v1x * v2z);
      		float z = (v1x * v2y) - (v1y * v2x);

      		norBuf[3 * eleBuf[i] + 0] += x;
      		norBuf[3 * eleBuf[i] + 1] += y;
      		norBuf[3 * eleBuf[i] + 2] += z;

      		norBuf[3 * eleBuf[i + 1] + 0] += x;
      		norBuf[3 * eleBuf[i + 1] + 1] += y;
      		norBuf[3 * eleBuf[i + 1] + 2] += z;

      		norBuf[3 * eleBuf[i + 2] + 0] += x;
      		norBuf[3 * eleBuf[i + 2] + 1] += y;
      		norBuf[3 * eleBuf[i + 2] + 2] += z;
		}

		for(int i=0; i<norBuf.size(); i+=3)
   		{
      		float nx = norBuf[i + 0];
      		float ny = norBuf[i + 1];
      		float nz = norBuf[i + 2];
      		glm::vec3 normalized = glm::vec3(nx, ny, nz);
      		normalized = glm::normalize(normalized);

      		norBuf[i + 0] = normalized.x;
      		norBuf[i + 1] = normalized.y;
      		norBuf[i + 2] = normalized.z;
   		}

   		glGenBuffers(1, &norBufID);
   		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
   		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), norBuf.data(), GL_STATIC_DRAW);

	} else {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty() || texOff) {
		texBufID = 0;
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
