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
File        : GUI_TTF.c
Purpose     : Implementation of external binary fonts
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_Private.h"
#include "GUI_FT_Private.h"

#ifndef   GUI_TTF_SUPPORT_AA
  #define GUI_TTF_SUPPORT_AA (1)
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define FAMILY_NAME 0
#define STYLE_NAME  1

#define CONTEXT_VALID(r)   if (_pContext == NULL) { return r; }

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static FT_CONTEXT * _pContext;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetCharMapIndex
*/
static int _GetCharMapIndex(FT_Face face, FT_Encoding enc) {
  int        i, r;
  FT_CharMap CMap;

  r = -1;
  for (i = 0; i < face->num_charmaps; i++) {
    CMap = face->charmaps[i];
    if (CMap->encoding == enc) {
      r = FT_Get_Charmap_Index(CMap);
    }
  }
  return r;
}

/*********************************************************************
*
*       _RequestGlyph
*
*  Function description
*    Draws a glyph (if DoRender == 1) and returns its width
*/
static int _RequestGlyph(U16P c, unsigned DoRender, unsigned AntiAlias) {
  const GUI_FONT   * pFont;
  GUI_TTF_CS       * pCS;
  FTC_ImageTypeRec * pImageType;
  FTC_FaceID         face_id;
  FT_Face            face;
  FT_UInt            glyph_index;
  FTC_SBitRec      * pSBit;
  FTC_ScalerRec      scaler;
  FT_Size            size;
  FT_Fixed           advance;
  int                r;
  int                BufferExist;
  int                TextMode;
  int                DrawMode;
  int                cmap_index;

  pFont = GUI_GetFont();
  r     = -1;
  CONTEXT_VALID(r);
  //
  // Get object pointer
  //
  pCS        = (GUI_TTF_CS *)pFont->p.pFontData;
  face_id    = (FTC_FaceID)pCS->pTTF;
  pImageType = (FTC_ImageTypeRec *)pCS->aImageTypeBuffer;
  //
  // Request face object from cache
  //
  if (FTC_Manager_LookupFace(_pContext->cache_manager, face_id, &face)) {
    return r;
  }
  //
  // Request size object from cache
  //
  scaler.face_id = face_id;
  scaler.width   = pImageType->width;
  scaler.height  = pImageType->height;
  scaler.pixel   = 1;
  if (FTC_Manager_LookupSize(_pContext->cache_manager, &scaler, &size)) {
    return r;
  }
  //
  // Request glyph index from cache
  //
  cmap_index  = _GetCharMapIndex(face, FT_ENCODING_UNICODE);
  glyph_index = FTC_CMapCache_Lookup(_pContext->cmap_cache, face_id, cmap_index, c);
  if (glyph_index == 0) {
    if (DoRender) {
      GUI__DrawNonExistingCharacter(c, GUI_GetDrawMode());
      return -1;
    } else {
      return GUI__GetNonExistingCharWidth(c, NULL);
    }
  }
  //
  // Request bitmap from cache
  //
  if (FTC_SBitCache_Lookup(_pContext->sbits_cache, pImageType, glyph_index, &pSBit, NULL)) {
    return r;
  }
  BufferExist = pSBit ? pSBit->buffer ? 1 : 0 : 0;
  if (BufferExist) {
    if (DoRender) {
      //
      // Set draw mode
      //
      TextMode = GUI_GetTextMode();
      DrawMode = GUI_GetDrawMode();
      GUI_SetDrawMode(TextMode);
      //
      // Rendering cache data using the bitmap routine
      //
#if (GUI_TTF_SUPPORT_AA)
      if (AntiAlias) {
#if (GUI_SUPPORT_ROTATION)
        if (GUI_pLCD_APIList) {
          GUI_pLCD_APIList->pfDrawBitmap(
                                GUI_pContext->DispPosX + pSBit->left,                  // XPos
                                GUI_pContext->DispPosY - pSBit->top + pFont->Baseline, // YPos
                                pSBit->width,                                          // XSize
                                pSBit->height,                                         // YSize
                                1, 1, 8,                                              
                                pSBit->pitch,                                          // BytesPerLine
                                (U8 const *)pSBit->buffer,                             // Pointer to pixel data
                                NULL);
        } else {
#else
        {
#endif // GUI_SUPPORT_ROTATION
          GUI_AA__DrawCharAA8(GUI_pContext->DispPosX + pSBit->left,                    // XPos
                              GUI_pContext->DispPosY - pSBit->top + pFont->Baseline,   // YPos
                              pSBit->width,                                            // XSize
                              pSBit->height,                                           // YSize
                              pSBit->pitch,                                            // BytesPerLine
                              (U8 const *)pSBit->buffer);                              // Pointer to pixel data
        }
      } else {
#else
      {
#endif // GUI_TTF_SUPPORT_AA
        GUI_FT__DrawGlyph(pSBit->left,
                          pSBit->top,
                          pSBit->width,
                          pSBit->height,
                          pSBit->pitch,
                          pSBit->buffer);
      }
      //
      // Reset draw mode
      //
      GUI_SetDrawMode(DrawMode);
    }
    r = pSBit->xadvance;
  } else {
    //
    // No bitmap data
    //
    pImageType->flags = FT_LOAD_NO_SCALE;
    FT_Get_Advance(face, glyph_index, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_IGNORE_TRANSFORM, &advance);
    pImageType->flags = (AntiAlias) ? FT_LOAD_DEFAULT : FT_LOAD_MONOCHROME;
    r = FT_MulFix(advance, face->size->metrics.x_scale) >> 6;
  }
  return r;
}

#if 0 // emDoc...
}}
#endif

