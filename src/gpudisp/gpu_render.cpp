#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <vector>
#include <array>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "gpu_render.h"
#include "drm_display.h"
#include "osal.h"

#define MAX_INPUT_NUM 6

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

typedef struct MTK_GPU_REND{
	void *gbm_dev;
	EGLDisplay egl_display;
	EGLContext egl_ctx;

    GLuint tex_program;
    GLuint tex_pos;
    GLuint tex_coords;

	GLuint line_program;
    GLuint line_pos_loc;
    GLuint line_tex_loc;
	GLuint line_color_loc;
	GLuint line_canvas_loc;
}MTK_GPU_REND_T;

struct gpu_tex_info{
	int flag; // 0: input tex 1:output tex
	EGLImageKHR eglimg;
	GLuint texid;
	GLuint glfb;
	GLuint glrb;
};

int local_dynamic_log_level = -1;

static PFNEGLGETPLATFORMDISPLAYEXTPROC pf_get_platform_display;

static PFNEGLCREATEIMAGEKHRPROC pfeglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC pfeglDestroyImageKHR;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfglEGLImageTargetTexture2DOES;
static PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC pfglEGLImageTargetRenderbufferStorageOES;

static int
format_check_yuv(uint32_t format)
{
	switch (format) {
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_AYUV:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV410:
	case DRM_FORMAT_YVU410:
	case DRM_FORMAT_YUV411:
	case DRM_FORMAT_YVU411:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YVU422:
	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_YVU444:
		return 0;
	default:
		return -1;
	}
}

static const char *get_gl_error(void)
{
	switch (glGetError()) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "Unknown error";
	}
}

static const char *get_egl_error(void)
{
	switch (eglGetError()) {
	case EGL_SUCCESS:
		return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED:
		return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT:
		return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG:
		return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE:
		return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY:
		return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE:
		return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH:
		return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER:
		return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP:
		return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST:
		return "EGL_CONTEXT_LOST";
	default:
		return "Unknown error";
	}
}

static void check_gl_error(const char* op) 
{
	GLint error=0;
	for (error = glGetError(); error; error = glGetError()) {
		LOG_ERR("after %s() glError (0x%x)\n", op, error);
	}
}

static int
_init_egl(MTK_GPU_REND_T *pctx)
{
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	const char *extensions;

	EGLint config_attribs[] = {
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_DEPTH_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_DONT_CARE,
		EGL_NONE
	};

	EGLint major, minor;
	EGLConfig egl_config;
	EGLint num_configs;

	LOG_INFO("eglGetDisplay Start\n");

	pctx->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	LOG_INFO("eglGetDisplay Done\n");

	if (EGL_NO_DISPLAY == pctx->egl_display) {
		LOG_ERR("failed to get egl display\n");
		return -1;
	} else {
		LOG_INFO("initialize egl: %s\n", get_egl_error());
	}

	LOG_INFO("eglInitialize Start major %p minor %p \n", &major, &minor);

	if (!eglInitialize(pctx->egl_display, &major, &minor)) {
		LOG_ERR("failed to initialize egl: %s\n", get_egl_error());
		return -2;
	}
	LOG_INFO("eglInitialize Done \n");

	LOG_INFO("eglBindAPI Start \n");
	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		LOG_ERR("failed to bind OpenGL ES: %s\n", get_egl_error());
		return -3;
	}
	LOG_INFO("eglBindAPI Done \n");

	extensions = eglQueryString(pctx->egl_display, EGL_EXTENSIONS);
	LOG_INFO("EGL Extensions: %s\n", extensions);

	if (!eglChooseConfig(pctx->egl_display, config_attribs, NULL, 0,
		&num_configs)) {
		LOG_ERR("eglChooseConfig() failed with error: %s\n", get_egl_error());
		return -1;
	}

	if (!eglChooseConfig(pctx->egl_display, config_attribs, &egl_config, 1,
		&num_configs)) {
		LOG_ERR("eglChooseConfig() failed with error: %s\n", get_egl_error());
		return -2;
	}

	pctx->egl_ctx = eglCreateContext(pctx->egl_display,
		egl_config,
		EGL_NO_CONTEXT /* No shared context */,
		context_attribs);
	if (pctx->egl_ctx == EGL_NO_CONTEXT){
		LOG_ERR("failed to create OpenGL ES Context: %s\n", get_egl_error());
		return -3;
	}

	if (!eglMakeCurrent(pctx->egl_display,
			EGL_NO_SURFACE /* No default draw surface */,
			EGL_NO_SURFACE /* No default draw read */,
			pctx->egl_ctx)) {
		LOG_ERR("failed to make the OpenGL ES Context current: %s\n", get_egl_error());
		return -4;
	}

	pfeglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	pfeglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	pfglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	pfglEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)eglGetProcAddress("glEGLImageTargetRenderbufferStorageOES");

	if (!pfeglCreateImageKHR || !pfeglDestroyImageKHR || !pfglEGLImageTargetTexture2DOES || !pfglEGLImageTargetRenderbufferStorageOES) {
		LOG_ERR("eglGetProcAddress returned NULL for a required extension entry point\n");
		return -5;
	}

	return 0;
}

