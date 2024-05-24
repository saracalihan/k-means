#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "raylib.h"
#include "plugin.h"

#define DOT_RAD 2
#define CENTER_RAD 5
#define DOT_COLOR WHITE
#define CENTER_COLOR RED

Color GROUP_COLORS[CLUSTER_COUNT] = {
    RED,
    GREEN,
    BLUE,
    GOLD,
    PURPLE,
    MAROON,
    ORANGE,
    DARKBLUE,
    PINK,
    YELLOW,
    VIOLET,
    DARKPURPLE,
    GRAY,
    LIGHTGRAY,
    SKYBLUE,
    LIME,
    BEIGE,
    DARKGRAY,
    BROWN,
    DARKBROWN,
    DARKGREEN,
};

static Plugin* p = NULL;

int SCREEN_WIDTH  =0,
    SCREEN_HEIGHT =0;

int myrand(int max){
    return rand()%max;
}

void fill_matrix(){
    for(int i=0; i<DOT_COUNT; i++){
        p->data[myrand(MATRIX_SIZE)][myrand(MATRIX_SIZE)] =  true;
    }
    for(int i=0; i< CLUSTER_COUNT; i++){
        int* d =malloc(sizeof(int));
        p->centers[i].x= myrand(MATRIX_SIZE);
        p->centers[i].y= myrand(MATRIX_SIZE);
    }
}

int distance(Vector2 dot, Vector2 center){
    return sqrt(
        pow( dot.x - center.x,2)+
        pow( dot.y - center.y,2)
    );
}

void analyse(){
    // measure distance bitween centers and dots
    if(p->iteration ==0){
        for(int i=0;i< MATRIX_SIZE; i++){
            for(int j=0;j< MATRIX_SIZE; j++){
                if(!p->data[i][j]){
                    continue;
                }
                Vector2 dot = {
                    .x=i,
                    .y=j
                };
                // compare distensce to other clusters
                int cluster_index =0;
                int dist1 = distance(dot, p->centers[0]);
                for(int c =1; c<CLUSTER_COUNT; c++){
                    int dist2 =distance(dot, p->centers[c]);
                    if(dist2< dist1){
                        cluster_index = c;
                        dist1 = dist2;
                    }
                }
                p->clusters[cluster_index].items[p->clusters[cluster_index].count] = dot;
                p->clusters[cluster_index].count++;
            }
        } 
    }

    // recenter means
    for(int i=0; i< CLUSTER_COUNT; i++){
        Cluster *g = &p->clusters[i];
        float total_x = 0;
        float total_y = 0;

        for(int size =0; size < g->count; size++){
            Vector2 dot = g->items[size];
            total_x+= g->items[size].x;
            total_y+= g->items[size].y;
        }
        if(g->count == 0){
            continue;
        }
        total_x /= g->count;
        total_y /= g->count;
        p->centers[i] = (Vector2){
            total_x,
            total_y
        };
    }

    Cluster clusters[CLUSTER_COUNT] ={0};
    // For all clusters
    for(int i=0; i< CLUSTER_COUNT; i++){
        Vector2 dot;

        // For all grouped elements
        for(int j=0; j< p->clusters[i].count; j++){
            dot = p->clusters[i].items[j];
            int cluster_index =0;

            // compare distensce to other clusters
            int dist1 = distance(dot, p->centers[0]);
            for(int c =1; c<CLUSTER_COUNT; c++){
                int dist2 =distance(dot, p->centers[c]);
                if(dist2< dist1){
                    cluster_index = c;
                    dist1 = dist2;
                }
            }
            // Push dot into selected cluster
            clusters[cluster_index].items[clusters[cluster_index].count] = dot;
            clusters[cluster_index].count++;
        }
    }
    memcpy(&p->clusters, &clusters, sizeof(Cluster) * CLUSTER_COUNT);
}



void flush_plugin();

