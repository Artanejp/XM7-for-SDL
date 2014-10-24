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
#include "agar_glutil.h"
#include "agar_logger.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"

#include "agar_glcl.h"

#define LOGSIZE 1024*1024

extern "C"{
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;
}

extern PFNGLBINDBUFFERPROC glBindBuffer;
extern float fBrightR;
extern float fBrightG;
extern float fBrightB;



GLCLDraw::GLCLDraw()
{
   cl_int ret;
   pixelBuffer = NULL;
   bModeOld = -1;
   TransferBuffer = malloc((640 * 200 * 18 + 2) * sizeof(Uint8));
   
}

GLCLDraw::~GLCLDraw()
{
   cl_int ret;
   int i;
   if(nkernels > 0) {
     for(i = 0; i < nkernels; i++) if(kernels_array[i] != NULL) ret = clReleaseKernel(kernels_array[i]);
   }
   if(program != NULL) ret |= clReleaseProgram(program);
   if(command_queue != NULL) ret |= clReleaseCommandQueue(command_queue);
   if(context != NULL) ret |= clReleaseContext(context);
   if(properties != NULL) free(properties);
   if(inbuf != NULL) ret |= clReleaseMemObject(inbuf);
   if(outbuf != NULL) ret |= clReleaseMemObject(outbuf);
   if(palette != NULL) ret |= clReleaseMemObject(palette);
   //   if(internalpal != NULL) ret |= clReleaseMemObject(internalpal);
   if(table != NULL) ret |= clReleaseMemObject(table);
   if(pixelBuffer != NULL) free(pixelBuffer);
   if(TransferBuffer != NULL) free(TransferBuffer);
}

static void cl_notify_log(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
   Uint8 dump[128];
   char dumpStr[1024];
   int i;
   
   dumpStr[0] = '\0';
   XM7_DebugLog(XM7_LOG_WARN, "CL Notify: %s", errinfo);
}

int GLCLDraw::GetGLEnabled(void)
{
  if(bCLEnableKhrGLShare != FALSE) return -1;
  return 0;
}

Uint32 *GLCLDraw::GetPixelBuffer(void)
{
#if 0
   int w = w2;
   int h = h2;
   cl_int ret;
   if(w == 320) {
      if(h != 200) return NULL;
   } else if(w == 640) {
      if((h != 200) && (h != 400)) return NULL;
   } else {
      return NULL;
   }
	
   ret = clEnqueueReadBuffer(command_queue, outbuf, CL_TRUE, 0,
				w * h * sizeof(Uint32), (void *)pixelBuffer
				, 1, &event_exec, &event_release);
   if(ret < CL_SUCCESS) return NULL;
   clFinish(command_queue);
   return pixelBuffer;
#else   
    Uint32 *p;
    int ret = 0;
    p = (Uint32 *) clEnqueueMapBuffer(command_queue, outbuf, CL_TRUE, CL_MAP_READ,
				       0, (size_t)(640 * 400 * sizeof(Uint32)),
				       1, &event_exec, &event_release, &ret);
    if(ret < 0) return NULL;
    clFlush(command_queue);
    return p;
#endif
}

int GLCLDraw::ReleasePixelBuffer(Uint32 *p)
{
#if 0
   return 0;
#else
  int ret;
  if(p == NULL) return 0;
//  clFlush(command_queue);
  ret |= clEnqueueUnmapMemObject(command_queue, outbuf,
				 p, 1, &event_release, NULL);
  clFinish(command_queue);
  return ret;
#endif
}


