#pragma once
// =============================================================================
//  render3d.h  –  3D Rendering abstraction: Blitz3D → vitaGL (OpenGL ES)
//
//  Blitz3D uses a retained-mode scene graph (entities, meshes, cameras,
//  lights, pivots).  This header provides a thin scene graph that maps to
//  vitaGL draw calls.  It is intentionally minimal: implement only what
//  Hard Time actually uses.
// =============================================================================

#include "blitz_compat.h"
#include <vitaGL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include <map>

// ---------------------------------------------------------------------------
//  Forward declarations
// ---------------------------------------------------------------------------
struct Entity;
struct Mesh;
struct Camera;
struct Light;

// ---------------------------------------------------------------------------
//  Global entity registry  (Blitz3D uses integer handles; we use pointers
//  wrapped in a handle map for source-level compatibility)
// ---------------------------------------------------------------------------
using Handle = int;

namespace R3D {

struct Transform {
    glm::vec3 pos   {0.0f};
    glm::vec3 rot   {0.0f};   // Euler degrees (Blitz3D convention: pitch/yaw/roll)
    glm::vec3 scale {1.0f};
    Handle    parent{0};

    glm::mat4 LocalMatrix() const {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, pos);
        m = glm::rotate(m, glm::radians(rot.y), glm::vec3(0,1,0));
        m = glm::rotate(m, glm::radians(rot.x), glm::vec3(1,0,0));
        m = glm::rotate(m, glm::radians(rot.z), glm::vec3(0,0,1));
        m = glm::scale(m, scale);
        return m;
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color{1.0f};
};

struct Surface {
    std::vector<Vertex>   verts;
    std::vector<uint16_t> indices;
    GLuint vbo = 0, ibo = 0, tex = 0;
    bool   uploaded = false;

