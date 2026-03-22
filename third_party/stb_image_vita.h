/*
 * stb_image_vita.h  –  Minimal PNG/BMP/TGA loader for PS Vita
 *
 * This is a self-contained implementation that does NOT require the
 * original stb_image.h (unavailable offline). It implements:
 *   - BMP (24-bit, 32-bit, uncompressed)
 *   - TGA (24-bit, 32-bit, uncompressed + RLE)
 *   - PNG (via PS Vita's built-in zlib + manual PNG chunk parsing)
 *
 * Usage:
 *   #define STB_IMAGE_VITA_IMPLEMENTATION  (in exactly one .cpp)
 *   #include "stb_image_vita.h"
 *
 *   int w, h, channels;
 *   unsigned char* data = vita_load_image(path, &w, &h, &channels);
 *   vita_free_image(data);
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

unsigned char* vita_load_image(const char* path, int* w, int* h, int* channels);
void           vita_free_image(unsigned char* data);

#ifdef __cplusplus
}
#endif

/* ============================================================ */
#ifdef STB_IMAGE_VITA_IMPLEMENTATION

#include <psp2/io/fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* ---- file helpers ---- */
static long vita_fsize(SceUID fd) {
    long cur = sceIoLseek(fd, 0, SCE_SEEK_CUR);
    long end = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, cur, SCE_SEEK_SET);
    return end;
}

static unsigned char* vita_read_file(const char* path, long* out_size) {
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) { *out_size=0; return NULL; }
    long sz = vita_fsize(fd);
    unsigned char* buf = (unsigned char*)malloc(sz);
    sceIoRead(fd, buf, sz);
    sceIoClose(fd);
    *out_size = sz;
    return buf;
}

/* ---- endian helpers ---- */
static inline unsigned short LE16(const unsigned char* p) { return p[0]|(p[1]<<8); }
static inline unsigned int   LE32(const unsigned char* p) { return p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24); }
static inline unsigned int   BE32(const unsigned char* p) { return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]; }

/* ============================================================
 *  BMP LOADER
 * ============================================================ */
static unsigned char* load_bmp(const unsigned char* data, long size,
                                int* ow, int* oh, int* och) {
    if (size < 54) return NULL;
    if (data[0]!='B'||data[1]!='M') return NULL;
    int offset     = LE32(data+10);
    int w          = (int)LE32(data+18);
    int h          = (int)LE32(data+22);
    int bpp        = LE16(data+28);
    int compression= LE32(data+30);
    if (compression!=0) return NULL;          /* only uncompressed */
    if (bpp!=24 && bpp!=32) return NULL;

    int channels = (bpp==32) ? 4 : 3;
    int row_size = ((w * bpp/8 + 3) / 4) * 4;   /* padded to 4 bytes */
    unsigned char* out = (unsigned char*)malloc(w * abs(h) * channels);
    if (!out) return NULL;

    int flip = (h > 0);                          /* positive height = bottom-up */
    h = abs(h);
    *ow=w; *oh=h; *och=channels;

    for (int row=0; row<h; row++) {
        int src_row = flip ? (h-1-row) : row;
        const unsigned char* src = data + offset + src_row * row_size;
        unsigned char* dst = out + row * w * channels;
        for (int x=0; x<w; x++) {
            dst[x*channels+0] = src[x*(bpp/8)+2];  /* R */
            dst[x*channels+1] = src[x*(bpp/8)+1];  /* G */
            dst[x*channels+2] = src[x*(bpp/8)+0];  /* B */
            if (channels==4) dst[x*4+3] = (bpp==32) ? src[x*4+3] : 255;
        }
    }
    return out;
}

/* ============================================================
 *  TGA LOADER
 * ============================================================ */
static unsigned char* load_tga(const unsigned char* data, long size,
                                int* ow, int* oh, int* och) {
    if (size < 18) return NULL;
    int id_len   = data[0];
    int cmap     = data[1];
    int img_type = data[2];
    if (cmap!=0) return NULL;
    if (img_type!=2 && img_type!=10) return NULL;  /* only true-colour + RLE */
    int w    = LE16(data+12);
    int h    = LE16(data+14);
    int bpp  = data[16];
    int attr = data[17];
    if (bpp!=24 && bpp!=32) return NULL;
    int channels = (bpp==32) ? 4 : 3;
    int flip_y   = !(attr & 0x20);

    const unsigned char* src = data + 18 + id_len;
    unsigned char* out = (unsigned char*)malloc(w*h*channels);
    if (!out) return NULL;
    *ow=w; *oh=h; *och=channels;

    if (img_type==2) {
        /* uncompressed */
        for (int row=0; row<h; row++) {
            int dst_row = flip_y ? (h-1-row) : row;
            unsigned char* dst = out + dst_row * w * channels;
            for (int x=0; x<w; x++) {
                dst[x*channels+2] = *src++;  /* B */
                dst[x*channels+1] = *src++;  /* G */
                dst[x*channels+0] = *src++;  /* R */
                if (channels==4) dst[x*4+3] = (bpp==32)?*src++:255;
            }
        }
    } else {
        /* RLE */
        int px=0, total=w*h;
        unsigned char* dst = out;
        while (px < total) {
            unsigned char hdr = *src++;
            int count = (hdr&0x7F)+1;
            if (hdr & 0x80) {
                /* run */
                unsigned char r=src[2],g=src[1],b=src[0];
                unsigned char a=(bpp==32)?src[3]:255;
                src += (bpp/8);
                for (int i=0;i<count;i++,px++) {
                    int row=px/w, col=px%w;
                    int dr=flip_y?(h-1-row):row;
                    out[(dr*w+col)*channels+0]=r;
                    out[(dr*w+col)*channels+1]=g;
                    out[(dr*w+col)*channels+2]=b;
                    if(channels==4)out[(dr*w+col)*4+3]=a;
                }
            } else {
                for (int i=0;i<count;i++,px++,src+=(bpp/8)) {
                    int row=px/w,col=px%w;
                    int dr=flip_y?(h-1-row):row;
                    out[(dr*w+col)*channels+0]=src[2];
                    out[(dr*w+col)*channels+1]=src[1];
                    out[(dr*w+col)*channels+2]=src[0];
                    if(channels==4)out[(dr*w+col)*4+3]=(bpp==32)?src[3]:255;
                }
            }
        }
        (void)dst;
    }
    return out;
}

