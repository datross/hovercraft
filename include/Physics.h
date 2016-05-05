#ifndef PHYSICS_H
#define PHYSICS_H

#include <Vec.h>
#include <math.h>

/* Structures géométriques de base */

typedef enum {
    CIRCLE, POLYGON
} ConvexShapeType;

typedef struct {
    Vec2  position;
    float radius;
} Circle;

typedef struct { 
    Vec2     position;
    Vec2     *vertices;
    unsigned nb_vertices;
} Polygon; /* Les coordonnées des sommets sont
              dans la base locale du polygone. */

typedef struct {
    ConvexShapeType type;
    union {
        Circle  circle;
        Polygon polygon;
    } shape;
} ConvexShape;

/* Forme de collision, statique. */
typedef struct Obstacle {
    Vec2 position;
    float rotation;
    ConvexShape shape;
    char visited; /* Si la collision est déjà testée. */
    struct Obstacle * next;
} Obstacle;

/* Solides physiques */
/* NOTE : les coordonnées dans un solide sont toutes dans le 
 * référentiel barycentrique. */
typedef struct Solid {
    /* Solide lui-même. */
    ConvexShape *collision_shapes; /* Boîtes de collision */
    unsigned    nb_collision_shapes;
    float       inertia_moment; /* Moment d'inertie. */
    float       mass;
    
    /* Infos de la base locale. */
    Vec2        position;   
    Vec2        speed;
    float       rotation;
    float       rotation_speed;

    struct Solid * next, * prev;
} Solid;

/* Monde physique */

typedef enum {
    NORMAL, COUPLE
} ForceType;

typedef struct Force {
    ForceType    type; /* Si c'est un couple, sa valeur est force.x. */
    Solid        *solid; /* Solide sur lequel s'applique la force */
    Vec2         position; /* Point d'application de la force sur le solide 
                      (dans le repère local) */
    Vec2         force; /* Vecteur de la force (repère global) */
    struct Force *next; /* Le monde contient une file (chaînée) des 
                          * forces à appliquer. */
} Force;

/* PAS POUR LE MOMENT */
/*typedef struct {
    Obstacle ** obstacles;
    unsigned nb_obstacles;
} Cell;
typedef struct {
    Cell ** grid;
    unsigned w, h;
} Grid;*/

typedef struct {
    Obstacle *obstacles; /* File de tous les obstacles du monde */
    Solid *solids; /* File de tous les objets du monde */
    /* File des forces à appliquer */
    Force *forces_head, *forces_tail; /* head pointe vers le début
                                         * et tail vers la fin (là où on
                                         * insère les élements dans la 
                                         * file).
                                         * Toutes ces forces ne sont PAS libérées
                                         * par le module physique directement. */
} PhysicWorld;

/* Fonctions */

void ConvexShape_free_content(ConvexShape * shape); /* Libère les sommets du polygones
                                                       si il y en a. */
void Circle_init(ConvexShape * shape, Vec2 position, float radius);
void Shape_init(ConvexShape * shape, Polygon polygon);
void Solid_init(Solid *solid, ConvexShape collision_shapes[], 
                                        unsigned nb_collision_shapes,
                                        float inertia_moment,
                                        float mass);
void World_clean(PhysicWorld * world);
void World_addForce(PhysicWorld * world, Force * force);
void World_addSolid(PhysicWorld * world, Solid * solid); 
void World_addObstacle(PhysicWorld * world, Obstacle * obs);

/* Retourne 1 si collision, 0 sinon.
 * p1, r1 et m1 sont les positions, rotations (on tient compte des positions
 * que contiennent les ConvexShape) et mouvement de la forme.
 * Si collision, pos_collision contient la position (repère global) de la 
 * collision, et collision_time_ratio indique quand pendant le mouvement 
 * la collision a lieu (donc €[0,1]). */
int ConvexShape_compute_collision(ConvexShape *s1, Vec2 p1, float r1, Vec2 m1,
                                    ConvexShape *s2, Vec2 p2, float r2, Vec2 m2,
                                      Vec2 * pos_collision, float *collision_time_ratio);
/* Calcule la force qu'exerce un solide en un point,
 * de part sa vitesse et sa rotation. */
void Compute_force(Solid * solid, Force * force);

/* Applique une force. */
void Apply_force(Force * force, float duration);
 
/* Met à jour les positions de toutes les entités du monde physique.
 * Retourne 0 si erreur, 1 sinon. */
int Process_physics(PhysicWorld * world, float elapsed_time);

#endif /* PHYSICS_H */
