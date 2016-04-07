#include <Physics.h>
#include <Game.h>


void Circle_init(ConvexShape * shape, Vec2 position, float radius) {
    shape->type = CERCLE;
    shape->shape.cercle.position = position;
    shape->shape.cercle.radius = radius;
}

void Shape_init(ConvexShape * shape, Polygon polygon) {
    shape->type = POLYGON;
    shape->shape.polygon = polygon;
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
    float mass;
    for(; i < solid->nb_collision_shapes; ++i) {
        switch(solid->collision_shapes[i].type) {
            case CERCLE: {
                float mass_temp = M_PI * solid->collision_shapes[i].shape.cercle.radius;
                mass += mass_temp;
                mass_center = AddVec2(mass_center, MulVec2(
                    solid->collision_shapes[i].shape.cercle.position, 
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
            case CERCLE: {
                solid->collision_shapes[i].shape.cercle.position = 
                    SubVec2(solid->collision_shapes[i].shape.cercle.position,
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
                
                break;
            }
            default:
                break;
        }
    }
    solid->next = NULL;
    solid->prev = NULL;
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

