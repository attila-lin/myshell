/*
 * Copyright (c) 2014, Yiyu Lin <linyiyu1992 at gmail dot com>
 * All rights reserved.
 */

#ifndef CMDDEF_H
#define CMDDEF_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "classheader.h"
#include "managerdef.h"
using namespace std;
extern ExecManager execm;

class BaseCmd
{
protected:
  string cname;
  vector<string> vargs;
  string infd;
  string outfd;
  bool isappend;
  bool isextern;
public:
  BaseCmd():isappend(false)
  {
    vargs.reserve(VEC_RESER);
  }

  void SetName(const string *ops)
  {
    if(ops&&!ops->empty())
      cname.assign(*ops);
  }

  void AddArgs(const string *ops)
  {
    if(ops&&!ops->empty())
      vargs.push_back(*ops);
  }

  void SetReDir(const ReDir* ops)
  {   // has name
    if(ops&&!ops->fname.empty())
    {
      switch(ops->type)
      {
        case ReDir::IN:
          infd.assign(ops->fname);
          break;
        case ReDir::OUT:
          outfd.assign(ops->fname);
          isappend=false;
          break;
        case ReDir::APPEND:
          outfd.assign(ops->fname);
          isappend=true;
          break;
        default:
          break;
      }
    }
  }

  char** transcArgs()
  {
    if(cname.empty())
      return NULL;
    else
    {
      char **tmp=new char*[vargs.size()+2];
      tmp[0]=new char[cname.length()+1];
      strcpy(tmp[0],cname.c_str());
      unsigned int i;
      for(i=0;i<vargs.size();i++)
      {
        tmp[i+1]=new char[vargs[i].length()+1];
        strcpy(tmp[i+1],vargs[i].c_str());
      }
      tmp[i+1]=NULL;
      return tmp;
    }
  }

  void freecArgs(char** arglist)
  {
    if(arglist==NULL)
      return;
    else
    {
      while(arglist)
      {
        delete[] (*arglist);
        arglist++;
      }
      delete[] arglist;
    }
  }

  bool RedirectOut()
  {
    if(!outfd.empty())
    {
      int rdfdo;

      if(!isappend)
        rdfdo=open(outfd.c_str(),O_WRONLY|O_CREAT|O_TRUNC);
      else
        rdfdo=open(outfd.c_str(),O_WRONLY|O_CREAT|O_APPEND);

      if(rdfdo==-1)
      {
        cerr<<"error: "<<outfd<<": No such file or directory"<<endl;
        return false;
      }
      else
      {
        if(dup2(rdfdo,STDOUT_FILENO)==-1)
        {
          close(rdfdo);
          return false;
        }
        close(rdfdo);
        chmod(outfd.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        // cout << outfd << endl;
        // chown(outfd.c_str(),0,0);
      }
    }
    return true;
  }

  bool RedirectIn(bool back=false)
  {
    if(!infd.empty())
    {
      int rdfdi;
      if((rdfdi=open(infd.c_str(),O_RDONLY))==-1)
      {
        cerr<<"error: "<<infd<<": No such file or directory"<<endl;
        return false;
      }
      else
      {
        if(dup2(rdfdi,STDIN_FILENO)==-1)
        {
          close(rdfdi);
          return false;
        }
        close(rdfdi);
        // cout << infd << endl;
        // chown(infd.c_str(),0,0);
      }
    }
    else if(back)
    {
      // 无重定向输入时候，而且后台，则屏蔽标准输入
      int rdfdn=open("/dev/null",O_RDONLY);
      dup2(rdfdn,STDIN_FILENO);
      close(rdfdn);
    }
    return true;
  }

  bool IsExtern() const
  {
    return isextern;
  }

  virtual int Execute()=0;

  virtual ~BaseCmd(){}
};

class ExitCmd:public BaseCmd
{
public:
  ExitCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    cerr<<"Goodbye"<<endl;
    return QUIT;
  }
  virtual ~ExitCmd(){}
};

class CdCmd:public BaseCmd
{
public:
  CdCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    if(vargs.empty())
    {
      cerr<<"cd: usage: cd [directory]"<<endl;
      return -1;
    }
    else if(chdir(vargs[0].c_str())==-1)
      cerr<<"error: cd: No such file or directory"<<endl;
    return 0;
  }
  virtual ~CdCmd(){}
};

class JobCmd:public BaseCmd
{
public:
  JobCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    execm.CheckBGTask(true);
    return 0;
  }
  virtual ~JobCmd(){}
};

class KillCmd:public BaseCmd
{
public:
  KillCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    pid_t pid;
    if(vargs.empty())
    {
      cerr<<"error: kill must have arguments"<<endl;
      return -1;
    }
    if((pid=abs(atoi(vargs[0].c_str())))==0)
    {
      cerr<<"error: kill: "<<vargs[0]<<": arguments invalid"<<endl;
      return -1;
    }
    if(kill(pid,SIGTERM) == -1)
    {
      cerr<<"error: kill: "<<pid<<"- No such process"<<endl;
      return -1;
    }
    return 0;
  }
  virtual ~KillCmd(){}
};

class EchoCmd:public BaseCmd
{
public:
  EchoCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    if(vargs.empty())
      cout<<endl;
    else
      cout<<vargs[0]<<endl;
    return 0;
  }
  virtual ~EchoCmd(){}
};

