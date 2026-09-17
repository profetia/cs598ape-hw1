#ifndef __BVH_H__
#define __BVH_H__
#include "vector.h"

#include <cstdint>
#include <span>
#include <vector>

class Triangle;

struct BBox {
  Vector close, far;
};

class BVH {
public:
  BVH(const std::vector<Triangle *> &triangles, uint32_t leafSize = 256);

  uint32_t nearestTriangle(const Ray &ray, double *t) const;

private:
  struct Node {
    BBox box;
    uint32_t start, end;
    uint32_t leftChild;
  };

  void split(const std::vector<Triangle *> &triangles,
             std::span<uint32_t> slots, uint32_t offset, uint32_t node);
  bool shouldSplit(std::span<uint32_t> slots) const;
  size_t partition(const std::vector<Triangle *> &triangles,
                   std::span<uint32_t> slots) const;
  bool intersects(const Node &node, const Ray &ray, const Vector &inverse,
                  double *entry) const;
  uint32_t nearestTriangleLeaf(const Node &node, const Ray &ray,
                               double *t) const;

  uint32_t leafSize;
  std::vector<Node> nodes;
  std::vector<uint32_t> order;
  std::vector<double> cx, cy, cz, e1x, e1y, e1z, e2x, e2y, e2z;
};

#endif
