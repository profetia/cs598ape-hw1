#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "Textures/colortexture.h"
#include "Textures/texture.h"
#include "camera.h"
#include "vector.h"

#include <vector>

class Light {
public:
  unsigned char *color;
  unsigned char *getColor(unsigned char a, unsigned char b, unsigned char c);
  Vector center;
  Light(const Vector &cente, unsigned char *colo);
};

class Shape;
class Triangle;

class Autonoma {
public:
  Camera camera;
  Texture *skybox;
  unsigned int depth;

  std::vector<Shape *> shapes;
  std::vector<Light> lights;
  Autonoma(const Camera &c);
  Autonoma(const Camera &c, Texture *tex);

  void addShape(Triangle *s);
  void addShape(Shape *s);
  void addLight(Light &&s);

  Triangle *nearestTriangle(const Ray &ray, double *t);
  Shape *nearestNonTriangle(const Ray &ray, double *t);

private:
  std::vector<Shape *> others;
  std::vector<Triangle *> triangles;
  std::vector<double> cx, cy, cz, e1x, e1y, e1z, e2x, e2y, e2z;
};

void getLight(double *toFill, Autonoma *aut, Vector point, Vector norm,
              unsigned char r);

#endif
