#include <Physics.h>
#include <Game.h>


void Circle_init(ConvexShape * shape, Vec2 position, float radius) {
    shape->type = CIRCLE;
    shape->shape.circle.position = position;
    shape->shape.circle.radius = radius;
}

void Shape_init(ConvexShape * shape, Polygon polygon) {
    shape->type = POLYGON;
    shape->shape.polygon = polygon;
}

void ConvexShape_free_content(ConvexShape * shape) {
    if(shape->type == POLYGON)
        free(shape->shape.polygon.vertices);
}

void Solid_init(Solid *solid, ConvexShape collision_shapes[], 
                                        unsigned nb_collision_shapes,
                                        float inertia_moment,
                                        float total_mass) {
    solid->collision_shapes = collision_shapes;
    solid->nb_collision_shapes = nb_collision_shapes;
    solid->inertia_moment = inertia_moment;
    
    solid->position = solid->speed = MakeVec2(0,0);
    solid->rotation = solid->rotation_speed = 0;
    solid->mass = total_mass;
    
    /* Translation de tous les points, par rapport au centre de gravité
     * qui va être calculé. */
    unsigned i = 0, j;
    Vec2 mass_center = MakeVec2(0, 0);
    float mass = .0001f;
    for(; i < solid->nb_collision_shapes; ++i) {
        switch(solid->collision_shapes[i].type) {
            case CIRCLE: {
                float mass_temp = M_PI * solid->collision_shapes[i].shape.circle.radius;
                mass += mass_temp;
                mass_center = AddVec2(mass_center, MulVec2(
                    solid->collision_shapes[i].shape.circle.position, 
                    mass_temp));
                    
                break;
            }
            case POLYGON: {
                /* Calcul intermédiaire du centre du polygone. */
                Vec2 polygon_center;
                unsigned vertex;
                for(vertex = 0; vertex < solid->collision_shapes[i].shape.polygon.nb_vertices;
                                                                                ++vertex) {
                    polygon_center = AddVec2(polygon_center,
                        solid->collision_shapes[i].shape.polygon.vertices[vertex]);
                }
                polygon_center = MulVec2(polygon_center, 
                    1. / solid->collision_shapes[i].shape.polygon.nb_vertices);
                solid->collision_shapes[i].shape.polygon.position = polygon_center;
                    
                /* Parcours de tous les sommets en formant des triangles
                 * autour du centre, pour calculer le barycentre de chaque. */
                for(vertex = 0; vertex < solid->collision_shapes[i].shape.polygon.nb_vertices;
                                                                                ++vertex) {
                    float mass_temp = fabs(Scal2(SubVec2(solid->collision_shapes[i].shape.polygon.vertices[vertex], 
                            polygon_center), OrthogonalVec2( 
                            SubVec2(solid->collision_shapes[i].shape.polygon.vertices[(vertex + 1) % solid->collision_shapes[i].shape.polygon.nb_vertices], 
                            polygon_center) )));
                    
                    mass += mass_temp;
                    mass_center = AddVec2(mass_center, AddVec2(polygon_center, 
                                  MulVec2( SubVec2( MulVec2( AddVec2( 
                                  solid->collision_shapes[i].shape.polygon.vertices[vertex], 
                                  solid->collision_shapes[i].shape.polygon.vertices[(vertex + 1) % solid->collision_shapes[i].shape.polygon.nb_vertices]), 0.5), 
                                  polygon_center), 2. / 3.)));
                }
                
                break;
            }
            default:
                break;
        }
    }
    mass_center = MulVec2(mass_center, 1. / mass);
    
    /* Translation de tous les points. */
    for(i = 0; i < solid->nb_collision_shapes; ++i) {
        switch(solid->collision_shapes[i].type) {
            case CIRCLE: {
                solid->collision_shapes[i].shape.circle.position = 
                    SubVec2(solid->collision_shapes[i].shape.circle.position,
                                mass_center);
                
                break;
            }
            case POLYGON: {
                for(j = 0; j < solid->collision_shapes[i].shape.polygon.nb_vertices;
                                                                                ++j) {
                    solid->collision_shapes[i].shape.polygon.vertices[j] = 
                        SubVec2(solid->collision_shapes[i].shape.polygon.vertices[j],
                        mass_center);
                }
                solid->collision_shapes[i].shape.polygon.position = 
                        SubVec2(solid->collision_shapes[i].shape.polygon.position,
                        mass_center);
                break;
            }
            default:
                break;
        }
    }
    solid->next = NULL;
    solid->prev = NULL;
}

