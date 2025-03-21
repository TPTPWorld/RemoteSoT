//-------------------------------------------------------------------------------------------------
typedef struct {
//----Options for processing
    int NoHTML;
    int Quietness;
//----What to do
    String SubmitButton;
    String ListStatus;
    String AutoMode;
    int AutoModeSystems;
    String System;
    int TimeLimit;
    String ProblemSource;
    String Problem;
    int Force;
    int TSTPOutput;
} OptionsType;
//-------------------------------------------------------------------------------------------------
#define SYSTEM_ON_TPTP_FORMREPLYURL "https://tptp.org/cgi-bin/SystemOnTPTPFormReply"
// #define SYSTEM_ON_TPTP_FORMREPLYURL "https://tptp.cs.miami.edu/cgi-bin/SystemOnTPTPFormReply"

#define USAGE "Usage: RemoteSoT <options> [<File name>]\n \
    <options> are ...\n \
    -h --help       --help       - print this help\n \
    -w[<status>]    --what       - list available ATP systems\n \
    -q<quietness>   --quiet      - control amount of output\n \
    -t<timelimit>   --time-limit - CPU time limit for system\n \
    -c<automode>    --auto       - one of N, E, S\n \
    -l<syslimit>    --limit      - maximal systems for automode\n \
    -s<system>      --system     - specified system to use\n \
    -f              --force      - force use of inappropriate system\n \
    -S              --tstp       - TSTP format output\n \
    -p<filename>    --problem    - TPTP problem name\n \
    -y<proxy:port>  --proxy      - use this proxy:port\n \
    <File name>                  - if not TPTP problem (-- for stdin)\n"
//-------------------------------------------------------------------------------------------------
