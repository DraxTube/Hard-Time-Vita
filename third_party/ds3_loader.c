/*
 * ds3_loader.c  –  Complete .3DS loader implementation
 *
 * .3DS chunk IDs used by Hard Time:
 *   0x4D4D  MAIN3DS
 *   0x3D3D  EDIT3DS
 *   0x4000  EDIT_OBJECT   (named object)
 *   0x4100  OBJ_TRIMESH   (triangle mesh)
 *   0x4110  TRI_VERTEXL   (vertex list)
 *   0x4120  TRI_FACEL1    (face list + flags)
 *   0x4130  TRI_MATERIAL  (material per face group)
 *   0x4140  TRI_MAPPINGCOORDS (UV per vertex)
 *   0x4160  TRI_LOCAL     (local 3x4 transform matrix)
 *   0xAFFF  MATERIAL_BLOCK
 *   0xA000  MAT_NAME
 *   0xB000  KEYFRAME_CHUNK (skipped)
 */

#define DS3_LOADER_IMPLEMENTATION
#include "ds3_loader.h"

#include <psp2/io/fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- file buffer ---- */
typedef struct { const unsigned char* data; long size; long pos; } Buf;
static inline unsigned char  B8(Buf* b)  { return b->data[b->pos++]; }
static inline unsigned short B16(Buf* b) { unsigned short v=b->data[b->pos]|(b->data[b->pos+1]<<8); b->pos+=2; return v; }
static inline unsigned int   B32(Buf* b) { unsigned int v=b->data[b->pos]|(b->data[b->pos+1]<<8)|(b->data[b->pos+2]<<16)|(b->data[b->pos+3]<<24); b->pos+=4; return v; }
static inline float          BF(Buf* b)  { float f; unsigned int i=B32(b); memcpy(&f,&i,4); return f; }
static void read_str(Buf* b, char* out, int maxlen) {
    int i=0; char c;
    while((c=(char)B8(b))!=0 && i<maxlen-1) out[i++]=c;
    out[i]=0;
}
static void skip(Buf* b, long n) { b->pos+=n; }

/* ---- compute flat normals ---- */
static void compute_normals(DS3Surface* s) {
    s->normals=(float*)calloc(s->n_verts*3,sizeof(float));
    for(int i=0;i<s->n_indices;i+=3){
        int a=s->indices[i],b_=s->indices[i+1],c=s->indices[i+2];
        float ax=s->verts[a*3],ay=s->verts[a*3+1],az=s->verts[a*3+2];
        float bx=s->verts[b_*3],by=s->verts[b_*3+1],bz=s->verts[b_*3+2];
        float cx=s->verts[c*3],cy=s->verts[c*3+1],cz=s->verts[c*3+2];
        float ex=bx-ax,ey=by-ay,ez=bz-az;
        float fx=cx-ax,fy=cy-ay,fz=cz-az;
        float nx=ey*fz-ez*fy, ny=ez*fx-ex*fz, nz=ex*fy-ey*fx;
        float len=sqrtf(nx*nx+ny*ny+nz*nz);
        if(len>0){nx/=len;ny/=len;nz/=len;}
        for(int j=0;j<3;j++){
            int vi=s->indices[i+j];
            s->normals[vi*3]+=nx; s->normals[vi*3+1]+=ny; s->normals[vi*3+2]+=nz;
        }
    }
    for(int i=0;i<s->n_verts;i++){
        float* n=s->normals+i*3;
        float len=sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
        if(len>0){n[0]/=len;n[1]/=len;n[2]/=len;}
    }
}

/* ---- DS3Object pool ---- */
#define MAX_OBJECTS 512
static DS3Object g_objects[MAX_OBJECTS];
static int       g_n_objects=0;

