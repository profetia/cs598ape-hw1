
#include "camera.h"
#include "light.h"
#include "shape.h"
#include "triangle.h"
#include <unordered_set>

Light::Light(const Vector &cente, unsigned char *colo) : center(cente) {
  color = colo;
}

Light::~Light() { free(color); }

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

Autonoma::~Autonoma() {
  std::unordered_set<Texture *> textures;
  textures.insert(skybox);

  for (Shape *shape : shapes) {
    textures.insert(shape->texture);
    if (shape->normalMap != NULL)
      textures.insert(shape->normalMap);
    delete shape;
  }

  for (Light *light : lights)
    delete light;

  for (Texture *texture : textures)
    delete texture;
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

void Autonoma::addLight(Light *s) { lights.push_back(s); }

void Autonoma::build() { bvh.emplace(triangles); }

Triangle *Autonoma::nearestTriangle(const Ray &ray, double *t) {
  if (triangles.empty()) {
    *t = inf;
    return nullptr;
  }

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
  if (allOpaque) {
    double t;
    if (nearestTriangle(ray, &t) != nullptr && t < 1.)
      return true;
    for (Shape *s : others)
      if (s->getLightIntersection(ray, lightColor))
        return true;
    return false;
  }
  for (Shape *s : shapes)
    if (s->getLightIntersection(ray, lightColor))
      return true;
  return false;
}

void getLight(double *tColor, Autonoma *aut, Vector point, Vector norm,
              unsigned char flip) {
  tColor[0] = tColor[1] = tColor[2] = 0.;

  for (Light *light : aut->lights) {
    double lightColor[3];
    lightColor[0] = light->color[0] / 255.;
    lightColor[1] = light->color[1] / 255.;
    lightColor[2] = light->color[2] / 255.;
    Vector ra = light->center - point;

    if (aut->blocked(Ray(point + ra * 0.01, ra), lightColor))
      continue;

    double perc = (norm.dot(ra) / (ra.mag() * norm.mag()));
    if (flip && perc < 0)
      perc = -perc;
    if (perc > 0) {
      tColor[0] = fmin(tColor[0] + perc * lightColor[0], 1.);
      tColor[1] = fmin(tColor[1] + perc * lightColor[0], 1.);
      tColor[2] = fmin(tColor[2] + perc * lightColor[0], 1.);
    }
  }
}
