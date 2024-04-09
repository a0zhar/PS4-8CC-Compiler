#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants for register mapping
const char *reg_map[] = {
    [0] = "rax",
    [1] = "rcx",
    [2] = "r10",
    [3] = "rdi",
    [4] = "r8"
};

// Constants for conditions
const char *conds[] = { "eq", "ne", "lt", "le", "gt", "ge" };

// Function to generate labels
static int static_cntr = 0;
char *make_label() {
    static char ans[16];
    snprintf(ans, sizeof(ans), "L%d", static_cntr++);
    return ans;
}

// Function to print warnings
void warn(char *check_env, char *msg) {
    if (check_env && getenv(check_env) == NULL) return;
    fprintf(stderr, "s2rop: warning: %s\n", msg);
}

// Function to perform register exchange
void do_exchange_regs(char *mapping[]) {
    if (mapping == NULL) return;
    if (strcmp(mapping[0], "rax") == 0 && strchr(mapping[0], ' ') == NULL) {
        printf("mov rax, %s\n", mapping[0]);
        return;
    }
    if (mapping[0] == NULL) mapping[0] = "rax";
    printf("# do_exchange_regs: %s\n", mapping[0]);
    char *labels_dst[5];

    for (int i = 0; i < 5; i++) labels_dst[i] = make_label();

    if (strcmp(mapping[0], "rax") == 0) {
        printf("pop rsi\n");
        printf("dp %s\n", labels_dst[0]);
        printf("mov [rsi], rax\n");
    }
  
    char *cur_rax = "rax";
    for (int i = 0; i < 5; i++) {
        if (mapping[i] != NULL &&
            strcmp(mapping[i], "rax") != 0 &&
            strchr(mapping[i], ' ') == NULL &&
            strcmp(mapping[i], "rax") != 0) {

            cur_rax = mapping[i];
            printf("mov rax, %s\n", mapping[i]);
            printf("pop rsi\n");
            printf("dp %s\n", labels_dst[i]);
            printf("mov [rsi], rax\n");
        }
    }
    if (mapping[0] != NULL && strchr(mapping[0], ' ') == NULL &&
        strcmp(mapping[0], "rax") != 0 &&
        strcmp(mapping[0], cur_rax) != 0 &&
        strchr(mapping[4], 'r11') == NULL) {

        printf("mov rax, %s\n", mapping[0]);
    }
    if (strchr(mapping[4], 'r11') != NULL) {
        printf("pop r11 ; mov rax, rdi\n");
        printf("%s:\n", labels_dst[4]);
        if (strchr(mapping[4], ' ') != NULL) printf("%s\n", mapping[4]);
        else printf("dq 0\n");
    }
    for (int i = 0; i < 5; i++) {
        if (mapping[i] != NULL && strchr(mapping[i], ' ') == NULL && strcmp(reg_map[i], "rsp") != 0 && strchr(mapping[i], 'r11') == NULL) {
            printf("pop %s\n", reg_map[i]);
            printf("%s:\n", labels_dst[i]);
            if (strchr(mapping[i], ' ') != NULL) printf("%s\n", mapping[i]);
            else printf("dq 0\n");
        }
    }
    if (strchr(mapping[3], 'rsp') != NULL) {
        printf("pop rsp\n");
        printf("%s:\n", labels_dst[3]);
        if (strchr(mapping[3], ' ') != NULL) printf("%s\n", mapping[3]);
        else printf("dq 0\n");
    }
}

// Function to exchange registers
void exchange_regs(char *mapping[]) {
    static char *cur_exchange[5];
    if (mapping == NULL) {
        do_exchange_regs(cur_exchange);
        memset(cur_exchange, 0, sizeof(cur_exchange));
        return;
    }
    char *new_cur[5];
    for (int i = 0; i < 5; i++) {
        if (cur_exchange[i] == NULL) cur_exchange[i] = mapping[i];
        char *v = cur_exchange[i];
        if (mapping[i] != NULL) v = mapping[i];
        if (v == NULL) continue;
        if (strcmp(v, cur_exchange[i]) != 0) new_cur[i] = v;
    }
    printf("# exchange_regs %s + %s\n", cur_exchange[0], mapping[0]);
    memset(cur_exchange, 0, sizeof(cur_exchange));
    for (int i = 0; i < 5; i++) {
        if (new_cur[i] != NULL) cur_exchange[i] = new_cur[i];
    }
}

// Function to emit instructions
void emit_instr(char *args) {
    printf("%s\n", args);
}

// Function to emit move instructions
void emit_mov(char *reg_dst, char *reg_src) {
    if (strcmp(reg_dst, "rax") == 0 && strchr(reg_dst, ' ') == NULL) {
        if (strcmp(reg_src, "rax") != 0) {
            printf("mov rax, %s\n", reg_src);
        }
    } else {
        if (strcmp(reg_src, "rax") != 0) {
            warn("CHECK_FALLBACK", "mov %s, %s: fallback", reg_dst, reg_src);
        }
        char *mapping[5] = { reg_dst, reg_src };
        exchange_regs(mapping);
    }
}


//
// TODO FINISH THESE FUNCTIONS
//
void emit_binary_op(char *instr, char *reg_dst, char *reg_src, char *m1[], char *m2[]);
void emit_unary_op(char *instr, char *reg);
void emit_load_imm(char *reg, char *imm);
void emit_binary_op_imm(char *instr, char *reg, char *imm);
void emit_jump_imm(char *dst);
void emit_jump_reg(char *src);
void emit_logic(char *opcode, char *a, char *b);
void emit_logic_imm(char *opcode, char *a, char *imm);
void emit_condjump(char *opcode, char *dst, char *a, char *b, int imm);
char *format_imm(char *imm);
void emit_nativecall(char *lbl);
int main();
