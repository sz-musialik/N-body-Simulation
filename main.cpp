// clang-format off
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <random>
#include <raylib.h>
#include <utility>
#include <vector>
#include "raymath.h"
// clang-format on

// Randomness
static std::random_device rd;
static std::mt19937 gen(rd());

// static std::lognormal_distribution<double> mass_dist(0.0, 0.7);
static std::normal_distribution<double> mass_dist(0.0, 10);

// Constants
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

const double G = 6.6743e-11;
const double LY_TO_METERS = 9.4607e15; // 1 Light year in meters
const double SOLAR_MASS = 1.989e30;
const double SOLAR_RADIUS = 6.9634e8;

const double SIM_WIDTH_LY = 10.0;
const double SIM_HEIGHT_LY = 10.0;
const double SIM_DEPTH_LY = 10.0;

static Camera3D camera = {.position = {5.0f, 5.0f, -15.0f},
                          .target = {5.0f, 5.0f, 5.0f},
                          .up = {0.0f, 1.0f, 0.0f},
                          .fovy = 60.0f,
                          .projection = CAMERA_PERSPECTIVE};

const int FPS = 60;
double DT;

enum class StellarType {
  MainSequence,
  Giant,
  WhiteDwarf,
  NeutronStar,
  BlackHole
};

double GetRandomFloat(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(gen);
};

// Radius translated from meters to Light Years [world scale]
float RadiusMetersToLy(double radius_m) {
  double radius_ratio = radius_m / SOLAR_RADIUS;

  float base_ly = 0.02f;

  return base_ly * static_cast<float>(pow(radius_ratio, 0.4));
}

class Star {
public:
  Vector3 pos;
  Vector3 vel;
  Vector3 acc; // acceleration
  Vector3 F;   // Force

  double mass;   // in kg
  double radius; // in meters
  Color color;
  bool active;
  StellarType type;

  Star() {
    // Random position in light years
    pos.x = GetRandomFloat(0, SIM_WIDTH_LY);
    pos.y = GetRandomFloat(0, SIM_HEIGHT_LY);
    pos.z = GetRandomFloat(0, SIM_DEPTH_LY);

    // Random velocity in m/s
    vel.x = GetRandomFloat(-20000, 20000);
    vel.y = GetRandomFloat(-20000, 20000);
    vel.z = GetRandomFloat(-20000, 20000);

    acc = {0, 0, 0};
    F = {0, 0, 0};

    InitializeMassRadius();

    color = GetStarColor();

    active = true;
    type = StellarType::MainSequence;
  }

  Matrix GetTransformMatrix() const {
    float radius_ly = RadiusMetersToLy(radius);
    Matrix scale = MatrixScale(radius_ly, radius_ly, radius_ly);
    Matrix translation = MatrixTranslate(pos.x, pos.y, pos.z);

    return MatrixMultiply(scale, translation);
  }

  void ResetForce() {
    F.x = 0;
    F.y = 0;
    F.z = 0;
  }

  void Update(double sim_dt) {
    if (!active)
      return;

    dt = sim_dt;

    UpdateAcceleration();
    UpdateVelocity();
    UpdatePosition();
  }

private:
  float dt;

  void InitializeMassRadius() {
    double m_ratio = mass_dist(gen);
    if (m_ratio < 0.1)
      m_ratio = 0.1;

    mass = m_ratio * SOLAR_MASS;

    if (mass < SOLAR_MASS) {
      radius = SOLAR_RADIUS * std::pow(m_ratio, 0.9);
    } else {
      radius = SOLAR_RADIUS * std::pow(m_ratio, 0.6);
    }
  }

  Color GetStarColor() {
    // Will eventually return/modify a color associated to its fields

    return WHITE;
  }

  void UpdateVelocity() {
    if (!active)
      return;

    vel.x = vel.x + acc.x * dt;
    vel.y = vel.y + acc.y * dt;
    vel.z = vel.z + acc.z * dt;
  }