/*********************************************************************
*
*       _DispChar
*/
static void _DispChar(U16P c) {
  int xDist;
  xDist = _RequestGlyph(c, 1, 0);
  if (xDist >= 0) {
    GUI_pContext->DispPosX += xDist;
  }
}

#if (GUI_TTF_SUPPORT_AA)
/*********************************************************************
*
*       _DispCharAA
*/
static void _DispCharAA(U16P c) {
  int xDist;
  xDist = _RequestGlyph(c, 1, 1);
  if (xDist >= 0) {
    GUI_pContext->DispPosX += xDist;
  }
}

/*********************************************************************
*
*       _ClearLineAA
*
*  Function description
*    If text should be rendered not in transparent mode first the whole line
*    needs to be cleared, because internally the characters always are drawn in
*    transparent mode to be sure, that also compound characters are drawn well.
*/
static void _ClearLineAA(const char * s, int Len) {
  const GUI_FONT * pFont;
  int              xDist;
  int              yDist;
  int              xSize;
  U16              c;

  pFont    = GUI_GetFont();
  xDist    = 0;
  yDist    = pFont->YDist * pFont->YMag;
  c        = 0;
  while (--Len >= 0) {
    c     = GUI_UC__GetCharCodeInc(&s);
    xSize = _RequestGlyph(c, 0, 1);
    if (xSize >= 0) {
      xDist += xSize;
    }
  }
  GUI__ClearTextBackground(xDist, yDist);
}

/*********************************************************************
*
*       _DispLineAA
*
*  Function description
*    Displays a string of anti aliased characters. If current text mode
*    is not transparent, the line is cleared before drawing the text.
*/
static void _DispLineAA(const char * s, int Len) {
  const GUI_FONT * pFont;
  U16              Char;
  int              TextMode;
  int              DrawMode;
    
  if (Len > 0) {
    TextMode = GUI_GetTextMode();
    //
    // Clear if transparency mode has not been selected
    //
    if (!(TextMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))) {
      _ClearLineAA(s, Len);
    }
    //
    // Draw characters
    //
    DrawMode = GUI_GetDrawMode();
    pFont    = GUI_GetFont();
    GUI_SetDrawMode(TextMode);
    while (--Len >= 0) {
      Char = GUI_UC__GetCharCodeInc(&s);
      pFont->pfDispChar(Char);
    }
    GUI_SetDrawMode(DrawMode);
  }
}