cl_int GLCLDraw::InitContext(void)
{
   cl_int ret;
   size_t len;
   char extension_data[1024];
   size_t extension_len;
   int i;
   
   properties = malloc(16 * sizeof(intptr_t));
   ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
   if(ret != CL_SUCCESS) return ret;
   
   ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 8, device_id,
                            &ret_num_devices);
   
   if(ret != CL_SUCCESS) return ret;

   for(i = 0; i < ret_num_devices; i++ ){
     clGetDeviceInfo(device_id[i], CL_DEVICE_EXTENSIONS,
		   1024, extension_data, &extension_len);
     XM7_DebugLog(XM7_LOG_DEBUG, "CL Extension features(%d):%s", i, extension_data);
     if(strcasestr(extension_data, "cl_khr_gl_sharing") != NULL) {
       bCLEnableKhrGLShare = -1;
     } else {
       bCLEnableKhrGLShare = 0;
     }
   }

   if(bCLEnableKhrGLShare != 0) { // This is only under X11. Must fix.
     XM7_DebugLog(XM7_LOG_DEBUG, "GL Interoperability enabled.");
     properties[0] = CL_GL_CONTEXT_KHR;
     properties[1] = (cl_context_properties)glXGetCurrentContext();
     XM7_DebugLog(XM7_LOG_DEBUG, "GL Context = %08x", glXGetCurrentContext());
     properties[2] = CL_GLX_DISPLAY_KHR;
     properties[3] = (cl_context_properties)glXGetCurrentDisplay();
     XM7_DebugLog(XM7_LOG_DEBUG, "GL Display = %08x", glXGetCurrentDisplay());
     properties[4] = CL_CONTEXT_PLATFORM;
     properties[5] = (cl_context_properties)platform_id;
     properties[6] = 0;
   } else {
     XM7_DebugLog(XM7_LOG_DEBUG, "GL Interoperability disabled.");
     properties[0] = CL_CONTEXT_PLATFORM;
     properties[1] = (cl_context_properties)platform_id;
     properties[2] = 0;
   }
//   if(device_id == NULL) return -1;
   
   context = clCreateContext(properties, 1, device_id, cl_notify_log, NULL, &ret);
   if(ret != CL_SUCCESS) return ret;
       
   command_queue = clCreateCommandQueue(context, device_id[0],
                                         CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);
//   command_queue = clCreateCommandQueue(context, device_id[0],
//                                         0, &ret);
   XM7_DebugLog(XM7_LOG_DEBUG, "CL: Command queue created.");
   return ret;
}


cl_int GLCLDraw::BuildFromSource(const char *p)
{
    cl_int ret;
    size_t codeSize;
    char *logBuf;
   
    codeSize = strlen(p);
    program = clCreateProgramWithSource(context, 1, (const char **)&p,
                                        (const size_t *)&codeSize, &ret);
    XM7_DebugLog(XM7_LOG_INFO, "CL: Build Result=%d", ret);
    if(ret < CL_SUCCESS) {
      return ret;
    }
    // Compile from source
    ret = clBuildProgram(program, 1, device_id, NULL, NULL, NULL);
    XM7_DebugLog(XM7_LOG_INFO, "Compile Result=%d", ret);
    if(ret != CL_SUCCESS) {  // Printout error log.
      cl_int r;
       logBuf = (char *)malloc(LOGSIZE * sizeof(char));
       memset(logBuf, 0x00, LOGSIZE * sizeof(char));
       
       r = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 
				   LOGSIZE - 1, (void *)logBuf, NULL);
       if(logBuf != NULL) XM7_DebugLog(XM7_LOG_INFO, "Build Log:\n%s", logBuf);
       free(logBuf);
       return ret;
    }
    ret = clCreateKernelsInProgram(program, sizeof(kernels_array) / sizeof(cl_kernel),
				   kernels_array, &nkernels);
    if(ret < CL_SUCCESS) {
      XM7_DebugLog(XM7_LOG_INFO, "Unable to build CL kernel. Status=%d", ret);
    } else {
      char funcname[128];
      int i;
      size_t size;
      XM7_DebugLog(XM7_LOG_INFO, "Built %d CL kernel(s).", nkernels);
      for(i = 0; i < nkernels; i++) {
	funcname[0] = '\0';
	if(clGetKernelInfo(kernels_array[i], CL_KERNEL_FUNCTION_NAME,
			   sizeof(funcname) / sizeof(char) - 1, 
			   funcname, size) == CL_SUCCESS){
	  if((strncmp(funcname, "getvram8", strlen("getvram8")) == 0) && (kernel_8colors == NULL)) kernel_8colors = &kernels_array[i];
	  if((strncmp(funcname, "getvram4096", strlen("getvram4096")) == 0) && (kernel_4096colors == NULL)) kernel_4096colors = &kernels_array[i];
	  if((strncmp(funcname, "getvram256k", strlen("getvram256k")) == 0) && (kernel_256kcolors == NULL)) kernel_256kcolors = &kernels_array[i];
	  if((strncmp(funcname, "CreateTable", strlen("CreateTable")) == 0) && (kernel_table == NULL)) kernel_table = &kernels_array[i];
	}
      }
    }
   return ret;
}
cl_int GLCLDraw::copysub(int hh)
{
  Uint8 *p;
  Uint8 *pp;
  Uint8 *q;
  cl_int ret;
  int line;
  int voffset = 0x4000;
  if(hh > 200) voffset = 0x8000;

  p = clEnqueueMapBuffer(command_queue, inbuf, CL_TRUE, CL_MAP_WRITE,
			 0, voffset * 3,
			 0, NULL, &event_uploadvram[0], &ret);
  if(p == NULL) return ret;
  if(ret < 0) return ret;
  pp = p;
  q = TransferBuffer;
  for(line = 0;line < hh; line++) {
    if(bDrawLine[line]) {
      memcpy(pp, q, 80);
      memcpy(&pp[voffset], &q[voffset], 80);
      memcpy(&pp[voffset * 2], &q[voffset * 2], 80);
      bDrawLine[line] = FALSE;
    }
    pp += 80;
    q += 80;
  }
  ret |= clEnqueueUnmapMemObject(command_queue, inbuf,
				 p, 0, NULL, &event_uploadvram[1]);
  return ret;
}


