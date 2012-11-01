/*
 * Renderer using OPENCL/GL
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * Nov 01,2012: Initial
 */


#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "agar_glcl.h"

#define LOGSIZE 1024*1024

void GLCLDraw::GLCLDraw()
{
   cl_int ret;
   
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,
                         &ret_num_devices);
    context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
    command_queue = clCreateCommandQueue(context, device_id,
                                         CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);
}

void GLCLDraw::~GLCLDraw()
{
    if(kernel != NULL) ret = clReleaseKernel(kernel);
    if(program != NULL) ret = clReleaseProgram(program);
    if(command_queue != NULL) ret = clReleaseCommandQueue(command_queue);
    if(context != NULL) ret = clReleaseContext(context);
    if(inbuf != NULL) ret |= clReleaseMemObject(inbuf);
    if(outbuf != NULL) ret |= clReleaseMemObject(outbuf);
    if(palette != NULL) ret |= clReleaseMemObject(outbuf);
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
   cl_int ret;
   int w;
   int h;
   Uint8 *pr,pg,pb;
   Uint32 *pal;

   pg = (Uint8 *)vram_pg;
   pr = (Uint8 *)vram_pr;
   pb = (Uint8 *)vram_pb;

   if(inbuf == NULL) return;
   if(outbuf == NULL) return;
   switch(mode) {
    case SCR_400LINE:
      w = 640;
      h = 400;
      pal = &rgbTTLGDI[0];
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
                              8 * sizeof(Uint32), (void *)(&rgbTTLGDI[0]);
                              , 0, NULL, &event_uploadvram[3]);
      break;
    case SCR_200LINE:
      w = 640;
      h = 200;
      pal = &rgbTTLGDI[0];
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
                              8 * sizeof(Uint32), (void *)(&rgbTTLGDI[0]);
                              , 0, NULL, &event_uploadvram[3]);
      break;
    case SCR_262144:
      break;
    case SCR_4096:
      w = 640;
      h = 200;
      kernel = clCreateKernel(program, "getvram4096", &ret);
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
                              4096 * sizeof(Uint32), (void *)(&rgbAnalogGDI[0]);
                              , 0, NULL, &event_uploadvram[3]);
      break;
   }
   ret |= clEnqueueAcquireGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  4, &event_uploadvram, &event_copytotexture);
   ret |= clEnqueueTask (command_queue,
			 kernel, 1, &event_copytotexture, &event_exec);
   
   ret |= clEnqueueReleaseGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  1, &event_exec, &event_release);
   return ret;
				   
}

cl_int GLCLDraw::SetupBuffer(GLuint texid)
{
   cl_int ret = 0;
   cl_int r;
   outbuf = clCreateFromGLTexture (context, CL_MEM_READ_WRITE,
			      GL_TEXTURE_2D, 0, texid, &r);
   ret |= r;
     
   inbuf = clCreateBuffer(context, CL_MEM_READ_WRITE,
 		  (size_t)(320 * 200 * 24 * sizeof(Uint8)), NULL, &r);
   ret |= r;
   
   palette = clCreateBuffer(context, CL_MEM_READ_WRITE,
 		  (size_t)(4096 * sizeof(Uint32)), NULL, &r);
   ret |= r;
   return ret;
}
