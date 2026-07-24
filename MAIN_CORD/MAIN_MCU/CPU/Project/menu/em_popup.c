/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_popup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-1-9
  * @brief       弹窗界面处理
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-12-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */



#include "em_menu.h"


/**
  * @brief 创建弹窗内子控件
  * @param  p_widget_para:弹窗内子控件结构体
  * @retval NULL
  */
void EmPopUpSubWidgetCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
{
    uint8_t i, j;
    uint8_t widget_id_offset = 1;
    int ex_flags;

    for(i = 0; i < p_widget_para->widget_num; i++)
    {
        p_widget_para->wid_handle[i] = 0;
    }

    /*创建控件*/
    for(i = 0; i < p_widget_para->widget_num; i++)
    {
        if(p_widget_para->sub_widget_para[i].widget_type == EM_WIDGET_TYPE_TEXT)
        {
            ex_flags = TEXT_CF_LEFT | TEXT_CF_VCENTER;
            p_widget_para->wid_handle[i] = TEXT_CreateEx(p_widget_para->sub_widget_para[i].x_relative_pos,
                                           p_widget_para->sub_widget_para[i].y_relative_pos,
                                           p_widget_para->sub_widget_para[i].x_size,
                                           p_widget_para->sub_widget_para[i].y_size,
                                           h_win_src, WM_CF_SHOW, ex_flags, p_widget_para->popup_start_id + widget_id_offset,
                                           p_widget_para->sub_widget_para[i].sub_widget.text_para.p_content);
            TEXT_SetFont(p_widget_para->wid_handle[i], p_widget_para->sub_widget_para[i].sub_widget.text_para.p_text_font);
            TEXT_SetTextColor(p_widget_para->wid_handle[i], p_widget_para->sub_widget_para[i].sub_widget.text_para.text_color);
            widget_id_offset++;
        }
        else if(p_widget_para->sub_widget_para[i].widget_type == EM_WIDGET_TYPE_IMG)
        {
            p_widget_para->wid_handle[i] = IMAGE_CreateEx(p_widget_para->sub_widget_para[i].x_relative_pos,
                                           p_widget_para->sub_widget_para[i].y_relative_pos,
                                           p_widget_para->sub_widget_para[i].x_size,
                                           p_widget_para->sub_widget_para[i].y_size,
                                           h_win_src, WM_CF_SHOW, IMAGE_CF_ALPHA | IMAGE_CF_AUTOSIZE, p_widget_para->popup_start_id + widget_id_offset);
            IMAGE_SetDTA(p_widget_para->wid_handle[i],
                         p_widget_para->sub_widget_para[i].sub_widget.img_para.p_img_buf,
                         p_widget_para->sub_widget_para[i].sub_widget.img_para.img_size);
            widget_id_offset++;
        }
        else if(p_widget_para->sub_widget_para[i].widget_type == EM_WIDGET_TYPE_PROGBAR)
        {
            p_widget_para->wid_handle[i] = PROGBAR_CreateEx(p_widget_para->sub_widget_para[i].x_relative_pos,
                                           p_widget_para->sub_widget_para[i].y_relative_pos,
                                           p_widget_para->sub_widget_para[i].x_size,
                                           p_widget_para->sub_widget_para[i].y_size,
                                           h_win_src, WM_CF_SHOW, PROGBAR_CF_HORIZONTAL, p_widget_para->popup_start_id + widget_id_offset);
            PROGBAR_SetValue(p_widget_para->wid_handle[i], p_widget_para->sub_widget_para[i].sub_widget.progbar_para.value);
            widget_id_offset++;
        }
    }
}