cl_int GLCLDraw::copy256k(void)
{
   cl_int ret = 0;
   Uint8 *p;
   Uint8 *pp;
   Uint8 *q;
   Uint8 *pr, *pg, *pb;
   int line;
   int vramoffset = 0x2000;
   int band = 40;
   int i, j;
   
   p = clEnqueueMapBuffer(command_queue, inbuf, CL_TRUE, CL_MAP_WRITE,
			  0, vramoffset * 18,
			  0, NULL, &event_uploadvram[0], &ret);
  if(ret < 0) return ret;
  if(p != NULL) {
    pp = p;
    q = TransferBuffer;
    pb = &q[0];
    pr = &q[vramoffset * 6];
    pg = &q[vramoffset * 12];
    for(line = 0; line < 200; line++) {
      if(bDrawLine[line]) {
	// B
	memcpy(&pp[0], &pb[0], band);  
	memcpy(&pp[vramoffset], &pb[vramoffset], band);  
	memcpy(&pp[vramoffset * 2], &pb[vramoffset * 2], band);  
	memcpy(&pp[vramoffset * 3], &pb[vramoffset * 3], band);
	memcpy(&pp[vramoffset * 4], &pb[vramoffset * 4], band);  
	memcpy(&pp[vramoffset * 5], &pb[vramoffset * 5], band);
	// R
	memcpy(&pp[vramoffset * 6], &pr[0], band);  
	memcpy(&pp[vramoffset * 7], &pr[vramoffset], band);  
	memcpy(&pp[vramoffset * 8], &pr[vramoffset * 2], band);  
	memcpy(&pp[vramoffset * 9], &pr[vramoffset * 3], band);  
	memcpy(&pp[vramoffset * 10], &pr[vramoffset * 4], band);  
	memcpy(&pp[vramoffset * 11], &pr[vramoffset * 5], band);  
	 
	 // G
	memcpy(&pp[vramoffset * 12], &pg[0], band);  
	memcpy(&pp[vramoffset * 13], &pg[vramoffset], band);  
	memcpy(&pp[vramoffset * 14], &pg[vramoffset * 2], band);  
	memcpy(&pp[vramoffset * 15], &pg[vramoffset * 3], band);  
	memcpy(&pp[vramoffset * 16], &pg[vramoffset * 4], band);  
	memcpy(&pp[vramoffset * 17], &pg[vramoffset * 5], band);
	bDrawLine[line] = FALSE;
      }
      pg += 40;
      pr += 40;
      pb += 40;
      pp += 40;
    }
     ret |= clEnqueueUnmapMemObject(command_queue, inbuf,
				    p, 0, NULL, &event_uploadvram[1]);
  }
  return ret;
}