  void UpdateAcceleration() {
    if (!active)
      return;

    acc.x = F.x / mass;
    acc.y = F.y / mass;
    acc.z = F.z / mass;
  }

  void UpdatePosition() {
    if (!active)
      return;

    pos.x += (vel.x * dt) / LY_TO_METERS;
    pos.y += (vel.y * dt) / LY_TO_METERS;
    pos.z += (vel.z * dt) / LY_TO_METERS;
  }
};

void RenderStarsInstanced(const std::vector<Star> &stars, Mesh sphereMesh,
                          Material sphereMaterial, int colorLoc) {
  static std::vector<Matrix> whiteStars;
  static std::vector<Matrix> redStars;

  whiteStars.clear();
  redStars.clear();

  whiteStars.reserve(stars.size());
  redStars.reserve(stars.size());

  for (const auto &star : stars) {
    if (star.active) {
      Matrix tx = star.GetTransformMatrix();

      if (star.color.r == RED.r && star.color.g == RED.g &&
          star.color.b == RED.b) {
        redStars.push_back(tx);
      } else if (star.color.r == WHITE.r && star.color.g == WHITE.g &&
                 star.color.b == WHITE.b) {
        whiteStars.push_back(tx);
      }
    }
  }

  if (!whiteStars.empty()) {
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    SetShaderValue(sphereMaterial.shader, colorLoc, color, SHADER_UNIFORM_VEC4);
    DrawMeshInstanced(sphereMesh, sphereMaterial, whiteStars.data(),
                      whiteStars.size());
  }

  if (!redStars.empty()) {
    float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    SetShaderValue(sphereMaterial.shader, colorLoc, color, SHADER_UNIFORM_VEC4);
    DrawMeshInstanced(sphereMesh, sphereMaterial, redStars.data(),
                      redStars.size());
  }
}

// float Hypotenuse(Star *s1, Star *s2) {
//   float dx_ly = s1->pos.x - s2->pos.x;
//   float dy_ly = s1->pos.y - s2->pos.y;
//
//   float dx_m = dx_ly * LY_TO_METERS;
//   float dy_m = dy_ly * LY_TO_METERS;
//
//   return sqrt(dx_m * dx_m + dy_m * dy_m);
// }

// void UpdateForce(Star *s1, Star *s2) {
//   if (!s1->active || !s2->active)
//     return;
//
//   float r_m = Hypotenuse(s1, s2);
//
//   // Prevents shooting the star off into space
//   float softening = 1e27;
//
//   // Scalar
//   float force = (G * s1->mass * s2->mass) / (r_m * r_m + softening);
//
//   // Force direction vector in meters
//   float dx_m = (s2->pos.x - s1->pos.x) * LY_TO_METERS;
//   float dy_m = (s2->pos.y - s1->pos.y) * LY_TO_METERS;
//
//   s1->F.x += force * (dx_m / r_m);
//   s1->F.y += force * (dy_m / r_m);
// }

struct OctreeRegion {
  double x;
  double y;
  double z;
  double size;

  bool Contains(Vector3 p) const {
    return (p.x >= x && p.x < x + size && p.y >= y && p.y < y + size &&
            p.z >= z && p.z < z + size);
  }
};

class OctreeNode {
public:
  OctreeRegion region;
  Star *star = nullptr;

  double totalMass = 0.0;
  Vector3 centerOfMass = {0.0f, 0.0f, 0.0f};

  bool isDivided = false;
  std::array<int, 8> children;

  OctreeNode(OctreeRegion reg) : region(reg) {
    children.fill(-1); // -1 means no child
  }

