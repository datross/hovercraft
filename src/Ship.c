#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

#define SIZE_COLLISION_SHAPES 0.2

void Ship_init(Ship *s) {
    /* Dès ici, c'est safe de se référer au ShipData pour choisir la shape
     * de collision. Surtout ce champs :
     *     s->data.above[0][0].half_size
     * Qui donne la moitié de la taille du quad texturé en coordonnées monde.
     */ 
    Vec2u nb_shapes;
    nb_shapes.x = 2. * s->data->above[0][0].half_size.x / SIZE_COLLISION_SHAPES;
    nb_shapes.y = 2. * s->data->above[0][0].half_size.y / SIZE_COLLISION_SHAPES;
    Vec2 size_shapes = MakeVec2(2. * s->data->above[0][0].half_size.x / (float)nb_shapes.x,
                                2. * s->data->above[0][0].half_size.y / (float)nb_shapes.y);
    ConvexShape * shape = malloc(nb_shapes.x * nb_shapes.y * sizeof(ConvexShape));
    if(!shape) {
        fprintf(stderr, "Erreur allocation boites des collision.\n");
        exit(EXIT_FAILURE);
    }
    unsigned i_shape = 0;
    for(unsigned x = 0; x < nb_shapes.x; ++x) {
        for(unsigned y = 0; y < nb_shapes.y; ++y) {
            Polygon polygon;
            polygon.nb_vertices = 4;
            polygon.vertices = malloc(polygon.nb_vertices * sizeof(Vec2));
            polygon.vertices[0] = MakeVec2(-s->data->above[0][0].half_size.x + x * size_shapes.x, 
                                           -s->data->above[0][0].half_size.y + y * size_shapes.y);
            polygon.vertices[1] = MakeVec2(-s->data->above[0][0].half_size.x + (x+1) * size_shapes.x, 
                                           -s->data->above[0][0].half_size.y + y * size_shapes.y);
            polygon.vertices[2] = MakeVec2(-s->data->above[0][0].half_size.x + (x+1) * size_shapes.x, 
                                           -s->data->above[0][0].half_size.y + (y+1) * size_shapes.y);
            polygon.vertices[3] = MakeVec2(-s->data->above[0][0].half_size.x + x * size_shapes.x, 
                                           -s->data->above[0][0].half_size.y + (y+1) * size_shapes.y);
            
            shape[i_shape].type = POLYGON;
            shape[i_shape].shape.polygon = polygon;
            ++i_shape;
        }
    }
    Solid_init(&(s->physic_solid), shape, i_shape, 1, 1);

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
    for(unsigned i = 0; i < s->physic_solid.nb_collision_shapes; ++i) {
        free(s->physic_solid.collision_shapes[i].shape.polygon.vertices);
    }
    free(s->physic_solid.collision_shapes);
    s->physic_solid.collision_shapes = NULL;
    s->physic_solid.nb_collision_shapes = 0;
    ParticleSystem_free_particles(&(s->particle_system_reactor));
}

void Ship_renderGuides(const Ship *s) {
    int i;
    for(i=s->guide_count-1 ; i>=0 ; --i) {
        const ShipGuide *guide = s->guides + i;
        glColor3f(1.f, 1.f, 1.f);
        glPushMatrix(); 
        {
            glTranslatef(s->physic_solid.position.x, s->physic_solid.position.y, 0);
            glRotatef(guide->theta,0,0,1);
            glTranslatef(0, 1.5f, 0);
            glScalef(guide->scale.x*1.1f, guide->scale.y*1.1f, 1.f);
            renderIsocelesTriangle(true);
        } 
        glPopMatrix();
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
        glRotatef(degf(s->physic_solid.rotation - M_PI/2.f),0,0,1);
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
    if(s->physic_solid.speed.x || s->physic_solid.speed.y)
    	ParticleSystem_draw_particles(&(s->particle_system_reactor));
    /* Ship_renderBoundingVolumes(s); */
    glPushMatrix();
    {
        glTranslatef(s->physic_solid.position.x, s->physic_solid.position.y, 0);
        glRotatef(degf(s->physic_solid.rotation-M_PI/2.f),0,0,1);
        Sprite_render(&s->data->above[s->above_index][s->palette_index]);
        
        /*
        for(unsigned i = 0; i < s->physic_solid.nb_collision_shapes; ++i) {
            glBegin(GL_LINE_LOOP);
            for(unsigned j = 0; j < s->physic_solid.collision_shapes[i].shape.polygon.nb_vertices; ++j) {
                glVertex2f(s->physic_solid.collision_shapes[i].shape.polygon.vertices[j].x,
                           s->physic_solid.collision_shapes[i].shape.polygon.vertices[j].y);
            }
            glEnd();
        }*/
        
    }
    glPopMatrix();
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
