//
// Created by bozhnyuks on 21.11.23.
//

#ifndef LAMA_VALUE_H
#define LAMA_VALUE_H

#include "common.h"

#define UNBOX_INT(x) (((int)(x)) >> 1)
#define BOX_INT(x) ((((int)(x)) << 1) | 0x0001)
#define IS_BOXED(x) (((int)(x)) & 1)

#endif //LAMA_VALUE_H
