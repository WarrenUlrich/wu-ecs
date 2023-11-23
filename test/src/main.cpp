#include <bitset>
#include <chrono>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

#include <wu-ecs/ecs.hpp>

// Component classes
class position {
public:
  int x, y;
};

class velocity {
public:
  int x, y;
};

class health {
public:
  int value;
};

class damage {
public:
  int value;
};

class acceleration {
public:
  int x, y;
};

// System classes
class movement_system {
public:
  void operator()(position &pos, velocity &vel) noexcept
  {
    pos.x += vel.x;
    pos.y += vel.y;
  }
};

class health_system {
public:
  void operator()(health &health, damage &damage)
  noexcept {
    health.value -= damage.value;
  }
};

class acceleration_system {
public:
  void operator()(velocity &vel,
                  acceleration &acc) noexcept {
    vel.x += acc.x;
    vel.y += acc.y;
  }
};

class collision_system {
public:
  void operator()(health &health) noexcept {
    // Simulating collision detection
    health.value -= 1; // Dummy collision effect
  }
};

class render_system {
public:
  void operator()(position &pos) noexcept {
    pos.x /= .5;
    pos.y /= .5;
  }
};

int main() {
  const size_t numEntities = 10000;
  ecs::context<position, velocity, health, damage,
               acceleration>
      ctx;

  // Random number generator setup
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(1, 10);

  // Entity creation with random components
  for (size_t i = 0; i < numEntities; ++i) {
    auto entity = ctx.new_entity();
    ctx.add_component<position>(
        entity, position{dist(rng), dist(rng)});
    ctx.add_component<velocity>(
        entity, velocity{dist(rng), dist(rng)});
    if (dist(rng) > 5)
      ctx.add_component<health>(entity, health{100});
    if (dist(rng) > 5)
      ctx.add_component<damage>(entity,
      damage{dist(rng)});
    if (dist(rng) > 5)
      ctx.add_component<acceleration>(
          entity, acceleration{dist(rng), dist(rng)});
  }

  // Benchmarking
  auto start = std::chrono::high_resolution_clock::now();
  
  std::cout << sizeof(ctx) << '\n';
  ctx.for_each_entity<position,
  velocity>(movement_system());
  ctx.for_each_entity<health, damage>(health_system());
  ctx.for_each_entity<velocity, acceleration>(
      acceleration_system());
  ctx.for_each_entity<health>(collision_system());
  ctx.for_each_entity<position>(render_system());

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(
          end - start);
  std::cout << "Benchmark completed in " <<
  duration.count()
            << " milliseconds\n";

  return 0;
}