/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
#ifdef S52_USE_GLSC2
    #include <GLSC2/glsc2.h>  // SD 2016AUG07 - test
#else
    #include <GLES2/gl2.h>
#endif  // S52_USE_GLSC2
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "texture-atlas.h"

#include <glib.h>


// ------------------------------------------------------ texture_atlas_new ---
texture_atlas_t *
texture_atlas_new( const size_t width,
                   const size_t height,
                   const size_t depth )
{
    assert( (depth == 1) || (depth == 3) || (depth == 4) );

    //texture_atlas_t *self = (texture_atlas_t *) malloc( sizeof(texture_atlas_t) );
    texture_atlas_t *self = (texture_atlas_t *) g_malloc( sizeof(texture_atlas_t) );

    if( self == NULL)
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    self->nodes  = vector_new( sizeof(ivec3) );
    self->used   = 0;
    self->width  = width;
    self->height = height;
    self->depth  = depth;
    self->id     = 0;

    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    ivec3 node = {{1,1,width-2}};

    vector_push_back( self->nodes, &node );

    //self->data = (unsigned char *) calloc( width*height*depth, sizeof(unsigned char) );
    self->data = (unsigned char *) g_malloc0( width*height*depth * sizeof(unsigned char) );

    if( self->data == NULL)
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }

    return self;
}


// --------------------------------------------------- texture_atlas_delete ---
void
texture_atlas_delete( texture_atlas_t *self )
{
    assert( self );
    vector_delete( self->nodes );
    if( self->data )
    {
        //free( self->data );
        g_free( self->data );
    }
    // SD 2014MAR22 - from diff in git
#if !defined S52_USE_GLSC2
    // no delete in GLSC2
    //if( !self->id )
    if(0 != self->id )
    {
        glDeleteTextures( 1, &self->id );
    }
#endif

    //free( self );
    g_free( self );
}


// ----------------------------------------------- texture_atlas_set_region ---
void
texture_atlas_set_region( texture_atlas_t * self,
                          const size_t x,
                          const size_t y,
                          const size_t width,
                          const size_t height,
                          const unsigned char * data,
                          const size_t stride )
{
    assert( self );
    assert( x > 0);
    assert( y > 0);
    assert( x < (self->width-1));
    assert( (x + width) <= (self->width-1));
    assert( y < (self->height-1));
    assert( (y + height) <= (self->height-1));

    size_t i;
    size_t depth = self->depth;
    size_t charsize = sizeof(char);
    for( i=0; i<height; ++i )
    {
        memcpy( self->data+((y+i)*self->width + x ) * charsize * depth, 
                data + (i*stride) * charsize, width * charsize * depth  );
    }
}


// ------------------------------------------------------ texture_atlas_fit ---
int
texture_atlas_fit( texture_atlas_t * self,
                   const size_t index,
                   const size_t width,
                   const size_t height )
{
    assert( self );

    ivec3 *node = (ivec3 *) (vector_get( self->nodes, index ));
    int x = node->XYZ.x, y, width_left = width;
	size_t i = index;

	if ( (x + width) > (self->width-1) )
    {
		return -1;
    }
	y = node->XYZ.y;
	while( width_left > 0 )
	{
        node = (ivec3 *) (vector_get( self->nodes, i ));
        if( node->XYZ.y > y )
        {
            y = node->XYZ.y;
        }
		if( (y + height) > (self->height-1) )
        {
			return -1;
        }
		width_left -= node->XYZ.z;
		++i;
	}
	return y;
}


// ---------------------------------------------------- texture_atlas_merge ---
void
texture_atlas_merge( texture_atlas_t * self )
{
    assert( self );

    ivec3 *node, *next;
    size_t i;

	for( i=0; i< self->nodes->size-1; ++i )
    {
        node = (ivec3 *) (vector_get( self->nodes, i ));
        next = (ivec3 *) (vector_get( self->nodes, i+1 ));
		if( node->XYZ.y == next->XYZ.y )
		{
			node->XYZ.z += next->XYZ.z;
            vector_erase( self->nodes, i+1 );
			--i;
		}
    }
}


