#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "shape.h"

class Triangle final : public Shape {
public:
  Vector a, b, c;
  Vector e1, e2;
  Vector normal;
  double d11, d12, d22, invDenom;

  Triangle(Vector c, Vector b, Vector a, Texture *t);

  double getIntersection(const Ray &ray);
  bool getLightIntersection(const Ray &ray, double *fill);

  void move();
  void getColor(unsigned char *toFill, double *am, double *op, double *ref,
                Autonoma *r, const Ray &ray, unsigned int depth);
  Vector getNormal(Vector point);
  unsigned char reversible();
  void setAngles(double yaw, double pitch, double roll);
  void setYaw(double d);
  void setPitch(double d);
  void setRoll(double d);
};

#endif
