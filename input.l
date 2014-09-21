%{  // define include
#include<iostream>
#include<string>
#include<vector>
#include<readline/readline.h>
#include<readline/history.h>
using namespace std;
#include"classheader.h"
#include"cmdparser.tab.h"
static int get_input(char *buf, int size);

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = get_input(buf, max_size);
%}

%%

[|<>&;\n]       {   return *yytext; }
[^|<>&; \t\n]+  {   yylval.sWord=new string(yytext);
                    return WORD;}
[ \t]           {};

%%

void GetPrompt(string& opd)
{
  opd.assign("SHELL@:");
  char currwd[MAX_CWD_SIZE];
  getcwd(currwd,MAX_CWD_SIZE);
  opd.append(currwd).append(" $ ");
}
static int get_input(char *buf, int size)
{
  //自己定义input函数，采用支持编辑的readline库
  char *line;
  int len=0;
  if (feof(yyin)){
    return YY_NULL;
  }

  // the remind string
  string prompt;
  GetPrompt(prompt);
  line = readline(prompt.c_str());
  if (!line){
  	// printf("EOF");
  	// strcpy(buf, line);
  	strcpy(buf, "exit\n");
  	free(line);
    // return YY_NULL;
    return strlen(buf);
  }

  strcpy(buf, line);
  //至少留下两个字符，一个存放\n，一个为NULL
  strncpy(buf,line,size-2);
  len=strlen(line);
  len=(len)>(size-2)?(size-2):len;
  buf[len]='\0';
  add_history(buf);
  //加上换行之前加入历史
  //strcat(buf, "\n");
  buf[len]='\n';
  buf[len+1]='\0';
  free(line);
  // printf("%s",buf);
  return strlen(buf);
}
int yywrap()
{
  return 1;
}
