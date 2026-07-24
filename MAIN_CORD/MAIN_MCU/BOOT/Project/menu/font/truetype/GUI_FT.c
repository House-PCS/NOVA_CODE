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
File        : GUI_TrueType.c
Purpose     : Implementation of routines needed for TrueType access
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_FT_Private.h"
#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static FT_CONTEXT    _FTContext;
static FT_CACHE_SIZE _FTCacheSize;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbFaceRequester
*
*  Function description
*    The face requester callback function is used by the cache manager
*    to translate a given FTC_FaceID into a new valid FT_Face object, on demand.
*
*  Parameters
*    face_id      - Id to be translated into a real face object.
*    library      - Handle to a FreeType library object (same as _FTContext.library).
*    pRequestData - Pointer given by FTC_Manager_New() that is passed to the requester each time it is called.
*    pFace        - Pointer to FT_Face object.
*
*  Return value
*    0 on success, != 0 on error.
*/
static FT_Error _cbFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer * pRequestData, FT_Face * pFace) {
  GUI_TTF_DATA * pTTF;
  GUI_USE_PARA(pRequestData);
  pTTF = (GUI_TTF_DATA *)face_id;
  return FT_New_Memory_Face(library, (const FT_Byte *)pTTF->pData, pTTF->NumBytes, 0, pFace);
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_FT__Init
*
*  Function description
*    Initializes only the TrueType-engine.
*/
int GUI_FT__Init(FT_Library ** ppLibrary) {
  //
  // Initialize library (on demand)
  //
  if (!_FTContext.library) {
    if (FT_Init_FreeType(&_FTContext.library)) {
      return 1; // Could not initialize FreeType
    }
  }
  if (ppLibrary) {
    *ppLibrary = &_FTContext.library;
  }
  return 0;
}

/*********************************************************************
*
*       GUI_FT__InitCache
*
*  Function description
*    Initializes TrueType-engine and caches.
*/
int GUI_FT__InitCache(FT_CONTEXT ** ppContext) {
  //
  // Initialize library (on demand)
  //
  if (GUI_FT__Init(NULL)) {
    return 1;
  }
  //
  // Initialize cache manager
  //
  if (!_FTContext.cache_manager) {
    if (FTC_Manager_New(_FTContext.library,
                        _FTCacheSize.MaxFaces,
                        _FTCacheSize.MaxSizes,
                        _FTCacheSize.MaxBytes,
                        (FTC_Face_Requester)_cbFaceRequester,
                        NULL, &_FTContext.cache_manager)) {
      return 1; // Could not initialize cache manager
    }
    //
    // Initialize charmap cache
    //
    if (FTC_CMapCache_New(_FTContext.cache_manager, &_FTContext.cmap_cache)) {
      return 1; // Could not initialize charmap cache
    }
    //
    // Initialize glyph image cache
    //
    if (FTC_ImageCache_New(_FTContext.cache_manager, &_FTContext.image_cache)) {
      return 1; // Could not initialize glyph image cache
    }
    //
    // Initialize bitmap cache
    //
    if (FTC_SBitCache_New(_FTContext.cache_manager, &_FTContext.sbits_cache)) {
      return 1; // Could not initialize small bitmaps cache
    }
  }
  if (ppContext) {
    *ppContext = &_FTContext;
  }
  return 0;
}

/*********************************************************************
*
*       GUI_FT__DrawGlyph
*
*  Function description
*    Initializes TrueType-engine and caches.
*/
void GUI_FT__DrawGlyph(FT_Int Left, FT_Int Top, FT_Byte Width, FT_Byte Height, FT_Short Pitch, FT_Byte * pBuffer) {
  const GUI_FONT * pFont;

  pFont = GUI_GetFont();
  LCD_DrawBitmap(GUI_pContext->DispPosX + Left,
                 GUI_pContext->DispPosY - Top + pFont->Baseline,
                 Width,
                 Height,
                 1, 1, 1,
                 Pitch,
                 pBuffer,
                 GUI_pContext->LCD_pBkColorIndex);
}

/*********************************************************************
*
*       GUI_FT__GetName
*
*  Function description
*    Returns a family or style name of a font face.
*/
int GUI_FT__GetName(FT_Face Face, char * pBuffer, int NumBytes, int Index) {
  int          Len;
  const char * pName;

  switch (Index) {
  case FAMILY_NAME:
    pName = Face->family_name;
    break;
  case STYLE_NAME:
    pName = Face->style_name;
    break;
  default:
    return 1;
  }
  Len = (int)strlen(pName);
  if (Len >= NumBytes) {
    Len = NumBytes - 1;
  }
  strncpy(pBuffer, pName, Len);
  *(pBuffer + Len) = 0;
  return 0;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_FT_SetCacheSize()
*
*  Function description
*    Sets the size parameters used to create the cache on the first
*    call of GUI_TTF_CreateFont() or GUI_BDF_CreateFont().
*
*  Parameters
*    MaxFaces: Maximum number of font faces the cache should be able to handle simultaneously.
*              0 selects default value.
*    MaxSizes: Maximum number of size objects the cache should be able to handle simultaneously. 
*              0 selects default value.
*    MaxBytes: Maximum number of bytes used for the bitmap cache. 0 selects default value.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*
*  Additional information
*    If for example 3 font faces should be used, each with 2 sizes, the cache should be
*    able to manage 6 size objects.
*    The default values used by the TTF engine are: 2 faces, 4 size objects and 200K of
*    bitmap data cache.
*/
void GUI_FT_SetCacheSize(unsigned MaxFaces, unsigned MaxSizes, U32 MaxBytes) {
  _FTCacheSize.MaxFaces = MaxFaces;
  _FTCacheSize.MaxSizes = MaxSizes;
  _FTCacheSize.MaxBytes = MaxBytes;
}

/*********************************************************************
*
*       GUI_FT_DestroyCache()
*
*  Function description
*    This function frees all memory allocated by the FreeType cache system
*    and destroys the cache.
*
*  Additional information
*    The next time GUI_TTF_CreateFont() or GUI_BDF_CreateFont() is used,
*    emWin automatically creates and initializes a new cache.
*/
void GUI_FT_DestroyCache(void) {
  if (_FTContext.cache_manager) {
    FTC_Manager_Done(_FTContext.cache_manager);
    _FTContext.cache_manager = 0;
  }
}

/*********************************************************************
*
*       GUI_FT_Done()
*
*  Function description
*    This function frees all memory allocated by the FreeType engine and its internal
*    cache system.
*
*  Additional information
*    The next time GUI_TTF_CreateFont() or GUI_BDF_CreateFont() is used,
*    emWin automatically initializes the TTF engine and creates and
*    initializes a new cache.
*/
void GUI_FT_Done(void) {
  GUI_FT_DestroyCache();
  if (_FTContext.library) {
    FT_Done_Library(_FTContext.library);
    _FTContext.library = 0;
  }
  GUI_SetFont(GUI_GetDefaultFont());
}

/*************************** End of file ****************************/