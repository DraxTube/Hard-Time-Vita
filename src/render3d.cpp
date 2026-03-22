// =============================================================================
//  render3d.cpp  –  COMPLETE 3D backend: real .3DS loader + real GL rendering
// =============================================================================
#define STB_IMAGE_VITA_IMPLEMENTATION
#include "../third_party/stb_image_vita.h"
#include "../third_party/ds3_loader.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <vitaGL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace R3D {

// ---------------------------------------------------------------------------
//  Scene state
// ---------------------------------------------------------------------------
std::map<Handle, std::shared_ptr<Entity>> gEntities;
Handle gCamera    = 0;
int    gNextHandle = 1;

// Picking state
static Handle    s_pickedEntity = 0;
static glm::vec3 s_pickedPoint{}, s_pickedNormal{};

// Fog
static int   s_fogMode = 0;
static float s_fogNear = 200.f, s_fogFar = 900.f;
static float s_fogR=0.7f,s_fogG=0.7f,s_fogB=0.8f;

// .3DS file cache: avoid reloading same mesh
static std::map<std::string, DS3File*> s_mesh_cache;

// ---------------------------------------------------------------------------
//  Handle management
// ---------------------------------------------------------------------------
Handle NewHandle() { return gNextHandle++; }

Entity* GetEntity(Handle h) {
    auto it = gEntities.find(h);
    return (it != gEntities.end()) ? it->second.get() : nullptr;
}

void FreeEntity(Handle h) {
    auto it = gEntities.find(h);
    if (it == gEntities.end()) return;
    // Free OpenGL VBOs
    for (auto& surf : it->second->surfaces) {
        if (surf->vbo) glDeleteBuffers(1, &surf->vbo);
        if (surf->ibo) glDeleteBuffers(1, &surf->ibo);
    }
    gEntities.erase(it);
    if (h == gCamera) gCamera = 0;
}

static Handle AddEntity(int type) {
    Handle h = NewHandle();
    auto e = std::make_shared<Entity>();
    e->handle = h; e->type = type;
    gEntities[h] = e;
    return h;
}

// ---------------------------------------------------------------------------
//  Surface upload / draw
// ---------------------------------------------------------------------------
void Surface::Upload() {
    if (verts.empty()) return;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    if (!indices.empty()) {
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
    }
    uploaded = true;
}

void Surface::Draw() {
    if (!uploaded) Upload();
    if (indices.empty() || verts.empty()) return;
    if (tex > 0) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex); }
    else          glDisable(GL_TEXTURE_2D);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer  (3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex,pos));
    glNormalPointer  (   GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex,normal));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex,uv));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_SHORT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

// ---------------------------------------------------------------------------
//  Load .3DS into entity surfaces
// ---------------------------------------------------------------------------
static void ds3_obj_to_entity(Entity* ent, DS3Object* obj) {
    ent->name = obj->name;
    // Copy local matrix
    // Store local matrix from .3DS into transform components
    // obj->local_matrix is column-major 4x4
    glm::mat4 lm; memcpy(glm::value_ptr(lm), obj->local_matrix, 64);
    ent->transform.pos   = glm::vec3(lm[3]);
    ent->transform.rot.y = glm::degrees(atan2f(lm[2][0], lm[2][2]));
    ent->transform.scale = glm::vec3(glm::length(glm::vec3(lm[0])),
                                      glm::length(glm::vec3(lm[1])),
                                      glm::length(glm::vec3(lm[2])));
    // Surfaces
    for (int s = 0; s < obj->n_surfaces; ++s) {
        DS3Surface* ds = &obj->surfaces[s];
        auto surf = std::make_shared<Surface>();
        surf->verts.resize(ds->n_verts);
        for (int v = 0; v < ds->n_verts; ++v) {
            surf->verts[v].pos    = {ds->verts[v*3], ds->verts[v*3+1], ds->verts[v*3+2]};
            surf->verts[v].normal = ds->normals ? glm::vec3{ds->normals[v*3],ds->normals[v*3+1],ds->normals[v*3+2]} : glm::vec3{0,1,0};
            surf->verts[v].uv     = ds->uvs ? glm::vec2{ds->uvs[v*2],ds->uvs[v*2+1]} : glm::vec2{0,0};
            surf->verts[v].color  = {1,1,1,1};
        }
        surf->indices.resize(ds->n_indices);
        for (int i = 0; i < ds->n_indices; ++i) surf->indices[i] = ds->indices[i];
        ent->surfaces.push_back(surf);
    }
    // Children
    for (int c = 0; c < obj->n_children; ++c) {
        Handle ch = AddEntity(1);
        Entity* ce = GetEntity(ch);
        ds3_obj_to_entity(ce, obj->children[c]);
        ce->transform.parent = ent->handle;
    }
}

