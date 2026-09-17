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

  std::vector<Light> lights;
  Autonoma(const Camera &c);
  Autonoma(const Camera &c, Texture *tex);

  void addShape(Triangle *s);
  void addShape(Shape *s);
  void addLight(Light &&s);

  int numShapes() { return shapes.size() + triangles.size(); }
  Shape *indexShape(int idx) {
    if (idx < triangles.size())
      return (Shape *)triangles[idx];
    return shapes[idx - triangles.size()];
  }

  template <typename F> void mapShapes(F func) {
    for (Triangle *triangle : triangles)
      if (func(triangle))
        return;
    for (Shape *shape : shapes)
      if (func(shape))
        return;
  };

private:
  std::vector<Triangle *> triangles;
  std::vector<Shape *> shapes;
};

void getLight(double *toFill, Autonoma *aut, Vector point, Vector norm,
              unsigned char r);

#endif
