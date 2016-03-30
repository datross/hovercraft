#ifndef PHYSICS_H
#define PHYSICS_H

#include <Vec.h>
#include <math.h>

/* Structures géométriques de base */

typedef struct {
    Vec2 position;
    float radius;
} Cercle;

typedef struct { 
    Vec2* vertices;
    unsigned nb_vertices;
} Polygon; /* Les coordonnées des sommets sont
              dans la base locale du polygone. */

typedef enum {
    CERCLE, POLYGON
} ConvexShapeType;

typedef struct {
    ConvexShapeType type;
    union {
        Cercle cercle;
        Polygon polygon;
    } shape;
} ConvexShape;

/* Solides physiques */
/* NOTE : les coordonnées dans un solide sont toutes dans le 
 * référentiel barycentrique. */
typedef struct {
    /* Solide lui-même. */
    ConvexShape * collision_shapes; /* Boîtes de collision */
    unsigned nb_collision_shapes;
    float inertia_moment; /* Moment d'inertie. */
    
    /* Infos de la base locale. */
    Vec2  position;   
    Vec2  speed;
    float rotation;
    float rotation_speed;
} Solid;

/* Monde physique */

typedef struct Force {
    Solid * solid; /* Solide sur lequel s'applique la force */
    Vec2 position; /* Point d'application de la force sur le solide 
                      (dans le repère local) */
    Vec2 force; /* Vecteur de la force (repère global) */
    
    struct Force * next; /* Le monde contient une file (chaînée) des 
                            forces à appliquer. */
} Force;

typedef struct {
    Solid * solids; /* Tous les objets du monde */
    
    /* File des forces à appliquer */
    Force * forces_head, * forces_tail; /* head pointe vers le début
                                           et tail vers la fin (là où on
                                           insère les élements dans la 
                                           file) */
} PhysicWorld;

/* Fonctions */

void MakeCircle(ConvexShape * shape, Vec2 position, float radius);
void MakePolygon(ConvexShape * shape, Polygon polygon);
void MakeSolid(Solid *solid, ConvexShape collision_shapes[], 
                                        unsigned nb_collision_shapes,
                                        float inertia_moment);
 
/* Calcule la force qu'exerce un solide en un point,
 * de part sa vitesse et sa rotation. 
 * Retourne 1 si ça s'est bien passé, 0 si erreur. */
int Compute_force(Solid * solid, Force * force);

/* Applique une force.
 * Retourne 0 si erreur, 0 sinon. */
int Apply_force(Force * force);
 
/* Met à jour les positions toutes les entités du monde physique.
 * Retourne 0 si erreur, 1 sinon. */
int Process_physics(PhysicWorld * world, float elapsed_time);

#endif /* PHYSICS_H */