    void Upload();
    void Draw();
};

struct Entity {
    Handle      handle;
    Transform   transform;
    bool        visible  = true;
    bool        deleted  = false;
    std::string name;
    int         type     = 0;   // 0=pivot,1=mesh,2=camera,3=light
    // For mesh entities
    std::vector<std::shared_ptr<Surface>> surfaces;
    // For camera
    float fov   = 60.0f;
    float nearZ = 0.1f;
    float farZ  = 1000.0f;
    // For light
    glm::vec3 lightColor{1.0f};
    float     lightRange = 20.0f;
    int       lightType  = 1;   // 1=point, 2=directional, 3=spot
    // Animation state
    float animTime       = 0.0f;
    float animLength     = 1.0f;
    float animSpeed      = 1.0f;
    float animTransition = 0.0f;
    int   animMode       = 0;    // 0=stopped, 1=loop, 3=once
    int   animSeq        = 0;    // current clip ID
};

// ---------------------------------------------------------------------------
//  Scene manager
// ---------------------------------------------------------------------------
extern std::map<Handle, std::shared_ptr<Entity>> gEntities;
extern Handle   gCamera;       // active camera handle
extern int      gNextHandle;

Handle   NewHandle();
Entity*  GetEntity(Handle h);
void     FreeEntity(Handle h);

// ---------------------------------------------------------------------------
//  Blitz3D-compatible API wrappers
// ---------------------------------------------------------------------------

//  Camera
Handle CreateCamera();
void   CameraRange(Handle cam, float nearZ, float farZ);
void   CameraFOV(Handle cam, float fov);
void   CameraClsColor(Handle cam, int r, int g, int b);
void   CameraViewport(Handle cam, int x, int y, int w, int h);
void   CameraZoom(Handle cam, float zoom);

// Light
Handle CreateLight(int lightType = 1);
void   LightColor(Handle light, int r, int g, int b);
void   LightRange(Handle light, float range);

// Pivot (empty node)
Handle CreatePivot(Handle parent = 0);

// Mesh loading / creation
Handle LoadMesh(const BBString& path, Handle parent = 0);
Handle LoadAnimMesh(const BBString& path, Handle parent = 0);
Handle CreateMesh(Handle parent = 0);
Handle CopyMesh(Handle src,  Handle parent = 0);
void   ScaleMesh(Handle mesh, float sx, float sy, float sz);
void   MeshColor(Handle mesh, int r, int g, int b, int a = 255);
void   PaintMesh(Handle mesh, GLuint texHandle);
void   FlipMesh(Handle mesh);
void   PositionMesh(Handle mesh, float x, float y, float z);

// Texture
GLuint LoadTexture(const BBString& path, int flags = 1);
GLuint LoadAnimTexture(const BBString& path, int flags, int fw, int fh, int first, int count);
void   EntityTexture(Handle ent, GLuint tex, int frame = 0, int surface = 0);
void   TextureBlend(GLuint tex, int blend);
void   TextureCoords(Handle ent, int coordSet);

// Transform
void   PositionEntity(Handle ent, float x, float y, float z, bool global = false);
void   RotateEntity(Handle ent, float x, float y, float z, bool global = false);
void   ScaleEntity(Handle ent, float x, float y, float z, bool global = false);
void   MoveEntity(Handle ent, float x, float y, float z);
void   TurnEntity(Handle ent, float x, float y, float z);
void   TranslateEntity(Handle ent, float x, float y, float z, bool global = false);
void   PointEntity(Handle ent, Handle target, float roll = 0.0f);
void   AlignToVector(Handle ent, float vx, float vy, float vz, int axis = 2, float tween = 1.0f);
void   EntityParent(Handle ent, Handle parent, bool keepGlobal = true);

// Query
float  EntityX(Handle ent, bool global = false);
float  EntityY(Handle ent, bool global = false);
float  EntityZ(Handle ent, bool global = false);
float  EntityPitch(Handle ent, bool global = false);
float  EntityYaw(Handle ent, bool global = false);
float  EntityRoll(Handle ent, bool global = false);
float  EntityScaleX(Handle ent);
float  EntityScaleY(Handle ent);
float  EntityScaleZ(Handle ent);

// Visibility
void   ShowEntity(Handle ent);
void   HideEntity(Handle ent);
void   EntityAlpha(Handle ent, float alpha);
void   EntityFX(Handle ent, int fx);
void   EntityOrder(Handle ent, int order);
void   EntityColor(Handle ent, int r, int g, int b);
void   EntityShininess(Handle ent, float shine);

// Collision / physics (simplified; Blitz3D collisions map to AABB tests)
void   EntityType(Handle ent, int type, bool recursive = false);
void   EntityRadius(Handle ent, float rx, float ry = 0.0f);
void   EntityBox(Handle ent, float x, float y, float z, float w, float h, float d);
void   Collisions(int src, int dst, int method, int response);
void   UpdateWorld(float elapsed = 1.0f);
int    CountCollisions(Handle ent);
Handle CollisionEntity(Handle ent, int index);
float  CollisionX(Handle ent, int index);
float  CollisionY(Handle ent, int index);
float  CollisionZ(Handle ent, int index);

// Animation (Blitz3D bone animation)
int    AnimSeq(Handle ent);
void   SetAnimTime(Handle ent, float time, int seq = 0);
float  AnimTime(Handle ent);
float  AnimLength(Handle ent);
void   Animate(Handle ent, int mode = 1, float speed = 1.0f, int seq = 0, float transition = 0.0f);
int    Animating(Handle ent);
Handle FindChild(Handle ent, const BBString& name);
Handle GetChild(Handle ent, int index);
int    CountChildren(Handle ent);

// Terrain
Handle CreateTerrain(int size, Handle parent = 0);
void   TerrainDetail(Handle ter, int detail, bool morph = false);
void   TerrainShading(Handle ter, bool enable);
float  TerrainY(Handle ter, float x, float z);
void   ModifyTerrain(Handle ter, int x, int z, float height, bool updateNormals = true);
void   TerrainSize(Handle ter, float size);

// Fog
void   FogMode(int mode);
void   FogColor(int r, int g, int b);
void   FogRange(float nearF, float farF);
void   FogDensity(float density);

// Picking / line-of-sight
float  LinePick(float x, float y, float z, float dx, float dy, float dz, float radius = 0.0f);
Handle PickedEntity();
float  PickedX();
float  PickedY();
float  PickedZ();
float  PickedNX();
float  PickedNY();
float  PickedNZ();

// Sprite / billboard
Handle CreateSprite(Handle parent = 0);
void   RotateSprite(Handle sprite, float angle);
void   ScaleSprite(Handle sprite, float x, float y);
void   SpriteViewMode(Handle sprite, int mode);
Handle CreateMirror(Handle parent = 0);
Handle CreatePlane(int detail = 1, Handle parent = 0);

// Render
void   RenderWorld();
void   UpdateWorld2D();

} // namespace R3D

// Expose without prefix (to match original .bb call sites)
using namespace R3D;
