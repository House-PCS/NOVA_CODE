/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       列表通用界面处理
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
  * @brief 创建LIST子控件
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void EmListWidgetCreate(emListPara_t *p_widget_para, WM_HWIN h_win_src)
{
    uint8_t i, j;
    uint8_t display_num;
    uint8_t widget_id_offset = 0;
    int ex_flags;
    
    /*先清零各个子控件句柄*/
    for(i = 0; i < p_widget_para->list_display_num; i++)
    {
        for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
        {
            p_widget_para->handle[j][i] = 0;
        }
    }
    
    /*创建控件*/
    if(p_widget_para->list_num <= p_widget_para->list_display_num)
    {
        display_num = p_widget_para->list_num;
    }
    else
    {
        display_num = p_widget_para->list_display_num;
    }
    
    for(i = 0; i < display_num; i++)
    {
        for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
        {
            if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_TEXT)
            {
				if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].align == EM_TEXT_DEFAULT_ALIGN)
				{
					if(j == 0)//左边
					{
						ex_flags = TEXT_CF_LEFT | TEXT_CF_VCENTER;
					}
					else//中间以及右边等
					{
						ex_flags = TEXT_CF_RIGHT | TEXT_CF_VCENTER;
					}
				}
				else if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].align == EM_TEXT_USER_ALIGN)
				{
					ex_flags = TEXT_CF_LEFT | TEXT_CF_VCENTER;
				}
                p_widget_para->handle[j][i] = TEXT_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                            p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                            p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                            p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size, 
                                                            h_win_src, WM_CF_SHOW, ex_flags, p_widget_para->list_start_id + widget_id_offset, 
                                                            p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_content);
                TEXT_SetFont(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_text_font);
                TEXT_SetTextColor(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.text_color); 
                widget_id_offset++;
                
            }
            else if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_IMG)
            {
                p_widget_para->handle[j][i] = IMAGE_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                             p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                             p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                             p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size,
                                                             h_win_src, WM_CF_SHOW, IMAGE_CF_ALPHA | IMAGE_CF_AUTOSIZE, p_widget_para->list_start_id + widget_id_offset);
                IMAGE_SetDTA(p_widget_para->handle[j][i], 
                             p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.p_img_buf, 
                             p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.img_size); 
                widget_id_offset++;
            }
        }
    }
    
    /*创建滚动条*/
    if(p_widget_para->scrollbar_enable == true)
    {
        SCROLLBAR_SKINFLEX_PROPS SCROLLBAR_Props;
        SCROLLBAR_GetSkinFlexProps(&SCROLLBAR_Props, SCROLLBAR_SKINFLEX_PI_UNPRESSED);
//        SCROLLBAR_Props.aColorShaft[0] = p_widget_para->scrollbar_bk_color;
//        SCROLLBAR_Props.aColorShaft[1] = p_widget_para->scrollbar_bk_color;
        
//        SCROLLBAR_Props.aColorFrame[0] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorFrame[1] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorFrame[2] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorUpper[0] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorUpper[1] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorLower[0] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.aColorLower[1] = GUI_TRANSPARENT;
//        SCROLLBAR_Props.ColorArrow = GUI_TRANSPARENT;
//        SCROLLBAR_Props.ColorGrasp = p_widget_para->scrollbar_color;
        
        SCROLLBAR_SetSkinFlexProps(&SCROLLBAR_Props, SCROLLBAR_SKINFLEX_PI_PRESSED); 
        SCROLLBAR_SetSkinFlexProps(&SCROLLBAR_Props, SCROLLBAR_SKINFLEX_PI_UNPRESSED);
        
        p_widget_para->scrollbar_handle = SCROLLBAR_CreateEx(p_widget_para->scrollbar_x_pos, 
                                                          p_widget_para->scrollbar_y_pos, 
                                                          p_widget_para->scrollbar_x_size, 
                                                          p_widget_para->scrollbar_y_size, 
                                                          h_win_src, WM_CF_SHOW, SCROLLBAR_CF_VERTICAL, p_widget_para->list_start_id + widget_id_offset);
        SCROLLBAR_SetNumItems(p_widget_para->scrollbar_handle, p_widget_para->list_num);
        SCROLLBAR_SetPageSize(p_widget_para->scrollbar_handle, p_widget_para->list_display_num);    
        SCROLLBAR_SetValue(p_widget_para->scrollbar_handle, p_widget_para->list_page);
        SCROLLBAR_SetColor(p_widget_para->scrollbar_handle, SCROLLBAR_CI_THUMB, p_widget_para->scrollbar_color);
        SCROLLBAR_SetColor(p_widget_para->scrollbar_handle, SCROLLBAR_CI_SHAFT, p_widget_para->scrollbar_bk_color);
        
        SCROLLBAR_SetSkinClassic(p_widget_para->scrollbar_handle);
        widget_id_offset++;
        
    }
    
}


