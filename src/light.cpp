
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

void Autonoma::addShape(Triangle *s) { triangles.push_back(s); }
void Autonoma::addShape(Shape *s) { shapes.push_back(s); }
void Autonoma::addLight(Light &&r) { lights.push_back(r); }

void getLight(double *tColor, Autonoma *aut, Vector point, Vector norm,
              unsigned char flip) {

  auto handleLight = [&](const Light &t) {
    double lightColor[3];
    lightColor[0] = t.color[0] / 255.;
    lightColor[1] = t.color[1] / 255.;
    lightColor[2] = t.color[2] / 255.;
    Vector ra = t.center - point;

    aut->mapShapes([&](auto *s) {
      if (s->getLightIntersection(Ray(point + ra * 0.01, ra), lightColor))
        return true;
      double perc = (norm.dot(ra) / (ra.mag() * norm.mag()));
      if (flip && perc < 0)
        perc = -perc;
      if (perc > 0) {
        tColor[0] += fmin(perc * (lightColor[0]), 1.);
        tColor[1] += fmin(perc * (lightColor[1]), 1.);
        tColor[2] += fmin(perc * (lightColor[2]), 1.);
      }
      return false;
    });
  };

  tColor[0] = tColor[1] = tColor[2] = 0.;
  for (const Light &t : aut->lights)
    handleLight(t);
}