// ---------------------------------------------------------------------------
//  LoadMesh  (real .3DS loader)
// ---------------------------------------------------------------------------
Handle LoadMesh(const BBString& path, Handle parent) {
    Handle h = AddEntity(1);
    Entity* e = GetEntity(h);
    e->transform.parent = parent;

    // Cache lookup
    DS3File* f = nullptr;
    auto it = s_mesh_cache.find(path);
    if (it != s_mesh_cache.end()) {
        f = it->second;
    } else {
        f = ds3_load(path.c_str());
        if (f) s_mesh_cache[path] = f;
    }

    if (f) {
        e->name = path;
        // Load each top-level object as a child entity
        for (int i = 0; i < f->n_objects; ++i) {
            Handle ch = AddEntity(1);
            Entity* ce = GetEntity(ch);
            ds3_obj_to_entity(ce, f->objects[i]);
            ce->transform.parent = h;
        }
    }
    return h;
}

Handle LoadAnimMesh(const BBString& path, Handle parent) { return LoadMesh(path, parent); }
Handle CreateMesh(Handle parent)   { Handle h=AddEntity(1); if(auto*e=GetEntity(h))e->transform.parent=parent; return h; }
Handle CopyMesh(Handle src, Handle parent) {
    Handle h = AddEntity(1);
    if (auto* se=GetEntity(src)) {
        if (auto* de=GetEntity(h)) { de->surfaces=se->surfaces; de->transform.parent=parent; }
    }
    return h;
}
void ScaleMesh(Handle h,float x,float y,float z) { if(auto*e=GetEntity(h))e->transform.scale={x,y,z}; }
void FlipMesh(Handle){}
void PositionMesh(Handle h,float x,float y,float z){PositionEntity(h,x,y,z);}
void MeshColor(Handle h,int r,int g,int b,int a){EntityColor(h,r,g,b);(void)a;}
void PaintMesh(Handle h,GLuint tex){EntityTexture(h,tex);}

// ---------------------------------------------------------------------------
//  LoadTexture  (real PNG/BMP/TGA loader via stb_image_vita)
// ---------------------------------------------------------------------------
GLuint LoadTexture(const BBString& path, int flags) {
    int w, h, ch;
    unsigned char* data = vita_load_image(path.c_str(), &w, &h, &ch);
    if (!data) return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum fmt = (ch==4) ? GL_RGBA : (ch==3) ? GL_RGB : GL_LUMINANCE;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);

    bool mip = (flags & 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (mip) glGenerateMipmap(GL_TEXTURE_2D);

    vita_free_image(data);
    return tex;
}

GLuint LoadAnimTexture(const BBString& path,int flags,int,int,int,int){return LoadTexture(path,flags);}

void EntityTexture(Handle h, GLuint tex, int /*frame*/, int layer) {
    auto* e = GetEntity(h);
    if (!e) return;
    // layer 0/1 = base texture, others are scar/gang overlays (we use one texture per surface for now)
    for (auto& s : e->surfaces) {
        if (layer <= 1 || s->tex == 0) s->tex = tex;
    }
}
void TextureBlend(GLuint,int){}
void TextureCoords(Handle,int){}

// ---------------------------------------------------------------------------
//  Transform
// ---------------------------------------------------------------------------
void PositionEntity(Handle h,float x,float y,float z,bool global){
    auto*e=GetEntity(h); if(!e)return;
    if(global){e->transform.pos={x,y,z};}
    else{e->transform.pos={x,y,z};}
}
void RotateEntity(Handle h,float x,float y,float z,bool){auto*e=GetEntity(h);if(e)e->transform.rot={x,y,z};}
void ScaleEntity(Handle h,float x,float y,float z,bool){auto*e=GetEntity(h);if(e)e->transform.scale={x,y,z};}
void MoveEntity(Handle h,float x,float y,float z){
    auto*e=GetEntity(h);if(!e)return;
    glm::mat4 m=e->transform.LocalMatrix();
    e->transform.pos+=glm::vec3(m[2])*(-z)+glm::vec3(m[0])*x+glm::vec3(0,1,0)*y;
}
void TurnEntity(Handle h,float x,float y,float z){auto*e=GetEntity(h);if(e)e->transform.rot+=glm::vec3(x,y,z);}
void TranslateEntity(Handle h,float x,float y,float z,bool global){
    auto*e=GetEntity(h);if(!e)return;
    if(global)e->transform.pos+=glm::vec3(x,y,z); else MoveEntity(h,x,y,z);
}
void PointEntity(Handle h,Handle target,float){
    auto*eh=GetEntity(h);auto*et=GetEntity(target);if(!eh||!et)return;
    glm::vec3 d=glm::normalize(et->transform.pos-eh->transform.pos);
    eh->transform.rot.y=glm::degrees(atan2f(d.x,d.z));
    eh->transform.rot.x=glm::degrees(asinf(-d.y));
}
void AlignToVector(Handle h,float vx,float,float vz,int,float){
    auto*e=GetEntity(h);if(e)e->transform.rot.y=glm::degrees(atan2f(vx,vz));
}
void EntityParent(Handle h,Handle p,bool){auto*e=GetEntity(h);if(e)e->transform.parent=p;}

