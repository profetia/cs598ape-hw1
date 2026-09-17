
#include "camera.h"
#include "light.h"
#include "shape.h"
#include "triangle.h"

Light::Light(const Vector &cente, unsigned char *colo) : center(cente) {
  color = colo;
}

unsigned char *Light::getColor(unsigned char a, unsigned char b,
                               unsigned char c) {
  unsigned char *r = (unsigned char *)malloc(sizeof(unsigned char) * 3);
  r[0] = a;
  r[1] = b;
  r[2] = c;
  return r;
}

Autonoma::Autonoma(const Camera &c) : camera(c) {
  depth = 10;
  skybox = BLACK;
}

Autonoma::Autonoma(const Camera &c, Texture *tex) : camera(c) {
  depth = 10;
  skybox = tex;
}

void Autonoma::addShape(Triangle *s) {
  shapes.push_back(s);
  triangles.push_back(s);
  allOpaque &= s->texture->opaque();
  bvh.reset();
}

void Autonoma::addShape(Shape *s) {
  shapes.push_back(s);
  others.push_back(s);
  allOpaque &= s->texture->opaque();
}

void Autonoma::addLight(Light &&r) { lights.push_back(r); }

void Autonoma::build() { bvh.emplace(triangles); }

Triangle *Autonoma::nearestTriangle(const Ray &ray, double *t) {
  const uint32_t idx = bvh->nearestTriangle(ray, t);
  return idx < triangles.size() ? triangles[idx] : nullptr;
}

Shape *Autonoma::nearestNonTriangle(const Ray &ray, double *t) {
  double best = inf;
  Shape *nearest = nullptr;
  for (Shape *s : others) {
    const double time = s->getIntersection(ray);
    if (time < best) {
      best = time;
      nearest = s;
    }
  }

  *t = best;
  return nearest;
}

bool Autonoma::blocked(const Ray &ray, double *lightColor) {
#ifndef NO_FAST_SHADOW
  if (allOpaque) {
    double t;
    if (nearestTriangle(ray, &t) != nullptr && t < 1.)
      return true;
    for (Shape *s : others)
      if (s->getLightIntersection(ray, lightColor))
        return true;
    return false;
  }
#endif
  for (Shape *s : shapes)
    if (s->getLightIntersection(ray, lightColor))
      return true;
  return false;
}

void getLight(double *tColor, Autonoma *aut, Vector point, Vector norm,
              unsigned char flip) {

  auto handleLight = [&](const Light &t) {
    double lightColor[3];
    lightColor[0] = t.color[0] / 255.;
    lightColor[1] = t.color[1] / 255.;
    lightColor[2] = t.color[2] / 255.;
    Vector ra = t.center - point;

    if (aut->blocked(Ray(point + ra * 0.01, ra), lightColor))
      return;

    double perc = (norm.dot(ra) / (ra.mag() * norm.mag()));
    if (flip && perc < 0)
      perc = -perc;
    if (perc > 0) {
      tColor[0] += fmin(perc * (lightColor[0]), 1.);
      tColor[1] += fmin(perc * (lightColor[1]), 1.);
      tColor[2] += fmin(perc * (lightColor[2]), 1.);
    }
  };

  tColor[0] = tColor[1] = tColor[2] = 0.;
  for (const Light &t : aut->lights)
    handleLight(t);
}