/*********************************************************************
*
*       _GetCharDistX_AA
*/
static int _GetCharDistX_AA(U16P c, int * pSizeX) {
  int xDist;
  xDist = _RequestGlyph(c, 0, 1);
  if (pSizeX) {
    *pSizeX = xDist; // Same as xDist here...
  }
  return (xDist >= 0) ? xDist : 0;
}

/*********************************************************************
*
*       _APIListAA
*/
static const tGUI_ENC_APIList _APIListAA = {
  NULL,
  NULL,
  _DispLineAA
};
#endif  // GUI_TTF_SUPPORT_AA

/*********************************************************************
*
*       _ClearLine
*
*  Function description
*    If text should be rendered not in transparent mode first the whole line
*    needs to be cleared, because internally the characters always are drawn in
*    transparent mode to be sure, that also compound characters are drawn well.
*/
static void _ClearLine(const char * s, int Len) {
  const GUI_FONT * pFont;
  int              xDist;
  int              yDist;
  int              xSize;
  U16              c;

  pFont    = GUI_GetFont();
  xDist    = 0;
  yDist    = pFont->YDist * pFont->YMag;
  c        = 0;
  while (--Len >= 0) {
    c     = GUI_UC__GetCharCodeInc(&s);
    xSize = _RequestGlyph(c, 0, 0);
    if (xSize >= 0) {
      xDist += xSize;
    }
  }
  GUI__ClearTextBackground(xDist, yDist);
}

/*********************************************************************
*
*       _DispLine
*
*  Function description
*    Displays a string. If current text mode is not transparent, the
*    line is cleared before.
*/
static void _DispLine(const char * s, int Len) {
  const GUI_FONT * pFont;
  U16              Char;
  int              TextMode;
  int              DrawMode;

  if (Len > 0) {
    pFont    = GUI_GetFont();
    TextMode = GUI_GetTextMode();
    DrawMode = GUI_GetDrawMode();
    //
    // Clear if not transparency mode has been selected
    //
    if (!(TextMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))) {
      _ClearLine(s, Len);
    }
    //
    // Draw characters always transparent
    //
    GUI_SetDrawMode(DrawMode | GUI_DM_TRANS);
    while (--Len >= 0) {
      Char = GUI_UC__GetCharCodeInc(&s);
      pFont->pfDispChar(Char);
    }
    GUI_SetDrawMode(DrawMode);
  }
}

/*********************************************************************
*
*       _APIList
*/
static const tGUI_ENC_APIList _APIList = {
  NULL,
  NULL,
  _DispLine
};

/*********************************************************************
*
*       _GetCharDistX
*/
static int _GetCharDistX(U16P c, int * pSizeX) {
  int xDist;
  xDist = _RequestGlyph(c, 0, 0);
  if (pSizeX) {
    *pSizeX = xDist; // Same as xDist here...
  }
  return (xDist >= 0) ? xDist : 0;
}

/*********************************************************************
*
*       _GetFontInfo
*/
static void _GetFontInfo(const GUI_FONT * pFont, GUI_FONTINFO * pfi) {
  pfi->Baseline = pFont->Baseline;
  pfi->LHeight  = pFont->LHeight;
  pfi->CHeight  = pFont->CHeight;
  pfi->Flags    = GUI_FONTINFO_FLAG_PROP;
}

/*********************************************************************
*
*       _IsInFont
*/
static char _IsInFont(const GUI_FONT * pFont, U16 c) {
  FTC_FaceID face_id;
  FT_Face    face;
  FT_UInt    glyph_index;
  int        cmap_index;

  //
  // Get object pointer
  //
  face_id = (FTC_FaceID)((GUI_TTF_CS *)pFont->p.pFontData)->pTTF;
  //
  // Request face object from cache
  //
  if (FTC_Manager_LookupFace(_pContext->cache_manager, face_id, &face)) {
    return 1;
  }
  cmap_index  = _GetCharMapIndex(face, FT_ENCODING_UNICODE);
  glyph_index = FTC_CMapCache_Lookup(_pContext->cmap_cache, face_id, cmap_index, c);
  return glyph_index ? 1 : 0;
}

