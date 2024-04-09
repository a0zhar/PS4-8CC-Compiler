#include <stdio.h>

int gid = 0;

void emit_text(char *label) {
    printf("global %s\n", label);
    printf("%s\n", label);
    printf("section .text\n");
    printf("global text_%s\n", label);
    printf("text_%s\n", label);
    printf("section .data\n");
}

void emit_gadget(char *instruction) {
    printf("dq gadget_%d\n", gid);
    printf("section .text\n");
    printf("gadget_%d:\n", gid);
    printf("%s\n", instruction);
    printf("ret\n");
    printf("section .data\n");
    gid++;
}

// TODO: Implement this function?
void emit_data(char *label){}

int main() {
    // Print .text section header
    printf("section .text\n");
    printf("use64\n");
    printf("global main\n");
    printf("main:\n");
    printf("mov rsp, rop_start\n");
    printf("ret\n");
    printf("extern exit\n");

    // Print .data section header
    printf("section .data\n");
    printf("align 8\n");
    printf("rop_start:\n");

    // Process input lines
    char line[1024];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        // Remove comments
        char *comment_pos = strchr(line, '#');
        if (comment_pos != NULL) {
            *comment_pos = '\0';
        }
        // Trim leading and trailing whitespace
        char *trimmed_line = line;
        while (*trimmed_line == ' ' || *trimmed_line == '\t') {
            trimmed_line++;
        }
        char *end_ptr = trimmed_line + strlen(trimmed_line) - 1;
        while (end_ptr > trimmed_line && (*end_ptr == ' ' || *end_ptr == '\t' || *end_ptr == '\n' || *end_ptr == '\r')) {
            *end_ptr = '\0';
            end_ptr--;
        }

        // Skip empty lines
        if (strlen(trimmed_line) == 0) {
            continue;
        }

        // Process line content
        if (strncmp(trimmed_line, "$pivot_addr", 11) == 0) {
            printf("mov rsp, [rdi+0x38] ; pop rdi\n");
        } else if (strncmp(trimmed_line, "$jop_frame_addr", 15) == 0) {
            printf("push rbp ; mov rbp, rsp ; mov rax, [rdi] ; call [rax]\n");
        } else if (trimmed_line[0] == '$') {
            if (strstr(trimmed_line, "_addr") != NULL) {
                printf("extern %s\n", trimmed_line + 1);
                printf("dq %s\n", trimmed_line + 1);
            } else {
                printf("%s\n", trimmed_line + 1);
            }
        } else if (trimmed_line[strlen(trimmed_line) - 1] == ':') {
            emit_text(trimmed_line);
        } else if (strncmp(trimmed_line, "db ", 3) == 0 || strncmp(trimmed_line, "dq ", 3) == 0) {
            printf("%s\n", trimmed_line);
        } else if (strncmp(trimmed_line, "dp ", 3) == 0) {
            printf("dq %s\n", trimmed_line + 3);
        } else {
            emit_gadget(trimmed_line);
        }
    }

    return 0;
}
