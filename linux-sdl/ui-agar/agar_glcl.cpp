/*
 * Renderer using OPENCL/GL
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * Nov 01,2012: Initial
 */


#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>


#include "api_draw.h"
#include "api_kbd.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

#include "agar_glcl.h"

#define LOGSIZE 1024*1024

extern "C"{
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;
}


GLCLDraw::GLCLDraw()
{
   cl_int ret;
   
   properties = malloc(16 * sizeof(intptr_t));
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,
                         &ret_num_devices);
   properties[0] = CL_GL_CONTEXT_KHR;
   properties[1] = (cl_context_properties)glXGetCurrentContext();
   properties[2] = CL_GLX_DISPLAY_KHR;
   properties[3] = (cl_context_properties)glXGetCurrentDisplay();
   properties[4] = CL_CONTEXT_PLATFORM;
   properties[5] = (cl_context_properties)platform_id;
   properties[6] = 0;

   context = clCreateContext(properties, 1, &device_id, NULL, NULL, &ret);
       
   command_queue = clCreateCommandQueue(context, device_id,
                                         CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);
}

GLCLDraw::~GLCLDraw()
{
   cl_int ret;
    if(kernel != NULL) ret = clReleaseKernel(kernel);
    if(program != NULL) ret |= clReleaseProgram(program);
    if(command_queue != NULL) ret |= clReleaseCommandQueue(command_queue);
    if(context != NULL) ret |= clReleaseContext(context);
    if(properties != NULL) free(properties);
    if(inbuf != NULL) ret |= clReleaseMemObject(inbuf);
    if(outbuf != NULL) ret |= clReleaseMemObject(outbuf);
    if(palette != NULL) ret |= clReleaseMemObject(palette);
}

cl_int GLCLDraw::BuildFromSource(const char *p)
{
    cl_int ret;
    size_t codeSize;
    char *logBuf;
   
    codeSize = strlen(p);
    program = clCreateProgramWithSource(context, 1, (const char **)&p,
                                        (const size_t *)&codeSize, &ret);
    printf("Build Result=%d\n", ret);

    // Compile from source
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    printf("Compile Result=%d\n", ret);
    if(ret != CL_SUCCESS) {  // Printout error log.
       logBuf = (char *)malloc(LOGSIZE * sizeof(char));
       memset(logBuf, 0x00, LOGSIZE * sizeof(char));
       
       ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 
				   LOGSIZE - 1, (void *)logBuf, NULL);
       if(logBuf != NULL) printf("Build Log:\n%s\n", logBuf);
       free(logBuf);
       return ret;
    }
   return ret;
}

cl_int GLCLDraw::GetVram(int bmode)
{
   cl_int ret = 0;
   int w;
   int h;
   Uint8 *pr,*pg,*pb;
   Uint32 *pal;

//   printf("STS: %d\n", ret);

   
   if(inbuf == NULL) return;
   if(outbuf == NULL) return;

   switch(bmode) {
    case SCR_400LINE:
      w = 640;
      h = 400;
      pal = &rgbTTLGDI[0];
      SetVram_200l(vram_dptr);
      pg = (Uint8 *)vram_pg;
      pr = (Uint8 *)vram_pr;
      pb = (Uint8 *)vram_pb;
      if((pb == NULL) || (pg == NULL) || (pr == NULL)) return;
      kernel = clCreateKernel(program, "getvram8", &ret);
      ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inbuf);
      ret |= clSetKernelArg(kernel, 1, sizeof(int),    (void *)&w);
      ret |= clSetKernelArg(kernel, 2, sizeof(int), (void *)&h);
      ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&outbuf);
      ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&palette);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0,
                              0x8000 * sizeof(unsigned char), (void *)pb
                              , 0, NULL, &event_uploadvram[0]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x8000,
                              0x8000 * sizeof(unsigned char), (void *)pr
                              , 0, NULL, &event_uploadvram[1]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x10000,
                              0x8000 * sizeof(unsigned char), (void *)pg
                              , 0, NULL, &event_uploadvram[2]);
      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
                              8 * sizeof(Uint32), (void *)(&rgbTTLGDI[0])
                              , 0, NULL, &event_uploadvram[3]);
      break;
    case SCR_200LINE:
      w = 640;
      h = 200;

      SetVram_200l(vram_dptr);
      pg = (Uint8 *)vram_pg;
      pr = (Uint8 *)vram_pr;
      pb = (Uint8 *)vram_pb;
      pal = &rgbTTLGDI[0];
      if((pb == NULL) || (pg == NULL) || (pr == NULL)) return;
      kernel = clCreateKernel(program, "getvram8", &ret);
      ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inbuf);
      ret |= clSetKernelArg(kernel, 1, sizeof(int),    (void *)&w);
      ret |= clSetKernelArg(kernel, 2, sizeof(int), (void *)&h);
      ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&outbuf);
      ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&palette);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0,
                              0x4000 * sizeof(unsigned char), (void *)pb
                              , 0, NULL, &event_uploadvram[0]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x4000,
                              0x4000 * sizeof(unsigned char), (void *)pr
                              , 0, NULL, &event_uploadvram[1]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x8000,
                              0x4000 * sizeof(unsigned char), (void *)pg
                              , 0, NULL, &event_uploadvram[2]);
      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
                              8 * sizeof(Uint32), (void *)(&rgbTTLGDI[0])
                              , 0, NULL, &event_uploadvram[3]);
      break;
    case SCR_262144:
      break;
    case SCR_4096:
      w = 640;
      h = 200;
      kernel = clCreateKernel(program, "getvram8", &ret);
      ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inbuf);
      ret |= clSetKernelArg(kernel, 1, sizeof(int),    (void *)&w);
      ret |= clSetKernelArg(kernel, 2, sizeof(int), (void *)&h);
      ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&outbuf);
      ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&palette);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0,
                              0x8000 * sizeof(unsigned char), (void *)pb
                              , 0, NULL, &event_uploadvram[0]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x8000,
                              0x8000 * sizeof(unsigned char), (void *)pr
                              , 0, NULL, &event_uploadvram[1]);
      ret |= clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0x10000,
                              0x8000 * sizeof(unsigned char), (void *)pg
                              , 0, NULL, &event_uploadvram[2]);
      
      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
                              4096 * sizeof(Uint32), (void *)(&rgbAnalogGDI[0])
                              , 0, NULL, &event_uploadvram[3]);
      break;
   }
   glFinish();
   ret |= clEnqueueAcquireGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  4, event_uploadvram, &event_copytotexture);
  
   ret |= clEnqueueTask (command_queue,
			 kernel, 1, &event_copytotexture, &event_exec);
   
   ret |= clEnqueueReleaseGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  1, &event_exec, &event_release);
   clFinish(command_queue);
   clReleaseKernel(kernel);
   kernel = NULL;
   return ret;
				   
}

cl_int GLCLDraw::SetupBuffer(GLuint texid)
{
   cl_int ret = 0;
   cl_int r;
   outbuf = clCreateFromGLTexture (context, CL_MEM_WRITE_ONLY,
			      GL_TEXTURE_2D, 0, texid, &r);
   ret |= r;
     
   inbuf = clCreateBuffer(context, CL_MEM_READ_WRITE,
 		  (size_t)(320 * 200 * 24 * sizeof(Uint8)), NULL, &r);
   ret |= r;
   
   palette = clCreateBuffer(context, CL_MEM_READ_WRITE,
 		  (size_t)(4096 * sizeof(Uint32)), NULL, &r);
   ret |= r;
   printf("Alloc STS: %d texid = %d\n", ret, texid);
   return ret;
}
