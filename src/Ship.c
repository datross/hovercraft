#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

void Ship_init(Ship *s) {
    /* Dès ici, c'est safe de se référer au ShipData pour choisir la shape
     * de collision. Surtout ce champs :
     *     s->data.above[0][0].half_size
     * Qui donne la moitié de la taille du quad texturé en coordonnées monde.
     */ 
    /* Temporaire */
    Polygon polygon;
    polygon.nb_vertices = 4;
    polygon.vertices = malloc(polygon.nb_vertices * sizeof(Vec2));
    polygon.vertices[0] = MakeVec2(0, 0);
    polygon.vertices[1] = MakeVec2(2, 0);
    polygon.vertices[2] = MakeVec2(2, 1);
    polygon.vertices[3] = MakeVec2(0, 1);
    
    ConvexShape shape;
    Shape_init(&shape, polygon);
    
    Solid_init(&(s->physic_solid), &shape, 1, 1, 1);

	s->physic_solid.rotation = M_PI / 2.;

    s->main_translate_force.type = NORMAL;
	s->main_translate_force.solid = &(s->physic_solid);
	s->main_translate_force.position = MakeVec2(0,0);
	s->main_rotate_force.type = COUPLE;
	s->main_rotate_force.solid = &(s->physic_solid);
    s->main_translation_friction.type = NORMAL;
	s->main_translation_friction.solid = &(s->physic_solid);
	s->main_translation_friction.position = MakeVec2(0,0);
	s->main_rotation_friction.type = COUPLE;
	s->main_rotation_friction.solid = &(s->physic_solid);
    
    ParticleSystem_allocate_particles(&(s->particle_system_reactor), REACTOR_PARTICLES_COUNT);
}

void Ship_deinit(Ship *s) {
    free(s->physic_solid.collision_shapes[0].shape.polygon.vertices);
    ParticleSystem_free_particles(&(s->particle_system_reactor));
}

void Ship_renderGuides(const Ship *s) {
    int i;
    for(i=s->guide_count-1 ; i>=0 ; --i) {
        const ShipGuide *guide = s->guides + i;
        glColor3f(guide->r, guide->g, guide->b);
        glPushMatrix(); 
        {
            glTranslatef(s->physic_solid.position.x, s->physic_solid.position.y, 0);
            glRotatef(guide->theta,0,0,1);
            glTranslatef(0, 1.5f, 0);
            glScalef(guide->scale.x, guide->scale.y, 1.f);
            renderIsocelesTriangle(true);
        } 
        glPopMatrix();
    }
}

void Ship_renderBoundingVolumes(const Ship *s) {
    glColor3f(1.f, 0.f, 0.f);
    glPushMatrix();
    {
        glTranslatef(s->physic_solid.position.x, s->physic_solid.position.y, 0);
        glRotatef((s->physic_solid.rotation - M_PI / 2.)*180.f/M_PI,0,0,1);
        glPushMatrix(); 
        {
            glTranslatef(0, .5f, 0);
            glScalef(.5f, .5f, 1.f);
            renderDisk(32);
        } 
        glPopMatrix();

        renderSquare(true);
    }
    glPopMatrix();
}
void Ship_render(const Ship *s) {
    /* Ship_renderBoundingVolumes(s); */
    glPushMatrix();
    {
        glTranslatef(s->physic_solid.position.x, s->physic_solid.position.y, 0);
        glRotatef((s->physic_solid.rotation - M_PI / 2.)*180.f/M_PI,0,0,1);
        Sprite_render(&s->data->above[s->above_index][s->palette_index]);
    }
    glPopMatrix();
    ParticleSystem_draw_particles(&(s->particle_system_reactor));
}
void Ship_refreshGuides(Ship *s) {
    size_t i;
    for(i=0 ; i<s->guide_count ; ++i) {
        Vec2 diff = {s->guides[i].pos.x - s->physic_solid.position.x, 
                     s->guides[i].pos.y - s->physic_solid.position.y};
        s->guides[i].theta = degf(atan2f(diff.y, diff.x))-90.f;
        s->guides[i].distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
        /*
        printf("Distance from guides[%zu] : %u units.\n", 
                i, (uint32_t)s->guides[i].distance);
        */
    }
}