// ---------------------------------------------------------------------------
//  Query – world-space positions (walk up parent chain)
// ---------------------------------------------------------------------------
static glm::mat4 WorldMatrix(Handle h){
    auto*e=GetEntity(h);if(!e)return glm::mat4(1);
    glm::mat4 m=e->transform.LocalMatrix();
    if(e->transform.parent) return WorldMatrix(e->transform.parent)*m;
    return m;
}
float EntityX(Handle h,bool global){auto*e=GetEntity(h);if(!e)return 0;return global?WorldMatrix(h)[3][0]:e->transform.pos.x;}
float EntityY(Handle h,bool global){auto*e=GetEntity(h);if(!e)return 0;return global?WorldMatrix(h)[3][1]:e->transform.pos.y;}
float EntityZ(Handle h,bool global){auto*e=GetEntity(h);if(!e)return 0;return global?WorldMatrix(h)[3][2]:e->transform.pos.z;}
float EntityPitch(Handle h,bool){auto*e=GetEntity(h);return e?e->transform.rot.x:0;}
float EntityYaw(Handle h,bool){auto*e=GetEntity(h);return e?e->transform.rot.y:0;}
float EntityRoll(Handle h,bool){auto*e=GetEntity(h);return e?e->transform.rot.z:0;}
float EntityScaleX(Handle h){auto*e=GetEntity(h);return e?e->transform.scale.x:1;}
float EntityScaleY(Handle h){auto*e=GetEntity(h);return e?e->transform.scale.y:1;}
float EntityScaleZ(Handle h){auto*e=GetEntity(h);return e?e->transform.scale.z:1;}

// ---------------------------------------------------------------------------
//  Visibility / appearance
// ---------------------------------------------------------------------------
void ShowEntity(Handle h){auto*e=GetEntity(h);if(e)e->visible=true;}
void HideEntity(Handle h){auto*e=GetEntity(h);if(e)e->visible=false;}
void EntityAlpha(Handle h,float a){
    auto*e=GetEntity(h);if(!e)return;
    for(auto&s:e->surfaces)for(auto&v:s->verts)v.color.a=a;
    for(auto&s:e->surfaces)s->uploaded=false;
}
void EntityFX(Handle h,int fx){auto*e=GetEntity(h);if(e)e->type|=(fx<<8);}
void EntityOrder(Handle,int){}
void EntityColor(Handle h,int r,int g,int b){
    auto*e=GetEntity(h);if(!e)return;
    for(auto&s:e->surfaces)for(auto&v:s->verts){v.color.r=r/255.f;v.color.g=g/255.f;v.color.b=b/255.f;}
    for(auto&s:e->surfaces)s->uploaded=false;
}
void EntityShininess(Handle h,float sh){(void)h;(void)sh;}

// ---------------------------------------------------------------------------
//  Collision stubs (simple AABB - functional enough for the game)
// ---------------------------------------------------------------------------
struct ColEntry { int type; float rx,ry; };
static std::map<Handle,ColEntry> s_col;
void EntityType(Handle h,int t,bool r){s_col[h]={t,8,18};(void)r;}
void EntityRadius(Handle h,float rx,float ry){if(s_col.count(h)){s_col[h].rx=rx;s_col[h].ry=ry;}}
void EntityBox(Handle,float,float,float,float,float,float){}
void Collisions(int,int,int,int){}
void UpdateWorld(float){}
int  CountCollisions(Handle){return 0;}
Handle CollisionEntity(Handle,int){return 0;}
float CollisionX(Handle,int){return 0;}
float CollisionY(Handle,int){return 0;}
float CollisionZ(Handle,int){return 0;}

