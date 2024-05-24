#ifndef PLUGIN_H
#define PLUGIN_H

#include <raylib.h>

#define MATRIX_SIZE 250
#define DOT_COUNT MATRIX_SIZE
#define CLUSTER_COUNT 5
#define CLUSTER_SIZE MATRIX_SIZE * MATRIX_SIZE

//static_assert(CLUSTER_COUNT >0 && "[ERROR] CLUSTER_COUNT must be bigger then 0!");


typedef struct{
        Vector2 items[CLUSTER_SIZE];
        int count;
} Cluster;

typedef struct{
        bool data[MATRIX_SIZE][MATRIX_SIZE];
        Vector2 centers[CLUSTER_COUNT];
        Cluster clusters[CLUSTER_COUNT];
        int iteration;
} Plugin;

#endif // PLUGIN_H
