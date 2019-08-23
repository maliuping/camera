#ifndef MTK_GPU_RENDER_H
#define MTK_GPU_RENDER_H

typedef struct REND_COORD{
	unsigned int tex_w;
	unsigned int tex_h;
	unsigned int src_x;
	unsigned int src_y;
	unsigned int src_w;
	unsigned int src_h;

	unsigned int x;   //coordinate x
	unsigned int y;   //coordinate y
	unsigned int w;   //width
	unsigned int h;   //height

	/*0: reserve 1:Top View 2: Front Wide View 3: Rear Wide View
	4: Front Top View 5: Rear Top View 6: Front Side View 7: Rear Side View
	8: Rear assistant View*/
	unsigned int tex_type;

	/* TBD for view line postion*/
}REND_COORD_T;

#ifdef __cplusplus
extern "C"{
#endif

void *gpu_render_init(int fd, int log_level);

void gpu_render_uninit(void *display);

void * gpu_render_get_tex(void *display, void *buf, int flag);

void gpu_render_free_tex(void *display, void *tex);

void gpu_render_lock(void *display);

void gpu_render_unlock(void *display);

void  gpu_render_2d_overlay(void *display, int w, int h,
		void *in_tex[], REND_COORD_T *coord[], int in_num,  void *out_tex);

#ifdef __cplusplus
}
#endif

#endif  /* MTK_GPU_RENDER_H */