class ExecCmd:public BaseCmd
{
public:
  ExecCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    if(vargs.empty())
      return 0;
    else
    {
      char** largs=transcArgs();
      if(largs==NULL)
        return 0;
      else if(execvp(largs[1],largs+1)==-1)
      {
        cerr<<"error: exec: "<<largs[1]<<" not found"<<endl;
        freecArgs(largs);
        return -1;
      }
      return 0;
    }
  }
  virtual ~ExecCmd(){}
};

class ExternCmd:public BaseCmd
{
public:
  ExternCmd():BaseCmd()
  {
    isextern=true;
  }
  virtual int Execute()
  {
    char** largs=transcArgs();
    if(largs==NULL)
      return 0;
    else if(execvp(largs[0],largs)==-1)
    {
      cerr<<"error: "<<*largs<<" command not found"<<endl;
      freecArgs(largs);
    }
    return 0;
  }
  virtual ~ExternCmd(){}
};

class AliasCmd:public BaseCmd
{
public:
  AliasCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    ExecManager* m = ExecManager::GetInstance();
    if(vargs.empty())
    {
      m->GetAliasMap()->show();
      return -1;
    }
    string f;
    string c;
    for (unsigned int i = 0; i < vargs.size(); ++i)
    {
      f = vargs[i];
      c = f;
      // cout << f << endl;
      // cout << f.find("=") << endl;
      if (f.find("=") != string::npos && f.find("\'") != string::npos && f[f.length()-1] != '\'')
      {
        do{
          i++;
          f = vargs[i];
          c += " " + f;
        }while(f.find("\'") == string::npos);
      }
      vector<string> strings;
      istringstream istr;
      istr.str(c);
      string s;
      // cout << c << endl;

      // split
      while (getline(istr, s, '=')) {
        strings.push_back(s);
      }
      // cout << "size" << strings.size() << endl;
      if(strings.size() == 1){ // show res : alias you= | alias you
        if (c.find('=') != string::npos) // has =
        {
          m->GetAliasMap()->addword(strings[0], "", 1);
        }
        else{ // run the commond
          ExecManager* m = ExecManager::GetInstance();
          string s = m->GetAliasMap()->searchfirst(strings[0]);
          if(s == "")
            cout << "alias: "<< strings[0] << ": not found" << endl;
          else
            cout << "alias " << strings[0] << "=" << s << endl;
        }

      }else{ // if has args
        string tmp = strings[1];
        if(tmp[0] == '\'')
          m->GetAliasMap()->addword(strings[0], tmp, 0);
        else
          m->GetAliasMap()->addword(strings[0], tmp, 1); // need to add ''
      }
    }
    return 0;

  }
  virtual ~AliasCmd(){}
};

class UnaliasCmd:public BaseCmd
{
public:
  UnaliasCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute()
  {
    if(vargs.empty())
    {
      cerr<<"unalias: usage: alise cmd"<<endl;
      return -1;
    }
    else{
      ExecManager* m = ExecManager::GetInstance();
      if (m->GetAliasMap()->searchfirst(vargs[0].c_str()) == "")
      {
        cerr << "unalias: " << vargs[0].c_str() << " not found" << endl;
      }
      else{
        m->GetAliasMap()->delword(vargs[0].c_str());
      }
    }
    return 0;
  }
  virtual ~UnaliasCmd(){}
};

class PwdCmd:public BaseCmd
{
public:
  PwdCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute(){
    char currwd[MAX_CWD_SIZE];
    getcwd(currwd,MAX_CWD_SIZE);
    cout << currwd << endl;
    return 0;
  }
  virtual ~PwdCmd(){}
};

class ExecAliasCmd:public BaseCmd
{
public:
  ExecAliasCmd():BaseCmd()
  {
    isextern=false;
  }
  virtual int Execute(){
    ExecManager* m = ExecManager::GetInstance();
    string s;
    string bef;
    string tmp;
    vector<string> cmd;
    vector<string> strings;
    istringstream istr;

    string stmp = "stmp";
    string btmp = "btmp";

    s = m->GetAliasMap()->search(cname);
    vector<string> befcmd ;
    befcmd.push_back(cname);

    bef = s;

    cmd.insert(cmd.begin(), bef);
    string res = "";
    for (unsigned int i = 0; i < cmd.size(); ++i)
    {
      res += cmd[i] + " ";
    }
    const char * ss = res.c_str();
    // FILE *fd;
  	// fd = fopen(".errorfd", "w+");
  	// fclose(fd);

    int errorfd ;
    errorfd = open("/dev/null", O_WRONLY|O_CREAT|O_TRUNC);

    int s_fd = dup(STDERR_FILENO);

    if(errorfd==-1)
    {
      cerr<<"error: "<<outfd<<": No such file or directory"<<endl;
      return false;
    }
    else
    {
      int n_fd = dup2(errorfd,STDERR_FILENO);
      if(n_fd==-1)
      {
        close(errorfd);
        return false;
      }

      if(!system(ss)){
        close(errorfd);
        return 0;
      }
      else{
        close(errorfd);
        if (dup2(s_fd, n_fd) < 0) {
          printf("err in dup2\n");
        }
        cerr << "error: "<< cname << " command not found" << endl;
        // cerr << "Error command: please unalias it" << endl;
        return -1;
      }
    }
    // remove(".errorfd");
    return 0;
  }
  virtual ~ExecAliasCmd(){}
};

#endif /*CMDDEF_H*/
