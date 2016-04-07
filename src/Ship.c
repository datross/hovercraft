#include <stdio.h>
#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

void Ship_init(Ship *s) {
    memset(s, 0, sizeof(Ship));
    s->accel_multiplier = 0.0005f;
    s->tilt_step = 0.0001f;
    s->friction = 0.005f;
    s->max_speed = 200.f;
    
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
	
    
    s->r = 1.f;
}

void Ship_deinit(Ship *s) {
    /* Temporaire */
    free(s->physic_solid.collision_shapes[0].shape.polygon.vertices);
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
    glColor3f(s->r,s->g,s->b);
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
    Ship_renderBoundingVolumes(s);
}
