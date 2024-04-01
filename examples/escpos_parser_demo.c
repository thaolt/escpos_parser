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

#include "escpos_parser.h"

#include <stdio.h>

const unsigned char data[] = {
    0x0A, 0x0A, 0x0A, 0x0A, 0x1B, 0x4D, 0x01, 0x1B,
    0x61, 0x01, 0x1B, 0x45, 0x01, 0x48, 0x65, 0x6C,
    0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64,
    0x0A, 0x1D, 0x6B, 0x04, 0x49, 0x50, 0x51, 0x52,
    0x53, 0x54, 0x55, 0x56, 0x00, 0x0A, 0x0A, 0x1D,
    0x56, 0x00
};
const size_t data_len = 42;

int main() {
    escpos_cmd_specs_t* cs = escpos_specs_init();

    escpos_cmd_declare(&cs, "\x0A", "line_feed", 1, false, "Line feed", 0);
    escpos_cmd_declare(&cs, "\x0D", "cr_line_feed", 1, false, "Line feed (CR)", 0);
    escpos_cmd_declare(&cs, "\xFF", "form_feed", 1, false, "Form feed", 0);
    escpos_cmd_declare(&cs, "\x1B\x40", "init_printer", 2, false, "Initialize printer", 0);
    escpos_cmd_declare(&cs, "\x1B\x4D", "select_font", 3, false, "Select font, param: A = 0, B = 1", 0);
    escpos_cmd_declare(&cs, "\x1B\x61", "pos_alignment", 3, false, "position alignment 0,1,2 -> left,center,right", 0);
    escpos_cmd_declare(&cs, "\x1B\x45", "set_bold", 3, false, "Turn emphasized mode on/off, param = 0 off, = 1 on", 0);
    escpos_cmd_declare(&cs, "\x1D\x56", "cut_paper", 3, false, "cut paper, param = 0 full cut, = 1 partial cut", 0);
    escpos_cmd_declare(&cs, "\x1D\x6B", "print_barcode_simple", 0, true, "print bar code specs.1, NUL terminated", 0);

    escpos_vec_token_t token_list;
    int token_len = escpos_parse(&token_list, &cs, data, data_len);

    printf("Parsed %d token(s)\n", token_len);

    for (int i = 0; i < token_len; i ++) {
        escpos_token_t* last_token = escpos_vec_token_get(token_list, i);

        if (!last_token) continue;

        if(last_token->cmd == NULL){
            printf("0x%04X: <text \"%.*s\">\n", last_token->addr, last_token->data_len, last_token->data);
        } else{
            printf("0x%04X: <%s", last_token->addr, last_token->cmd->id);
            for (int d = 0; d < last_token->data_len; d++) {
                printf(" 0x%02X", last_token->data[d]);
            }
            printf(">\n");
        }
    }

    escpos_vec_token_free(token_list);

    escpos_specs_free(cs);

    return 0;
}
