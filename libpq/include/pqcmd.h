/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef AMLOGIC_PQCMD_H
#define AMLOGIC_PQCMD_H

typedef enum pqcmd_e {
    //PQ CMD(201~299)
    PQ_MOUDLE_CMD_START           = 200,
    PQ_SET_PICTURE_MODE           = 201,
    PQ_GET_PICTURE_MODE           = 202,
    PQ_SET_COLOR_TEMPERATURE_MODE = 203,
    PQ_GET_COLOR_TEMPERATURE_MODE = 204,
    PQ_SET_BRIGHTNESS             = 205,
    PQ_GET_BRIGHTNESS             = 206,
    PQ_SET_CONTRAST               = 207,
    PQ_GET_CONTRAST               = 208,
    PQ_SET_SATUATION              = 209,
    PQ_GET_SATUATION              = 210,
    PQ_SET_HUE                    = 211,
    PQ_GET_HUE                    = 212,
    PQ_SET_SHARPNESS              = 213,
    PQ_GET_SHARPNESS              = 214,
    PQ_SET_NOISE_REDUCTION_MODE   = 215,
    PQ_GET_NOISE_REDUCTION_MODE   = 216,
    PQ_SET_EYE_PROTECTION_MODE    = 217,
    PQ_GET_EYE_PROTECTION_MODE    = 218,
    PQ_SET_GAMMA                  = 219,
    PQ_GET_GAMMA                  = 220,
    PQ_SET_DISPLAY_MODE           = 221,
    PQ_GET_DISPLAY_MODE           = 222,
    PQ_SET_BACKLIGHT              = 223,
    PQ_GET_BACKLIGHT              = 224,
    PQ_SET_DYNAMICBACKLIGHT       = 225,
    PQ_GET_DYNAMICBACKLIGHT       = 226,
    PQ_SET_LOCALCONTRAST          = 227,
    PQ_GET_LOCALCONTRAST          = 228,
    PQ_SET_CM                     = 229,
    PQ_GET_CM                     = 230,
    PQ_SET_SOURCE_CHANNEL         = 231,
    PQ_GET_SOURCE_CHANNEL         = 232,

    PQ_MOUDLE_CMD_MAX             = 299,
    //PQ factory(300~399)
    PQ_FACTORY_CMD_START                = 300,
    PQ_FACTORY_SET_WB_RED_GAIN          = 301,
    PQ_FACTORY_GET_WB_RED_GAIN          = 302,
    PQ_FACTORY_SET_WB_GREE_GAIN         = 303,
    PQ_FACTORY_GET_WB_GREE_GAIN         = 304,
    PQ_FACTORY_SET_WB_BLUE_GAIN         = 305,
    PQ_FACTORY_GET_WB_BLUE_GAIN         = 306,
    PQ_FACTORY_SET_WB_RED_POSTOFFSET    = 307,
    PQ_FACTORY_GET_WB_RED_POSTOFFSET    = 308,
    PQ_FACTORY_SET_WB_GREE_POSTOFFSET   = 309,
    PQ_FACTORY_GET_WB_GREE_POSTOFFSET   = 310,
    PQ_FACTORY_SET_WB_BLUE_POSTOFFSET   = 311,
    PQ_FACTORY_GET_WB_BLUE_POSTOFFSET   = 312,
    PQ_FACTORY_SET_RGB_PATTERN          = 313,
    PQ_FACTORY_GET_RGB_PATTERN          = 314,
    PQ_FACTORY_SET_GRAY_PATTERN         = 315,
    PQ_FACTORY_GET_GRAY_PATTERN         = 316,
    PQ_FACTORY_CMD_MAX                  = 399,
}pqcmd_t;

#endif  //AMLOGIC_PQCMD_H