cl_int GLCLDraw::copy8(void)
{
   WORD wdtop, wdbtm;
   cl_int ret = 0;
   
   if(bPaletFlag) { // 描画モードでVRAM変更
      Palet640();
      bPaletFlag = FALSE;
      nDrawTop = 0;
      nDrawBottom = 400;
      nDrawLeft = 0;
      nDrawRight = 640;
//      SetDrawFlag(TRUE);
   }
   if (bClearFlag) {
//      AllClear();
   }
   return copysub(200);
}

cl_int GLCLDraw::copy8_400l(void)
{
   WORD wdtop, wdbtm;
   cl_int ret = 0;
   
   if (bClearFlag) {
      //AllClear();
   }
   if(bPaletFlag) { // 描画モードでVRAM変更
      Palet640();
      bPaletFlag = FALSE;
      nDrawTop = 0;
      nDrawBottom = 400;
      nDrawLeft = 0;
      nDrawRight = 640;
//      SetDrawFlag(TRUE);
   }
   ret = copysub(400);
   nDrawTop = 0;
   nDrawBottom = 400;
   nDrawLeft = 0;
   nDrawRight = 640;
   return ret;
}

cl_int GLCLDraw::copy4096(void)
{
   cl_int ret = 0;
   Uint8 *p;
   Uint8 *pp;
   Uint8 *q;
   int line;
   int voffset = 0x2000;
   int i, j;
   
   if(bPaletFlag) { // 描画モードでVRAM変更
      Palet320();
      bPaletFlag = FALSE;
      nDrawTop = 0;
      nDrawBottom = 200;
      nDrawLeft = 0;
      nDrawRight = 320;
//      SetDrawFlag(TRUE);
   }
//   if (bClearFlag) {
//      AllClear();
//   }
   
  p = clEnqueueMapBuffer(command_queue, inbuf, CL_TRUE, CL_MAP_WRITE,
			 0, voffset * 12,
			 0, NULL, &event_uploadvram[0], &ret);
  if(ret < 0) return ret;
  if(p == NULL) return ret;
  pp = p;
  q = TransferBuffer;
  for(line = 0;line < 200; line++) {
    if(bDrawLine[line]) {
      j = 0;
      for(i = 0; i < 12; i++) {
	memcpy(&pp[j], &q[j], 40);
	j += voffset;
      }
      bDrawLine[line] = FALSE;
    }
    pp += 40;
    q += 40;
  }
  clFlush(command_queue);
  ret |= clEnqueueUnmapMemObject(command_queue, inbuf,
				 p, 0, NULL, &event_uploadvram[1]);
  return ret;
}

Uint8 *GLCLDraw::GetBufPtr(void)
{
  return TransferBuffer;
}



