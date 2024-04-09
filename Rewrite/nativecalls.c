#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LABELS 100

// Helper
char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t length = getline(&line, &bufsize, stdin);
    if (length == -1) {
        free(line);
        return NULL;
    }
    if (line[length - 1] == '\n')
        line[length - 1] = '\0';
    return line;
}

int main() {
    char *lines[MAX_LINES];
    char *labels[MAX_LABELS];
    int num_labels = 0;

    // Read lines until End of file?
    int i = 0;
    while (1) {
        char *line = read_line();
        if (!line)
            break;
        lines[i++] = line;
    }
    lines[i++] = ":"; // Append sentinel

    // Extract labels
    for (int j = 0; j < i; j++) {
        char *line = lines[j];
        if (line[strlen(line) - 1] == ':' && line[0] != '.') {
            labels[num_labels++] = strdup(line);
        }
    }

    // Add predefined labels
    labels[num_labels++] = "A";
    labels[num_labels++] = "B";
    labels[num_labels++] = "C";
    labels[num_labels++] = "SP";
    labels[num_labels++] = "BP";

    char *new_lines[MAX_LINES];
    int new_lines_count = 0;

    // Process lines
    int j = 0;
    for (int k = 0; k < i; k++) {
        char *line = lines[k];
        char *stripped_line = strdup(line);
        char *ptr = strtok(stripped_line, " \t");
        if (ptr && ptr[strlen(ptr) - 1] == ':' && ptr[0] != '.') {
            // Label line
            char *ncalls[MAX_LABELS];
            int num_ncalls = 0;

            for (int ii = j; ii < new_lines_count; ii++) {
                char *ll = new_lines[ii];
                char *trimmed_ll = strdup(ll);
                char *token = strtok(trimmed_ll, " \t");
                if (token && strcmp(token, "jmp") == 0) {
                    char *jmp_target = strtok(NULL, " \t");
                    if (jmp_target && strncmp(jmp_target, ".", 1) != 0) {
                        ncalls[num_ncalls++] = strdup(jmp_target);
                        sprintf(ll, "jmp ._native_%s", jmp_target);
                    }
                }
                free(trimmed_ll);
            }
            for (int x = 0; x < num_ncalls; x++) {
                new_lines[new_lines_count++] = strdup(ncalls[x]);
            }
            j = new_lines_count;
        }
        new_lines[new_lines_count++] = strdup(line);
        free(stripped_line);
    }

    // Output lines
    for (int k = 0; k < new_lines_count - 1; k++) {
        printf("%s\n", new_lines[k]);
        free(new_lines[k]);
    }

    // Free memory
    for (int k = 0; k < num_labels; k++) {
        free(labels[k]);
    }

    return 0;
}
