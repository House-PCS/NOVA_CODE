/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        filter_calc.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __FILTER_CALC_H_
#define __FILTER_CALC_H_



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


uint16_t FilterCalcMedian16(uint16_t *p_data, uint16_t count);
void FilterCalcSlide32f(float in, float *p_avg_sum, float *p_out, uint16_t N, bool accu);
void FilterCalcSlide16u(uint16_t in, float *p_avg_sum, uint16_t *p_out, uint16_t N, bool accu);
void FilterCalcSlideMaxValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu);
void FilterCalcSlideMinValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu);
void FilterCalcSlideMidianValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu);
void FilterCalcSlideMadMidianValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu);

#endif


