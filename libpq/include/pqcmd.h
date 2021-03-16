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
    PQ_SET_COLORGAMUT             = 233,
    PQ_GET_COLORGAMUT             = 234,
    PQ_GET_SOURCE_HDR_TYPE        = 235,

    PQ_MOUDLE_CMD_MAX             = 299,
    //PQ factory(300~399)
    PQ_FACTORY_CMD_START                      = 300,
    PQ_FACTORY_RESET_PICTURE_MODE             = 301,
    PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE   = 302,
    PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE     = 303,
    PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE     = 304,
    PQ_FACTORY_SET_BRIGHTNESS                 = 305,
    PQ_FACTORY_GET_BRIGHTNESS                 = 306,
    PQ_FACTORY_SET_CONTRAST                   = 307,
    PQ_FACTORY_GET_CONTRAST                   = 308,
    PQ_FACTORY_SET_SATUATION                  = 309,
    PQ_FACTORY_GET_SATUATION                  = 310,
    PQ_FACTORY_SET_HUE                        = 311,
    PQ_FACTORY_GET_HUE                        = 312,
    PQ_FACTORY_SET_SHARPNESS                  = 313,
    PQ_FACTORY_GET_SHARPNESS                  = 314,
    PQ_FACTORY_SET_OVERSCAN                   = 315,
    PQ_FACTORY_GET_OVERSCAN                   = 316,
    PQ_FACTORY_SET_WB_RED_GAIN                = 317,
    PQ_FACTORY_GET_WB_RED_GAIN                = 318,
    PQ_FACTORY_SET_WB_GREEN_GAIN              = 319,
    PQ_FACTORY_GET_WB_GREEN_GAIN              = 320,
    PQ_FACTORY_SET_WB_BLUE_GAIN               = 321,
    PQ_FACTORY_GET_WB_BLUE_GAIN               = 322,
    PQ_FACTORY_SET_WB_RED_OFFSET              = 323,
    PQ_FACTORY_GET_WB_RED_OFFSET              = 324,
    PQ_FACTORY_SET_WB_GREEN_OFFSET            = 325,
    PQ_FACTORY_GET_WB_GREEN_OFFSET            = 326,
    PQ_FACTORY_SET_WB_BLUE_OFFSET             = 327,
    PQ_FACTORY_GET_WB_BLUE_OFFSET             = 328,
    PQ_FACTORY_SET_RGB_PATTERN                = 329,
    PQ_FACTORY_GET_RGB_PATTERN                = 330,
    PQ_FACTORY_SET_GRAY_PATTERN               = 331,
    PQ_FACTORY_GET_GRAY_PATTERN               = 332,

    PQ_FACTORY_CMD_MAX                        = 399,
}pqcmd_t;

#endif  //AMLOGIC_PQCMD_H
