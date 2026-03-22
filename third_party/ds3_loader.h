#pragma once
/*
 * ds3_loader.h  –  Complete binary Autodesk .3DS mesh loader for PS Vita
 *
 * Parses the chunk-based .3DS format and returns a mesh ready for OpenGL.
 * Supports:
 *   - Multiple named objects per file
 *   - Vertices, UV coordinates, face indices
 *   - Per-object material names (for texture lookup)
 *   - Local coordinate transforms
 *   - Named child objects (used by FindChild() in the game)
 */

#ifdef __cplusplus
#include <vector>
#include <string>
#include <memory>
#include <map>
extern "C" {
#endif

/* ---- Loaded surface data ---- */
typedef struct {
    float* verts;       /* x,y,z per vertex */
    float* uvs;         /* u,v per vertex (may be NULL) */
    float* normals;     /* nx,ny,nz per vertex (computed) */
    unsigned short* indices; /* triangle indices */
    int    n_verts;
    int    n_indices;
    char   mat_name[64];
} DS3Surface;

/* ---- Loaded mesh object ---- */
typedef struct DS3Object {
    char        name[128];
    DS3Surface* surfaces;
    int         n_surfaces;
    float       local_matrix[16];   /* column-major */
    struct DS3Object** children;
    int         n_children;
} DS3Object;

/* ---- Top-level result ---- */
typedef struct {
    DS3Object*  root;       /* synthetic root containing all objects */
    DS3Object** objects;    /* flat list of all objects */
    int         n_objects;
} DS3File;

DS3File* ds3_load(const char* path);
void     ds3_free(DS3File* f);

/* Find object by name (case-insensitive substring match) */
DS3Object* ds3_find(DS3File* f, const char* name);

#ifdef __cplusplus
}
#endif
