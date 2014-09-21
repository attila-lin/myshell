/*
 * Copyright (c) 2014, Yiyu Lin <linyiyu1992 at gmail dot com>
 * All rights reserved.
 */

#pragma once
#include <string>
#define   QUIT 9
#define   VEC_RESER 100
#define   MAX_CWD_SIZE 256

struct ReDir
{
  enum {IN,OUT,APPEND}type; // type
  std::string fname;        // name
};

class BaseCmd;
class PipeLine;
class CmdList;
class ExitCmd;
class CdCmd;
class JobCmd;
class KillCmd;
class EchoCmd;
class ExecCmd;
class AliasCmd;
class UnaliasCmd;
class PwdCmd;
class ExternCmd;
class ExecAliasCmd;