/* ============================================================
 *  PNG LOADER  (using zlib already available on Vita)
 * ============================================================ */
static unsigned char paeth(unsigned char a,unsigned char b,unsigned char c){
    int pa=abs((int)b-(int)c),pb=abs((int)a-(int)c),pc=abs((int)a+(int)b-2*(int)c);
    return (pa<=pb&&pa<=pc)?a:(pb<=pc)?b:c;
}

static unsigned char* load_png(const unsigned char* data, long size,
                                int* ow, int* oh, int* och) {
    /* PNG signature */
    if (size<8||memcmp(data,"\x89PNG\r\n\x1a\n",8)!=0) return NULL;

    int w=0,h=0,bit_depth=0,color_type=0,interlace=0;
    unsigned char* idat_buf = NULL;
    long idat_size=0;

    const unsigned char* p = data+8;
    const unsigned char* end = data+size;
    while (p < end-12) {
        unsigned int chunk_len  = BE32(p);
        unsigned char type[5];
        memcpy(type,p+4,4); type[4]=0;
        const unsigned char* chunk_data = p+8;

        if (memcmp(type,"IHDR",4)==0) {
            w          = BE32(chunk_data);
            h          = BE32(chunk_data+4);
            bit_depth  = chunk_data[8];
            color_type = chunk_data[9];
            interlace  = chunk_data[12];
        } else if (memcmp(type,"IDAT",4)==0) {
            idat_buf = (unsigned char*)realloc(idat_buf, idat_size+chunk_len);
            memcpy(idat_buf+idat_size, chunk_data, chunk_len);
            idat_size += chunk_len;
        } else if (memcmp(type,"IEND",4)==0) {
            break;
        }
        p += 12 + chunk_len;
    }

    if (!idat_buf||w==0||h==0||interlace!=0) { free(idat_buf); return NULL; }
    if (bit_depth!=8) { free(idat_buf); return NULL; }
    /* Support: RGB(2), RGBA(6), Grayscale(0), Grayscale+Alpha(4) */

    int channels;
    switch(color_type) {
        case 0: channels=1; break;
        case 2: channels=3; break;
        case 4: channels=2; break;
        case 6: channels=4; break;
        default: free(idat_buf); return NULL;
    }

    int stride = w*channels+1;   /* +1 for filter byte */
    uLongf decomp_size = (uLongf)stride*h;
    unsigned char* raw = (unsigned char*)malloc(decomp_size);
    if (uncompress(raw, &decomp_size, idat_buf, idat_size) != Z_OK) {
        free(raw); free(idat_buf); return NULL;
    }
    free(idat_buf);

    unsigned char* out = (unsigned char*)malloc(w*h*channels);
    /* Apply PNG filters row by row */
    for (int row=0; row<h; row++) {
        unsigned char filter = raw[row*stride];
        unsigned char* src   = raw + row*stride + 1;
        unsigned char* dst   = out + row*w*channels;
        unsigned char* prev  = (row>0) ? (out+(row-1)*w*channels) : NULL;
        for (int i=0;i<w*channels;i++) {
            int a=(i>=channels)?dst[i-channels]:0;
            int b=prev?prev[i]:0;
            int c=(prev&&i>=channels)?prev[i-channels]:0;
            switch(filter){
                case 0: dst[i]=src[i]; break;
                case 1: dst[i]=src[i]+(unsigned char)a; break;
                case 2: dst[i]=src[i]+(unsigned char)b; break;
                case 3: dst[i]=src[i]+(unsigned char)((a+b)/2); break;
                case 4: dst[i]=src[i]+paeth((unsigned char)a,(unsigned char)b,(unsigned char)c); break;
                default: dst[i]=src[i];
            }
        }
    }
    free(raw);
    *ow=w; *oh=h; *och=channels;
    return out;
}

/* ============================================================
 *  PUBLIC API
 * ============================================================ */
unsigned char* vita_load_image(const char* path, int* w, int* h, int* ch) {
    long size;
    unsigned char* data = vita_read_file(path, &size);
    if (!data) return NULL;
    unsigned char* result = NULL;

    /* Detect format */
    if (size>=2 && data[0]=='B' && data[1]=='M')
        result = load_bmp(data, size, w, h, ch);
    else if (size>=8 && memcmp(data,"\x89PNG",4)==0)
        result = load_png(data, size, w, h, ch);
    else
        result = load_tga(data, size, w, h, ch);

    free(data);
    return result;
}

void vita_free_image(unsigned char* data) {
    free(data);
}

#endif /* STB_IMAGE_VITA_IMPLEMENTATION */