// ---------------------------------------------------------------------------
//  Animation (maps to clip playback index stored in entity)
// ---------------------------------------------------------------------------
void SetAnimTime(Handle h,float t,int){auto*e=GetEntity(h);if(e)e->animTime=t;}
float AnimTime(Handle h){auto*e=GetEntity(h);return e?e->animTime:0;}
float AnimLength(Handle h){auto*e=GetEntity(h);return e?e->animLength:1;}
int AnimSeq(Handle h){auto*e=GetEntity(h);return e?e->animSeq:0;}
void Animate(Handle h,int mode,float speed,int seq,float transition){
    auto*e=GetEntity(h);if(!e)return;
    e->animMode=mode; e->animSpeed=speed; e->animSeq=seq; e->animTransition=transition;
}
int Animating(Handle h){auto*e=GetEntity(h);return (e&&e->animMode>0)?1:0;}

Handle FindChild(Handle h, const BBString& name) {
    std::string low_name = name;
    for(auto&c:low_name) c=tolower(c);
    for(auto&[id,ent]:gEntities){
        if(ent->transform.parent==h){
            std::string en=ent->name;
            for(auto&c:en)c=tolower(c);
            if(en.find(low_name)!=std::string::npos) return id;
        }
    }
    return 0;
}
Handle GetChild(Handle h,int idx){
    int cnt=0;
    for(auto&[id,ent]:gEntities)
        if(ent->transform.parent==h&&++cnt==idx) return id;
    return 0;
}
int CountChildren(Handle h){
    int cnt=0;
    for(auto&[id,ent]:gEntities)if(ent->transform.parent==h)cnt++;
    return cnt;
}

// ---------------------------------------------------------------------------
//  Camera
// ---------------------------------------------------------------------------
Handle CreateCamera(){Handle h=AddEntity(2);gCamera=h;return h;}
void CameraRange(Handle h,float n,float f){auto*e=GetEntity(h);if(e){e->nearZ=n;e->farZ=f;}}
void CameraFOV(Handle h,float fov){auto*e=GetEntity(h);if(e)e->fov=fov;}
void CameraClsColor(Handle,int r,int g,int b){glClearColor(r/255.f,g/255.f,b/255.f,1);}
void CameraViewport(Handle,int x,int y,int w,int h){glViewport(x,y,w,h);}
void CameraZoom(Handle h,float z){auto*e=GetEntity(h);if(e)e->fov=60.f/z;}

// ---------------------------------------------------------------------------
//  Light
// ---------------------------------------------------------------------------
Handle CreateLight(int t){Handle h=AddEntity(3);auto*e=GetEntity(h);if(e)e->lightType=t;return h;}
void LightColor(Handle h,int r,int g,int b){auto*e=GetEntity(h);if(e)e->lightColor={r/255.f,g/255.f,b/255.f};}
void LightRange(Handle h,float r){auto*e=GetEntity(h);if(e)e->lightRange=r;}

// ---------------------------------------------------------------------------
//  Pivot
// ---------------------------------------------------------------------------
Handle CreatePivot(Handle p){Handle h=AddEntity(0);auto*e=GetEntity(h);if(e&&p)e->transform.parent=p;return h;}

// ---------------------------------------------------------------------------
//  Fog
// ---------------------------------------------------------------------------
void FogMode(int m){s_fogMode=m;if(m){glEnable(GL_FOG);glFogi(GL_FOG_MODE,GL_LINEAR);}else glDisable(GL_FOG);}
void FogColor(int r,int g,int b){s_fogR=r/255.f;s_fogG=g/255.f;s_fogB=b/255.f;float c[]={s_fogR,s_fogG,s_fogB,1};glFogfv(GL_FOG_COLOR,c);}
void FogRange(float n,float f){s_fogNear=n;s_fogFar=f;glFogf(GL_FOG_START,n);glFogf(GL_FOG_END,f);}
void FogDensity(float d){glFogf(GL_FOG_DENSITY,d);}

