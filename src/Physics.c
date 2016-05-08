#include <Physics.h>
#include <Game.h>


void Circle_init(ConvexShape * shape, Vec2 position, float radius) {
    shape->type = CIRCLE;
    shape->position = position;
    shape->shape.circle.radius = radius;
}

void ConvexShape_free_content(ConvexShape * shape) {
    if(shape->type == POLYGON)
        free(shape->shape.polygon.vertices);
}

void Solid_init(Solid *solid, ConvexShape *collision_shapes, 
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
                    solid->collision_shapes[i].position, 
                    mass_temp));
                    
                break;
            }
            case POLYGON: {
                /* Calcul intermédiaire du centre du polygone. */
                Vec2 polygon_center = MakeVec2(0,0);
                unsigned vertex;
                for(vertex = 0; vertex < solid->collision_shapes[i].shape.polygon.nb_vertices;
                                                                                ++vertex) {
                    polygon_center = AddVec2(polygon_center,
                        solid->collision_shapes[i].shape.polygon.vertices[vertex]);
                }
                polygon_center = MulVec2(polygon_center, 
                    1. / solid->collision_shapes[i].shape.polygon.nb_vertices);
                solid->collision_shapes[i].position = polygon_center;
                    
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
            solid->collision_shapes[i].dist_to_center = sqrt(SqrNorm(solid->collision_shapes[i].position));
        }
    }
    mass_center = MulVec2(mass_center, 1. / mass);
    
    /* Translation de tous les points. */
    for(i = 0; i < solid->nb_collision_shapes; ++i) {
        switch(solid->collision_shapes[i].type) {
            case CIRCLE: {
                solid->collision_shapes[i].position = 
                    SubVec2(solid->collision_shapes[i].position,
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
                solid->collision_shapes[i].position = 
                        SubVec2(solid->collision_shapes[i].position,
                        mass_center);
                break;
            }
            default:
                break;
        }
    }
    solid->next = NULL;
    solid->prev = NULL;
    
    for(unsigned i = 0; i < COLLISION_FORCE_NBR; ++i) {
        solid->collision_forces[i].solid = solid;
        solid->collision_forces[i].type = NORMAL;
    }
}

void World_clean(PhysicWorld * world) {
    world->obstacles = NULL;
    world->solids = NULL;
    world->forces_head = NULL;
    world->forces_tail = NULL;
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
    Vec2 n = OrthogonalVec2(mv);
    {
        Vec2 _p_1 = SubVec2(seg1, p),
             _p_2 = SubVec2(seg2, p);
        if( Scal2(_p_1, n) * Scal2(_p_2, n) >= 0 )
            return 0;
    }
    n = OrthogonalVec2( SubVec2(seg2, seg1) );
    Vec2 _1_p  = SubVec2(p, seg1),
         _1_pm = SubVec2( AddVec2(p, mv), seg1);
    float scal_p  = Scal2(_1_p , n),
          scal_pm = Scal2(_1_pm, n);
    if( scal_p * scal_pm >= 0 )
        return 0;
    *t = fabs(scal_p) / (fabs(scal_p) + fabs(scal_pm));
    *pos = AddVec2(p, MulVec2(mv, *t));
    return 1;
}

/* Le même que précédente, mais là c'est le segment qui se déplace et pas le point. */
int Collision_segment_point(Vec2 p, Vec2 seg1, Vec2 seg2, Vec2 mv, float *t, Vec2 * pos) {
    if(!Collision_point_segment(p, MulVec2(mv, -1), seg1, seg2, t, pos))
        return 0;
    *pos = p;
    return 1;
}

//int Collision_cercle_segment(Vec2 c, float, r, Vec2 mv, Vec2 seg1, Vec2 seg2, float *t, Vec2 *pos) {
    

