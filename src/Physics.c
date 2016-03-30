#include <Physics.h>
#include <Game.h>


void MakeCircle(ConvexShape * shape, Vec2 position, float radius) {
    shape->type = CERCLE;
    shape->shape.cercle.position = position;
    shape->shape.cercle.radius = radius;
}

void MakePolygon(ConvexShape * shape, Polygon polygon) {
    shape->type = POLYGON;
    shape->shape.polygon = polygon;
}

void MakeSolid(Solid *solid, ConvexShape collision_shapes[], 
                                        unsigned nb_collision_shapes,
                                        float inertia_moment) {
    solid->collision_shapes = collision_shapes;
    solid->nb_collision_shapes = nb_collision_shapes;
    solid->inertia_moment = inertia_moment;
    
    solid->position = solid->speed = MakeVec2(0,0);
    solid->rotation = solid->rotation_speed = 0;
    
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
}
    

int Compute_force(Solid * solid, Force * force) {
    return 1;
}

int Process_physics(PhysicWorld * world, float elapsed_time) {
    return 1;
}