  void Subdivide(std::vector<OctreeNode> &pool) {
    double subSize = region.size / 2.0;

    int firstChildIdx = static_cast<int>(pool.size());

    pool.push_back(
        OctreeNode(OctreeRegion{region.x, region.y, region.z, subSize}));
    pool.push_back(OctreeNode(
        OctreeRegion{region.x + subSize, region.y, region.z, subSize}));
    pool.push_back(OctreeNode(
        OctreeRegion{region.x, region.y + subSize, region.z, subSize}));
    pool.push_back(OctreeNode(OctreeRegion{
        region.x + subSize, region.y + subSize, region.z, subSize}));

    pool.push_back(OctreeNode(
        OctreeRegion{region.x, region.y, region.z + subSize, subSize}));
    pool.push_back(OctreeNode(OctreeRegion{region.x + subSize, region.y,
                                           region.z + subSize, subSize}));
    pool.push_back(OctreeNode(OctreeRegion{region.x, region.y + subSize,
                                           region.z + subSize, subSize}));
    pool.push_back(OctreeNode(OctreeRegion{
        region.x + subSize, region.y + subSize, region.z + subSize, subSize}));

    for (size_t i = 0; i < 8; i++) {
      children[i] = firstChildIdx + i;
    }

    isDivided = true;
  }

  void Insert(Star *newStar, std::vector<OctreeNode> &pool) {
    if (!newStar->active || !region.Contains(newStar->pos))
      return;

    if (totalMass == 0.0) {
      totalMass = newStar->mass;
      centerOfMass = newStar->pos;
    } else {
      double oldMass = totalMass;
      totalMass += newStar->mass;

      centerOfMass.x = static_cast<float>(
          (centerOfMass.x * oldMass + newStar->pos.x * newStar->mass) /
          totalMass);
      centerOfMass.y = static_cast<float>(
          (centerOfMass.y * oldMass + newStar->pos.y * newStar->mass) /
          totalMass);
      centerOfMass.z = static_cast<float>(
          (centerOfMass.z * oldMass + newStar->pos.z * newStar->mass) /
          totalMass);
    }

    if (!isDivided && star == nullptr) {
      star = newStar;
      return;
    }

    if (region.size < 0.001)
      return;

    if (!isDivided) {
      Subdivide(pool);

      Star *existingStar = star;
      star = nullptr;

      for (int childIdx : children) {
        if (pool[childIdx].region.Contains(existingStar->pos)) {
          pool[childIdx].Insert(existingStar, pool);
          break; // Star can be only in 1 subregion
        }
      }
    }

    for (int childIdx : children) {
      if (pool[childIdx].region.Contains(newStar->pos)) {
        pool[childIdx].Insert(newStar, pool);
        break;
      }
    }
  }

  void CalculateStarForce(Star *targetStar, double theta, double softening,
                          const std::vector<OctreeNode> &pool) const {
    if (totalMass == 0.0 || !targetStar->active)
      return;

    if (star == targetStar)
      return;

    double dx_ly = centerOfMass.x - targetStar->pos.x;
    double dy_ly = centerOfMass.y - targetStar->pos.y;
    double dz_ly = centerOfMass.z - targetStar->pos.z;
    double dist_ly = sqrt(dx_ly * dx_ly + dy_ly * dy_ly + dz_ly * dz_ly);

    if (dist_ly < 1e-6)
      return;

    if (!isDivided || (region.size / dist_ly) < theta || region.size < 0.001) {
      // Conversion to meters
      double r_m = dist_ly * LY_TO_METERS;
      double dx_m = dx_ly * LY_TO_METERS;
      double dy_m = dy_ly * LY_TO_METERS;
      double dz_m = dz_ly * LY_TO_METERS;

      // Newton Force
      double force =
          (G * targetStar->mass * totalMass) / (r_m * r_m + softening);

      targetStar->F.x += static_cast<float>(force * (dx_m / r_m));
      targetStar->F.y += static_cast<float>(force * (dy_m / r_m));
      targetStar->F.z += static_cast<float>(force * (dz_m / r_m));
    } else {
      for (int childIdx : children) {
        if (childIdx != -1) {
          pool[childIdx].CalculateStarForce(targetStar, theta, softening, pool);
        }
      }
    }
  }