cl_int GLCLDraw::GetVram(int bmode)
{
   cl_int ret = 0;
   cl_int r;
   cl_kernel *kernel = NULL;
   int w = 0;
   int h = 0;
   Uint8 *pr,*pg,*pb;
   size_t gws[] = {nCLGlobalWorkThreads}; // Parallel jobs.
   size_t lws[] = {1}; // local jobs.
   size_t *goff = NULL;
   int mpage = multi_page;
   int dummy = 0;
   int vpage;
   int crtflag = crt_flag;
   cl_float4 bright;

   bright.s[0] = fBrightR; // R
   bright.s[1] = fBrightG; // G
   bright.s[2] = fBrightB; // B
   bright.s[3] = 1.0; // A
 
   
   bModeOld = bmode;
   if(inbuf == NULL) return -1;
   if(outbuf == NULL) return -1;
   if(TransferBuffer == NULL) return -1;

   //   clFinish(command_queue);
   kernel = NULL;
   switch(bmode) {
    case SCR_400LINE:
    case SCR_200LINE:
      w = 640;
      h = 200;
      if(bmode == SCR_400LINE) h = 400;
      vpage = (~(multi_page >> 4)) & 0x07;
      //gws[0] = h;
      if(kernel_8colors != NULL) kernel = kernel_8colors;
      if(kernel != NULL) {
	 if(bmode == SCR_400LINE) {
	    ret |= copy8_400l();
	 } else {
	    ret |= copy8();
	 }
	 ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
				  8 * sizeof(Uint8), (void *)&ttl_palet[0]
                              , 0, NULL, &event_uploadvram[2]);
      //      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
      //                        8 * sizeof(Uint32), (void *)&rgbTTLGDI[0]
      //                        , 0, NULL, &event_uploadvram[2]);
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem), (void *)&inbuf);
	 ret |= clSetKernelArg(*kernel, 1, sizeof(int),    (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(int), (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem), (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem), (void *)&palette);
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_mem), (void *)&table);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(int), (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(int), (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(int), (void *)&vpage);
	 ret |= clSetKernelArg(*kernel, 9, sizeof(cl_float4), (void *)&bright);
      }
      break;
    case SCR_262144:// Windowはなし
      w = 320;
      h = 200;
      //gws[0] = h;

      //      if(kernel == NULL) kernel = clCreateKernel(program, "getvram256k", &ret);
      if(kernel_256kcolors != NULL) kernel = kernel_256kcolors;
      if(kernel != NULL) {
	 ret |= copy256k();
      /*
       * Below transfer is dummy.
       */
	 ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
				     8 * sizeof(Uint8), (void *)&ttl_palet[0]
				     , 0, NULL, &event_uploadvram[2]);
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem),  (void *)&inbuf);
	 ret |= clSetKernelArg(*kernel, 1, sizeof(cl_int),  (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(cl_int),  (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem),  (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem),  (void *)&table);
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_uint), (void *)&mpage);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(cl_int),  (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(cl_int),  (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(cl_float4), (void *)&bright);
      }
      break;
    case SCR_4096:
      w = 320;
      h = 200;
      //gws[0] = h;
      //if(kernel == NULL) kernel = clCreateKernel(program, "getvram4096", &ret);
      if(kernel_4096colors != NULL) kernel = kernel_4096colors;
      if(kernel != NULL) {
	 ret |= copy4096();
	   {
	      cl_event event_local[2];
	      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 0,
					  4096 * sizeof(Uint8), (void *)&apalet_b[0]
					  , 0, NULL, &event_local[0]);
	      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 4096,
					  4096 * sizeof(Uint8), (void *)&apalet_r[0]
					  , 0, NULL, &event_local[1]);
	      ret |= clEnqueueWriteBuffer(command_queue, palette, CL_TRUE, 4096 * 2,
					  4096 * sizeof(Uint8), (void *)&apalet_g[0]
					  , 2, event_local, &event_uploadvram[2]);
	   }
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem),  (void *)&inbuf);
	 ret |= clSetKernelArg(*kernel, 1, sizeof(cl_int),  (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(cl_int),  (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem),  (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem),  (void *)&palette);
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_mem),  (void *)&table);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(cl_int),  (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(cl_int),  (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(cl_uint), (void *)&mpage);
	 ret |= clSetKernelArg(*kernel, 9, sizeof(cl_float4), (void *)&bright);
      //clFinish(command_queue);
	 clFlush(command_queue);
      }
      break;
   }
   w2 = w;
   h2 = h;
   if(bCLEnableKhrGLShare != 0) {
     glFlush();
     ret |= clEnqueueAcquireGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  3, event_uploadvram, &event_copytotexture);
     if(kernel != NULL) {
       if(bCLSparse) {
	 ret = clEnqueueNDRangeKernel(command_queue, *kernel, 1, 
				      goff, gws, lws, 
				      1, &event_copytotexture,  &event_exec);
       } else {
	 ret = clEnqueueTask(command_queue,
			     *kernel, 1, &event_copytotexture, &event_exec);
       }
     }
      
     ret |= clEnqueueReleaseGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  1, &event_exec, &event_release);
     clFinish(command_queue);
     glFinish();
   } else {
     if(kernel != NULL) {
       if(bCLSparse) {
	 ret = clEnqueueNDRangeKernel(command_queue, *kernel, 1, 
				      goff, gws, lws, 
				      3, event_uploadvram,  &event_exec);
       } else {
	 ret = clEnqueueTask(command_queue,
			     *kernel, 3, event_uploadvram, &event_exec);
       }
     }
   }
   return ret;
}