// ----------------------------------------------- texture_atlas_get_region ---
ivec4
texture_atlas_get_region( texture_atlas_t * self,
                          const size_t width,
                          const size_t height )
{
    assert( self );

    //int y, best_height, best_width, best_index;
    int y, best_width, best_index;
    size_t best_height;
    ivec3 *node, *prev;
    ivec4 region = {{0,0,width,height}};
    size_t i;

    best_height = INT_MAX;
    best_index  = -1;
    best_width  = INT_MAX;
    for( i=0; i<self->nodes->size; ++i )
    {
        y = texture_atlas_fit( self, i, width, height );
        if( y >= 0 )
        {
            node = (ivec3 *) vector_get( self->nodes, i );
            if( ( (y + height) < best_height ) || ( ((y + height) == best_height) && (node->XYZ.z < best_width)) )
            {
                best_height   = y + height;
                best_index    = i;
                best_width    = node->XYZ.z;
                region.XYZW.x = node->XYZ.x;
                region.XYZW.y = y;
            }
        }
    }
   
    if( best_index == -1 )
    {
        region.XYZW.x = -1;
        region.XYZW.y = -1;
        region.XYZW.z = 0;
        region.XYZW.w = 0;
        return region;
    }

    //node = (ivec3 *) malloc( sizeof(ivec3) );
    node = (ivec3 *) g_malloc( sizeof(ivec3) );

    if( node == NULL)
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    node->XYZ.x = region.XYZW.x;
    node->XYZ.y = region.XYZW.y + height;
    node->XYZ.z = width;
    vector_insert( self->nodes, best_index, node );

    //free( node );
    g_free( node );

    for(i = best_index+1; i < self->nodes->size; ++i)
    {
        node = (ivec3 *) vector_get( self->nodes, i );
        prev = (ivec3 *) vector_get( self->nodes, i-1 );

        if (node->XYZ.x < (prev->XYZ.x + prev->XYZ.z) )
        {
            int shrink = prev->XYZ.x + prev->XYZ.z - node->XYZ.x;
            node->XYZ.x += shrink;
            node->XYZ.z -= shrink;
            if (node->XYZ.z <= 0)
            {
                vector_erase( self->nodes, i );
                --i;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    texture_atlas_merge( self );
    self->used += width * height;
    return region;
}


// ---------------------------------------------------- texture_atlas_clear ---
void
texture_atlas_clear( texture_atlas_t * self )
{
    assert( self );
    assert( self->data );

    vector_clear( self->nodes );
    self->used = 0;
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    ivec3 node = {{1,1,self->width-2}};
    vector_push_back( self->nodes, &node );
    memset( self->data, 0, self->width*self->height*self->depth );
}


// --------------------------------------------------- texture_atlas_upload ---
void
texture_atlas_upload( texture_atlas_t * self )
{
    assert( self );
    assert( self->data );

    if( !self->id )
    {
        glGenTextures( 1, &self->id );
    }
    glBindTexture( GL_TEXTURE_2D, self->id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    if( self->depth == 4 )
    {
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, self->data );
#else
#ifdef S52_USE_GLSC2
        glTexStorage2D (GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self->width, self->height, GL_RGBA, GL_UNSIGNED_BYTE, self->data);
#else  // S52_USE_GLSC2
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, self->data );
#endif  // S52_USE_GLSC2
#endif  // GL_UNSIGNED_INT_8_8_8_8_REV
    }
    else if( self->depth == 3 )
    {
#ifdef S52_USE_GLSC2
        glTexStorage2D (GL_TEXTURE_2D, 0, GL_RGB, self->width, self->height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self->width, self->height, GL_RGB, GL_UNSIGNED_BYTE, self->data);
#else  // S52_USE_GLSC2
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, self->width, self->height, 0, GL_RGB, GL_UNSIGNED_BYTE, self->data );
#endif  // S52_USE_GLSC2
    }
#if !defined S52_USE_GLSC2
    // SD 2016AUG13 - no GL_ALPHA in GLSC2 so no depth=1
    else
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, self->width, self->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, self->data );
    }
#endif
}