void World_clean(PhysicWorld * world) {
    world->obstacles = world->solids 
                    = world->forces_head = world->forces_tail = NULL;
}
    

void World_addForce(PhysicWorld * world, Force * force) {
    if(!world->forces_head) {
        world->forces_head = force;
        world->forces_tail = force;
    } else {
        world->forces_tail->next = force;
        world->forces_tail = force;
    }
    force->next = NULL;
}

void World_addSolid(PhysicWorld * world, Solid * solid) {
    Solid * tmp = world->solids;
    world->solids = solid;
    solid->prev = NULL;
    solid->next = tmp;
    if(tmp) tmp->prev = solid;
}

void World_addObstacle(PhysicWorld * world, Obstacle * obs) {
    Obstacle *tmp = world->obstacles;
    obs->next = tmp;
    world->obstacles = obs;
}

int Collision_point_segment(Vec2 p, Vec2 mv, Vec2 seg1, Vec2 seg2, float *t, Vec2 * pos) {
    
}

int ConvexShape_compute_collision(ConvexShape *s1, Vec2 pos1, Vec2 mov1,
                                    ConvexShape *s2, Vec2 pos2, Vec2 mov2,
                                      Vec2 * pos_collision, float *collision_time_ratio) {
    /* On se place dans le repère de s2 */
    Vec2 p1 = SubVec2(pos1, pos2);
    Vec2 p2 = MakeVec2(0,0);
    Vec2 m1 = SubVec2(mov1, mov2);
    Vec2 m2 = MakeVec2(0,0);
    if(s1->type == CIRCLE && s2->type == CIRCLE) {
        Vec2 _1_2 = SubVec2(p2, p1);
        Vec2 _3   = AddVec2(p1, m1);
        Vec2 _1_3 = SubVec2(_3, _1);
        float dist_min = fabs(Scal2( _1_2, OrthogonalVec2(_1_3)));
        if(dist_min >= s1->shape.circle.radius + s2->shape.circle.radius) {
            return 0;
        }
        float a = Scal2(m1, m1),
              b = 2 * Scal2(m1, p1),
              c = Scal2(p1, p1) - (s1->shape.circle.radius + s2->shape.circle.radius)
                                * (s2->shape.circle.radius + s2->shape.circle.radius);
        float delta = b*b - 4*a*c;
        if(delta < 0)
            return 0;
        float t = (-b - sqrt(delta)) / (2*a);
        if(t < 0 || t > 1)
            return 0;
        *collision_time_ratio = t;
        *pos_collision = AddVec2(MulVec2(AddVec2(p1, MulVec2(m1, t)), 
                (s2->shape.circle.radius + s2->shape.circle.radius) /
                (s2->shape.circle.radius + s2->shape.circle.radius + s1->shape.circle.radius + s2->shape.circle.radius)),
                            pos2);
        return 1;
    } else if(s1->type == POLYGON && s2->type == POLYGON) {
    
    } else {
    
    }
}

void Compute_force(Solid * solid, Force * force) {
    return;
}

void Apply_force(Force * force, float duration) {
    if(force->type == COUPLE) { /* Si la force est un couple. */
        force->solid->rotation_speed += 
            duration * force->force.x / force->solid->inertia_moment;
    } else { /* C'est une force normale. */
        /* Action sur le barycentre. */
        force->solid->speed = AddVec2( force->solid->speed,
                MulVec2( force->force, duration / force->solid->mass));
        /* Action sur la rotation. */
        float pseudo_prod_vect = PseudoVectProd2(SubVec2(LocalToGlobal2(force->position, 
                                                force->solid->position, force->solid->rotation),
                                             force->solid->position), force->force);
        force->solid->rotation_speed += duration * pseudo_prod_vect / force->solid->inertia_moment;
    }
}

int Process_physics(PhysicWorld * world, float elapsed_time) {
    /* Application de toutes les forces. */
    for(; world->forces_head ; world->forces_head = world->forces_head->next) {
        Apply_force(world->forces_head, elapsed_time);
    }
    world->forces_tail = NULL;
    world->forces_head = NULL;
    
    /* Gestion des collisions blabla. */
    
    
    /* Mise à jour des positions et rotations. */
	Solid *tmp = world->solids;
    for(; tmp; tmp = tmp->next) {
        tmp->position = AddVec2(tmp->position,
                                            tmp->speed);
                                            
        tmp->rotation += tmp->rotation_speed;
    }
    
    return 1;
}

