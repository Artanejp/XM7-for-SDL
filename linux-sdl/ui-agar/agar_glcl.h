/*
 * Header for CL with GL
 * (C) 2012 K.Ohta
 * Notes:
 *   Not CL model: VramDraw->[Virtual Vram]->AGEventDraw2->drawUpdateTexture->[GL Texture]->Drawing
 *       CL Model: AGEvenDraw2 -> GLCL_DrawEventSub -> [GL/CL Texture] ->Drawing
 * History:
 *   Nov 01,2012: Initial.
 */
#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#ifdef _USE_OPENCL
 #include <CL/cl.h>
 #include <CL/cl_gl.h>
 #include <CL/cl_gl_ext.h>


extern GLuint uVramTextureID;

class GLCLDraw {
 public:
   GLCLDraw();
   ~GLCLDraw();
   cl_int GetVram(int bmode);
   cl_int BuildFromSource(const char *p);
   cl_int SetupBuffer(GLuint texid);
   cl_int SetupTable(void);
   cl_int InitContext(void);
   GLuint GLCLDraw::GetPbo(void);
   
   cl_platform_id platform_id = NULL;
   cl_uint ret_num_platforms;
   cl_device_id device_id = NULL;
   cl_uint ret_num_devices;
   cl_context context = NULL;
   cl_command_queue command_queue = NULL;

   /* Program Object */
   const char *source = NULL;
   cl_program program = NULL;
   cl_kernel kernel = NULL;
 private:
   cl_event event_exec;
   cl_event event_uploadvram[4];
   cl_event event_copytotexture;
   cl_event event_release;
   cl_mem inbuf = NULL;
   cl_mem outbuf = NULL;
   cl_mem palette = NULL;
   cl_mem table = NULL;
   cl_context_properties *properties = NULL;	
   GLuint pbo = 0;
   cl_int window_copy8(void);
   cl_int window_copy8_400l(void);
   cl_int window_copy4096(void);
   cl_int copysub(int xbegin, int ybegin, int drawwidth, int drawheight, int w, int h, int mpage);
   cl_int copy4096sub(int xbegin, int ybegin, int drawwidth, int drawheight, int w, int h, int mpage);
   cl_int copy256ksub(int xbegin, int ybegin, int drawwidth, int drawheight, int w, int h, int mpage);
};

#endif /* _USE_OPENCL */