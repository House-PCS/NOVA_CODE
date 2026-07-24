/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2020  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V6.20 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_FT_Private.h
Purpose     : Implementation of private routines needed for FreeType
              access
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUI_FT_PRIVATE_H
#define GUI_FT_PRIVATE_H

#include "GUI.h"

#include "ft2build.h"
#include "ftadvanc.h"

#include FT_FREETYPE_H

#include FT_MODULE_H

#include FT_CACHE_H
#include FT_CACHE_MANAGER_H

#if defined(__cplusplus)
  extern "C" {             // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define FAMILY_NAME 0
#define STYLE_NAME  1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  FT_Library        library;           // the FreeType library
  FTC_Manager       cache_manager;     // the cache manager
  FTC_ImageCache    image_cache;       // the glyph image cache
  FTC_SBitCache     sbits_cache;       // the glyph small bitmaps cache
  FTC_CMapCache     cmap_cache;        // the charmap cache
} FT_CONTEXT;

typedef struct {
  unsigned MaxFaces; // If not set the default will be 2
  unsigned MaxSizes; // If not set the default will be 4
  U32      MaxBytes; // If not set the default will be 200000L
} FT_CACHE_SIZE;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
void GUI_FT__DrawGlyph (FT_Int Left, FT_Int Top, FT_Byte Width, FT_Byte Height, FT_Short Pitch, FT_Byte * pBuffer);
int  GUI_FT__GetName   (FT_Face Face, char * pBuffer, int NumBytes, int Index);
int  GUI_FT__InitCache (FT_CONTEXT ** ppContext);
int  GUI_FT__Init      (FT_Library ** ppLibrary);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_FT_PRIVATE_H

/*************************** End of file ****************************/