static void
_fini_egl(MTK_GPU_REND_T *pctx)
{
		eglTerminate(pctx->egl_display);
		eglReleaseThread();
}

static GLuint _load_shader(GLenum shaderType, const char *pSource)
{
    GLint compiled = 0;
    GLint infoLen = 0;
    char* buf;
    GLuint shader = glCreateShader(shaderType);

    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOG_ERR("Could not compile shader %d:\n%s\n",
                        shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }

    return shader;
}

static GLuint _create_program(const char *pVertexSource, const char *pFragmentSource)
{
    GLuint vertexShader, pixelShader;
    GLuint program;

    vertexShader = _load_shader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
            return 0;

    pixelShader = _load_shader(GL_FRAGMENT_SHADER, pFragmentSource);
        if (!pixelShader)
        return 0;

    program = glCreateProgram();
    if (program) {
        GLint linkStatus = GL_FALSE;
        glAttachShader(program, vertexShader);
        check_gl_error("glAttachShader");
        glAttachShader(program, pixelShader);
        check_gl_error("glAttachShader");
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOG_ERR("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }

    return program;
}

static const char vertexShader[] =
    "attribute vec4 position;\n"
    "attribute vec2 texCoords;\n"
    "varying vec2 outTexCoords;\n"
    "void main(void) {\n"
    "    outTexCoords = texCoords;\n"
    "    gl_Position = position;\n"
    "}\n";

static const char fragmentShader[] = 
	"#extension GL_OES_EGL_image_external : enable\n"
	"varying mediump vec2 outTexCoords;\n"
	"uniform samplerExternalOES texture;\n"
	"void main(void) {\n"
	"    gl_FragColor = texture2D(texture, outTexCoords);\n"
	"}\n";

static const char lineVert[] =
	"attribute vec4 position;\n"
	"attribute vec2 texCoords;\n"
	"varying vec2 outTexCoords;\n"
	"\n"
	"uniform vec4 canvas;\n"
	"const float f = 100.0;\n"
	"const float n = 0.1;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    float l = canvas.x;\n"
	"    float r = canvas.y;\n"
	"    float t = canvas.z;\n"
	"    float b = canvas.w;\n"
	"    mat4 M;\n"
	"    M[0][0] = 2.0 / (r - l);\n"
	"    M[0][1] = 0.0;\n"
	"    M[0][2] = 0.0;\n"
	"    M[0][3] = 0.0;\n"
	"\n"
	"    M[1][0] = 0.0;\n"
	"    M[1][1] = 2.0 / (b - t);\n"
	"    M[1][2] = 0.0;\n"
	"    M[1][3] = 0.0;\n"
	"\n"
	"    M[2][0] = 0.0;\n"
	"    M[2][1] = 0.0;\n"
	"    M[2][2] = -2.0 / (f - n);\n"
	"    M[2][3] = 0.0;\n"
	"\n"
	"    M[3][0] = -(r + l) / (r - l);\n"
	"    M[3][1] = -(b + t) / (b - t);\n"
	"    M[3][2] = -(f + n) / (f - n);\n"
	"    M[3][3] = 1.0;\n"
	"\n"
	"    gl_Position = M * position;\n"
	"    gl_Position.z = position.z;\n"
	"    //gl_Position.w = 1.0;\n"
	"    //gl_Position = position;\n"
	"    outTexCoords = texCoords;\n"
	"}\n";

static const char lineFrag[] =
	"#extension GL_OES_EGL_image_external : enable\n"
    "precision mediump float;\n"
	"uniform vec4 line_color;\n"
	"varying mediump vec2 outTexCoords;\n"
	"void main()\n"
	"{\n"
	"    //float a = outTexCoords.x / 30.0;\n"
	"    //a = clamp(a * 25.0, 0.0, 1.0);\n"
	"    //a = clamp(10.0 * (1.0 - a), 0.0, 1.0);\n"
	"    gl_FragColor = line_color;\n"
    "    //gl_FragColor.x = a;\n"
    "    //gl_FragColor = vec4(0.4, 1.0, 0.4, 1.0);\n"
	"}\n";

static int _init_gl(MTK_GPU_REND_T *pctx)
{
	char pszInfoLog[1024];
	int nShaderStatus, nInfoLogLength;
	GLuint program;

	/* load texture program */
	program = _create_program(vertexShader, fragmentShader);
	if (!program) {
		LOG_ERR("create program fail\n");
		return -1;
	}

	glValidateProgram(program);

	glGetProgramiv(program, GL_VALIDATE_STATUS, &nShaderStatus);

	if (nShaderStatus != GL_TRUE) {
		LOG_ERR("Error: Failed to validate GLSL program\n");
		glGetProgramInfoLog(program, 1024, &nInfoLogLength, pszInfoLog);
		LOG_ERR("%s", pszInfoLog);
		return -2;
	}

	pctx->tex_program = program;
	pctx->tex_pos = glGetAttribLocation(program, "position");
	pctx->tex_coords = glGetAttribLocation(program, "texCoords");

    LOG_ERR("TexProg id=%d, pos=%d, texcoord=%d", program, pctx->tex_pos, pctx->tex_coords);

	/* load guideline program */
    #if 1
	program = _create_program(lineVert, lineFrag);
	if (!program) {
		LOG_ERR("create program fail\n");
		return -1;
	}

	glValidateProgram(program);

	glGetProgramiv(program, GL_VALIDATE_STATUS, &nShaderStatus);

	if (nShaderStatus != GL_TRUE) {
		LOG_ERR("Error: Failed to validate GLSL program\n");
		glGetProgramInfoLog(program, 1024, &nInfoLogLength, pszInfoLog);
		LOG_ERR("%s", pszInfoLog);
		return -2;
	}

	pctx->line_program = program;
	pctx->line_pos_loc = glGetAttribLocation(pctx->line_program, "position");
	// pctx->line_tex_loc = glGetAttribLocation(pctx->line_program, "texCoords");
	pctx->line_color_loc = glGetUniformLocation(pctx->line_program, "line_color");
	pctx->line_canvas_loc = glGetUniformLocation(pctx->line_program, "canvas");
    LOG_ERR("Line program id=%d, pos=%d, texcoord=%d, color=%d, canvas=%d", 
									program, 
									pctx->line_pos_loc, 
									pctx->line_tex_loc, 
									pctx->line_color_loc,
									pctx->line_canvas_loc);
    #endif

	return 0;
}

static void setup_texture(REND_COORD_T *pcoord, int w, int h,
	GLfloat *verts, GLfloat *coords)
{
	GLfloat x0 = 0.0f;
	GLfloat y0 = 0.0f;
	GLfloat x1 = 0.0f;
	GLfloat y1 = 0.0f;
	GLfloat src_x0 = 0.0f;
	GLfloat src_y0 = 0.0f;
	GLfloat src_x1 = 0.0f;
	GLfloat src_y1 = 0.0f;

	x0 = (GLfloat)pcoord->x *2.0f/(GLfloat)w;
	y0 = (GLfloat)pcoord->y *2.0f/(GLfloat)h;
	x1 = (GLfloat)(pcoord->x + pcoord->w)*2.0f/(GLfloat)w;
	y1 = (GLfloat)(pcoord->y + pcoord->h)*2.0f/(GLfloat)h;

	verts[0] = verts[6] = x0-1.0f;
	verts[1] = verts[4] = y1-1.0f;
	verts[3] = verts[9] = x1-1.0f;
	verts[7] = verts[10] = y0-1.0f;
	verts[2] = verts[5] = verts[8] = verts[11] = 0.0f;

	if((pcoord->src_x == 0) && (pcoord->src_y == 0)
		&& (pcoord->src_w == 0) && (pcoord->src_h == 0)){
		coords[0] = coords[4] = coords[5] = coords[7] = 0.0f;
		coords[1] = coords[2] = coords[3] = coords[6] = 1.0f;
	} else {
		coords[0] = coords[4] = (GLfloat)pcoord->src_x/(GLfloat)pcoord->tex_w;
		coords[5] = coords[7] = (GLfloat)pcoord->src_y/(GLfloat)pcoord->tex_h;
		coords[2] = coords[6] = (GLfloat)(pcoord->src_x + pcoord->src_w)/(GLfloat)pcoord->tex_w;
		coords[1] = coords[3] = (GLfloat)(pcoord->src_y + pcoord->src_h)/(GLfloat)pcoord->tex_h;
	}
}

void *gpu_render_init(int fd, int log_level)
{
	MTK_GPU_REND_T *pctx = NULL;

	local_dynamic_log_level = log_level;
	pctx = (MTK_GPU_REND_T *)malloc(sizeof(MTK_GPU_REND_T));
	if(NULL == pctx) {
		LOG_ERR("handle is NULL \n");
		return NULL;
	}

	memset(pctx, 0, sizeof(*pctx));

	if(_init_egl(pctx) < 0){
		free(pctx);
		return NULL;
	}

	if (_init_gl(pctx)) {
		_fini_egl(pctx);
		free(pctx);
		return NULL;
	}

	return (void *)pctx;
}

void gpu_render_uninit(void *display)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *) display;

	if(NULL == pctx) {
		LOG_ERR("handle is NULL \n");
		return;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	eglMakeCurrent(pctx->egl_display, EGL_NO_SURFACE,
		EGL_NO_SURFACE, EGL_NO_CONTEXT);

	_fini_egl(pctx);

	free(pctx);
}
void * gpu_render_get_tex(void *display, void *buf, int flag)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *)display;
	struct raw_texture  *pbuf = (struct raw_texture *)buf;
	struct gpu_tex_info *tex_info = NULL;
	EGLImageKHR eglimg;
	EGLint attribs[30];
	int atti = 0;
	int idx_1 = 0;
	int idx_2 = 1;
	int idx_3 = 2;

	tex_info = (struct gpu_tex_info *)malloc(sizeof(struct gpu_tex_info));
	if(NULL == tex_info) {
		LOG_ERR("malloc tex info fail \n");
		return NULL;
	}

	if(pbuf->fourcc == DRM_FORMAT_YVU420) {
		idx_2 = 2;
		idx_3 = 1;
	}

	LOG_DBG("gpu_render_get_tex pctx: %p \n", pctx);

	attribs[atti++] = EGL_WIDTH;
	attribs[atti++] = pbuf->width;
	attribs[atti++] = EGL_HEIGHT;
	attribs[atti++] = pbuf->height;
	attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[atti++] = pbuf->fourcc;
	LOG_DBG("EGL_WIDTH: %d\n", pbuf->width);
	LOG_DBG("EGL_HEIGHT: %d\n", pbuf->height);
	LOG_DBG("EGL_LINUX_DRM_FOURCC_EXT: %d\n", pbuf->fourcc);
	/* XXX: Add modifier here when supported */

	if (pbuf->plane_nums > 0) {
		attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
		attribs[atti++] = pbuf->fds[idx_1];
		attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
		attribs[atti++] = pbuf->offset[idx_1];
		attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
		attribs[atti++] = pbuf->pitch[idx_1];
		LOG_DBG("EGL_DMA_BUF_PLANE0_FD_EXT: %d\n", pbuf->fds[idx_1]);
		LOG_DBG("EGL_DMA_BUF_PLANE0_OFFSET_EXT: %d\n", pbuf->offset[idx_1]);
		LOG_DBG("EGL_DMA_BUF_PLANE0_PITCH_EXT: %d\n", pbuf->pitch[idx_1]);
	}

	if (pbuf->plane_nums > 1) {
		attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
		attribs[atti++] = pbuf->fds[idx_2];
		attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
		attribs[atti++] = pbuf->offset[idx_2];
		attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
		attribs[atti++] = pbuf->pitch[idx_2];
		LOG_DBG("EGL_DMA_BUF_PLANE1_FD_EXT: %d\n", pbuf->fds[idx_2]);
		LOG_DBG("EGL_DMA_BUF_PLANE1_OFFSET_EXT: %d\n", pbuf->offset[idx_2]);
		LOG_DBG("EGL_DMA_BUF_PLANE1_PITCH_EXT: %d\n", pbuf->pitch[idx_2]);
	}

	if (pbuf->plane_nums > 2) {
		attribs[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
		attribs[atti++] = pbuf->fds[idx_3];
		attribs[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
		attribs[atti++] = pbuf->offset[idx_3];
		attribs[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
		attribs[atti++] = pbuf->pitch[idx_3];
		LOG_DBG("EGL_DMA_BUF_PLANE2_FD_EXT: %d\n", pbuf->fds[idx_3]);
		LOG_DBG("EGL_DMA_BUF_PLANE2_OFFSET_EXT: %d\n", pbuf->offset[idx_3]);
		LOG_DBG("EGL_DMA_BUF_PLANE2_PITCH_EXT: %d\n", pbuf->pitch[idx_3]);
	}

	if (0 == format_check_yuv(pbuf->fourcc)) {
		attribs[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
		attribs[atti++] = EGL_YUV_NARROW_RANGE_EXT;
		attribs[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
		attribs[atti++] = EGL_ITU_REC601_EXT;
		LOG_DBG("EGL_SAMPLE_RANGE_HINT_EXT: %d \n", EGL_YUV_NARROW_RANGE_EXT);
		LOG_DBG("EGL_YUV_COLOR_SPACE_HINT_EXT: %d\n", EGL_ITU_REC601_EXT);
	}
	attribs[atti++] = EGL_NONE;

	LOG_DBG("start to pfeglCreateImageKHR \n");
	eglimg = pfeglCreateImageKHR(
				pctx->egl_display,
				EGL_NO_CONTEXT,
				EGL_LINUX_DMA_BUF_EXT,
				NULL,
				attribs);

	if (eglimg == EGL_NO_IMAGE_KHR) {
		LOG_ERR("failed to create egl image: %s\n", get_egl_error());
		free(tex_info);
		return NULL;
	}
	if(flag == 0){
		glGenTextures(1, &tex_info->texid);
	}

	if(flag == 1){
		glGenFramebuffers(1, &tex_info->glfb);
		glGenRenderbuffers(1, &tex_info->glrb);

		glBindRenderbuffer(GL_RENDERBUFFER, tex_info->glrb);
		glBindFramebuffer(GL_FRAMEBUFFER, tex_info->glfb);

		pfglEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, (EGLImageKHR)eglimg);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_RENDERBUFFER, tex_info->glrb);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG_ERR("failed to create framebuffer: %s\n", get_gl_error());
		}
	}

	tex_info->eglimg = eglimg;
	tex_info->flag = flag;

	return (void *)tex_info;
}

void gpu_render_free_tex(void *display, void *tex)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *)display;
	struct gpu_tex_info *tex_info = (struct gpu_tex_info *)tex;

	if(tex_info == NULL)
		return;

	if(tex_info->flag == 0){
		glDeleteTextures(1, &tex_info->texid);
	}

	if(tex_info->flag == 1){
		glDeleteFramebuffers(1, &tex_info->glfb);
		glDeleteRenderbuffers(1, &tex_info->glrb);
	}

	pfeglDestroyImageKHR(pctx->egl_display, (EGLImageKHR)tex_info->eglimg);

	free(tex_info);
}

