#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>

#include <gl\gl.h>				// Header File For The OpenGL32 Library
#include <gl\glu.h>				// Header File For The GLu32 Library
#include "../Include/glaux.h"	// Header File For The Glaux Library

#include "../Useful/Useful.h"

#include "OpenGLObjects.h"
#include "OpenGLColors.h"
#include "OpenGLUseful.h"
#include "OpenGLTextures.h"

#define MAX_TEXTURES 200
static GLuint _texture_list[MAX_TEXTURES];
static char _texture_filename_cache[MAX_TEXTURES][1024];
static GLuint _textures = 0;
static GLuint _next_texture = 0;

/*************************************************************************************/


Texture::Texture( char *filename, double u_length, double v_length ) {

  strcpy( bitmap_filename, filename );
  this->u_length = u_length;
  this->v_length = v_length;
  Define();

}

/*************************************************************************************/

void Texture::Use( void ) {

	if ( TextureImage )									// If Texture Exists
	{
		// Use this texture for future drawing.
		glBindTexture( GL_TEXTURE_2D, _texture_list[ texture_index ] );
  }
}

/*************************************************************************************/

AUX_RGBImageRec *Texture::LoadBMP(char *filename)				// Loads A Bitmap Image
{
	FILE *file=NULL;								      // File Handle

  if ( !filename ) return NULL;		      // Make Sure A Filename Was Given
	file=fopen( filename, "r" );		      // Check To See If The File Exists
	// Does The File Exist?
	if ( !file )		{
		MessageBox( NULL, filename, "File not found.", MB_OK );
	}
	else {
		fclose(file);									      // Close The Handle
		return auxDIBImageLoad( filename );	// Load The Bitmap And Return A Pointer
	}
	return NULL;										      // If Load Failed Return NULL
}

/***********************************************************************************/

int Texture::LoadBitmap( char *filename )							// Load Bitmaps And Convert To Textures
{
	int Status=FALSE;									                  // Status Indicator
	TextureImage = 0;                                   // Set The Pointer To NULL
  // Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if ( TextureImage = LoadBMP( filename ) )
	{
		Status = TRUE;									                  // Set The Status To TRUE
		glBindTexture(GL_TEXTURE_2D, _texture_list[ texture_index ] );
		glTexImage2D( GL_TEXTURE_2D, 0, 3, 
      TextureImage->sizeX, TextureImage->sizeY, 
      0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    if ( TextureImage->data )							          // If Texture Image Exists
		{
			free( TextureImage->data );					          // Free The Texture Image Memory
		}
		free( TextureImage );								            // Free The Image Structure
	}
  else MessageBox( NULL, "Error loading bitmap file.", "Texture Error!", MB_OK );

	return Status;										                  // Return The Status
}
 
/*************************************************************************************/
void Texture::Define( void ) {

  unsigned int i;

  if ( ! _textures ) {
		glGenTextures( MAX_TEXTURES, &_texture_list[0] );					// Create a set of textures.
    _textures = MAX_TEXTURES;
    _next_texture = 0;
  }
  // See if we are already have loaded this bitmap.
  for ( i = 0; i < _next_texture; i++ ) {
    if ( !strcmp( _texture_filename_cache[i], bitmap_filename ) ) break;
  }
  texture_index = i;
  // If this is a bitmap file that we haven't seen before, load it.
  if ( i == _next_texture )  {
    LoadBitmap( bitmap_filename );
    strcpy( _texture_filename_cache[_next_texture], bitmap_filename );
    _next_texture++;
  }

}