void plugin_task(){
    SCREEN_HEIGHT = GetScreenHeight();
    SCREEN_WIDTH = GetScreenWidth();
    int center_x = (SCREEN_WIDTH - MATRIX_SIZE)/2,
        center_y = (SCREEN_HEIGHT+MATRIX_SIZE)/2 -30;
    if(IsKeyPressed(KEY_F)){
        flush_plugin();
    }
    if(IsKeyPressed(KEY_N)){
        analyse();
        p->iteration++;
    }
    BeginDrawing();
        ClearBackground(CLITERAL(Color){17, 18, 18, 255});

        // Draw grid
        DrawLine(
            center_x,
            center_y,
            center_x,
            center_y - MATRIX_SIZE,
            LIGHTGRAY
        );
        DrawLine(
            center_x,
            center_y,
            center_x + MATRIX_SIZE,
            center_y,
            LIGHTGRAY
        );
        // 0th iteration
        if(p->iteration ==0){
            for(int i=0;i< MATRIX_SIZE; i++){
                // Draw dots
                for(int j=0;j< MATRIX_SIZE; j++){
                    if(p->data[i][j])
                        DrawCircle(center_x+ i, center_y-j, DOT_RAD, DOT_COLOR);
                }
            }
            for(int i=0; i< CLUSTER_COUNT; i++){
                DrawCircle(
                    center_x + p->centers[i].x,
                    center_y - p->centers[i].y,
                    CENTER_RAD,
                    GROUP_COLORS[i]
                );
            }
        } else{
            // Draw dots
            for(int i=0; i< CLUSTER_COUNT; i++){
                for(int j=0; j< p->clusters[i].count; j++){
                    DrawCircle(
                        center_x+ p->clusters[i].items[j].x,
                        center_y-p->clusters[i].items[j].y,
                        DOT_RAD,
                        GROUP_COLORS[i]
                    );
                }
            }
            // Draw centers
            for(int i=0; i< CLUSTER_COUNT; i++){
                DrawCircle(
                    center_x + p->centers[i].x,
                    center_y - p->centers[i].y,
                    CENTER_RAD,
                    GROUP_COLORS[i]
                );
            }
        }

        // Draw element size of clusters
        char str[25];
        int y=0;
        sprintf(str, "iteration: %i", p->iteration);
        DrawText(str, center_x, center_y +  18, 18, WHITE);
        for(int i=0; i<CLUSTER_COUNT; i++){
            sprintf(str, "%i", p->clusters[i].count);
            int text_size = MeasureText(str, 9);
            DrawText(str, center_x + y, center_y +  40, 18, GROUP_COLORS[i]);
            y+=text_size + 18;
        }
    EndDrawing();
}

void flush_plugin(){
    if(p == NULL){
        p = malloc(sizeof(Plugin));
        TraceLog(LOG_INFO, "Plugin data allocated");
    }
    assert(p != NULL && "Plugin malloc error!");

    memset(p, 0, sizeof(Plugin));

    TraceLog(LOG_INFO, "Plugin data flushed!");
}

void load_assets(){
    if(p == NULL){
        return TraceLog(LOG_ERROR, "[LOAD ASSET] Plugin is null!");
    }

    // TODO: LOAD ASSETS HERE

    srand(time(NULL));
    flush_plugin();
    fill_matrix();
    TraceLog(LOG_INFO, "[LOAD ASSET] Assets loaded.");
}

void unload_assets(){
    if(p == NULL){
        return TraceLog(LOG_ERROR, "[LOAD ASSET] Plugin is null!");
    }

    // TODO: UNLOAD ASSETS HERE

    TraceLog(LOG_INFO, "[LOAD ASSET] Assets unloaded.");
}



Plugin* plugin_init(){
    TraceLog(LOG_INFO, "[PLUGIN] Plugin initialized!");
    flush_plugin();
    load_assets();
    return p;
}

Plugin* plugin_preload(){
    unload_assets();
    TraceLog(LOG_INFO, "[PLUGIN] Plugin preload run!");
    return p;
}

void plugin_postload(Plugin* plugin){
    TraceLog(LOG_INFO, "[PLUGIN] Plugin postload run!");
    p = plugin;
    load_assets();
}