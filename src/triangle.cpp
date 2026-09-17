#include "triangle.h"
#include "vector.h"

Triangle::Triangle(Vector c, Vector b, Vector a, Texture *t)
    : Shape(c, t, 0, 0, 0), a(a), b(b), c(c), e1(b - c), e2(a - c),
      normal((c - a).cross(c - b).normalize()) {
  d11 = e1.dot(e1);
  d12 = e1.dot(e2);
  d22 = e2.dot(e2);
  invDenom = 1. / (d11 * d22 - d12 * d12);
}

static double intersect(const Vector &c, Vector e1, Vector e2, const Ray &ray,
                        double *u, double *v) {
  Vector dir = ray.vector;
  Vector s = ray.point - c;

  Vector p = dir.cross(e2);
  const double det = e1.dot(p);
  if (det == 0.)
    return inf;
  const double inv = 1. / det;

  *u = s.dot(p) * inv;
  if (*u < 0. || *u > 1.)
    return inf;

  Vector q = s.cross(e1);
  *v = dir.dot(q) * inv;
  if (*v < 0. || *u + *v > 1.)
    return inf;

  const double t = e2.dot(q) * inv;
  return (t > 0.) ? t : inf;
}

double Triangle::getIntersection(const Ray &ray) {
  double u, v;
  return intersect(c, e1, e2, ray, &u, &v);
}

bool Triangle::getLightIntersection(const Ray &ray, double *fill) {
  double u, v;
  const double r = intersect(c, e1, e2, ray, &u, &v);
  if (r >= 1.)
    return false;

  if (texture->opacity > 1 - 1E-6)
    return true;
  unsigned char temp[4];
  double amb, op, ref;
  texture->getColor(temp, &amb, &op, &ref, fix(u), fix(v));
  if (op > 1 - 1E-6)
    return true;
  fill[0] *= temp[0] / 255.;
  fill[1] *= temp[1] / 255.;
  fill[2] *= temp[2] / 255.;
  return false;
}

void Triangle::move() {}

void Triangle::getColor(unsigned char *toFill, double *am, double *op,
                        double *ref, Autonoma *r, const Ray &ray,
                        unsigned int depth) {
  Vector p = ray.point - c;
  const double dp1 = p.dot(e1);
  const double dp2 = p.dot(e2);
  const double u = (d22 * dp1 - d12 * dp2) * invDenom;
  const double v = (d11 * dp2 - d12 * dp1) * invDenom;
  texture->getColor(toFill, am, op, ref, fix(u), fix(v));
}

Vector Triangle::getNormal(Vector point) { return normal; }

unsigned char Triangle::reversible() { return 1; }

void Triangle::setAngles(double yaw, double pitch, double roll) {
  Shape::setAngles(yaw, pitch, roll);
}

void Triangle::setYaw(double d) { Shape::setYaw(d); }

void Triangle::setPitch(double d) { Shape::setPitch(d); }

void Triangle::setRoll(double d) { Shape::setRoll(d); }
