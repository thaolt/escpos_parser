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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"
#include "escpos_parser.h"
#include "tinyexpr.h"

typedef struct escpos_ctree_node_t escpos_ctree_node_t;

struct escpos_ctree_node_t {
    int idx;
    int nodes_size;
    escpos_ctree_node_t* nodes;
    escpos_cmd_t* cmd;
    UT_hash_handle hh;
};

unsigned char *te_data = NULL;

char * escpos_strdup(const char *str) {
    if (str == NULL) return NULL;
    size_t len = strlen(str);
    char *x = (char *)malloc(len+1); /* +1 for the null terminator */
    if(!x) return NULL; /* malloc could not allocate memory */
    memcpy(x,str,len+1); /* copy the string into the new buffer */
    return x;
}

int caculate_cmd_data_length(escpos_cmd_t** cmd, unsigned char *data);


escpos_cmd_specs_t *escpos_specs_init()
{
    escpos_ctree_node_t* root = (escpos_ctree_node_t*) calloc(1, sizeof(escpos_ctree_node_t));
    return (escpos_cmd_specs_t *) root;
}

void escpos_cmd_declare(escpos_cmd_specs_t** cs, char* signature, char* id, unsigned int fixed_length, bool nul_terminated, char* description, char *lcexpr)
{
    escpos_ctree_node_t** root = (escpos_ctree_node_t**) cs;

    if (!*root) {
        *root = (escpos_ctree_node_t*) calloc(1, sizeof(escpos_ctree_node_t));
        (*root)->nodes = NULL;
    }

    escpos_cmd_t* cmd = (escpos_cmd_t*) calloc(1, sizeof(escpos_cmd_t));
    cmd->id = escpos_strdup( id );
    cmd->signature = escpos_strdup( signature );
    cmd->fixed_length = fixed_length;
    cmd->nul_terminated = nul_terminated;
    cmd->description = escpos_strdup( description );
    cmd->lcexpr = escpos_strdup( lcexpr );

    escpos_ctree_node_t** nodes = &((*root)->nodes);

    unsigned char siglen = strlen(signature);

    for (int i = 0; i < siglen; i++) {
        int sig = signature[i];

        escpos_ctree_node_t* found_node = NULL;
        if (*nodes) HASH_FIND_INT(*nodes, &sig, found_node);
        if (!found_node) {
            found_node = (escpos_ctree_node_t*) calloc(1, sizeof (escpos_ctree_node_t));
            found_node->idx = sig;
            if (i == siglen - 1) {
                found_node->cmd = cmd;
            }
            HASH_ADD_INT(*nodes, idx, found_node);
            (*nodes)->nodes_size++;
        }
        nodes = &(found_node->nodes);
    }
}

