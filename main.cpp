#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MaxLineChar  1024
#define MaxLineNum 1024

struct analysis {
    char **source;
    int line_num;
    int char_num;
    int *error;
    int errocount;

    int numcount;
    int id;
    int keyword;
    int relop; //运算符
    int equal; //赋值
    int period; //标点
    int note; //注释
    int string;
    //token **token;
};

char *tar_addr = "main.c";
char *out_addr = "main.out";


char *keyword[] = {"double", "int", "struct", "break", "else", "long", "switch", "case", "enum", "typedef", "char",
                   "return", "const", "float", "unsigned", "continue", "for", "void", "default", "goto", "if", "while",
                   "do", "static"};

analysis *openfile(char *filename) {
    FILE *fp = NULL;
    char *strline = NULL;
    char **res = new char *[MaxLineNum];
    analysis *out = new analysis();
    out->char_num = 0;
    out->line_num = 0;
    out->numcount = 0;
    out->id = 0;
    out->keyword = 0;

    out->relop = 0;
    out->equal = 0;
    out->period = 0;
    out->note = 0;
    out->string = 0;
    out->source = NULL;
    if ((fp = fopen(filename, "r")) == NULL) {
        printf("open failed!\n");
        system("exit");
    }
    for (out->line_num = 0; !feof(fp); out->line_num++) {
        strline = new char[MaxLineChar];
        fgets(strline, 1024, fp);
        res[out->line_num] = strline;
        out->char_num += strlen(strline);
    }
    out->source = res;
    return out;
}


