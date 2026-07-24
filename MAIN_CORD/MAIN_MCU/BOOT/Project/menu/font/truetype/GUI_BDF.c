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
File        : GUI_BDF.c
Purpose     : Implementation of BDF fonts
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_Private.h"
#include "GUI_FT_Private.h"

#include "ftbdf.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// BDF properties
//
#define BDF_PROP_BASELINE "FONT_ASCENT"

#define LIBRARY_VALID(r)   if (_pLibrary == NULL) { return r; }

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static FT_Library * _pLibrary;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetIntProp
*
*  Function description
*    Returns an integer property in the BDF file
*/
static int _GetIntProp(FT_Face Face, const char * sProp) {
  BDF_PropertyRec Prop;

  FT_Get_BDF_Property(Face, sProp, &Prop);
  if (Prop.type == BDF_PROPERTY_TYPE_INTEGER) {
    return Prop.u.integer;
  } else {
    return -1;
  }
}

/*********************************************************************
*
*       _RequestGlyph
*
*  Function description
*    Draws a glyph (if DoRender == 1) and returns its width
*/
static int _RequestGlyph(U16P c, unsigned DoRender) {
  int              r;
  FT_UInt          index;
  FT_Bitmap        Bmp;
  FT_Face          face;
  const GUI_FONT * pFont;
  int              TextMode;
  int              DrawMode;

  pFont = GUI_GetFont();
  r     = -1;
  if (pFont->p.pFontData == NULL) {
    return r;
  }
  face  = (FT_Face)pFont->p.pFontData;
  index = FT_Get_Char_Index(face, c);
  if (index == 0) {
    if (DoRender) {
      GUI__DrawNonExistingCharacter(c, GUI_GetDrawMode());
      return -1;
    } else {
      return GUI__GetNonExistingCharWidth(c, NULL);
    }
  }
  FT_Load_Glyph(face, index, FT_LOAD_NO_SCALE);
  FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
  Bmp = face->glyph->bitmap;
  if (DoRender) {
    //
    // Set draw mode
    //
    TextMode = GUI_GetTextMode();
    DrawMode = GUI_GetDrawMode();
    GUI_SetDrawMode(TextMode);
    //
    GUI_FT__DrawGlyph(face->glyph->bitmap_left,
                      face->glyph->bitmap_top,
                      Bmp.width,
                      Bmp.rows,
                      Bmp.pitch,
                      Bmp.buffer);
    GUI_SetDrawMode(DrawMode);
  }
  r = face->glyph->advance.x >> 6;
  return r;
}

/*********************************************************************
*
*       _DispChar
*/
static void _DispChar(U16P c) {
  int xDist;
  xDist = _RequestGlyph(c, 1);
  if (xDist >= 0) {
    GUI_pContext->DispPosX += xDist;
  }
}