/*********************************************************************
*
*       _GetName
*/
static int _GetName(GUI_FONT * pFont, char * pBuffer, int NumBytes, int Index) {
  FTC_FaceID face_id;
  FT_Face    face;

  CONTEXT_VALID(1);
  //
  // Get object pointer
  //
  face_id = (FTC_FaceID)((GUI_TTF_CS *)pFont->p.pFontData)->pTTF;
  //
  // Request face object from cache
  //
  if (FTC_Manager_LookupFace(_pContext->cache_manager, face_id, &face)) {
    return 1;
  }
  return GUI_FT__GetName(face, pBuffer, NumBytes, Index);
}

/*********************************************************************
*
*       _CreateFont
*/
static int _CreateFont(GUI_FONT * pFont, GUI_TTF_CS * pCS, FT_Int Flags, void (* pfDispChar)(U16), const tGUI_ENC_APIList * pAPIList, int (* pfGetCharDistX)(U16P, int *)) {
  FTC_ImageTypeRec * pImageType;
  FTC_FaceID         face_id;
  FTC_ScalerRec      scaler;
  FT_Face            face;
  FT_Size            size;
  FT_GlyphSlot       slot;
  FT_Glyph           glyph;
  FT_UInt            glyph_index;

  //
  // Check initialization
  //
  GUI_FT__InitCache(&_pContext);
  CONTEXT_VALID(1);
  //
  // face_id is nothing but the address of the font file
  //
  face_id = (FTC_FaceID)pCS->pTTF;
  //
  // Request face object from cache
  //
  if (FTC_Manager_LookupFace(_pContext->cache_manager, face_id, &face)) {
    return 1;
  }
  FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  //
  // Set image type attributes
  //
  pImageType          = (FTC_ImageTypeRec *)pCS->aImageTypeBuffer;
  pImageType->face_id = face_id;
  pImageType->width   = 0;
  pImageType->height  = pCS->PixelHeight;
  pImageType->flags   = Flags;
  //
  // Request size object from cache
  //
  scaler.face_id = face_id;
  scaler.width   = pImageType->width;
  scaler.height  = pImageType->height;
  scaler.pixel   = 1;
  if (FTC_Manager_LookupSize(_pContext->cache_manager, &scaler, &size)) {
    return 1;
  }
  //
  // Magnification is always 1
  //
  pFont->XMag = 1;
  pFont->YMag = 1;
  //
  // Set function pointers
  //
  pFont->p.pFontData    = pCS;
  pFont->pfDispChar     = pfDispChar;
  pFont->pfGetCharDistX = pfGetCharDistX;
  pFont->pfGetFontInfo  = _GetFontInfo;
  pFont->pfIsInFont     = _IsInFont;
  pFont->pafEncode      = pAPIList;
  //
  // Calculate baseline and vertical size
  //
  pFont->Baseline = (U8)(face->size->metrics.ascender >> 6);
  pFont->YSize    = (pFont->Baseline - ((U8)(face->size->metrics.descender >> 6)));
  pFont->YDist    = pFont->YSize;
  slot            = face->glyph;
  //
  // Calculate lowercase height
  //
  glyph_index = FTC_CMapCache_Lookup(_pContext->cmap_cache, face_id, 0, 'g');
  if (FTC_ImageCache_Lookup(_pContext->image_cache, pImageType, glyph_index, &glyph, NULL)) {
    return 1;
  }
  pFont->LHeight = (U8)(slot->metrics.horiBearingY >> 6);
  //
  // Calculate capital height
  //
  glyph_index = FTC_CMapCache_Lookup(_pContext->cmap_cache, face_id, 0, 'M');
  if (FTC_ImageCache_Lookup(_pContext->image_cache, pImageType, glyph_index, &glyph, NULL)) {
    return 1;
  }
  pFont->CHeight = (U8)(slot->metrics.height >> 6);
  //
  // Select the currently created font
  //
  GUI_SetFont(pFont);
  return 0;
}

