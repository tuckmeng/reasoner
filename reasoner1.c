#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_KNOWLEDGE 500
#define MAX_WORD 50
#define MAX_LINE 256

typedef enum {
    VERB,
    HAVE,
    ARE
} RelationType;

typedef struct {
    char subject[MAX_WORD];
    char predicate[MAX_WORD];
    RelationType type;
} Fact;

Fact knowledge[MAX_KNOWLEDGE];
int fact_count = 0;

// Lowercase utility
void to_lowercase(char *s) {
    for (int i = 0; s[i]; i++)
        s[i] = tolower((unsigned char)s[i]);
}

// Improved trim punctuation and newline
void trim_punct(char *s) {
    // Remove trailing newline and carriage return first
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
    // Then remove trailing '.' or '?'
    if (len > 0 && (s[len - 1] == '.' || s[len - 1] == '?')) {
        s[len - 1] = '\0';
    }
}

// Add a fact to knowledge base
void add_fact(const char *subject, const char *predicate, RelationType type) {
    if (fact_count >= MAX_KNOWLEDGE) return;
    strcpy(knowledge[fact_count].subject, subject);
    strcpy(knowledge[fact_count].predicate, predicate);
    knowledge[fact_count].type = type;
    fact_count++;
}

// Check if any fact exists about the subject
int subject_known(const char *subject) {
    for (int i = 0; i < fact_count; i++) {
        if (strcmp(knowledge[i].subject, subject) == 0) {
            return 1;
        }
    }
    return 0;
}

// Check if any fact exists about the predicate/property (for given type)
int predicate_known(const char *predicate, RelationType type) {
    for (int i = 0; i < fact_count; i++) {
        if (strcmp(knowledge[i].predicate, predicate) == 0 &&
            knowledge[i].type == type) {
            return 1;
        }
    }
    return 0;
}

// Direct match check for a fact
int match_query(const char *subject, const char *predicate, RelationType type) {
    for (int i = 0; i < fact_count; i++) {
        if (strcmp(knowledge[i].subject, subject) == 0 &&
            strcmp(knowledge[i].predicate, predicate) == 0 &&
            knowledge[i].type == type) {
            return 1;
        }
    }
    return 0;
}

// Recursive inheritance check:
// Returns 1 if subject has predicate of given type, either directly or by inheritance
int inherits(const char *subject, const char *predicate, RelationType type) {
    // Direct match?
    if (match_query(subject, predicate, type)) {
        return 1;
    }

    // Recurse over all "are" parents
    for (int i = 0; i < fact_count; i++) {
        if (knowledge[i].type == ARE && strcmp(knowledge[i].subject, subject) == 0) {
            if (inherits(knowledge[i].predicate, predicate, type)) {
                return 1;
            }
        }
    }
    return 0;
}

// Parse a fact line
int parse_fact_line(char *line) {
    to_lowercase(line);
    trim_punct(line);

    char subject[MAX_WORD], predicate[MAX_WORD];

    if (sscanf(line, "all %s have %s", subject, predicate) == 2) {
        add_fact(subject, predicate, HAVE);
        return 1;
    }
    if (sscanf(line, "all %s are %s", subject, predicate) == 2) {
        add_fact(subject, predicate, ARE);
        return 1;
    }
    if (sscanf(line, "all %s %s", subject, predicate) == 2) {
        add_fact(subject, predicate, VERB);
        return 1;
    }

    return 0;
}

// Handle a query line
int handle_query(char *line) {
    to_lowercase(line);
    trim_punct(line);

    char subject[MAX_WORD], predicate[MAX_WORD];

    // Handle "Do X have Y?"
    if (strncmp(line, "do ", 3) == 0) {
        // Check if "do X have Y?"
        if (sscanf(line, "do %s have %s", subject, predicate) == 2) {
            if (!subject_known(subject)) {
                printf("No information about %s.\n", subject);
                return 1;
            }
            if (!predicate_known(predicate, HAVE)) {
                printf("No information about property '%s'.\n", predicate);
                return 1;
            }
            if (inherits(subject, predicate, HAVE)) {
                printf("Yes, %s have %s.\n", subject, predicate);
            } else {
                printf("No, %s do not have %s.\n", subject, predicate);
            }
            return 1;
        }
        // Otherwise "Do X <verb>?"
        if (sscanf(line, "do %s %s", subject, predicate) == 2) {
            if (!subject_known(subject)) {
                printf("No information about %s.\n", subject);
                return 1;
            }
            if (!predicate_known(predicate, VERB)) {
                printf("No information about verb '%s'.\n", predicate);
                return 1;
            }
            if (inherits(subject, predicate, VERB)) {
                printf("Yes, %s %s.\n", subject, predicate);
            } else {
                printf("No, %s do not %s.\n", subject, predicate);
            }
            return 1;
        }
    }

    // Handle "Are X Y?"
    if (sscanf(line, "are %s %s", subject, predicate) == 2) {
        if (!subject_known(subject)) {
            printf("No information about %s.\n", subject);
            return 1;
        }
        if (!predicate_known(predicate, ARE)) {
            printf("No information about noun '%s'.\n", predicate);
            return 1;
        }
        if (inherits(subject, predicate, ARE)) {
            printf("Yes, %s are %s.\n", subject, predicate);
        } else {
            printf("No, %s are not %s.\n", subject, predicate);
        }
        return 1;
    }

    printf("Unrecognized query format: %s\n", line);
    return 0;
}

// Print help message
void print_help(const char *program_name) {
    printf("Usage:\n");
    printf("  %s [statements and/or queries...]\n", program_name);
    printf("  %s --file <filename>\n", program_name);
    printf("  %s --help\n", program_name);
    printf("\nAccepted statement formats:\n");
    printf("  All X <verb>.\n");
    printf("  All X have <property>.\n");
    printf("  All X are <noun>.\n");
    printf("\nAccepted query formats:\n");
    printf("  Do X <verb>?\n");
    printf("  Do X have <property>?\n");
    printf("  Are X <noun>?\n");
    printf("\nExamples:\n");
    printf("  %s \"All cats meow.\" \"All cats have fur.\" \"Do cats meow?\" \"Do cats have fur?\"\n", program_name);
    printf("  %s --file input.txt\n", program_name);
}

// Read lines from a file
void process_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return;
    }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        if (!parse_fact_line(line)) {
            handle_query(line); // Try as query
        }
    }
    fclose(fp);
}

// Main function processes command line args
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--file") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: --file requires a filename\n");
            return 1;
        }
        process_file(argv[2]);
        return 0;
    }

    // Treat all other arguments as statements or queries
    for (int i = 1; i < argc; i++) {
        if (!parse_fact_line(argv[i])) {
            handle_query(argv[i]); // Try as query
        }
    }

    return 0;
}
