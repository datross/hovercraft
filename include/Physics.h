#ifndef PHYSICS_H
#define PHYSICS_H

#include <Vec.h>

/* Structures géométriques de base */

typedef Vec2* Polygon; /* Les coordonnées des sommets sont dans 
                          dans la base locale du polygone. */

typedef enum {
    CERCLE, POLYGON
} ConvexShapeType;

typedef union {
    ConvexShapeType type;
    float radius;
    Polygon polygon;   
} ConvexShape;

/* Solides physiques */

typedef struct {
    ConvexShape shape;
    
    Vec2  position;       /* Infos de la base locale */
    float rotation;
    Vec2  speed;
    float angular_speed;
} Solid;

/* Monde physique */

typedef struct Force {
    Solid * solid; /* Solide sur lequel s'applique la force */
    Vec2 position; /* Point d'application de la force sur le solide 
                      (dans le repère local) */
    Vec2 force; /* Vecteur de la force */
    
    struct Force * next; /* Le monde contient une file (chaînée) des 
                            forces à appliquer. */
} Force;

typedef struct {
    Solid * solids; /* Tous les objets du monde */
    
    /* File des forces à appliquer */
    Force * forces_head, * forces_tail; /* head pointe vers le début
                                           et tail vers la fin (là où on
                                           insère les élements dans le file) */
} PhysicWorld;

/* Fonctions */

/* NOTE : Pour l'instant je fais pas trop de fonctions 
 * de construction des objets physiques, on verra quand 
 * on fera le chargement des maps. */
 
/* Calcule la force qu'exerce un solide en un point,
 * de part sa vitesse et sa rotation. 
 * Retourne 1 si ça s'est bien passé, 0 si erreur. */
int Compute_force(Solid * solid, Force * force);
 
/* Met à jour les positions toutes les entités du monde physique.
 * Retourne 0 si erreur, 1 sinon. */
int Process_physics(PhysicWorld * world, float elapsed_time);

#endif /* PHYSICS_H */