cl_int GLCLDraw::SetupTable(void)
{
   cl_int r = CL_INVALID_KERNEL;
   cl_uint pages;
   cl_event tbl_ev;
   pages = 12;

   if(kernel_table != NULL) {
     r = 0;
      r |= clSetKernelArg(*kernel_table, 0, sizeof(cl_mem),     (void *)&table);
      r |= clSetKernelArg(*kernel_table, 1, sizeof(cl_uint),    (void *)&pages);
      r |= clEnqueueTask(command_queue,
			     *kernel_table, 0, NULL, NULL);
      clFinish(command_queue);
    }
   return r;
}


cl_int GLCLDraw::SetupBuffer(GLuint *texid)
{
   cl_int ret = 0;
   cl_int r = 0;
   unsigned int size = 640 * 400 * sizeof(cl_uchar4);
   
   inbuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, // Reduce HOST-CPU usage.
                            (size_t)(0x8000 * 6 * sizeof(Uint8)), NULL, &r);
   //inbuf = clCreateBuffer(context, CL_MEM_READ_ONLY | 0, // Reduce HOST-CPU usage.
   //                       (size_t)(0x8000 * 6 * sizeof(Uint8)), NULL, &r);
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: inbuf : %d", r);

   palette = clCreateBuffer(context, CL_MEM_READ_ONLY | 0,
 		  (size_t)(4096 * 3 * sizeof(Uint8)), NULL, &r);
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: palette : %d", r);


   table = clCreateBuffer(context, CL_MEM_READ_WRITE | 0,
 		  (size_t)(0x100 * 8 * 12 * sizeof(cl_uint)), NULL, &r);
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: table : %d", r);

   // Texture直接からPBO使用に変更 20121102
   if((bCLEnableKhrGLShare != 0) && (bGL_PIXEL_UNPACK_BUFFER_BINDING != FALSE)){
       glGenBuffers(1, &pbo);
       if(pbo < 0) {
	 bCLEnableKhrGLShare = FALSE;
	 goto _fallback;
       }

       glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
       glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
       //    XM7_DebugLog(XM7_LOG_DEBUG, "CL: PBO=%08x Size=%d context=%08x", pbo, size, context);
       outbuf = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY | 0, 
				     pbo, &r);
       if(r != GL_NO_ERROR) {
	 glDeleteBuffers(1, &pbo);
//	 pbo = 0;
	 bCLEnableKhrGLShare = FALSE;
	 goto _fallback;
       }
       glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
       ret |= r;
       XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: outbuf (GLCL Interop): %d", r);
       return ret;
   }
 _fallback:
#if 0
   pixelBuffer = (Uint32 *)malloc(640 * 400 * sizeof(Uint32));
   outbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR ,
			   (size_t)(640 * 400 * sizeof(Uint32)), pixelBuffer, &r);
#else
   //pixelBuffer = (Uint32 *)malloc(640 * 400 * sizeof(Uint32));
   outbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
			   (size_t)(640 * 400 * sizeof(Uint32)), NULL, &r);
#endif
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: outbuf (CL): %d", r);
   return ret;
}

GLuint GLCLDraw::GetPbo(void)
{
   return pbo;
}