unsigned int escpos_parse(escpos_vec_token_t* token_list, escpos_cmd_specs_t** cs, const unsigned char* data, unsigned int datalen)
{
    unsigned int token_count = 0;
    escpos_ctree_node_t** ct = (escpos_ctree_node_t**) cs;
    escpos_vec_token_init(*token_list);

    escpos_token_t* last_token = NULL;

    for (size_t i = 0; i < datalen; i++) {
        unsigned char c = data[i];
        unsigned int addr = i;

        if (last_token == NULL) {
            last_token = (escpos_token_t*) calloc(1, sizeof(escpos_token_t));
        }

        if (c <= 0x1F) {
            int sig = c;

            escpos_ctree_node_t** nodes = &((*ct)->nodes);
            escpos_ctree_node_t* found_node = NULL;

            HASH_FIND_INT(*nodes, &sig, found_node);

            if (found_node != NULL) {
                if (last_token && last_token->cmd == NULL && last_token->data_len > 0) {
                    escpos_vec_token_append(*token_list, last_token);
                    token_count ++;
                    last_token = (escpos_token_t*) calloc(1, sizeof (escpos_token_t));
                }

                nodes = &(found_node->nodes);

                while (*nodes != NULL && (*nodes)->nodes_size != 0 && i < datalen) {
                    sig = data[i+1];
                    found_node = NULL;
                    HASH_FIND_INT(*nodes, &sig, found_node);
                    i++;
                    if (found_node != NULL)
                        nodes = &(found_node->nodes);
                    else
                        break;
                }

                if (found_node != NULL) {
                    last_token->cmd = found_node->cmd;
                    int siglen = strlen(last_token->cmd->signature);

                    // copy data according to specs
                    if (last_token->cmd->fixed_length > 0) {
                        last_token->data_len = last_token->cmd->fixed_length - siglen;
                    } else if (last_token->cmd->nul_terminated) {
                        unsigned int l = i;
                        while (data[l] != 0 && l < datalen) l++;
                        last_token->data_len += l - i;
                    } else {
                        last_token->data_len = caculate_cmd_data_length(&(last_token->cmd), &(data[i+1]));
                    }

                    // move global data index to after datalen
                    if (last_token->data_len > 0) {
                        last_token->data = malloc(last_token->data_len);
                        memcpy(last_token->data, &data[i+1], last_token->data_len);
                        i += last_token->data_len;
                    }

                    last_token->addr = addr;
                    escpos_vec_token_append(*token_list, last_token);
                    token_count++;
                    last_token = NULL;
                }
            }
        } else { // cmd not found, data instead
            if (last_token && last_token->cmd == NULL) {
                last_token->data_len++;
                last_token->data = (unsigned char *) realloc(last_token->data, last_token->data_len+1);

            } else {
                last_token = (escpos_token_t*) calloc(1, sizeof(escpos_token_t));
                last_token->data = (uint8_t *) calloc(2, sizeof(uint8_t));
                last_token->data_len++;
                last_token->addr = addr;
                escpos_vec_token_append(*token_list, last_token);
                token_count++;
            }
            last_token->data[last_token->data_len - 1] = c;
            last_token->data[last_token->data_len] = 0; // NUL terminated
        }
    } // for data

    if (last_token && last_token->data_len > 0 && last_token->cmd == NULL) {
        escpos_vec_token_append(*token_list, last_token);
        last_token = NULL;
        token_count++;
    }
    return token_count;
}

double _escpos_lcexpr_data_func(double i) {
    double ret = 0;
    unsigned int idx = i;
    int p = te_data[idx];
    ret = (double) p;
    return ret;
}

int caculate_cmd_data_length(escpos_cmd_t** cmd, unsigned char* data)
{
    if (!(*cmd)->lcexpr) return 0;
    te_data = data;
    int err;
    unsigned int ret = 0;
    te_variable vars[] = {
        {"d", _escpos_lcexpr_data_func, TE_FUNCTION1}
    };
    te_expr *e = te_compile((*cmd)->lcexpr, vars, 1, &err);

    if (e) {
        ret = te_eval(e);
        te_free(e);
    }

    return ret;
}

escpos_cmd_t * escpos_cmd_free(escpos_cmd_t *cmd)
{
    if (!cmd) return NULL;
    if (cmd->lcexpr) { free(cmd->lcexpr); cmd->lcexpr = NULL; }
    if (cmd->description) { free(cmd->description); cmd->description = NULL; }
    if (cmd->signature) { free(cmd->signature); cmd->signature = NULL; }
    if (cmd->id) { free(cmd->id); cmd->id = NULL; }
    free(cmd);
    return NULL;
}

void escpos_vec_token_free(escpos_vec_token_t token_list)
{
    int token_len = escpos_vec_token_size(token_list);
    while (token_len > 0) {
        escpos_token_t* token = escpos_vec_token_pop(token_list);
        if (token) {
            // not freeing cmd because cmd belongs to specs
            token->cmd = NULL;
            if (token->data) {
                free(token->data);
                token->data = NULL;
            }
        }
        token_len = escpos_vec_token_size(token_list);

    }
    if (token_list->arr) free(token_list->arr);
    token_list->arr = NULL;
}

void escpos_specs_free(escpos_cmd_specs_t *cs)
{
    escpos_ctree_node_t *ct = cs;
    if (ct->cmd) ct->cmd = escpos_cmd_free(ct->cmd);
    escpos_ctree_node_t *current_node, *tmp;
    HASH_ITER(hh, ct->nodes, current_node, tmp) {
        escpos_specs_free(current_node);
        HASH_DEL(ct->nodes, current_node);
        free(current_node);
    }
    ct->nodes_size = 0;
}

