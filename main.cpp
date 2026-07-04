#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <raylib.h>
#include <utility>
#include <vector>

// Randomness
static std::random_device rd;
static std::mt19937 gen(rd());

// Constants
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

const double G = 6.6743e-11;
const double LY_TO_METERS = 9.4607e15; // 1 rok świetlny w metrach
const double SOLAR_MASS = 1.989e30;

const double SIM_WIDTH_LY = 10.0;
const double SIM_HEIGHT_LY =
    SIM_WIDTH_LY * (static_cast<double>(WINDOW_HEIGHT) / WINDOW_WIDTH);

float LyToPixelsX(double ly) {
  return static_cast<float>(ly * (WINDOW_WIDTH / SIM_WIDTH_LY));
}
float LyToPixelsY(double ly) {
  return static_cast<float>(ly * (WINDOW_HEIGHT / SIM_HEIGHT_LY));
}

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

class Star {
public:
  // Raylibs Vector2 is of type [float, float]
  // A custom Vec2 might be needed for better precision with double
  Vector2 pos;
  Vector2 vel;
  Vector2 acc; // acceleration
  Vector2 F;   // Force

  double mass;
  double radius;
  Color color;
  bool active;
  StellarType type;

  Star() {
    // Random position in light years
    pos.x = GetRandomFloat(0, SIM_WIDTH_LY);
    pos.y = GetRandomFloat(0, SIM_HEIGHT_LY);

    // Random velocity in m/s
    vel.x = GetRandomFloat(-20000, 20000);
    vel.y = GetRandomFloat(-20000, 20000);

    acc = {0, 0};
    F = {0, 0};

    mass = GetRandomFloat(0.5, 5) * SOLAR_MASS;
    radius = GetRandomFloat(1, 10);
    // radius = GetRandomFloat(0.25, 1.0) * mass;

    color = GetStarColor();

    active = true;
    type = StellarType::MainSequence;
  }

  void Draw() {
    if (!active)
      return;

    DrawCircle(LyToPixelsX(pos.x), LyToPixelsY(pos.y), radius, color);
  }

  void ResetForce() {
    F.x = 0;
    F.y = 0;
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
  float initVel = 100;
  float minRadius = 1;
  float maxRadius = 10;
  float dt;

  Color GetStarColor() {
    // Will eventually return/modify a color associated to its fields

    return WHITE;
  }

  void UpdateVelocity() {
    if (!active)
      return;

    vel.x = vel.x + acc.x * dt;
    vel.y = vel.y + acc.y * dt;
  }

  void UpdateAcceleration() {
    if (!active)
      return;

    acc.x = F.x / mass;
    acc.y = F.y / mass;
  }

  void UpdatePosition() {
    if (!active)
      return;

    pos.x += (vel.x * dt) / LY_TO_METERS;
    pos.y += (vel.y * dt) / LY_TO_METERS;
  }
};

float Hypotenuse(Star *s1, Star *s2) {
  float dx_ly = s1->pos.x - s2->pos.x;
  float dy_ly = s1->pos.y - s2->pos.y;

  float dx_m = dx_ly * LY_TO_METERS;
  float dy_m = dy_ly * LY_TO_METERS;

  return sqrt(dx_m * dx_m + dy_m * dy_m);
}

void UpdateForce(Star *s1, Star *s2) {
  if (!s1->active || !s2->active)
    return;

  float r_m = Hypotenuse(s1, s2);

  // Prevents shooting the star off into space
  float softening = 1e27;

  // Scalar
  float force = (G * s1->mass * s2->mass) / (r_m * r_m + softening);

  // Force direction vector in meters
  float dx_m = (s2->pos.x - s1->pos.x) * LY_TO_METERS;
  float dy_m = (s2->pos.y - s1->pos.y) * LY_TO_METERS;

  s1->F.x += force * (dx_m / r_m);
  s1->F.y += force * (dy_m / r_m);
}

void RenderStars(std::vector<Star> stars) {
  for (auto &star : stars) {
    star.Draw();
  }
}

void HandleCollisions(std::vector<Star> &stars) {
  for (size_t i = 0; i < stars.size(); ++i) {
    for (size_t j = i + 1; j < stars.size(); ++j) {
      if (!stars[i].active || !stars[j].active)
        continue;

      float screen_x1 = LyToPixelsX(stars[i].pos.x);
      float screen_y1 = LyToPixelsY(stars[i].pos.y);
      float screen_x2 = LyToPixelsX(stars[j].pos.x);
      float screen_y2 = LyToPixelsY(stars[j].pos.y);

      float dx = screen_x2 - screen_x1;
      float dy = screen_y2 - screen_y1;
      float dist_pixels = sqrt(dx * dx + dy * dy);

      if (dist_pixels < (stars[i].radius + stars[j].radius)) {
        Star *target = &stars[i];
        Star *source = &stars[j];

        if (source->radius > target->radius) {
          std::swap(target, source);
        }

        target->vel.x =
            (target->mass * target->vel.x + source->mass * source->vel.x) /
            (target->mass + source->mass);
        target->vel.y =
            (target->mass * target->vel.y + source->mass * source->vel.y) /
            (target->mass + source->mass);

        target->mass += source->mass;
        target->radius += source->radius * 0.2f;

        if (target->radius > 30.0f)
          target->radius = 30.0f;

        source->active = false;
      }
    }
  }
}

int main() {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "N Body Simulation");
  SetTargetFPS(FPS);

  unsigned int star_amount = 50;

  std::vector<Star> stars;
  for (size_t i = 0; i < star_amount; ++i) {
    Star star;
    stars.push_back(star);
  }

  while (!WindowShouldClose()) {
    double dt = GetFrameTime();

    // Prevents stutters
    if (dt > 0.1)
      dt = 0.1;

    // 1 realtime second = 5000 years
    double time_scale = 5000.0 * 365.0 * 24.0 * 3600.0;
    double sim_dt = dt * time_scale;

    // Updating Stars
    for (Star &star : stars) {
      star.ResetForce();
    }

    HandleCollisions(stars);

    for (size_t i = 0; i < star_amount; ++i) {
      for (size_t j = 0; j < star_amount; ++j) {
        if (i == j)
          continue;
        UpdateForce(&stars[i], &stars[j]);
      }
    }

    for (Star &star : stars) {
      star.Update(sim_dt);
    }

    BeginDrawing();
    ClearBackground(GetColor(0x000D20FF));
    DrawFPS(10, 10);

    // Drawing Stars
    RenderStars(stars);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
