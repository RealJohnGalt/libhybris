/*
 * NULL-safe shared implementations of the glMapBuffer family.
 *
 * This file is compiled into BOTH libGLESv2_libhybris.so and
 * libGLESv1_CM_libhybris.so so that eglGetProcAddress (and any direct
 * dlsym / g_module lookup) returns a non-NULL, working pointer for these
 * procedures no matter which GLES library is consulted.  Compositors such
 * as gnome-shell/mutter probe these through eglGetProcAddress and call the
 * resulting pointer unguarded (e.g. cogl-buffer-impl-gl.c), so a missing
 * procedure previously ended in a NULL call at swap time.
 *
 * The actual mapped-buffer functionality is provided by the underlying
 * Android driver (libGLESv2.so).  If the driver does not export the
 * procedure, we return NULL instead of crashing the caller.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <dlfcn.h>
#include <stdlib.h>

#include <hybris/common/binding.h>

static void *map_family_handle = NULL;

static void *map_family_dlsym(const char *symbol)
{
	if (map_family_handle == NULL)
		map_family_handle = android_dlopen(
			getenv("LIBGLESV2") ? getenv("LIBGLESV2") : "libGLESv2.so",
			RTLD_LAZY);
	if (map_family_handle)
		return android_dlsym(map_family_handle, symbol);
	return NULL;
}

void *glMapBuffer(GLenum target, GLenum access)
{
	static void *(*f)(GLenum, GLenum) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLenum)) map_family_dlsym("glMapBuffer");
	if (f == NULL)
		return NULL;
	return f(target, access);
}

void *glMapBufferOES(GLenum target, GLenum access)
{
	static void *(*f)(GLenum, GLenum) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLenum)) map_family_dlsym("glMapBufferOES");
	if (f == NULL)
		return NULL;
	return f(target, access);
}

void *glMapBufferOESEXT(GLenum target, GLenum access)
{
	static void *(*f)(GLenum, GLenum) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLenum)) map_family_dlsym("glMapBufferOESEXT");
	if (f == NULL)
		return NULL;
	return f(target, access);
}

void *glMapBufferEXT(GLenum target, GLenum access)
{
	static void *(*f)(GLenum, GLenum) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLenum)) map_family_dlsym("glMapBufferEXT");
	if (f == NULL)
		return NULL;
	return f(target, access);
}

void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	static void *(*f)(GLenum, GLintptr, GLsizeiptr, GLbitfield) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLintptr, GLsizeiptr, GLbitfield)) map_family_dlsym("glMapBufferRange");
	if (f == NULL)
		return NULL;
	return f(target, offset, length, access);
}

void *glMapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	static void *(*f)(GLenum, GLintptr, GLsizeiptr, GLbitfield) FP_ATTRIB = NULL;
	if (f == NULL)
		f = (void *(*)(GLenum, GLintptr, GLsizeiptr, GLbitfield)) map_family_dlsym("glMapBufferRangeEXT");
	if (f == NULL)
		return NULL;
	return f(target, offset, length, access);
}