/*********************************************************************
*
*       _GetCharDistX
*/
static int _GetCharDistX(U16P c, int * pSizeX) {
  int xDist;
  xDist = _RequestGlyph(c, 0);
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
  FT_Face face;

  face          = (FT_Face)pFont->p.pFontData;
  pfi->Baseline = pFont->Baseline;
  pfi->LHeight  = pFont->LHeight;
  pfi->CHeight  = pFont->CHeight;
  pfi->Flags    = (FT_IS_FIXED_WIDTH(face)) ? GUI_FONTINFO_FLAG_MONO : GUI_FONTINFO_FLAG_PROP;
}

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

  pFont   = GUI_GetFont();
  xDist   = 0;
  yDist   = pFont->YDist * pFont->YMag;
  c       = 0;
  while (--Len >= 0) {
    c     = GUI_UC__GetCharCodeInc(&s);
    xSize = _RequestGlyph(c, 0);
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
*       _IsInFont
*/
static char _IsInFont(const GUI_FONT * pFont, U16 c) {
  FT_UInt    index;
  FT_Error   err;
  FT_Face    face;

  if (pFont->p.pFontData == NULL) {
    return 0;
  }
  face = (FT_Face)pFont->p.pFontData;
  //
  // Get object pointer
  //
  index = FT_Get_Char_Index(face, c);
  if (index) {
    err   = FT_Load_Glyph(face, index, FT_LOAD_NO_SCALE);
    return (err) ? 0 : 1;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       _GetName
*/
static int _GetName(GUI_FONT * pFont, char * pBuffer, int NumBytes, int Index) {
  FT_Face face;

  LIBRARY_VALID(1);
  face = (FT_Face)pFont->p.pFontData;
  return GUI_FT__GetName(face, pBuffer, NumBytes, Index);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_BDF_CreateFont()
*
*  Function description
*    Creates and selects an emWin font by using a BDF font file.
*
*  Parameters
*    pFont: Pointer to a GUI_FONT structure in RAM filled by the function.
*    pData: Pointer to data buffer that contains the BDF font file.
*    Size:  Size of the data buffer.
*
*  Return value
*    == 0: Success.
*    == 1: Error.
*
*  Additional information
*    When using the function the first time it initializes the TTF engine.
*    The routine GUI_BDF_DeleteFont() deletes a font. By calling GUI_BDF_Done()
*    all memory allocated by the TTF engine will be deleted.
*/
int GUI_BDF_CreateFont(GUI_FONT * pFont, const U8 * pData, U32 Size) {
  FT_Error err;
  FT_Face  face;

  //
  // Initialize FreeType
  //
  if (GUI_FT__Init(&_pLibrary)) {
    return 1; // Could not initialize FreeType
  }
  LIBRARY_VALID(1); // Check if library is valid
  //
  // Load font. We are using face index 0 since BDF fonts only have 1 font face.
  //
  err = FT_New_Memory_Face(*_pLibrary, (const FT_Byte *)pData, (FT_Long)Size, 0, &face);
  if (err) {
    return 1;
  }
  err = FT_Set_Charmap(face, face->charmaps[0]);
  if (err) {
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
  pFont->p.pFontData    = (const void *)face;
  pFont->pfDispChar     = _DispChar;
  pFont->pfGetCharDistX = _GetCharDistX;
  pFont->pfGetFontInfo  = _GetFontInfo;
  pFont->pfIsInFont     = _IsInFont;
  pFont->pafEncode      = &_APIList;
  //
  // Calculate lowercase height
  //
  if (_IsInFont(pFont, 'g') == 0) {
    return 1;
  }
  pFont->LHeight = (U8)(face->glyph->metrics.horiBearingY >> 6);
  //
  // Calculate capital height
  //
  if (_IsInFont(pFont, 'M') == 0) {
    return 1;
  }
  pFont->CHeight = (U8)(face->glyph->metrics.height >> 6);
  //
  // Calculate baseline and vertical size
  //
  pFont->Baseline = _GetIntProp(face, BDF_PROP_BASELINE);
  pFont->YSize    = (U8)(face->glyph->metrics.vertAdvance >> 6);
  pFont->YDist    = pFont->YSize;
  //
  // Select the currently created font
  //
  GUI_SetFont(pFont);
  return 0;
}

/*********************************************************************
*
*       GUI_BDF_GetFamilyName()
*
*  Function description
*    The function returns the font family name defined in the font file.
*
*  Parameters
*    pFont:    Pointer to a GUI_FONT structure which has been created using GUI_BDF_CreateFont().
*    pBuffer:  Buffer to be filled with the family name.
*    NumBytes: Size of buffer in bytes.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*/
int GUI_BDF_GetFamilyName(GUI_FONT * pFont, char * pBuffer, int NumBytes) {
  return _GetName(pFont, pBuffer, NumBytes, FAMILY_NAME);
}

/*********************************************************************
*
*       GUI_BDF_GetStyleName()
*
*  Function description
*    The function returns the style name (bold, regular, ...) defined in the font file.
*
*  Parameters
*    pFont:    Pointer to a GUI_FONT structure which has been created using GUI_BDF_CreateFont().
*    pBuffer:  Buffer to be filled with the style name.
*    NumBytes: Size of buffer in bytes.
*
*  Return value
*    == 0: on success
*    == 1: on error.
*/
int GUI_BDF_GetStyleName(GUI_FONT * pFont, char * pBuffer, int NumBytes) {
  return _GetName(pFont, pBuffer, NumBytes, STYLE_NAME);
}

/*********************************************************************
*
*       GUI_BDF_DeleteFont()
*
*  Function description
*    Frees all allocated memory for an emWin font created from a BDF font
*    file.
*
*  Parameters
*    pFont: Pointer to font.
*
*  Return value
*    == 0: Font was successfully deleted.
*    == 1: Error when deleting the font.
*
*  Additional information
*    To free all memory allocated by the TTF engine, call GUI_TF_Done().
*/
int GUI_BDF_DeleteFont(GUI_FONT * pFont) {
  FT_Face  face;
  FT_Error err;

  if ((pFont->p.pFontData == NULL) || (_pLibrary == NULL)) {
    return 1;
  }
  face = (FT_Face)pFont->p.pFontData;
  err  = FT_Done_Face(face);
  return (err) ? 1 : 0;
}

/*************************** End of file ****************************/