int Lexical_analysis(analysis *out) {
    out->error = new int[out->line_num];
    out->errocount = 0;
    char **source = out->source;
    for (int i = 0; i < out->line_num; i++) {
        int line_char = strlen(source[i]);
        char *line = source[i];
        for (int j = 0; j < line_char; j++) {
            start:
            if (line[j] == '\n' || j >= line_char || line[j] == '\0' || line[j] == 13)
                break;
            else if (line[j] == ' ' || line[j] == '\t')
                continue;
            else if (line[j] == '/') { //进入注释
                j++;
                if (line[j] == '/') {
                    printf("进入单行注释%d,%d\n", i, j);
                    out->note++;
                    break;
                } else if (line[j] == '*') {
                    printf("进入多行注释%d,%d\n", i, j);
                    for (; i < out->line_num; i++) {
                        printf("跳过%d\n", i);
                        char *line = source[i];
                        int line_char = strlen(source[i]);
                        while (line[++j] != '\n' && j < line_char) {
                            if (line[j] == '*' && line[j + 1] == '/') {
                                printf("结束多行注释%d,%d\n", i, j);
                                j++;
                                out->note++;
                                goto start;
                            }
                        }
                        j = -1;

                    }
                    printf("错误:%d行%d列%c(多行注释不配对)\n", i, j, line[j]);
                    out->error[out->errocount] = i;
                    out->errocount++;
                    break;
                } else {
                    printf("错误:%d行%d列%c\n", i, j, line[j]);
                    out->error[out->errocount] = i;
                    out->errocount++;
                    break;
                }
            } else if (isalpha(line[j])) { //进入标识符
                int k = j;
                bool isId = false;
                while (isalnum(line[k]) && k < line_char) {
                    if (isdigit(line[k]))
                        isId = true;
                    k++;
                }
                char dest[10];
                strncpy(dest, line + j, k - j);
                dest[k - j] = '\0';
                if (isId) { //是id
                    out->id++;
                    printf("out:{id:%s},r:%d\n", dest, i);
                } else {
                    isId = true;
                    for (int x = 0; x < sizeof(keyword) / sizeof(char *); x++) {
                        if (strcmp(keyword[x], dest) == 0) {
                            out->keyword++;
                            printf("out:{keyword:%s},r:%d\n", dest, i);
                            isId = false;
                        }
                    }
                    if (isId) {
                        out->id++;
                        printf("out:{id:%s},r:%d\n", dest, i);
                    }
                }
                j = k - 1;
            } else if (isdigit(line[j])) { //进入num
                int k = j;
                while (isdigit(line[k]) && k < line_char)k++;
                if (isalpha(line[k])) {
                    printf("错误:%d行%d列%c,标识符以数字开头\n", i, j, line[j]);
                    out->error[out->errocount] = i;
                    out->errocount++;
                    break;
                } else {
                    char dest[10];
                    strncpy(dest, line + j, k - j);
                    dest[k - j] = '\0';
                    printf("out:{num:%s},r:%d\n", dest, i);
                    j = k - 1;
                    out->numcount++;
                }

            } else if (ispunct(line[j])) { //进入标点符号
                if (line[j] == '<' || line[j] == '>') {
                    if (line[j + 1] == '=') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        j++;
                    } else
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                    out->relop++;
                    continue;
                }
                if (line[j] == '!') {
                    if (line[j + 1] == '=') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        j++;
                    } else
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                    out->relop++;
                    continue;
                }
                if (line[j] == '&') {
                    if (line[j + 1] == '&') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        j++;
                    } else
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                    out->relop++;
                    continue;
                }
                if (line[j] == '|') {
                    if (line[j + 1] == '|') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        j++;
                    } else
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                    out->relop++;
                    continue;
                }
                if (line[j] == '=') {
                    if (line[j + 1] == '=') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->relop++;
                        j++;
                    } else {
                        printf("out:{equal:%c},r:%d\n", line[j], i);
                        out->equal++;
                    }
                    continue;
                }
                if (line[j] == '+') {
                    if (line[j + 1] == '=') {
                        printf("out:{equal:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->equal++;
                        j++;
                    } else if (line[j + 1] == '+') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->equal++;
                        j++;
                    } else {
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                        out->relop++;
                    }
                    continue;
                }
                if (line[j] == '-') {
                    if (line[j + 1] == '>') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->relop++;
                        j++;
                    }
                    if (line[j + 1] == '=') {
                        printf("out:{equal:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->equal++;
                        j++;
                    } else if (line[j + 1] == '-') {
                        printf("out:{relop:%c%c},r:%d\n", line[j], line[j + 1], i);
                        out->equal++;
                        j++;
                    } else {
                        printf("out:{relop:%c},r:%d\n", line[j], i);
                        out->relop++;
                    }
                    continue;
                }
                if (line[j] == '"') {
                    int k = j + 1;
                    while (line[k] != '"' && k < line_char)k++;
                    if (line[k] != '"') {
                        printf("错误:%d行%d列%c,字符串不匹配\n", i, j, line[j]);
                        out->error[out->errocount] = i;
                        out->errocount++;
                        break;
                    } else {
                        char dest[50];
                        strncpy(dest, line + j, k - j + 1);
                        dest[k - j + 1] = '\0';
                        printf("out:{string:%s},r:%d\n", dest, i);
                        j = k + 1;
                        out->string++;
                        continue;
                    }
                }
                printf("out:{period:%c},r:%d\n", line[j], i);
                out->period++;


            } else { //报错
                printf("未知字符,错误:%d行%d列%d\n", i, j, line[j]);
                out->error[out->errocount] = i;
                out->errocount++;
                break;
            }
        }
    }
    return 0;
}

int main(int argv, char *argc[]) {
    if (argv >= 2) {
        tar_addr = argc[1];
    }
    analysis *out = openfile(tar_addr);
    Lexical_analysis(out);
    printf("count: erro:%d,keyword:%d,id:%d,relop:%d,equal:%d\nperiod:%d,string:%d,note:%d,num:%d\n", out->errocount,
           out->keyword, out->id, out->relop, out->equal, out->period, out->string, out->note, out->numcount);
    printf("total line %d , total char %d\n", out->line_num, out->char_num);
    //system("pause");
}