/**
  * @brief 选中框删除
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void SelectedDel(emListPara_t *p_widget_para, WM_HWIN h_win_src)
{
    WM_HWIN hWinOld;
    hWinOld = WM_SelectWindow(WM_GetClientWindow(h_win_src));//先选择要在哪个窗口绘制
    GUI_SetColor(p_widget_para->selected_unvalid_color);//上色
    
    if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FRAME)
    {
        GUI_DrawRoundedFrame(p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                             p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset,
                             p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                             p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                             p_widget_para->selected_r, p_widget_para->selected_size);
    }
    else if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FILLED)
    {
        GUI_FillRoundedRect(p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                            p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset,
                            p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                            p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                            p_widget_para->selected_r);
    }
    WM_SelectWindow(hWinOld);//恢复绘制之前的窗口选择
}

/**
  * @brief 选中框绘制
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void SelectedDraw(emListPara_t *p_widget_para, WM_HWIN h_win_src)
{
    WM_HWIN hWinOld;
    hWinOld = WM_SelectWindow(WM_GetClientWindow(h_win_src));//先选择要在哪个窗口绘制
    GUI_SetColor(p_widget_para->selected_valid_color);//上色
    
    if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FRAME)
    {
        GUI_DrawRoundedFrame(p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                             p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset,
                             p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                             p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                             p_widget_para->selected_r, p_widget_para->selected_size);
    }
    else if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FILLED)
    {
        GUI_FillRoundedRect(p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                            p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset,
                            p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                            p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                            p_widget_para->selected_r);
    }
    WM_SelectWindow(hWinOld);//恢复绘制之前的窗口选择
    
}



/**
  * @brief 选中框删除
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void SelectedDelInWMPaint(emListPara_t *p_widget_para,  uint8_t list_sel)
{
    GUI_SetColor(p_widget_para->selected_unvalid_color);//上色

    if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FRAME)
    {
        GUI_DrawRoundedFrame(p_widget_para->sub_region[list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->selected_y_start_offset,
                             p_widget_para->sub_region[list_sel].x_pos + p_widget_para->sub_region[list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->sub_region[list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                             p_widget_para->selected_r, p_widget_para->selected_size);
    }
    else if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FILLED)
    {
        GUI_FillRoundedRect(p_widget_para->sub_region[list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                            p_widget_para->sub_region[list_sel].y_pos + p_widget_para->selected_y_start_offset,
                            p_widget_para->sub_region[list_sel].x_pos + p_widget_para->sub_region[list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                            p_widget_para->sub_region[list_sel].y_pos + p_widget_para->sub_region[list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                            p_widget_para->selected_r);
    }
}

/**
  * @brief 选中框绘制
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void SelectedDrawInWMPaint(emListPara_t *p_widget_para, uint8_t list_sel)
{
    WM_HWIN hWinOld;
    GUI_SetColor(p_widget_para->selected_valid_color);//上色
    
    if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FRAME)
    {
        GUI_DrawRoundedFrame(p_widget_para->sub_region[list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->selected_y_start_offset,
                             p_widget_para->sub_region[list_sel].x_pos + p_widget_para->sub_region[list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->sub_region[list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                             p_widget_para->selected_r, p_widget_para->selected_size);
    }
    else if(p_widget_para->selected_type == EN_LIST_SELECTED_TYPE_FILLED)
    {
		GUI_FillRoundedRect(p_widget_para->sub_region[list_sel].x_pos + p_widget_para->selected_x_start_offset, 
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->selected_y_start_offset,
                             p_widget_para->sub_region[list_sel].x_pos + p_widget_para->sub_region[list_sel].x_size - 1 + p_widget_para->selected_x_end_offset,
                             p_widget_para->sub_region[list_sel].y_pos + p_widget_para->sub_region[list_sel].y_size - 1 + p_widget_para->selected_y_end_offset,
                             p_widget_para->selected_r);
		
       /* GUI_FillRoundedRect(p_widget_para->sub_region[list_sel].x_pos, 
                            p_widget_para->sub_region[list_sel].y_pos,
                            p_widget_para->sub_region[list_sel].x_pos + p_widget_para->sub_region[list_sel].x_size - 1,
                            p_widget_para->sub_region[list_sel].y_pos + p_widget_para->sub_region[list_sel].y_size - 1,
                            p_widget_para->selected_r);*/
    }
    
}