// Minimal 2D vector class - only for gpu_render_draw_line
class Vec2 {
public:
    float x;
    float y;

public:
    constexpr Vec2():x(0.0f), y(0.0f){}

    constexpr Vec2(float _x, float _y):x(_x), y(_y){}

    constexpr Vec2 sub(const Vec2& rhs) const {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    constexpr Vec2 add(const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    constexpr Vec2 mul(float m) const {
        return Vec2(x * m, y * m);
    }

	Vec2 normalize() const {
        const float length = sqrtf(x * x + y * y);
        // normalize vector
        return Vec2(x / length, y / length);
    }

    void normalize_() {
        const float length = sqrtf(x * x + y * y);
        // normalize vector
        x /= length;
        y /= length;
    }

	constexpr Vec2 perpendicular() const {
		return Vec2(y, -x);
	}
};

class Color4 {
public:
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;

public:
	constexpr Color4():r(0.3f), g(0.5f), b(0.9f), a(1.0f){}
	constexpr Color4(GLfloat r, GLfloat g, GLfloat b, GLfloat a): r(r), g(g), b(b), a(a){}
	constexpr Color4(GLfloat r, GLfloat g, GLfloat b): r(r), g(g), b(b), a(1.0f){}
	constexpr Color4(GLfloat grey): r(grey), g(grey), b(grey), a(1.0f){}

	static constexpr Color4 gold_alpha() {
		return Color4(0.99f, 0.6f, 0.2f, 0.5f);
	}

	static constexpr Color4 gold() {
		return Color4(0.99f, 0.6f, 0.2f);
	}

	static constexpr Color4 red() {
		return Color4(0.99f, 0.14f, 0.26f);
	}

	static constexpr Color4 blue() {
		return Color4(0.10, 0.64f, 0.86f);
	}

	static constexpr Color4 green() {
		return Color4(0.42f, 0.74f, 0.21f);
	}
};

static void gpu_render_draw_line(void *display, const GLfloat* points, const size_t point_count, const float thickness, const Color4& color)
{
	//
	//	Tesellation
	//	Note the current tesellation does not prevent overdraw when
	// 	the arc curve changes rapidly.
	//	It may also generate de-generated primitives when thickness > distances between points
	//
	const GLfloat normal_length = thickness / 2.0f;
	std::vector<Vec2> extruded;
	std::vector<Vec2> uv_coord;
	extruded.reserve(point_count * 2);
	uv_coord.reserve(point_count * 2);
	for(size_t i = 0; i < (point_count - 1); ++i) {
		const Vec2 p1_screen(points[i * 2], points[i * 2 + 1]);
		const Vec2 p2_screen(points[(i+1) * 2], points[(i+1)* 2 + 1]);

		const Vec2 p1 = p1_screen; // .mul(1.f / 50.f).sub(Vec2(1.f, 1.f));
		const Vec2 p2 = p2_screen; // .mul(1.f / 50.f).sub(Vec2(1.f, 1.f));
		
		const Vec2 tangent = p2.sub(p1).normalize();
		Vec2 normal_offset = tangent.perpendicular().mul(normal_length);
		normal_offset.x *= 0.8571f;	// TODO: this is the aspect ratio of the output canvas
		const Vec2 n1 = p1.add(normal_offset);
		const Vec2 n2 = p1.sub(normal_offset);
		extruded.push_back(n1);
		extruded.push_back(n2);
        uv_coord.push_back(Vec2(0.f, i == 0 ? 0.0f : 1.0f));
		uv_coord.push_back(Vec2(normal_length, i == 0 ? 0.0f : 1.0f));

		// special case: last segment 
		if((point_count - 2) == i) {
			const Vec2 n3 = p2.add(normal_offset);
			const Vec2 n4 = p2.sub(normal_offset);
			extruded.push_back(n3);
			extruded.push_back(n4);
			uv_coord.push_back(Vec2(0.f, i == 0 ? 0.0f : 1.0f));
			uv_coord.push_back(Vec2(normal_length, i == 0 ? 0.0f : 1.0f));
		}
	}

	//
	// 	Emit OpenGL command
	//
	const MTK_GPU_REND_T* pctx = reinterpret_cast<MTK_GPU_REND_T*>(display);
    glCullFace(GL_FRONT_AND_BACK);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glUniform4f(pctx->line_color_loc, color.r, color.g, color.b, color.a);
    check_gl_error("glUniform4f - line_color_loc");
	glVertexAttribPointer(pctx->line_pos_loc,            // Array "ID"
						2,            // 2 element (x, y)
						GL_FLOAT,     // float type
						GL_FALSE,     // not normalized
						0,            // stride = 0, tightly packed
						&extruded[0] );  // pointer to attribute array
	
	/*
	glVertexAttribPointer(pctx->line_tex_loc,            // Array "ID"
						2,            // 2 element (u, v)
						GL_FLOAT,     // float type
						GL_FALSE,     // not normalized
						0,            // stride = 0, tightly packed
						&uv_coord[0] );  // pointer to attribute array
	*/
	glEnableVertexAttribArray(pctx->line_pos_loc);
	//glEnableVertexAttribArray(pctx->line_tex_loc);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, extruded.size());  // 2 points forms 1 lines
    // check_gl_error("glDrawArrays - GL_TRIANGLE_STRIP");
	glDisableVertexAttribArray(pctx->line_pos_loc);
	//glDisableVertexAttribArray(pctx->line_tex_loc);
}

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / (2*sizeof(_Array[0])))
#endif

#include <time.h>
unsigned int get_type() {
	struct timespec ts;\
	clock_gettime(CLOCK_REALTIME, &ts);\
	
	return (unsigned int)(ts.tv_sec % 8) + 1;
}

void  gpu_render_2d_overlay(void *display, int w, int h,
		void *in_tex[], REND_COORD_T *coord[], int in_num,  void *out_tex)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *)display;
	GLfloat verts[12];
	GLfloat coords[8];
	struct gpu_tex_info *in_tex_info = NULL;
	struct gpu_tex_info *out_tex_info = (struct gpu_tex_info *)out_tex;
	int i;

	if (0 == in_num) {
		LOG_WARN("Need set input buffer!");
		return;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, out_tex_info->glrb);
	glBindFramebuffer(GL_FRAMEBUFFER, out_tex_info->glfb);

	glViewport(0, 0,(GLint)w, (GLint)h);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    glUseProgram(pctx->tex_program);
    //check_gl_error("glUseProgram - Texprog");

	glActiveTexture(GL_TEXTURE0);

	for (i = 0; i < in_num; i++) {
		in_tex_info = (struct gpu_tex_info *)in_tex[i];

		/* bind texture */
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, in_tex_info->texid);
		check_gl_error("glBindTexture");

		pfglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)in_tex_info->eglimg);
		check_gl_error("pfglEGLImageTargetTexture2DOES");

		setup_texture(coord[i], w, h, verts, coords);

		LOG_VERBOSE(1000, "image verts: \n");
		LOG_VERBOSE(1000, "%f %f %f \n", verts[0], verts[1], verts[2]);
		LOG_VERBOSE(1000, "%f %f %f \n", verts[3], verts[4], verts[5]);
		LOG_VERBOSE(1000, "%f %f %f \n", verts[6], verts[7], verts[8]);
		LOG_VERBOSE(1000, "%f %f %f \n", verts[9], verts[10], verts[11]);

		/* draw vertexs */
		glVertexAttribPointer(pctx->tex_pos, 3, GL_FLOAT, GL_FALSE, 0, verts);
		check_gl_error("glVertexAttribPointer");

		LOG_VERBOSE(1000, "image coords: \n");
		LOG_VERBOSE(1000, "%f %f \n", coords[0], coords[1]);
		LOG_VERBOSE(1000, "%f %f \n", coords[2], coords[3]);
		LOG_VERBOSE(1000, "%f %f \n", coords[4], coords[5]);
		LOG_VERBOSE(1000, "%f %f \n", coords[6], coords[7]);

		glVertexAttribPointer(pctx->tex_coords, 2, GL_FLOAT, GL_FALSE, 0, coords);
		check_gl_error("glVertexAttribPointer");

		glEnableVertexAttribArray(pctx->tex_pos);
		glEnableVertexAttribArray(pctx->tex_coords);
		check_gl_error("glEnableVertexAttribArray");

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		check_gl_error("glDrawArrays");

		glDisableVertexAttribArray(pctx->tex_pos);
		glDisableVertexAttribArray(pctx->tex_coords);
	}

	//
	// draw line
	//
	for (i = 0; i < in_num; i++) {
		const REND_COORD_T& rect = *coord[i];

		// detect if "wide" view
		// const bool drawline = true;
		const int drawline = true; //(rect.w == 1120) && (rect.h == 960);

		static const unsigned int TOP_VIEW = 1;
		static const unsigned int FRONT_WIDE_VIEW = 2;
		static const unsigned int REAR_WIDE_VIEW = 3;
		static const unsigned int FRONT_TOP_VIEW = 4;
		static const unsigned int REAR_TOP_VIEW = 5;
		static const unsigned int FRONT_SIDE_VIEW = 6;
		static const unsigned int REAR_SIDE_VIEW = 7;
		static const unsigned int REAR_ASSISTANT_VIEW = 8;
		
		if(drawline) {
			//LOG_ERR("draw x=%d, y=%d, w=%d, h=%d", rect.x, rect.y, rect.w, rect.h);
			glUseProgram(pctx->line_program);
			check_gl_error("glUseProgram - line_program");

			// on-display output rectangle
			glViewport(rect.x, rect.y, rect.w, rect.h);

			// The canvas coordinate system of the "line points"
			// are:
			//	(0, 0) -> left-top
			//	(100, 100) --> right-bottom
			const GLfloat left = 0.f;
			const GLfloat right = 100.f;
			const GLfloat top = 0.f;
			const GLfloat bottom = 100.f;
			glUniform4f(pctx->line_canvas_loc, left, right, top, bottom);
			check_gl_error("glUniform4f - line_canvas_loc");

			unsigned int tex_type = get_type();

			switch(rect.tex_type){
			case TOP_VIEW: // Top View
				{
					// Front Left wheel
					const GLfloat wheel_FL[] = {
						36.8f,  34.1f,
						36.8f,  63.7f,
					};

					// Front Right wheel
					const GLfloat wheel_FR[] = {
						70.7f - 5.56f, 23.4f + 10.27f,
						75.0f - 5.56f, 32.1f + 10.27f,
						78.8f - 5.56f, 43.0f + 10.27f,
						80.8f - 5.56f, 52.2f + 10.27f,
						81.5f - 5.56f, 60.0f + 10.27f,
						// 81.9f - 5.56f, 67.2f + 10.27f,
					};

					// Rear Left wheel
					const GLfloat wheel_RL[] = {
						29.3f + 5.56f, 79.2f - 6.85f,
						28.0f + 5.56f, 84.8f - 6.85f,
						25.6f + 5.56f, 91.6f - 6.85f,
						21.5f + 5.56f, 99.5f - 6.85f,
					};

					// Rear Right wheel
					const GLfloat wheel_RR[] = {
						70.0f - 5.56f, 79.2f - 6.85f,
						68.7f - 5.56f, 87.0f - 6.85f,
						66.6f - 5.56f, 92.7f - 6.85f,
						63.9f - 5.56f, 99.0f - 6.85f,
					};

					// gpu_render_draw_line(display, wheel_FL, _countof(wheel_FL), 0.65f, Color4::gold());
					gpu_render_draw_line(display, wheel_FR, _countof(wheel_FR), 0.65f, Color4::gold());
					gpu_render_draw_line(display, wheel_RL, _countof(wheel_RL), 0.65f, Color4::gold());
					gpu_render_draw_line(display, wheel_RR, _countof(wheel_RR), 0.65f, Color4::gold());
				}
				break;
			case FRONT_WIDE_VIEW:
				break;
			case REAR_WIDE_VIEW:
				{
					const GLfloat gold_left[] = {
						10.f, 79.f,
						13.f, 64.f,
						16.f, 51.f,
						19.f, 39.f,
					};

					const GLfloat gold_top[] = {
						19.f, 39.f,
						27.f, 36.f,
						39.f, 33.f,
						52.f, 30.f,
					};

					const GLfloat gold_middle[] = {
						13.f, 64.f,
						28.f, 60.f,
						51.f, 56.f,
						74.f, 53.f,
					};

					const GLfloat gold_right[] = {
						52.f, 30.f,
						63.f, 38.f,
						74.f, 53.f,
						77.f, 58.f,
						81.f, 66.f,
					};

					const GLfloat red_left[] = {
						9.f, 84.f,
						10.f, 79.f,
					};

					const GLfloat red_right[] = {
						81.f, 66.f,
						85.f, 75.f,
						88.f, 85.f,
					};

					const GLfloat red_top[] = {
						10.f, 79.f,
						27.f, 75.f,
						44.f, 73.f,
						63.f, 69.f,
						81.f, 66.f,
					};

					const GLfloat blue_left[] = {
						11.f, 84.f,
						12.f, 77.f,
						15.f, 66.f,
						19.f, 55.f,
						25.f, 45.f,
						28.f, 38.f,
						31.f, 33.f,
					};

					const GLfloat blue_right[] = {
						89.f, 85.f,
						86.f, 72.f,
						80.f, 56.f,
						73.f, 43.f,
						67.f, 33.f,
					};

					const GLfloat blue_top[] = {
						31.f, 33.f,
						50.f, 32.5f,
						67.f, 33.f,
					};
					
					gpu_render_draw_line(display, gold_left, _countof(gold_left), 0.65f, Color4::gold());
					gpu_render_draw_line(display, gold_right, _countof(gold_right), 0.65f, Color4::gold());
					gpu_render_draw_line(display, gold_top, _countof(gold_top), 0.65f, Color4::gold());
					gpu_render_draw_line(display, gold_middle, _countof(gold_middle), 0.65f, Color4::gold());
					
					gpu_render_draw_line(display, red_left, _countof(red_left), 0.65f, Color4::red());
					gpu_render_draw_line(display, red_right, _countof(red_right), 0.65f, Color4::red());
					gpu_render_draw_line(display, red_top, _countof(red_top), 0.65f, Color4::red());

					gpu_render_draw_line(display, blue_left, _countof(blue_left), 0.65f, Color4::blue());
					gpu_render_draw_line(display, blue_right, _countof(blue_right), 0.65f, Color4::blue());
					gpu_render_draw_line(display, blue_top, _countof(blue_top), 0.65f, Color4::blue());
				}
				break;
			case FRONT_TOP_VIEW:
				{
					const GLfloat left[] = {
						39.0f - 2.94f, 56.f,
						40.3f - 2.94f, 45.5f,
						43.9f - 2.94f, 34.4f,
						48.6f - 2.94f, 24.4f,
						53.9f - 2.94f, 17.2f,
					};

					const GLfloat right[] = {
						61.4f + 2.94f, 56.8f,
						62.3f + 2.94f, 48.9f,
						64.6f + 2.94f, 42.4f,
						67.0f + 2.94f, 35.7f,
						69.1f + 2.94f, 30.9f,
						72.9f + 2.94f, 25.8f,
					};
					gpu_render_draw_line(display, left, _countof(left), 2.55f, Color4::gold_alpha());
					gpu_render_draw_line(display, right, _countof(right), 2.55f, Color4::gold_alpha());
				}
				break;
			case REAR_TOP_VIEW:
				{
					const GLfloat left_1[] = {
						25.5f + 4.441f, 92.5f - 10.273f,
						25.5f + 4.441f, 60.8f - 10.273f,
						25.5f + 4.441f, 53.7f - 10.273f,
					};

					const GLfloat left_2[] = {
						25.45f + 4.441f, 60.8f - 10.273f,
						30.0f + 4.441f, 60.8f - 10.273f,
					};

					const GLfloat left_3[] = {
						25.25f + 4.441f, 92.5f - 10.273f,
						30.0f + 4.441f, 92.5f - 10.273f,
					};

					const GLfloat right_1[] = {
						72.5f - 4.441f + 2.5f, 92.5f - 10.273f,
						72.5f - 4.441f + 2.5f, 60.8f - 10.273f,
						72.5f - 4.441f + 2.5f, 53.7f - 10.273f,
					};

					const GLfloat right_2[] = {
						72.75f - 4.441f + 2.5f, 60.8f - 10.273f,
						68.0f - 4.441f + 2.5f, 60.8f - 10.273f,
					};

					const GLfloat right_3[] = {
						72.75f - 4.441f + 2.5f, 92.5f - 10.273f,
						68.0f - 4.441f + 2.5f, 92.5f - 10.273f,
					};

					const GLfloat dynamic_trajectory[] = {
						49.0f, 57.9f - 10.273f,
						51.7f, 68.1f - 10.273f,
						55.5f, 77.7f - 10.273f,
						60.8f, 86.8f - 10.273f,
						65.3f, 92.7f - 10.273f,
					};

					gpu_render_draw_line(display, left_1, _countof(left_1), 0.65f, Color4::blue());
					gpu_render_draw_line(display, left_2, _countof(left_2), 0.65f, Color4::blue());
					gpu_render_draw_line(display, left_3, _countof(left_3), 0.65f, Color4::blue());
					
					gpu_render_draw_line(display, right_1, _countof(right_1), 0.65f, Color4::blue());
					gpu_render_draw_line(display, right_2, _countof(right_2), 0.65f, Color4::blue());
					gpu_render_draw_line(display, right_3, _countof(right_3), 0.65f, Color4::blue());

					gpu_render_draw_line(display, dynamic_trajectory, _countof(dynamic_trajectory), 1.2f, Color4::gold());
				}
				break;
			case FRONT_SIDE_VIEW: // Front Side View 
			case REAR_SIDE_VIEW: // Rear Side View
				break;
			case REAR_ASSISTANT_VIEW: // Rear assistant View
				{
					const GLfloat guideline_L1[] = {
						0.f, 74.8f,
						14.6f, 61.0f,
					};

					const GLfloat guideline_L2[] = {
						5.0f, 70.0f,
						17.0f, 70.0f,
					};

					const GLfloat guideline_L3[] = {
						23.0f, 53.8f,
						31.2f, 45.9f,
					};

					const GLfloat guideline_L4[] = {
						27.0f, 50.0f,
						33.0f, 50.0f,
					};

					const GLfloat guideline_L5[] = {
						40.3f, 37.7f,
						43.5f, 34.9f,
					};

					const GLfloat guideline_L6[] = {
						42.25f, 36.0f,
						45.05f, 36.0f,
					};

					gpu_render_draw_line(display, guideline_L1, _countof(guideline_L1), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_L2, _countof(guideline_L2), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_L3, _countof(guideline_L3), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_L4, _countof(guideline_L4), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_L5, _countof(guideline_L5), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_L6, _countof(guideline_L6), 0.65f, Color4::gold());

					const GLfloat guideline_R1[] = {
						100.f, 74.8f,
						85.4f, 61.0f,
					};

					const GLfloat guideline_R2[] = {
						83.0f, 70.0f,
						95.0f, 70.0f,
					};

					const GLfloat guideline_R3[] = {
						77.0f, 53.8f,
						68.8f, 45.0f,
					};

					const GLfloat guideline_R4[] = {
						73.25f, 50.0f,
						67.25f, 50.0f,
					};

					const GLfloat guideline_R5[] = {
						59.7f, 37.7f,
						57.2f, 34.9f,
					};

					const GLfloat guideline_R6[] = {
						54.75f, 36.0f,
						57.95f, 36.0f,
					};

					gpu_render_draw_line(display, guideline_R1, _countof(guideline_R1), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_R2, _countof(guideline_R2), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_R3, _countof(guideline_R3), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_R4, _countof(guideline_R4), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_R5, _countof(guideline_R5), 0.65f, Color4::gold());
					gpu_render_draw_line(display, guideline_R6, _countof(guideline_R6), 0.65f, Color4::gold());
				}
				break;
			default:
				// draw nothing
				break;
			}

		}
	}

	glFinish();
}

void gpu_render_lock(void *display)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *)display;

	if (!eglMakeCurrent(pctx->egl_display,
		EGL_NO_SURFACE /* No default draw surface */,
		EGL_NO_SURFACE /* No default draw read */,
		pctx->egl_ctx)) {
		LOG_ERR("failed to make the OpenGL ES Context current: %s\n", get_egl_error());
	}
}

void gpu_render_unlock(void *display)
{
	MTK_GPU_REND_T *pctx = (MTK_GPU_REND_T *)display;
	eglMakeCurrent(pctx->egl_display, EGL_NO_SURFACE,
		EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