static DS3Object* new_obj(const char* name){
    if(g_n_objects>=MAX_OBJECTS) return NULL;
    DS3Object* o=&g_objects[g_n_objects++];
    memset(o,0,sizeof(*o));
    strncpy(o->name,name,127);
    /* identity matrix */
    float id[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(o->local_matrix,id,64);
    return o;
}

/* ---- parse TRI_MATERIAL chunk: associate material name with face range ---- */
static void parse_tri_material(Buf* b, long end, DS3Surface* s){
    char mat[64]; read_str(b,mat,64);
    strncpy(s->mat_name,mat,63);
    int nfaces=B16(b);
    /* skip face indices - we use material per surface */
    skip(b, nfaces*2);
    (void)end;
}

/* ---- parse OBJ_TRIMESH chunk ---- */
static void parse_trimesh(Buf* b, long end, DS3Object* obj){
    float* verts=NULL; int n_verts=0;
    float* uvs=NULL;
    unsigned short* faces=NULL; int n_faces=0;
    float local[16]; int has_local=0;
    char mat_name[64]={0};

    while(b->pos < end){
        unsigned short chunk_id=B16(b);
        unsigned int   chunk_len=B32(b);
        long chunk_end=b->pos + chunk_len - 6;

        if(chunk_id==0x4110){ /* TRI_VERTEXL */
            n_verts=B16(b);
            verts=(float*)malloc(n_verts*3*sizeof(float));
            for(int i=0;i<n_verts;i++){
                verts[i*3+0]=BF(b);
                verts[i*3+2]=BF(b);  /* .3DS: Y and Z swapped vs OpenGL */
                verts[i*3+1]=BF(b);
            }
        } else if(chunk_id==0x4120){ /* TRI_FACEL1 */
            n_faces=B16(b);
            faces=(unsigned short*)malloc(n_faces*3*sizeof(unsigned short));
            for(int i=0;i<n_faces;i++){
                faces[i*3+0]=B16(b);
                faces[i*3+2]=B16(b);  /* .3DS winding fix */
                faces[i*3+1]=B16(b);
                B16(b); /* flags */
            }
            /* sub-chunks (material) */
            while(b->pos < chunk_end){
                unsigned short sid=B16(b);
                unsigned int   slen=B32(b);
                long send=b->pos+slen-6;
                if(sid==0x4130){ /* TRI_MATERIAL */
                    read_str(b,mat_name,64);
                    int nf=B16(b); skip(b,nf*2);
                }
                b->pos=send;
            }
        } else if(chunk_id==0x4140){ /* TRI_MAPPINGCOORDS */
            int n=B16(b);
            uvs=(float*)malloc(n*2*sizeof(float));
            for(int i=0;i<n;i++){ uvs[i*2+0]=BF(b); uvs[i*2+1]=1.0f-BF(b); }
        } else if(chunk_id==0x4160){ /* TRI_LOCAL */
            /* 3x4 row-major matrix in file → column-major for GL */
            float row[12];
            for(int i=0;i<12;i++) row[i]=BF(b);
            local[0]=row[0]; local[4]=row[1]; local[8]=row[2];  local[12]=row[9];
            local[1]=row[3]; local[5]=row[4]; local[9]=row[5];  local[13]=row[10];
            local[2]=row[6]; local[6]=row[7]; local[10]=row[8]; local[14]=row[11];
            local[3]=0;      local[7]=0;      local[11]=0;      local[15]=1;
            has_local=1;
        }
        b->pos=chunk_end;
    }

    if(has_local) memcpy(obj->local_matrix,local,64);

    if(verts && faces && n_faces>0){
        obj->n_surfaces=1;
        obj->surfaces=(DS3Surface*)calloc(1,sizeof(DS3Surface));
        DS3Surface* s=obj->surfaces;
        s->verts=verts;
        s->uvs=uvs;
        s->n_verts=n_verts;
        s->n_indices=n_faces*3;
        s->indices=faces;
        strncpy(s->mat_name,mat_name,63);
        compute_normals(s);
    } else {
        free(verts); free(uvs); free(faces);
    }
}

/* ---- parse EDIT3DS chunk ---- */
static void parse_edit3ds(Buf* b, long end, DS3File* out){
    while(b->pos < end){
        unsigned short chunk_id=B16(b);
        unsigned int   chunk_len=B32(b);
        long chunk_end=b->pos+chunk_len-6;

        if(chunk_id==0x4000){ /* EDIT_OBJECT */
            char name[128]; read_str(b,name,128);
            DS3Object* obj=new_obj(name);
            /* Next sub-chunk should be OBJ_TRIMESH (0x4100) */
            while(b->pos < chunk_end){
                unsigned short sid=B16(b);
                unsigned int   slen=B32(b);
                long send=b->pos+slen-6;
                if(sid==0x4100) parse_trimesh(b,send,obj);
                b->pos=send;
            }
        }
        b->pos=chunk_end;
    }
}

/* ---- public API ---- */
DS3File* ds3_load(const char* path){
    /* Read file */
    SceUID fd=sceIoOpen(path,SCE_O_RDONLY,0);
    if(fd<0) return NULL;
    long sz=sceIoLseek(fd,0,SCE_SEEK_END);
    sceIoLseek(fd,0,SCE_SEEK_SET);
    unsigned char* raw=(unsigned char*)malloc(sz);
    sceIoRead(fd,raw,sz);
    sceIoClose(fd);

    Buf buf={raw,sz,0};
    g_n_objects=0;

    /* Check MAIN3DS signature */
    if(sz<6||B16(&buf)!=0x4D4D){ free(raw); return NULL; }
    unsigned int main_len=B32(&buf);
    long main_end=6+main_len-6;

    while(buf.pos < main_end && buf.pos < sz){
        unsigned short cid=B16(&buf);
        unsigned int   clen=B32(&buf);
        long cend=buf.pos+clen-6;
        if(cid==0x3D3D) parse_edit3ds(&buf,cend,NULL); /* EDIT3DS */
        buf.pos=cend;
    }
    free(raw);

    if(g_n_objects==0) return NULL;

    DS3File* f=(DS3File*)calloc(1,sizeof(DS3File));
    f->n_objects=g_n_objects;
    f->objects=(DS3Object**)malloc(g_n_objects*sizeof(DS3Object*));
    for(int i=0;i<g_n_objects;i++) f->objects[i]=&g_objects[i];

    /* Create synthetic root */
    static DS3Object root_obj;
    memset(&root_obj,0,sizeof(root_obj));
    strcpy(root_obj.name,"__root__");
    root_obj.n_children=g_n_objects;
    root_obj.children=(DS3Object**)malloc(g_n_objects*sizeof(DS3Object*));
    memcpy(root_obj.children,f->objects,g_n_objects*sizeof(DS3Object*));
    float id[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    memcpy(root_obj.local_matrix,id,64);
    f->root=&root_obj;
    return f;
}

void ds3_free(DS3File* f){
    if(!f) return;
    for(int i=0;i<f->n_objects;i++){
        DS3Object* o=f->objects[i];
        for(int s=0;s<o->n_surfaces;s++){
            free(o->surfaces[s].verts);
            free(o->surfaces[s].uvs);
            free(o->surfaces[s].normals);
            free(o->surfaces[s].indices);
        }
        free(o->surfaces);
        free(o->children);
    }
    free(f->root->children);
    free(f->objects);
    free(f);
}

static int istr_contains(const char* hay, const char* needle){
    if(!hay||!needle) return 0;
    char h[128],n[64];
    int i;
    for(i=0;hay[i]&&i<127;i++) h[i]=(hay[i]>='A'&&hay[i]<='Z')?(hay[i]+32):hay[i];
    h[i]=0;
    for(i=0;needle[i]&&i<63;i++) n[i]=(needle[i]>='A'&&needle[i]<='Z')?(needle[i]+32):needle[i];
    n[i]=0;
    return strstr(h,n)!=NULL;
}

DS3Object* ds3_find(DS3File* f, const char* name){
    if(!f||!name) return NULL;
    for(int i=0;i<f->n_objects;i++)
        if(istr_contains(f->objects[i]->name,name))
            return f->objects[i];
    return NULL;
}
