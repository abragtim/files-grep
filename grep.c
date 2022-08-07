/* ----------------------------------------------------------------------------
 * BAB36PRGA - Programování v C
 * HW4 - Hledání textu v souborech
 * Uploaded at 8.4.2022 13:51
 * Score: 6/6
 * ------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ARG_NUM 2
#define READING_MODE "r"
#define SIZE_INIT 10
#define START_CHAR 'a'
#define WRONG_LEN -1000
#define SIZE_OF_COLOR_ALWAYS 15
#define SIZE_OF_REGULAR_MODE 3

#define RED_BEFORE "\033[01;31m\033[K"
#define RED_AFTER "\033[m\033[K"

#define bool _Bool

char *readline(FILE *file, int *line_len);
int mystrlen(char *string);
bool find_pattern(char *string, int sting_len, char *pattern, bool regular_mode);
void printline(char *line, int line_len, bool colored_mode, char *pattern);
bool compare_strings(char *string1, char *string2, int size);
int doubling(char *string, char letter, int start_iteration_point);

int main(int argc, char *argv[])
{
    int ret = EXIT_FAILURE;
    if (argc < ARG_NUM)
    {
        fprintf(stderr,
                "Error: enter arguments './grep [PATTERN] [FILE]\n");
        ret = EXIT_FAILURE;
        return ret;
    }
    char *pattern = argv[argc - 2];
    FILE *file;
    file = fopen(argv[argc - 1], READING_MODE);
    if (file == NULL)
    {
        if (argv[2] == NULL)
        {
            file = stdin;
            pattern = argv[argc - 1];
        }
        else
        {
            fprintf(stderr, "Error: can't open file\n");
            ret = EXIT_FAILURE;
            return ret;
        }
    }
    bool regular_mode = false;
    bool colored_mode = false;
    if ((argc > 2) &&
        compare_strings(argv[argc - 3], "--color=always", SIZE_OF_COLOR_ALWAYS))
    {
        colored_mode = true;
    }
    if ((argc > 2) &&
        compare_strings(argv[argc - 3], "-E", SIZE_OF_REGULAR_MODE))
    {
        regular_mode = true;
    }
    int line_len;
    char *line = readline(file, &line_len);
    if (line == NULL)
    {
        fclose(file);
        return EXIT_FAILURE;
    }
    if (line)

        while (line[0] != EOF)
        {
            if (find_pattern(line, line_len, pattern, regular_mode))
            {
                printline(line, line_len, colored_mode, pattern);
                ret = EXIT_SUCCESS;
            }
            free(line);
            line = NULL;
            line_len = 0;
            line = readline(file, &line_len); // new line for next cykle
            if (line == NULL)
            {
                fclose(file);
                return EXIT_FAILURE;
            }
        }

    free(line);
    fclose(file);
    return ret;
}

char *readline(FILE *file, int *line_len)
{
    *line_len = SIZE_INIT;
    char *line = (char *)malloc(sizeof(char) * (*line_len));
    if (line == NULL)
    {
        fprintf(stderr, "Error: allocation fault\n");
        return line;
    }

    int counter = 0; // iteration counting
    char letter = START_CHAR;
    while (letter != '\0')
    {
        if (counter == *line_len)
        {
            *line_len = *line_len * 2;
            char *tmp = (char *)realloc(line, sizeof(char) * (*line_len));
            if (tmp == NULL)
            {
                fprintf(stderr, "Error: reallocation fault\n");
                return NULL;
            }
            line = tmp;
        }
        letter = getc(file);
        if (letter == EOF)
        {
            line[0] = letter;
            return line;
        }
        if (letter == '\n')
        {
            letter = '\0';
        }
        line[counter++] = letter;
    }
    *line_len = counter;
    if (*line_len == 0) // empty line
    {
        char *tmp = (char *)realloc(line, sizeof(char));
        if (tmp == NULL)
        {
            fprintf(stderr, "Error: reallocation fault\n");
            return NULL;
        }
        line = tmp;
        line[0] = '\0';
        return line;
    }
    char *tmp = (char *)realloc(line, sizeof(char) * (*line_len));
    if (tmp == NULL)
    {
        fprintf(stderr, "Error: reallocation fault\n");
        return NULL;
    }
    line = tmp;

    return line; // dyn. allocated line
}

int mystrlen(char *string)
{
    int count = 0;
    char letter = START_CHAR;
    while (letter != '\0')
    {
        letter = string[count++];
        if (letter == EOF)
        {
            fprintf(stderr, "Error: EOF in strlen() function.\n");
            return WRONG_LEN;
        }
        if (letter == '\n')
        {
            fprintf(stderr, "Error: \\n in strlen() function.\n");
            return WRONG_LEN;
        }
    }

    return count; // staring from 1.
}

bool find_pattern(char *string, int string_len, char *pattern, bool regular)
{
    bool ret = false;
    int pattern_len = mystrlen(pattern);
    int regular_extra = 0;

    int i = 0; // match index for pattern
    int s = 0;
    for (int c = 0; c < string_len;)
    {
        if (regular && (pattern[i] == '?' || pattern[i] == '*' || pattern[i] == '+'))
        { // if regular == '?' '*' '+'?
            if (pattern[i + 1] == '\0') // type: colou?
            {
                ret = true;
                break;
            }
            else
            {
                ++i;
            }
        }
        if (!i)
        {
            s = c;
        }
        if (i == pattern_len - 1 - regular_extra)
        {
            ret = true;
            break;
        }
        if (string[s] == pattern[i])
        {
            if (s == -1)
            {
                s = c;
            }
            ++i;
            ++s;
            int doubled;
            if (regular && (pattern[i + 1] == '*') &&
                (doubled = doubling(string, pattern[i], s))) // means more
            {
                s = s + doubled;
            }
            if (regular && (pattern[i + 1] == '+') &&
                (doubled = doubling(string, pattern[i], s))) // means more
            {
                s = s + doubled - 1; // -1 because of ++s on 217 line;
            }
        }
        else
        {
            if (regular && (pattern[i + 1] == '?') &&
                (pattern[i + 2] == string[s]))
            {
                ++i;
                ++s;
                ++regular_extra;
            }
            else if (regular && (pattern[i + 1] == '*') &&
                     (pattern[i + 2] == string[s])) // means zero
            {
                ++i;
                ++s;
                ++regular_extra;
            }
            else
            {
                i = 0;
                ++c;
            }
        }
    }

    return ret;
}

void printline(char *line, int line_len, bool colored_mode, char *pattern)
{
    bool colorful = false;
    int colorful_mana;
    int pattern_len;
    if (colored_mode)
    {
        pattern_len = mystrlen(pattern);
    }
    line_len = line_len - 1;
    for (int i = 0; i < line_len; i++)
    {
        char letter = line[i];
        if (colored_mode && (letter == pattern[0]) && colorful == false)
        {
            for (int j = 0; j < pattern_len; ++j)
            {
                if (line[i + j] != pattern[j])
                {
                    break;
                }
            }
            colorful = true;
            colorful_mana = pattern_len - 1;
        }
        if (colored_mode && colorful && colorful_mana)
        {
            if (colorful_mana == pattern_len - 1)
            {
                printf(RED_BEFORE);
            }
            printf("%c", letter);
            colorful_mana--;
            if (!colorful_mana)
            {
                colorful = false;
                printf(RED_AFTER);
            }
        }
        else
        {
            printf("%c", letter);
        }
    }
    putchar('\n');
}

bool compare_strings(char *string1, char *string2, int size)
{
    bool ret;
    if (mystrlen(string1) != size)
    {
        ret = false;
        return ret;
    }
    for (int i = 0; i < size; ++i) // comparing each letter
    {
        if (string1[i] != string2[i])
        {
            ret = false;
            return ret;
        }
    }

    ret = true;
    return ret;
}

int doubling(char *string, char letter, int s)
{
    int counter = 0;
    while ((string[s] == letter) || string[s] == '\0')
    {
        ++s;
        counter++;
    }

    return counter;
}