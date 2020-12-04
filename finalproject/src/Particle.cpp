//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	a(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start)
{
	// Random initialization
	rebirth(0.0f, start);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start)
{
	time = 0;

	charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x.x = 0;
	x.y = 0;
	x.z = randFloat(-3.f, -3.f);
	v.x = randFloat(-0.8f, 0.8f);
	v.y = randFloat(0.5f, 2.0f);
	v.z = randFloat(-0.1f, 0.1f);
	lifespan = randFloat(0.5f, 2.0f);
	tEnd = t + lifespan;

	scale = randFloat(0.01f, 0.5f);
	color.r = randFloat(0.5, 1.0f);
	color.g = randFloat(0.0f, 0.2f);
	color.b = randFloat(0.0, 0.2f);
	color.a = randFloat(0, 1.0f);
}

void Particle::update(float t, float h, const vec3& g, const vec3 start)
{
	time += 0.01;

	if (t > tEnd)
	{
		rebirth(t, start);
	}

	float dy = v.y * time + 0.5 * -9.81 * (time * time);

	vec3 newV = v;
	newV.y += dy;

	// very simple update
	x += h * newV;
	color.a = (tEnd - t) / lifespan;
}
