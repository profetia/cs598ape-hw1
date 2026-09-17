#include "bvh.h"
#include "triangle.h"

#include <algorithm>
#include <utility>

static double component(const Vector &v, uint32_t axis) {
  return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

static Vector centroid(const Triangle *triangle) {
  return (triangle->a + triangle->b + triangle->c) * (1. / 3.);
}

BVH::BVH(const std::vector<Triangle *> &triangles, uint32_t leafSize)
    : leafSize(leafSize) {
  const uint32_t n = triangles.size();
  order.resize(n);
  for (uint32_t i = 0; i < n; i++)
    order[i] = i;

  nodes.resize(1);
  split(triangles, std::span<uint32_t>(order), 0, 0);

  cx.resize(n);
  cy.resize(n);
  cz.resize(n);
  e1x.resize(n);
  e1y.resize(n);
  e1z.resize(n);
  e2x.resize(n);
  e2y.resize(n);
  e2z.resize(n);
  for (uint32_t slot = 0; slot < n; slot++) {
    const Triangle *triangle = triangles[order[slot]];
    cx[slot] = triangle->c.x;
    cy[slot] = triangle->c.y;
    cz[slot] = triangle->c.z;
    e1x[slot] = triangle->e1.x;
    e1y[slot] = triangle->e1.y;
    e1z[slot] = triangle->e1.z;
    e2x[slot] = triangle->e2.x;
    e2y[slot] = triangle->e2.y;
    e2z[slot] = triangle->e2.z;
  }
}

void BVH::split(const std::vector<Triangle *> &triangles,
                std::span<uint32_t> slots, uint32_t offset, uint32_t node) {
  Vector close(inf, inf, inf), far(-inf, -inf, -inf);
  for (uint32_t slot : slots) {
    const Triangle *triangle = triangles[slot];
    const Vector corners[3] = {triangle->a, triangle->b, triangle->c};
    for (const Vector &corner : corners) {
      close = Vector::min(close, corner);
      far = Vector::max(far, corner);
    }
  }

  nodes[node].box.close = close;
  nodes[node].box.far = far;
  nodes[node].start = offset;
  nodes[node].end = offset + slots.size();
  nodes[node].leftChild = 0;

  if (!shouldSplit(slots))
    return;

  const size_t middle = partition(triangles, slots);

  const uint32_t left = nodes.size();
  nodes.resize(left + 2);
  nodes[node].leftChild = left;
  split(triangles, slots.subspan(0, middle), offset, left);
  split(triangles, slots.subspan(middle), offset + middle, left + 1);
}

bool BVH::shouldSplit(std::span<uint32_t> slots) const {
  return slots.size() > leafSize && slots.size() > 1;
}

size_t BVH::partition(const std::vector<Triangle *> &triangles,
                      std::span<uint32_t> slots) const {
  Vector close(inf, inf, inf), far(-inf, -inf, -inf);
  for (uint32_t slot : slots) {
    const Vector middle = centroid(triangles[slot]);
    close = Vector::min(close, middle);
    far = Vector::max(far, middle);
  }

  const Vector extent = far - close;
  uint32_t axis = 0;
  if (extent.y > extent.x)
    axis = 1;
  if (extent.z > component(extent, axis))
    axis = 2;

  const size_t middle = slots.size() / 2;
  std::nth_element(slots.begin(), slots.begin() + middle, slots.end(),
                   [&](uint32_t left, uint32_t right) {
                     return component(centroid(triangles[left]), axis) <
                            component(centroid(triangles[right]), axis);
                   });
  return middle;
}

bool BVH::intersects(const Node &node, const Ray &ray, const Vector &inverse,
                     double *entry) const {
  const double x0 = (node.box.close.x - ray.point.x) * inverse.x;
  const double x1 = (node.box.far.x - ray.point.x) * inverse.x;
  const double y0 = (node.box.close.y - ray.point.y) * inverse.y;
  const double y1 = (node.box.far.y - ray.point.y) * inverse.y;
  const double z0 = (node.box.close.z - ray.point.z) * inverse.z;
  const double z1 = (node.box.far.z - ray.point.z) * inverse.z;

  const double near = std::fmax(std::fmax(std::fmin(x0, x1), std::fmin(y0, y1)),
                                std::fmin(z0, z1));
  const double away = std::fmin(std::fmin(std::fmax(x0, x1), std::fmax(y0, y1)),
                                std::fmax(z0, z1));

  *entry = near;
  return near <= away && away > 0.;
}

uint32_t BVH::nearestTriangleLeaf(const Node &node, const Ray &ray,
                                  double *t) const {
  const double dx = ray.vector.x, dy = ray.vector.y, dz = ray.vector.z;
  const double ox = ray.point.x, oy = ray.point.y, oz = ray.point.z;

  double best = inf;
  uint32_t bestSlot = node.end;
  for (uint32_t i = node.start; i < node.end; i++) {
    const double sx = ox - cx[i], sy = oy - cy[i], sz = oz - cz[i];

    const double px = dy * e2z[i] - dz * e2y[i];
    const double py = dz * e2x[i] - dx * e2z[i];
    const double pz = dx * e2y[i] - dy * e2x[i];
    const double det = e1x[i] * px + e1y[i] * py + e1z[i] * pz;
    const double inverse = 1. / det;

    const double u = (sx * px + sy * py + sz * pz) * inverse;

    const double qx = sy * e1z[i] - sz * e1y[i];
    const double qy = sz * e1x[i] - sx * e1z[i];
    const double qz = sx * e1y[i] - sy * e1x[i];
    const double v = (dx * qx + dy * qy + dz * qz) * inverse;

    const double tt = (e2x[i] * qx + e2y[i] * qy + e2z[i] * qz) * inverse;

    const bool hit = (det != 0.) & (u >= 0.) & (u <= 1.) & (v >= 0.) &
                     (u + v <= 1.) & (tt > 0.);
    const double ti = hit ? tt : inf;
    if (ti < best) {
      best = ti;
      bestSlot = i;
    }
  }

  *t = best;
  return bestSlot < node.end ? order[bestSlot] : order.size();
}

uint32_t BVH::nearestTriangle(const Ray &ray, double *t) const {
  const Vector inverse = 1. / ray.vector;

  std::vector<std::pair<uint32_t, double>> pending;
  pending.reserve(32);

  double entry;
  if (intersects(nodes[0], ray, inverse, &entry))
    pending.push_back({0, entry});

  double best = inf;
  uint32_t bestTriangle = order.size();
  while (!pending.empty()) {
    const auto [index, entryDistance] = pending.back();
    pending.pop_back();
    if (entryDistance > best)
      continue;

    const Node &node = nodes[index];
    if (node.leftChild == 0) {
      double leafTime;
      const uint32_t triangle = nearestTriangleLeaf(node, ray, &leafTime);
      if (leafTime < best || (leafTime == best && triangle < bestTriangle)) {
        best = leafTime;
        bestTriangle = triangle;
      }
      continue;
    }

    double leftEntry, rightEntry;
    const bool left =
        intersects(nodes[node.leftChild], ray, inverse, &leftEntry);
    const bool right =
        intersects(nodes[node.leftChild + 1], ray, inverse, &rightEntry);

    if (left && right) {
      const bool leftFirst = leftEntry <= rightEntry;
      pending.push_back({node.leftChild + (leftFirst ? 1u : 0u),
                         leftFirst ? rightEntry : leftEntry});
      pending.push_back({node.leftChild + (leftFirst ? 0u : 1u),
                         leftFirst ? leftEntry : rightEntry});
    } else if (left) {
      pending.push_back({node.leftChild, leftEntry});
    } else if (right) {
      pending.push_back({node.leftChild + 1, rightEntry});
    }
  }

  *t = best;
  return bestTriangle;
}
