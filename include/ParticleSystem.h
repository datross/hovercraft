#ifndef PARTICLES
#define PARTICLES

#include <Vec.h>
#include <Utils.h>

typedef struct {
    Vec2 position;
    Vec2 speed;
    Color3 color;
    float size;
    float life_time, life_time_elapsed;
    float value;
} Particle;

typedef struct {
    Particle * particles;
    unsigned particle_count;
    Color3 particle_color;
    Vec2 source_position, 
         source_speed,
         emit_direction; /* Comprend aussi la vitesse. */
    float source_radius_max, /* La zone émétrice est un disque. */
          particle_size,
          particle_size_dispersion,
          emit_direction_dispersion,
          emit_speed_dispersion,
          life_time_max,
          life_time_dispersion,
          color_dispersion,
          dampness;
} ParticleSystem;

void ParticleSystem_allocate_particles(ParticleSystem * ps, unsigned nb_particles);
void ParticleSystem_free_particles(ParticleSystem * ps);
void ParticleSystem_init_particles(ParticleSystem * ps);
void ParticleSystem_compute_step(const ParticleSystem * ps, float elapsed_time);
void ParticleSystem_draw_particles(const ParticleSystem * ps);
void Particle_init(Particle * p, const ParticleSystem * ps);


#endif /* PARTICLES */