/**
  * @brief 向下翻页
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmListDownward(emListPara_t *p_widget_para, WM_HWIN h_win_src)
{
    uint8_t max_page;
    
    if(p_widget_para->list_num <= p_widget_para->list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = p_widget_para->list_num + 1 - p_widget_para->list_display_num;
    }

    if(p_widget_para->list_sel == p_widget_para->list_display_num - 1)
    {
        if(p_widget_para->list_page < max_page - 1)
        {
            uint8_t i, j;
            int ex_flags;
            uint8_t widget_id_offset = 0;
            p_widget_para->list_page++;
            if(p_widget_para->scrollbar_enable == true)
            {
                SCROLLBAR_SetValue(p_widget_para->scrollbar_handle, p_widget_para->list_page);
            }
            for(i = 0; i < p_widget_para->list_display_num; i++)
            {
                for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
                {
                    if(p_widget_para->handle[j][i] != 0)
                    {
                        WM_DeleteWindow(p_widget_para->handle[j][i]);
                        p_widget_para->handle[j][i] = 0;
                    }
                }
            }
            for(i = 0; i < p_widget_para->list_display_num; i++)
            {
                for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
                {
                    if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_TEXT)
                    {
                        if(j == 0)//左边
                        {
                            ex_flags = TEXT_CF_LEFT | TEXT_CF_VCENTER;
                        }
                        else//中间以及右边等
                        {
                            ex_flags = TEXT_CF_RIGHT | TEXT_CF_VCENTER;
                        }
                        p_widget_para->handle[j][i] = TEXT_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size, 
                                                                    h_win_src, WM_CF_SHOW, ex_flags, p_widget_para->list_start_id + widget_id_offset, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_content);
                        TEXT_SetFont(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_text_font);
                        TEXT_SetTextColor(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.text_color); 
                        widget_id_offset++;
                        WM_BringToBottom(p_widget_para->handle[j][i]);
                        
                    }
                    else if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_IMG)
                    {
                        p_widget_para->handle[j][i] = IMAGE_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size,
                                                                     h_win_src, WM_CF_SHOW, IMAGE_CF_ALPHA | IMAGE_CF_AUTOSIZE, p_widget_para->list_start_id + widget_id_offset);
                        IMAGE_SetDTA(p_widget_para->handle[j][i], 
                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.p_img_buf, 
                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.img_size); 
                        widget_id_offset++;
                        WM_BringToBottom(p_widget_para->handle[j][i]);
                    }
                }
            }
        }
    }
    else
    {
        static GUI_RECT rect;
        p_widget_para->list_pre_sel = p_widget_para->list_sel;
        p_widget_para->list_sel++;
//        WM_InvalidateWindow(h_win_src);
        rect.x0 = p_widget_para->sub_region[p_widget_para->list_pre_sel].x_pos + p_widget_para->selected_x_start_offset;
        rect.y0 = p_widget_para->sub_region[p_widget_para->list_pre_sel].y_pos + p_widget_para->selected_y_start_offset;
        rect.x1 = p_widget_para->sub_region[p_widget_para->list_pre_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_pre_sel].x_size - 1 + p_widget_para->selected_x_end_offset;
        rect.y1 = p_widget_para->sub_region[p_widget_para->list_pre_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_pre_sel].y_size - 1 + p_widget_para->selected_y_end_offset;
        WM_InvalidateArea(&rect);
        rect.x0 = p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset;
        rect.y0 = p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset;
        rect.x1 = p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset+p_widget_para->list_x_offset;
        rect.y1 = p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset+p_widget_para->list_y_offset;
        WM_InvalidateArea(&rect);
//        WM_Paint(h_win_src);
    }
}

/**
  * @brief 向上翻页
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
void EmListUpward(emListPara_t *p_widget_para, WM_HWIN h_win_src)
{
    //uint8_t max_page;
    
    if(p_widget_para->list_num <= p_widget_para->list_display_num)
    {
        //max_page = 1;
    }
    else
    {
        //max_page = p_widget_para->list_num + 1 - p_widget_para->list_display_num;
    }
    
    if(p_widget_para->list_sel == 0)
    {
        if(p_widget_para->list_page > 0)
        {
            uint8_t i, j;
            int ex_flags;
            uint8_t widget_id_offset = 0;
            p_widget_para->list_page--;
            if(p_widget_para->scrollbar_enable == true)
            {
                SCROLLBAR_SetValue(p_widget_para->scrollbar_handle, p_widget_para->list_page);
            }
            for(i = 0; i < p_widget_para->list_display_num; i++)
            {
                for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
                {
                    if(p_widget_para->handle[j][i] != 0)
                    {
                        WM_DeleteWindow(p_widget_para->handle[j][i]);
                        p_widget_para->handle[j][i] = 0;
                    }
                }
            }
            for(i = 0; i < p_widget_para->list_display_num; i++)
            {
                for(j = 0; j < EM_LIST_ONE_LINE_MAX_WIDGET_NUM; j++)
                {
                    if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_TEXT)
                    {
                        if(j == 0)//左边
                        {
                            ex_flags = TEXT_CF_LEFT | TEXT_CF_VCENTER;
                        }
                        else//中间以及右边等
                        {
                            ex_flags = TEXT_CF_RIGHT | TEXT_CF_VCENTER;
                        }
                        p_widget_para->handle[j][i] = TEXT_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size, 
                                                                    h_win_src, WM_CF_SHOW, ex_flags, p_widget_para->list_start_id + widget_id_offset, 
                                                                    p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_content);
                        TEXT_SetFont(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.p_text_font);
                        TEXT_SetTextColor(p_widget_para->handle[j][i], p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.text_para.text_color); 
                        widget_id_offset++;
                        WM_BringToBottom(p_widget_para->handle[j][i]);
                        
                    }
                    else if(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].widget_type == EM_WIDGET_TYPE_IMG)
                    {
                        p_widget_para->handle[j][i] = IMAGE_CreateEx(p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_relative_pos + p_widget_para->sub_region[i].x_pos, 
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_relative_pos + p_widget_para->sub_region[i].y_pos, 
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].x_size,
                                                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].y_size,
                                                                     h_win_src, WM_CF_SHOW, IMAGE_CF_ALPHA | IMAGE_CF_AUTOSIZE, p_widget_para->list_start_id + widget_id_offset);
                        IMAGE_SetDTA(p_widget_para->handle[j][i], 
                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.p_img_buf, 
                                     p_widget_para->sub_widget_para[j][p_widget_para->list_page + i].sub_widget.img_para.img_size); 
                        widget_id_offset++;
                        WM_BringToBottom(p_widget_para->handle[j][i]);
                    }
                }
            }
        }
    }
    else
    {
        static GUI_RECT rect;
        p_widget_para->list_pre_sel = p_widget_para->list_sel;
        p_widget_para->list_sel--;
//        WM_InvalidateWindow(h_win_src);
        rect.x0 = p_widget_para->sub_region[p_widget_para->list_pre_sel].x_pos + p_widget_para->selected_x_start_offset;
        rect.y0 = p_widget_para->sub_region[p_widget_para->list_pre_sel].y_pos + p_widget_para->selected_y_start_offset;
        rect.x1 = p_widget_para->sub_region[p_widget_para->list_pre_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_pre_sel].x_size - 1 + p_widget_para->selected_x_end_offset+p_widget_para->list_x_offset;
        rect.y1 = p_widget_para->sub_region[p_widget_para->list_pre_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_pre_sel].y_size - 1 + p_widget_para->selected_y_end_offset+p_widget_para->list_y_offset;
        WM_InvalidateArea(&rect);
        rect.x0 = p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->selected_x_start_offset;
        rect.y0 = p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->selected_y_start_offset;
        rect.x1 = p_widget_para->sub_region[p_widget_para->list_sel].x_pos + p_widget_para->sub_region[p_widget_para->list_sel].x_size - 1 + p_widget_para->selected_x_end_offset;
        rect.y1 = p_widget_para->sub_region[p_widget_para->list_sel].y_pos + p_widget_para->sub_region[p_widget_para->list_sel].y_size - 1 + p_widget_para->selected_y_end_offset;
        WM_InvalidateArea(&rect);
//        WM_Paint(h_win_src);
    }
}



