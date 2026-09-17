
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
  cx.push_back(s->c.x);
  cy.push_back(s->c.y);
  cz.push_back(s->c.z);
  e1x.push_back(s->e1.x);
  e1y.push_back(s->e1.y);
  e1z.push_back(s->e1.z);
  e2x.push_back(s->e2.x);
  e2y.push_back(s->e2.y);
  e2z.push_back(s->e2.z);
}

void Autonoma::addShape(Shape *s) {
  shapes.push_back(s);
  others.push_back(s);
}

void Autonoma::addLight(Light &&r) { lights.push_back(r); }

Triangle *Autonoma::nearestTriangle(const Ray &ray, double *t) {
  const size_t n = triangles.size();
  const double dx = ray.vector.x, dy = ray.vector.y, dz = ray.vector.z;
  const double ox = ray.point.x, oy = ray.point.y, oz = ray.point.z;

  double best = inf;
  size_t idx = n;
  for (size_t i = 0; i < n; i++) {
    const double sx = ox - cx[i], sy = oy - cy[i], sz = oz - cz[i];

    const double px = dy * e2z[i] - dz * e2y[i];
    const double py = dz * e2x[i] - dx * e2z[i];
    const double pz = dx * e2y[i] - dy * e2x[i];
    const double det = e1x[i] * px + e1y[i] * py + e1z[i] * pz;
    const double inv = 1. / det;

    const double u = (sx * px + sy * py + sz * pz) * inv;

    const double qx = sy * e1z[i] - sz * e1y[i];
    const double qy = sz * e1x[i] - sx * e1z[i];
    const double qz = sx * e1y[i] - sy * e1x[i];
    const double v = (dx * qx + dy * qy + dz * qz) * inv;

    const double tt = (e2x[i] * qx + e2y[i] * qy + e2z[i] * qz) * inv;

    const bool hit = (det != 0.) & (u >= 0.) & (u <= 1.) & (v >= 0.) &
                     (u + v <= 1.) & (tt > 0.);
    const double ti = hit ? tt : inf;
    if (ti < best) {
      best = ti;
      idx = i;
    }
  }

  *t = best;
  return idx < n ? triangles[idx] : nullptr;
}

void getLight(double *tColor, Autonoma *aut, Vector point, Vector norm,
              unsigned char flip) {

  auto handleLight = [&](const Light &t) {
    double lightColor[3];
    lightColor[0] = t.color[0] / 255.;
    lightColor[1] = t.color[1] / 255.;
    lightColor[2] = t.color[2] / 255.;
    Vector ra = t.center - point;

    for (Shape *s : aut->shapes) {
      if (s->getLightIntersection(Ray(point + ra * 0.01, ra), lightColor))
        return;
      double perc = (norm.dot(ra) / (ra.mag() * norm.mag()));
      if (flip && perc < 0)
        perc = -perc;
      if (perc > 0) {
        tColor[0] += fmin(perc * (lightColor[0]), 1.);
        tColor[1] += fmin(perc * (lightColor[1]), 1.);
        tColor[2] += fmin(perc * (lightColor[2]), 1.);
      }
    }
  };

  tColor[0] = tColor[1] = tColor[2] = 0.;
  for (const Light &t : aut->lights)
    handleLight(t);
}
