#include <ParticleSystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>

void ParticleSystem_allocate_particles(ParticleSystem * ps, unsigned nb_particles) {
    free(ps->particles);
    if(!(ps->particles = calloc(nb_particles, sizeof(Particle)))) {
        fprintf(stderr, "Erreur mémoire allocation particules.\n");
        exit(EXIT_FAILURE);
    }
    ps->particle_count = nb_particles;
}

void ParticleSystem_free_particles(ParticleSystem * ps) {
    free(ps->particles);
    ps->particle_count = 0;
}

void ParticleSystem_init_particles(ParticleSystem * ps) {
    for(unsigned i = 0; i < ps->particle_count; ++i) {
        Particle_init(&(ps->particles[i]), ps);
    }
}

void ParticleSystem_compute_step(const ParticleSystem * ps, float elapsed_time) {
    for(unsigned i = 0; i < ps->particle_count; ++i) {
        ps->particles[i].position.x += ps->particles[i].speed.x;
        ps->particles[i].position.y += ps->particles[i].speed.y;
        ps->particles[i].speed.x *= ps->dampness;
        ps->particles[i].speed.y *= ps->dampness;
        ps->particles[i].life_time_elapsed += elapsed_time;
        if(ps->particles[i].life_time_elapsed >= ps->particles[i].life_time) {
            Particle_init(&(ps->particles[i]), ps);
            continue;
        }
    }
}

void ParticleSystem_draw_particles(const ParticleSystem * ps) {
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH); /* TODO : Ne marche pas */
    GLfloat col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);
    for(unsigned i = 0; i < ps->particle_count; ++i) {
        glPointSize(ps->particles[i].size);
        glColor3f(ps->particles[i].color.r,
                  ps->particles[i].color.g,
                  ps->particles[i].color.b);
        glBegin(GL_POINTS);
        glVertex2f(ps->particles[i].position.x, ps->particles[i].position.y);
        glEnd();
    }
    glColor4f(col[0], col[1], col[2], col[3]);
    glDisable(GL_BLEND);
}
        
void Particle_init(Particle * p, const ParticleSystem * ps) {
    float tmp_x = Rand_f_1();
    float tmp_y = sqrt(1. - tmp_x * tmp_x) * Rand_f_1();
    p->position = MakeVec2(ps->source_position.x + ps->source_radius_max * tmp_x,
                            ps->source_position.y + ps->source_radius_max * tmp_y);
    p->speed = AddVec2(MulVec2(Rotate2(ps->emit_direction, ps->emit_direction_dispersion * Rand_f_1()), 
                    1. + ps->emit_speed_dispersion * Rand_f_1()), ps->source_speed);
    p->color = ps->particle_color;
    p->color.r += ps->color_dispersion * Rand_f_1();
    p->color.g += ps->color_dispersion * Rand_f_1();
    p->color.b += ps->color_dispersion * Rand_f_1();
    p->size = ps->particle_size + ps->particle_size_dispersion * Rand_f_1();
    p->life_time_elapsed = 0.;
    p->life_time = ps->life_time_max * (1. - ps->life_time_dispersion * Rand_f_0_1());
    p->value = 0.;
}