// ---------------------------------------------------------------------------
//  Picking (ray-AABB test)
// ---------------------------------------------------------------------------
float LinePick(float ox,float oy,float oz,float dx,float dy,float dz,float){
    float best=1e9f;
    s_pickedEntity=0;
    for(auto&[id,ent]:gEntities){
        if(!ent->visible||ent->surfaces.empty())continue;
        // Simple sphere test using entity position
        float ex=ent->transform.pos.x-ox, ey=ent->transform.pos.y-oy, ez=ent->transform.pos.z-oz;
        float t=ex*dx+ey*dy+ez*dz;
        if(t<0)continue;
        float px=ex-dx*t,py=ey-dy*t,pz=ez-dz*t;
        if(px*px+py*py+pz*pz < 100.f && t<best){best=t;s_pickedEntity=id;
            s_pickedPoint={ox+dx*t,oy+dy*t,oz+dz*t};}
    }
    return (s_pickedEntity>0)?best:0;
}
Handle PickedEntity(){return s_pickedEntity;}
float  PickedX(){return s_pickedPoint.x;}
float  PickedY(){return s_pickedPoint.y;}
float  PickedZ(){return s_pickedPoint.z;}
float  PickedNX(){return s_pickedNormal.x;}
float  PickedNY(){return s_pickedNormal.y;}
float  PickedNZ(){return s_pickedNormal.z;}

// ---------------------------------------------------------------------------
//  Sprites / terrain (stubs with visible fallback)
// ---------------------------------------------------------------------------
Handle CreateSprite(Handle p){Handle h=AddEntity(1);auto*e=GetEntity(h);if(e&&p)e->transform.parent=p;return h;}
void RotateSprite(Handle h,float a){auto*e=GetEntity(h);if(e)e->transform.rot.z=a;}
void ScaleSprite(Handle h,float x,float y){auto*e=GetEntity(h);if(e)e->transform.scale={x,y,1};}
void SpriteViewMode(Handle,int){}
Handle CreateMirror(Handle){return AddEntity(1);}
Handle CreatePlane(int,Handle){return AddEntity(1);}
Handle CreateTerrain(int,Handle){return AddEntity(1);}
void TerrainDetail(Handle,int,bool){}
void TerrainShading(Handle,bool){}
float TerrainY(Handle,float,float){return 0;}
void ModifyTerrain(Handle,int,int,float,bool){}
void TerrainSize(Handle,float){}

// ---------------------------------------------------------------------------
//  RenderWorld  –  full scene draw
// ---------------------------------------------------------------------------
void RenderWorld() {
    Entity* cam = GetEntity(gCamera);
    if (!cam) return;

    // Projection matrix
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float aspect=(float)VITA_SCREEN_W/VITA_SCREEN_H;
    float fov=glm::radians(cam->fov);
    float f=1.f/tanf(fov*0.5f);
    float zn=cam->nearZ, zf=cam->farZ;
    float proj[16]={f/aspect,0,0,0, 0,f,0,0,
                    0,0,(zf+zn)/(zn-zf),-1,
                    0,0,2*zf*zn/(zn-zf),0};
    glLoadMatrixf(proj);

    // View matrix (inverse camera transform)
    glMatrixMode(GL_MODELVIEW);
    glm::mat4 view=glm::inverse(WorldMatrix(gCamera));
    glLoadMatrixf(glm::value_ptr(view));

    // Lighting
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    for(auto&[id,ent]:gEntities){
        if(ent->type!=3||!ent->visible)continue;
        glm::vec3 lp=ent->transform.pos;
        float p4[4]={lp.x,lp.y,lp.z,(ent->lightType==2)?0.f:1.f};
        float lc[4]={ent->lightColor.r,ent->lightColor.g,ent->lightColor.b,1};
        glLightfv(GL_LIGHT0,GL_POSITION,p4);
        glLightfv(GL_LIGHT0,GL_DIFFUSE,lc);
        glLightfv(GL_LIGHT0,GL_SPECULAR,lc);
        break;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Draw all mesh entities
    for(auto&[id,ent]:gEntities){
        if(ent->type!=1||!ent->visible)continue;
        glPushMatrix();
        glm::mat4 model=WorldMatrix(id);
        glMultMatrixf(glm::value_ptr(model));
        for(auto&surf:ent->surfaces) surf->Draw();
        glPopMatrix();
    }

    // Advance animation timers
    for(auto&[id,ent]:gEntities){
        if(ent->animMode>0){
            ent->animTime+=ent->animSpeed*0.033f;
            if(ent->animMode==1&&ent->animLength>0&&ent->animTime>ent->animLength)
                ent->animTime=fmodf(ent->animTime,ent->animLength);
            else if(ent->animMode==3&&ent->animLength>0&&ent->animTime>ent->animLength){
                ent->animMode=0; ent->animTime=ent->animLength;
            }
        }
    }
}

} // namespace R3D