/*********************************************************************
*
*       Undefined externals, added
*
**********************************************************************
*/
/*********************************************************************
*
*       z_verbose, z_error
*
*  Description
*    Undefined externals referenced by zlib. Added here
*/
int z_verbose = 0;
void z_error(char * pMessage);
void z_error(char * pMessage) {
  GUI_USE_PARA(pMessage);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TTF_CreateFont()
*
*  Function description
*    Creates and selects an emWin font by using a TTF font file.
*
*  Parameters
*    pFont: Pointer to a GUI_FONT structure in RAM filled by the function.
*    pCS: Pointer to a GUI_TTF_CS structure containing the creation parameters.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*
*  Additional information
*    When using the function the first time it initializes the TTF engine and the internal
*    cache system. If the cache should use other values as defined per default it needs to
*    be configured before the first call of this function. For details how to configure the
*    cache, refer to GUI_TTF_SetCacheSize(). \n
*    The internal data cache manages the complete mechanism of creating fonts and
*    caching bitmap data. Font faces are uniquely identified from the cache by the
*    address given in parameter pTTF and the parameter FaceIndex, which normally is 0.
*    If the same font file for example should be used for creating fonts of different sizes
*    the parameter pTTF should point to the same location of a GUI_TTF_DATA structure.
*    The parameter PixelHeight  specifies the height of the surrounding rectangle
*    between the glyphs 'g' and 'f'. The value PixelHeight does not represent the offset
*    between lines.
*/
int GUI_TTF_CreateFont(GUI_FONT * pFont, GUI_TTF_CS * pCS) {
  return _CreateFont(pFont, pCS, FT_LOAD_TARGET_MONO, _DispChar, &_APIList, _GetCharDistX);
}

/*********************************************************************
*
*       GUI_TTF_CreateFontAA()
*
*  Function description
*    Creates and selects an antialised emWin font by using a TTF font file.
*
*  Parameters
*    pFont: Pointer to a GUI_FONT structure in RAM filled by the function.
*    pCS: Pointer to a GUI_TTF_CS structure containing the creation parameters.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*/
#if GUI_TTF_SUPPORT_AA
int GUI_TTF_CreateFontAA(GUI_FONT * pFont, GUI_TTF_CS * pCS) {
  return _CreateFont(pFont, pCS, 0, _DispCharAA, &_APIListAA, _GetCharDistX_AA);
}
#endif

/*********************************************************************
*
*       GUI_TTF_GetFamilyName()
*
*  Function description
*    The function returns the font family name defined in the font file.
*
*  Parameters
*    pFont: Pointer to a GUI_FONT structure which has been created using GUI_TTF_CreateFont().
*    pBuffer: Buffer to be filled with the family name.
*    NumBytes: Size of buffer in bytes.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*/
int GUI_TTF_GetFamilyName(GUI_FONT * pFont, char * pBuffer, int NumBytes) {
  return _GetName(pFont, pBuffer, NumBytes, FAMILY_NAME);
}

/*********************************************************************
*
*       GUI_TTF_GetStyleName()
*
*  Function description
*    The function returns the style name (bold, regular, ...) defined in the font file.
*
*  Parameters
*    pFont: Pointer to a GUI_FONT structure which has been created using GUI_TTF_CreateFont().
*    pBuffer: Buffer to be filled with the style name.
*    NumBytes: Size of buffer in bytes.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*/
int GUI_TTF_GetStyleName(GUI_FONT * pFont, char * pBuffer, int NumBytes) {
  return _GetName(pFont, pBuffer, NumBytes, STYLE_NAME);
}

/*************************** End of file ****************************/