int ConvexShape_compute_collision(ConvexShape *s1, Vec2 pos1, float rot1, Vec2 mov1,
                                    ConvexShape *s2, Vec2 pos2, float rot2, Vec2 mov2,
                                      Vec2 * pos_collision, float *collision_time_ratio,
                                      Vec2 * normal) {
    /* On se place dans le repère de s2 */
    Vec2 p1 = GlobalToLocal2(pos1, pos2, rot2);
    Vec2 m1 = SubVec2(GlobalToLocal2(AddVec2(pos1, mov1), AddVec2(pos2, mov2), rot2), 
                                        GlobalToLocal2(pos1, pos2, rot2));
    float r1 = rot1 - rot2;
    if(s1->type == CIRCLE && s2->type == CIRCLE) {
        Vec2 _1_2 = MulVec2(p1, -1);
        Vec2 _3   = AddVec2(p1, m1);
        Vec2 _1_3 = SubVec2(_3, p1);
        float dist_min = fabs(Scal2( _1_2, OrthogonalVec2(_1_3)));
        if(dist_min >= s1->shape.circle.radius + s2->shape.circle.radius) {
            return 0;
        }
        float a = SqrNorm(m1),
              b = 2 * Scal2(m1, p1),
              c = SqrNorm(p1) - (s1->shape.circle.radius + s2->shape.circle.radius)
                                * (s2->shape.circle.radius + s2->shape.circle.radius);
        float delta = b*b - 4*a*c;
        if(delta < 0)
            return 0;
        float t = (-b - sqrt(delta)) / (2*a);
        if(t < 0 || t > 1)
            return 0;
        *collision_time_ratio = t;
        Vec2 _2_1_collision = SubVec2(AddVec2(pos1, MulVec2(mov1, *collision_time_ratio)), 
                                    AddVec2(pos2, MulVec2(mov2, *collision_time_ratio)));
        *pos_collision = AddVec2( AddVec2(pos2, MulVec2(mov2, *collision_time_ratio)), 
                    MulVec2(_2_1_collision, s2->shape.circle.radius / (s2->shape.circle.radius + s1->shape.circle.radius)));
        *normal = MulVec2(_2_1_collision, 1. / sqrt(SqrNorm(_2_1_collision)));
        return 1;
    } else if(s1->type == POLYGON && s2->type == POLYGON) {
        /* On parcourt tous les segments de s1 puis s2, en testant une 
         * collision avec tous les points de l'autre polygone */
        float t_current;
        Vec2  pos_current;
        *collision_time_ratio = 1.;
        for(unsigned i = 0; i < s1->shape.polygon.nb_vertices; ++i) {
            unsigned i_plus_1 = (i == s1->shape.polygon.nb_vertices - 1) ? 0 : i + 1;
            for(unsigned i_point = 0; i_point < s2->shape.polygon.nb_vertices; ++i_point) {
                if(Collision_segment_point(s2->shape.polygon.vertices[i_point], 
                        LocalToGlobal2(s1->shape.polygon.vertices[i], p1, r1),
                        LocalToGlobal2(s1->shape.polygon.vertices[i_plus_1], p1, r1),
                        m1,
                        &t_current,
                        &pos_current)) {
                    if(t_current < *collision_time_ratio) {
                        *collision_time_ratio = t_current;
                        *pos_collision = pos_current;
                        *normal = OrthogonalVec2(SubVec2(LocalToGlobal2(s1->shape.polygon.vertices[i], p1, r1),
                                                         LocalToGlobal2(s1->shape.polygon.vertices[i_plus_1], p1, r1)));
                        *normal = MulVec2(*normal, 1. / sqrt(SqrNorm(*normal)));
                        printf("%f %f\n", normal->x, normal->y);
                    }
                }
            }
        }
        for(unsigned i = 0; i < s2->shape.polygon.nb_vertices; ++i) {
            unsigned i_plus_1 = (i == s2->shape.polygon.nb_vertices - 1) ? 0 : i + 1;
            for(unsigned i_point = 0; i_point < s1->shape.polygon.nb_vertices; ++i_point) {
                if(Collision_point_segment(LocalToGlobal2(s1->shape.polygon.vertices[i_point], p1, r1), 
                        m1,
                        s2->shape.polygon.vertices[i],
                        s2->shape.polygon.vertices[i_plus_1],
                        &t_current,
                        &pos_current)) {
                    if(t_current < *collision_time_ratio) {
                        *collision_time_ratio = t_current;
                        *pos_collision = pos_current;
                        *normal = OrthogonalVec2(SubVec2(s2->shape.polygon.vertices[i],
                                                         s2->shape.polygon.vertices[i_plus_1]));
                        *normal = MulVec2(*normal, 1. / sqrt(SqrNorm(*normal)));
                        printf("%f %f\n", normal->x, normal->y);
                    }
                }
            }
        }
        return *collision_time_ratio < 1.;
    } else {
        return 0;
    }
}