  void CheckCollisionWithTree(Star *targetStar,
                              const std::vector<OctreeNode> &pool) {
    if (totalMass == 0.0 || !targetStar->active)
      return;

    if (!isDivided && star != nullptr && star != targetStar) {
      if (!star->active)
        return;

      float dx = targetStar->pos.x - star->pos.x;
      float dy = targetStar->pos.y - star->pos.y;
      float dz = targetStar->pos.z - star->pos.z;
      float dist_ly = sqrt(dx * dx + dy * dy + dz * dz);

      float r1_ly = RadiusMetersToLy(targetStar->radius);
      float r2_ly = RadiusMetersToLy(star->radius);

      if (dist_ly < (r1_ly + r2_ly)) {
        Star *target = targetStar;
        Star *source = star;

        if (source->radius > target->radius) {
          std::swap(target, source);
        }

        target->vel.x =
            (target->mass * target->vel.x + source->mass * source->vel.x) /
            (target->mass + source->mass);
        target->vel.y =
            (target->mass * target->vel.y + source->mass * source->vel.y) /
            (target->mass + source->mass);
        target->vel.z =
            (target->mass * target->vel.z + source->mass * source->vel.z) /
            (target->mass + source->mass);

        target->mass += source->mass;

        double new_mass_ratio = target->mass / SOLAR_MASS;

        if (new_mass_ratio <= 1.0) {
          target->radius = SOLAR_RADIUS * std::pow(new_mass_ratio, 0.9);
        } else {
          target->radius = SOLAR_RADIUS * std::pow(new_mass_ratio, 0.6);
        }

        source->active = false;
        target->color = RED;
      }
      return;
    }

    if (isDivided) {
      float radius_ly = RadiusMetersToLy(targetStar->radius);

      for (int childIdx : children) {
        if (childIdx == -1)
          continue;

        const auto &childRegion = pool[childIdx].region;
        bool overlap =
            (targetStar->pos.x + radius_ly >= childRegion.size &&
             targetStar->pos.x - radius_ly <=
                 childRegion.x + childRegion.size) &&
            (targetStar->pos.y + radius_ly >= childRegion.y &&
             targetStar->pos.y - radius_ly <=
                 childRegion.y + childRegion.size) &&
            (targetStar->pos.z + radius_ly >= childRegion.z &&
             targetStar->pos.z - radius_ly <= childRegion.z + childRegion.size);

        if (overlap) {
          const_cast<OctreeNode &>(pool[childIdx])
              .CheckCollisionWithTree(targetStar, pool);
        }
      }
    }
  }
};

