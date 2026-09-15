
#include "camera.h"
#include "light.h"
#include "shape.h"
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

void Autonoma::addShape(Shape *s) { shapes.push_back(s); }
void Autonoma::addLight(Light *s) { lights.push_back(s); }

void getLight(double *tColor, Autonoma *aut, Vector point, Vector norm,
              unsigned char flip) {
  tColor[0] = tColor[1] = tColor[2] = 0.;

  for (Light *light : aut->lights) {
    double lightColor[3];
    lightColor[0] = light->color[0] / 255.;
    lightColor[1] = light->color[1] / 255.;
    lightColor[2] = light->color[2] / 255.;
    Vector ra = light->center - point;

    bool hit = false;
    for (Shape *s : aut->shapes) {
      if (s->getLightIntersection(Ray(point + ra * .01, ra), lightColor)) {
        hit = true;
        break;
      }
    }

    double perc = (norm.dot(ra) / (ra.mag() * norm.mag()));
    if (!hit) {
      if (flip && perc < 0)
        perc = -perc;
      if (perc > 0) {

        tColor[0] += perc * (lightColor[0]);
        tColor[1] += perc * (lightColor[0]);
        tColor[2] += perc * (lightColor[0]);
        if (tColor[0] > 1.)
          tColor[0] = 1.;
        if (tColor[1] > 1.)
          tColor[1] = 1.;
        if (tColor[2] > 1.)
          tColor[2] = 1.;
      }
    }
  }
}
