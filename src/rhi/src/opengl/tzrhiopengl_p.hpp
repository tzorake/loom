#pragma once

// OpenGL 3.3 Core function loader (subset used by this backend).
// On desktop platforms we use the system GL headers + a minimal loader that
// fetches the extension/core procs at context-creation time.
//
// We avoid linking to a separate GLEW/glad to keep the module self-contained.

#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  define TZ_GL_LOAD_PROCS()  // All functions available via static link on macOS
#elif defined(_WIN32)
#  include <windows.h>
#  include <GL/gl.h>
   // Declare the entry points we need
   extern "C" {
       typedef void    (*PFNGLGENVERTEXARRAYSPROC)(GLsizei, GLuint *);
       typedef void    (*PFNGLBINDVERTEXARRAYPROC)(GLuint);
       typedef void    (*PFNGLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint *);
       typedef void    (*PFNGLGENBUFFERSPROC)(GLsizei, GLuint *);
       typedef void    (*PFNGLBINDBUFFERPROC)(GLenum, GLuint);
       typedef void    (*PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void *, GLenum);
       typedef void    (*PFNGLBUFFERSUBDATAPROC)(GLenum, GLintptr, GLsizeiptr, const void *);
       typedef void    (*PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint *);
       typedef void    (*PFNGLGENTEXTURESPROC)(GLsizei, GLuint *);
       typedef void    (*PFNGLBINDTEXTUREPROC)(GLenum, GLuint);
       typedef void    (*PFNGLTEXIMAGE2DPROC)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*);
       typedef void    (*PFNGLTEXSUBIMAGE2DPROC)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void*);
       typedef void    (*PFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
       typedef void    (*PFNGLDELETETEXTURESPROC)(GLsizei, const GLuint *);
       typedef void    (*PFNGLGENSAMPLERSPROC)(GLsizei, GLuint *);
       typedef void    (*PFNGLBINDSAMPLERPROC)(GLuint, GLuint);
       typedef void    (*PFNGLSAMPLERPARAMETERIPROC)(GLuint, GLenum, GLint);
       typedef void    (*PFNGLDELETESAMPLERSPROC)(GLsizei, const GLuint *);
       typedef GLuint  (*PFNGLCREATESHADERPROC)(GLenum);
       typedef void    (*PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar *const *, const GLint *);
       typedef void    (*PFNGLCOMPILESHADERPROC)(GLuint);
       typedef void    (*PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint *);
       typedef void    (*PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
       typedef void    (*PFNGLDELETESHADERPROC)(GLuint);
       typedef GLuint  (*PFNGLCREATEPROGRAMPROC)();
       typedef void    (*PFNGLATTACHSHADERPROC)(GLuint, GLuint);
       typedef void    (*PFNGLLINKPROGRAMPROC)(GLuint);
       typedef void    (*PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint *);
       typedef void    (*PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
       typedef void    (*PFNGLUSEPROGRAMPROC)(GLuint);
       typedef void    (*PFNGLDELETEPROGRAMPROC)(GLuint);
       typedef void    (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
       typedef void    (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
       typedef void    (*PFNGLDRAWELEMENTSPROC)(GLenum, GLsizei, GLenum, const void *);
       typedef void    (*PFNGLDRAWARRAYSPROC)(GLenum, GLint, GLsizei);
       typedef void    (*PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum,GLsizei,GLenum,const void*,GLint);
       typedef void    (*PFNGLBINDBUFFERRANGEPROC)(GLenum,GLuint,GLuint,GLintptr,GLsizeiptr);
       typedef GLuint  (*PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint, const GLchar *);
       typedef void    (*PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint, GLuint, GLuint);
       typedef void    (*PFNGLACTIVETEXTUREPROC)(GLenum);
       typedef void    (*PFNGLVIEWPORTPROC)(GLint,GLint,GLsizei,GLsizei);
       typedef void    (*PFNGLCLEARCOLORPROC)(GLfloat,GLfloat,GLfloat,GLfloat);
       typedef void    (*PFNGLCLEARPROC)(GLbitfield);
       typedef void    (*PFNGLDEPTHFUNCPROC)(GLenum);
       typedef void    (*PFNGLDEPTHMASKPROC)(GLboolean);
       typedef void    (*PFNGLBLENDFUNCSEPARATEPROC)(GLenum,GLenum,GLenum,GLenum);
       typedef void    (*PFNGLBLENDEQUATIONSEPARATEPROC)(GLenum, GLenum);
       typedef void    (*PFNGLCULLFACEPROC)(GLenum);
       typedef void    (*PFNGLFRONTFACEPROC)(GLenum);
       typedef void    (*PFNGLENABLEPROC)(GLenum);
       typedef void    (*PFNGLDISABLEPROC)(GLenum);
       typedef void    (*PFNGLSCISSORPROC)(GLint,GLint,GLsizei,GLsizei);
       typedef void    (*PFNGLLINEWIDTHPROC)(GLfloat);
       typedef void    (*PFNGLFLUSHPROC)();
       typedef void    (*PFNGLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
       typedef void    (*PFNGLGENFRAMEBUFFERSPROC)(GLsizei, GLuint *);
       typedef void    (*PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei, const GLuint *);
       typedef void    (*PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum,GLenum,GLenum,GLuint,GLint);
   }
   // Externs declared in tzrhiopengl.cpp
#  define GL_ARRAY_BUFFER              0x8892
#  define GL_ELEMENT_ARRAY_BUFFER      0x8893
#  define GL_UNIFORM_BUFFER            0x8A11
#  define GL_STATIC_DRAW               0x88B4
#  define GL_DYNAMIC_DRAW              0x88E8
#  define GL_FRAGMENT_SHADER           0x8B30
#  define GL_VERTEX_SHADER             0x8B31
#  define GL_COMPILE_STATUS            0x8B81
#  define GL_LINK_STATUS               0x8B82
#  define GL_TEXTURE_2D                0x0DE1
#  define GL_TEXTURE0                  0x84C0
#  define GL_RGBA                      0x1908
#  define GL_RGBA8                     0x8058
#  define GL_BGRA                      0x80E1
#  define GL_RED                       0x1903
#  define GL_R8                        0x8229
#  define GL_RG                        0x8227
#  define GL_RG8                       0x822B
#  define GL_HALF_FLOAT                0x140B
#  define GL_R16F                      0x822D
#  define GL_RGBA16F                   0x881A
#  define GL_RGBA32F                   0x8814
#  define GL_DEPTH_COMPONENT           0x1902
#  define GL_DEPTH_COMPONENT16         0x81A5
#  define GL_DEPTH_COMPONENT32F        0x8CAC
#  define GL_DEPTH24_STENCIL8          0x88F0
#  define GL_DEPTH_STENCIL             0x84F9
#  define GL_UNSIGNED_INT_24_8         0x84FA
#  define GL_TEXTURE_MIN_FILTER        0x2801
#  define GL_TEXTURE_MAG_FILTER        0x2800
#  define GL_TEXTURE_WRAP_S            0x2802
#  define GL_TEXTURE_WRAP_T            0x2803
#  define GL_NEAREST                   0x2600
#  define GL_LINEAR                    0x2601
#  define GL_LINEAR_MIPMAP_LINEAR      0x2703
#  define GL_REPEAT                    0x2901
#  define GL_MIRRORED_REPEAT           0x8370
#  define GL_CLAMP_TO_EDGE             0x812F
#  define GL_COLOR_BUFFER_BIT          0x00004000
#  define GL_DEPTH_BUFFER_BIT          0x00000100
#  define GL_STENCIL_BUFFER_BIT        0x00000400
#  define GL_UNSIGNED_BYTE             0x1401
#  define GL_UNSIGNED_SHORT            0x1403
#  define GL_UNSIGNED_INT              0x1405
#  define GL_FLOAT                     0x1406
#  define GL_TRIANGLES                 0x0004
#  define GL_TRIANGLE_STRIP            0x0005
#  define GL_LINES                     0x0001
#  define GL_POINTS                    0x0000
#  define GL_LESS                      0x0201
#  define GL_LEQUAL                    0x0203
#  define GL_EQUAL                     0x0202
#  define GL_GREATER                   0x0204
#  define GL_NOTEQUAL                  0x0205
#  define GL_GEQUAL                    0x0206
#  define GL_ALWAYS                    0x0207
#  define GL_NEVER                     0x0200
#  define GL_FRONT                     0x0404
#  define GL_BACK                      0x0405
#  define GL_CW                        0x0900
#  define GL_CCW                       0x0901
#  define GL_CULL_FACE                 0x0B44
#  define GL_DEPTH_TEST                0x0B71
#  define GL_BLEND                     0x0BE2
#  define GL_SCISSOR_TEST              0x0C11
#  define GL_ZERO                      0
#  define GL_ONE                       1
#  define GL_SRC_COLOR                 0x0300
#  define GL_ONE_MINUS_SRC_COLOR       0x0301
#  define GL_DST_COLOR                 0x0306
#  define GL_ONE_MINUS_DST_COLOR       0x0307
#  define GL_SRC_ALPHA                 0x0302
#  define GL_ONE_MINUS_SRC_ALPHA       0x0303
#  define GL_DST_ALPHA                 0x0304
#  define GL_ONE_MINUS_DST_ALPHA       0x0305
#  define GL_CONSTANT_COLOR            0x8001
#  define GL_ONE_MINUS_CONSTANT_COLOR  0x8002
#  define GL_SRC_ALPHA_SATURATE        0x0308
#  define GL_FUNC_ADD                  0x8006
#  define GL_FUNC_SUBTRACT             0x800A
#  define GL_FUNC_REVERSE_SUBTRACT     0x800B
#  define GL_MIN                       0x8007
#  define GL_MAX                       0x8008
#  define GL_FRAMEBUFFER               0x8D40
#  define GL_COLOR_ATTACHMENT0         0x8CE0
#  define GL_VERTEX_ARRAY_BINDING      0x85B5
#  define GL_TEXTURE_BINDING_2D        0x8069
   extern PFNGLGENVERTEXARRAYSPROC      glGenVertexArrays;
   extern PFNGLBINDVERTEXARRAYPROC      glBindVertexArray;
   extern PFNGLDELETEVERTEXARRAYSPROC   glDeleteVertexArrays;
   extern PFNGLGENBUFFERSPROC           glGenBuffers;
   extern PFNGLBINDBUFFERPROC           glBindBuffer;
   extern PFNGLBUFFERDATAPROC           glBufferData;
   extern PFNGLBUFFERSUBDATAPROC        glBufferSubData;
   extern PFNGLDELETEBUFFERSPROC        glDeleteBuffers;
   extern PFNGLGENTEXTURESPROC          glGenTextures;
   extern PFNGLBINDTEXTUREPROC          glBindTexture;
   extern PFNGLTEXIMAGE2DPROC           glTexImage2D;
   extern PFNGLTEXSUBIMAGE2DPROC        glTexSubImage2D;
   extern PFNGLTEXPARAMETERIPROC        glTexParameteri;
   extern PFNGLDELETETEXTURESPROC       glDeleteTextures;
   extern PFNGLGENSAMPLERSPROC          glGenSamplers;
   extern PFNGLBINDSAMPLERPROC          glBindSampler;
   extern PFNGLSAMPLERPARAMETERIPROC    glSamplerParameteri;
   extern PFNGLDELETESAMPLERSPROC       glDeleteSamplers;
   extern PFNGLCREATESHADERPROC         glCreateShader;
   extern PFNGLSHADERSOURCEPROC         glShaderSource;
   extern PFNGLCOMPILESHADERPROC        glCompileShader;
   extern PFNGLGETSHADERIVPROC          glGetShaderiv;
   extern PFNGLGETSHADERINFOLOGPROC     glGetShaderInfoLog;
   extern PFNGLDELETESHADERPROC         glDeleteShader;
   extern PFNGLCREATEPROGRAMPROC        glCreateProgram;
   extern PFNGLATTACHSHADERPROC         glAttachShader;
   extern PFNGLLINKPROGRAMPROC          glLinkProgram;
   extern PFNGLGETPROGRAMIVPROC         glGetProgramiv;
   extern PFNGLGETPROGRAMINFOLOGPROC    glGetProgramInfoLog;
   extern PFNGLUSEPROGRAMPROC           glUseProgram;
   extern PFNGLDELETEPROGRAMPROC        glDeleteProgram;
   extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
   extern PFNGLVERTEXATTRIBPOINTERPROC  glVertexAttribPointer;
   extern PFNGLDRAWARRAYSPROC           glDrawArrays;
   extern PFNGLDRAWELEMENTSPROC         glDrawElements;
   extern PFNGLDRAWELEMENTSPROC         glDrawElementsBaseVertex;
   extern PFNGLBINDBUFFERRANGEPROC      glBindBufferRange;
   extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
   extern PFNGLUNIFORMBLOCKBINDINGPROC  glUniformBlockBinding;
   extern PFNGLACTIVETEXTUREPROC        glActiveTexture;
   extern PFNGLVIEWPORTPROC             glViewport;
   extern PFNGLCLEARCOLORPROC           glClearColor;
   extern PFNGLCLEARPROC                glClear;
   extern PFNGLDEPTHFUNCPROC            glDepthFunc;
   extern PFNGLDEPTHMASKPROC            glDepthMask;
   extern PFNGLBLENDFUNCSEPARATEPROC    glBlendFuncSeparate;
   extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
   extern PFNGLCULLFACEPROC             glCullFace;
   extern PFNGLFRONTFACEPROC            glFrontFace;
   extern PFNGLENABLEPROC               glEnable;
   extern PFNGLDISABLEPROC              glDisable;
   extern PFNGLSCISSORPROC              glScissor;
   extern PFNGLLINEWIDTHPROC            glLineWidth;
   extern PFNGLFLUSHPROC                glFlush;
   extern PFNGLBINDFRAMEBUFFERPROC      glBindFramebuffer;
   extern PFNGLGENFRAMEBUFFERSPROC      glGenFramebuffers;
   extern PFNGLDELETEFRAMEBUFFERSPROC   glDeleteFramebuffers;
   extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
#  define TZ_GL_LOAD_PROCS() tzgl_load_procs()
   void tzgl_load_procs();
#else // Linux
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  include <GL/glext.h>
#  define TZ_GL_LOAD_PROCS()
#endif