OctreeRegion CalculateOctreeBoundary(const std::vector<Star> &stars) {
  double min_x = SIM_WIDTH_LY;
  double max_x = 0.0;

  double min_y = SIM_HEIGHT_LY;
  double max_y = 0.0;

  double min_z = SIM_HEIGHT_LY;
  double max_z = 0.0;

  bool any_active = false;

  for (const Star &star : stars) {
    if (star.active) {
      if (!any_active) {
        min_x = star.pos.x;
        max_x = star.pos.x;

        min_y = star.pos.y;
        max_y = star.pos.y;

        min_z = star.pos.z;
        max_z = star.pos.z;

        any_active = true;
      } else {
        if (star.pos.x < min_x)
          min_x = star.pos.x;
        if (star.pos.x > max_x)
          max_x = star.pos.x;

        if (star.pos.y < min_y)
          min_y = star.pos.y;
        if (star.pos.y > max_y)
          max_y = star.pos.y;

        if (star.pos.z < min_z)
          min_z = star.pos.z;
        if (star.pos.z > max_z)
          max_z = star.pos.z;
      }
    }
  }

  if (!any_active) {
    min_x = 0;
    max_x = SIM_WIDTH_LY;

    min_y = 0;
    max_y = SIM_HEIGHT_LY;

    min_z = 0;
    max_z = SIM_HEIGHT_LY;
  }

  double width = max_x - min_x;
  double height = max_y - min_y;
  double depth = max_z - min_z;

  double max_size = std::max(width, height);
  max_size = std::max(max_size, depth);

  // In case all stars in the same point
  if (max_size < 0.001)
    max_size = 0.001;

  double center_x = min_x + width / 2.0;
  double center_y = min_y + height / 2.0;
  double center_z = min_z + depth / 2.0;

  OctreeRegion boundary{center_x - (max_size * 1.01) / 2.0,
                        center_y - (max_size * 1.01) / 2.0,
                        center_z - (max_size * 1.01) / 2.0, max_size * 1.01};

  return boundary;
}

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "N Body Simulation");
  // SetTargetFPS(FPS);

  DisableCursor();

  unsigned int star_amount = 2000;

  std::vector<Star> stars;
  for (size_t i = 0; i < star_amount; ++i) {
    Star star;
    stars.push_back(star);
  }

  std::vector<OctreeNode> nodePool;
  nodePool.reserve(star_amount * 2);

  Mesh sphereMesh = GenMeshSphere(1.0f, 8, 8);

  Shader lightingShader = LoadShader("lighting.vs", "lighting.fs");

  lightingShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocationAttrib(lightingShader, "instanceTransform");

  int instanceColorLoc = GetShaderLocation(lightingShader, "u_color");

  Material sphereMaterial = LoadMaterialDefault();
  sphereMaterial.shader = lightingShader;
  sphereMaterial.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

  while (!WindowShouldClose()) {
    double dt = GetFrameTime();

    // Prevents stutters
    if (dt > 0.1)
      dt = 0.1;

    // 1 realtime second = 5000 years
    double time_scale = 5000.0 * 365.0 * 24.0 * 3600.0;
    double sim_dt = dt * time_scale;

    // Force reset
    for (Star &star : stars) {
      star.ResetForce();
    }

    // Naive approach O(n^2)
    // for (size_t i = 0; i < star_amount; ++i) {
    //   for (size_t j = 0; j < star_amount; ++j) {
    //     if (i == j)
    //       continue;
    //     UpdateForce(&stars[i], &stars[j]);
    //   }
    // }

    OctreeRegion boundary = CalculateOctreeBoundary(stars);

    nodePool.clear();
    nodePool.push_back(OctreeNode(boundary));

    for (Star &star : stars) {
      if (star.active) {
        nodePool[0].Insert(&star, nodePool);
      }
    }

    // Collision handling
    for (Star &star : stars) {
      if (star.active) {
        nodePool[0].CheckCollisionWithTree(&star, nodePool);
      }
    }

    // Force calculation using Octree
    const double theta = 0.5;
    const double softening = 1e27;

    for (Star &star : stars) {
      if (star.active) {
        nodePool[0].CalculateStarForce(&star, theta, softening, nodePool);
      }
    }

    // Acceleration, velocity and position update
    for (Star &star : stars) {
      star.Update(sim_dt);
    }

    // Camera handling
    // First Person
    // UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    // Showcase
    UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();
    ClearBackground(GetColor(0x000309FF));

    // Drawing Stars
    BeginMode3D(camera);

    RenderStarsInstanced(stars, sphereMesh, sphereMaterial, instanceColorLoc);

    DrawCubeWiresV({5.0f, 5.0f, 5.0f}, {10.0f, 10.0f, 10.0f},
                   GetColor(0xFFFFFF22));
    EndMode3D();

    // DrawFPS(10, 10);

    int active_stars = 0;
    // Active stars
    for (const auto &star : stars) {
      if (star.active) {
        active_stars++;
      }
    }

    DrawText(TextFormat("Stars amount: %i", active_stars), 10, 32, 16, WHITE);
    EndDrawing();
  }
  UnloadMesh(sphereMesh);
  UnloadShader(lightingShader);

  CloseWindow();
  return 0;
}
