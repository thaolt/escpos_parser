/**
 * Copyright 2024 (c) thaolt@songphi.com - https://github.com/thaolt/escpos_parser
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#ifndef ESCPOS_H_INCLUDED
#define ESCPOS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char* id;
    char* signature;
    unsigned int fixed_length;
    bool nul_terminated;
    char* description;
    char* lcexpr;
} escpos_cmd_t;

typedef struct {
    unsigned int addr;
    escpos_cmd_t* cmd;
    uint8_t* data;
    unsigned int data_len;
} escpos_token_t;

#include "vector_token.h"

typedef void escpos_cmd_specs_t;

escpos_cmd_specs_t * escpos_specs_init();

void escpos_cmd_declare(escpos_cmd_specs_t** cs, char* signature, char* id, unsigned int fixed_length, bool nul_terminated, char* description, char *lcexpr);

unsigned int escpos_parse(escpos_vec_token_t* token_list, escpos_cmd_specs_t** cs, const unsigned char* data, unsigned int datalen);

void escpos_specs_free(escpos_cmd_specs_t* cs);
escpos_cmd_t *escpos_cmd_free(escpos_cmd_t* cmd);
void escpos_vec_token_free(escpos_vec_token_t token_list);

void escpos_print_tokens(escpos_vec_token_t token_list);


#ifdef __cplusplus
}
#endif


#endif // ESCPOS_H_INCLUDED
