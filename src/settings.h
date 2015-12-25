#include <pebble.h>
#pragma once

#define NOZERO(s) ((*s == '0')?(s+1):s)

typedef enum {
  KEY_THEME = 0,
  KEY_ALIGN,
  KEY_FONT,
  KEY_LAST
} MessageKey;

enum {
  THEME_DARK = 0,
  THEME_LIGHT,
  THEME_GREEN,
  THEME_LAST
};

enum {
  ALIGN_LEFT = 0,
  ALIGN_CENTER,
  ALIGN_RIGHT,
  ALIGN_LAST
};

enum {
  FONT_HELVETICA = 0,
  FONT_AVERIA,
  FONT_UBUNTU,
  FONT_LAST
};

GColor8 *themes[THEME_LAST][3] = {
  {&GColorBlack, &GColorWhite, &GColorWhite},
  {&GColorWhite, &GColorBlack, &GColorBlack},
  {&GColorDarkGreen, &GColorWhite, &GColorWhite}
};

GTextAlignment aligns[ALIGN_LAST] = {
  GTextAlignmentLeft,
  GTextAlignmentCenter,
  GTextAlignmentRight
};

const int fonts[FONT_LAST][2] = {
#ifdef PBL_RECT
  {RESOURCE_ID_FONT_HELVETICA_R_28, RESOURCE_ID_FONT_HELVETICA_B_48},
  {RESOURCE_ID_FONT_AVERIA_R_28, RESOURCE_ID_FONT_AVERIA_B_48},
  {RESOURCE_ID_FONT_UBUNTU_R_28, RESOURCE_ID_FONT_UBUNTU_B_48}
#else
  {RESOURCE_ID_FONT_HELVETICA_R_28, RESOURCE_ID_FONT_HELVETICA_B_64},
  {RESOURCE_ID_FONT_AVERIA_R_28, RESOURCE_ID_FONT_AVERIA_B_64},
  {RESOURCE_ID_FONT_UBUNTU_R_28, RESOURCE_ID_FONT_UBUNTU_B_64}
  #endif
};