void Compute_force(Solid * solid, Force * force, float duration) {
    float coeff_rebond = 0, f = 0;
    
    Vec2 r = SubVec2(LocalToGlobal2(force->position, solid->position, solid->rotation),
                                         solid->position);
    Vec2 u = OrthogonalVec2(r);
    float nu = sqrt(SqrNorm(u));
    if(nu != 0)
        u = MulVec2(u, 1. / sqrt(SqrNorm(u)));
    float temp_1 = coeff_rebond - Scal2(solid->speed, force->force)
                - sqrt(SqrNorm(force->position)) * solid->rotation_speed
                * Scal2(u, force->force);
    float temp_2 = duration * ( (1 / solid->mass) +  PseudoVectProd2(r, force->force)
                                        / solid->inertia_moment);
    if(temp_2 != 0)
        f = temp_1 / temp_2;
    
    if(f < 0)
        f = 0;
        
    force->force = MulVec2(force->force, f);
    
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
    
    
    /* Gestion des collisions */
    Vec2 normal = MakeVec2(0,0), pos_collision;
    float collision_time_ratio = 0;
    for(Solid * current_solid = world->solids; current_solid != NULL; current_solid = current_solid->next) {
        unsigned force_to_apply_nbr = 0;
        for(unsigned i_shape = 0; i_shape < current_solid->nb_collision_shapes && force_to_apply_nbr < COLLISION_FORCE_NBR; ++i_shape) {
            /* Mouvement de la shape */
            Vec2 speed_shape = AddVec2(current_solid->speed, 
                MakeVec2(-current_solid->collision_shapes[i_shape].dist_to_center 
                         * current_solid->rotation_speed * sin(current_solid->rotation), 
                         current_solid->collision_shapes[i_shape].dist_to_center 
                         * current_solid->rotation_speed * cos(current_solid->rotation)));
            Vec2 position_shape = LocalToGlobal2(current_solid->collision_shapes[i_shape].position,
                    current_solid->position, current_solid->rotation);
            
            /* Parcourt de tous les obstacles. */
            for(Obstacle * current_obstacle = world->obstacles; 
                    current_obstacle && force_to_apply_nbr < COLLISION_FORCE_NBR; current_obstacle = current_obstacle->next) {
                if(ConvexShape_compute_collision(
                            &(current_solid->collision_shapes[i_shape]), 
                            position_shape, 
                            current_solid->rotation,    
                            speed_shape,
                            &(current_obstacle->shape), 
                            current_obstacle->position,
                            current_obstacle->rotation, 
                            MakeVec2(0,0),
                            &pos_collision, 
                            &collision_time_ratio, 
                            &normal)) {
                    current_solid->collision_forces[force_to_apply_nbr].position = GlobalToLocal2(pos_collision, 
                                          current_solid->position,
                                          current_solid->rotation);
                    current_solid->collision_forces[force_to_apply_nbr].force = normal;
                    current_solid->collision_forces[force_to_apply_nbr].next = NULL;
                    Compute_force(current_solid,
                            &(current_solid->collision_forces[force_to_apply_nbr]), elapsed_time);
                    ++force_to_apply_nbr;
                    
                    printf("%f %f\n", current_solid->collision_forces[force_to_apply_nbr].force.x,
                                      current_solid->collision_forces[force_to_apply_nbr].force.y);
                }
            }
        }
        /* Ajout des forces à appliquer sur le solide. */
        for(unsigned i = 0; i < force_to_apply_nbr; ++i) {
            World_addForce(world, &(current_solid->collision_forces[i]));
        }
    } 
    
    /* Mise à jour des positions et rotations. */
	Solid *tmp = world->solids;
    for(; tmp; tmp = tmp->next) {
        tmp->position = AddVec2(tmp->position,
                                            tmp->speed);
        tmp->rotation += tmp->rotation_speed;
    }
    
    return 1;
}

