//-------------------------------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>

#include "DataTypes.h"
#include "Utilities.h"
#include "FileUtilities.h"
#include "PrintTSTP.h"
#include "SystemOnTPTP.h"

#include "RemoteSoT.h"
//-------------------------------------------------------------------------------------------------
int ProcessCommandLine(int argc,char * argv[],OptionsType * OptionValues) {

    int OptionChar;

//----Options for processing
    OptionValues->NoHTML = 1;
    OptionValues->Quietness = 1;
//----What to do
    strcpy(OptionValues->SubmitButton,"RunParallel");
    strcpy(OptionValues->AutoMode,"-cE");
    OptionValues->AutoModeSystems = 3;
    OptionValues->TimeLimit = 60;
    strcpy(OptionValues->ProblemSource,"UPLOAD");
    strcpy(OptionValues->Problem,"--");
    OptionValues->Force = 0;
    OptionValues->TSTPOutput = 0;

    while ((OptionChar = getopt(argc,argv,"hq:ws:t:p:fS")) != -1) {
        switch (OptionChar) {
//----Options for processing
            case 'h':
                printf("%s",USAGE);
                exit(EXIT_SUCCESS);
                break;
//----URL parameters
            case 'q':
                OptionValues->Quietness = atoi(optarg);
                break;
            case 'w':
                strcpy(OptionValues->SubmitButton,"ListSystems");
                strcpy(OptionValues->ListStatus,"READY");
                break;
            case 's':
                strcpy(OptionValues->SubmitButton,"RunSelectedSystems");
                sprintf(OptionValues->System,"%s",optarg);
                break;
            case 't':
                OptionValues->TimeLimit = atoi(optarg);
                break;
            case 'p':
                strcpy(OptionValues->ProblemSource,"TPTP");
                strcpy(OptionValues->Problem,optarg);
                break;
            case 'f':
                OptionValues->Force = 1;
                break;
            case 'S':
                OptionValues->TSTPOutput = 1;
                break;
            default:
                printf("ERROR: Something wrong in getopt\n");
                exit(EXIT_FAILURE);
                break;
        }
        if (argv[optind] != NULL) {
            strcpy(OptionValues->Problem,argv[optind]);
        }
    }
    return(1);
}
//-------------------------------------------------------------------------------------------------
void URLEncodeAndExtendString(OptionsType OptionValues,char ** URLParameters,char * DataToEncode,
int * URLParametersLength,CURL * CurlHandle) {

    char * URLEncoded;

    if ((URLEncoded = curl_easy_escape(CurlHandle,DataToEncode,0)) != NULL) {
        ExtendString(URLParameters,URLEncoded,URLParametersLength);
        curl_free(URLEncoded);
    } else {
        printf("ERROR: Could not URL encode %s\n",DataToEncode);
        exit(EXIT_FAILURE);
    }
}
//-------------------------------------------------------------------------------------------------
//----Caller is responsible for freeing the malloced memory that is returned.
curl_mime * BuildRemoteSoTURLParameters(OptionsType OptionValues,CURL * CurlHandle) {

    curl_mime * MultipartForm;
    curl_mimepart * MultipartField;
    String OneParameter;
    FILE * TemporaryFILE;
    extern String TemporaryFileName;
    String OneLine;

    MultipartForm = curl_mime_init(CurlHandle);

    MultipartField = curl_mime_addpart(MultipartForm);
    curl_mime_name(MultipartField,"NoHTML");
    sprintf(OneParameter,"%d",OptionValues.NoHTML);
    curl_mime_data(MultipartField,OneParameter,CURL_ZERO_TERMINATED);
    MultipartField = curl_mime_addpart(MultipartForm);
    curl_mime_name(MultipartField,"QuietFlag");
    sprintf(OneParameter,"-q%d",OptionValues.Quietness);
    curl_mime_data(MultipartField,OneParameter,CURL_ZERO_TERMINATED);

//----Decide what to do
//----List systems
    MultipartField = curl_mime_addpart(MultipartForm);
    curl_mime_name(MultipartField,"SubmitButton");
    if (!strcmp(OptionValues.SubmitButton,"ListSystems")) {
        curl_mime_data(MultipartField,"ListSystems",CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"ListStatus");
        curl_mime_data(MultipartField,"READY",CURL_ZERO_TERMINATED);
//----Run a particular system
    } else if (!strcmp(OptionValues.SubmitButton,"RunSelectedSystems")) {
        curl_mime_data(MultipartField,"RunSelectedSystems",CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        sprintf(OneParameter,"System___%s",OptionValues.System);
        curl_mime_name(MultipartField,OneParameter);
        curl_mime_data(MultipartField,OptionValues.System,CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        sprintf(OneParameter,"TimeLimit___%s",OptionValues.System);
        curl_mime_name(MultipartField,OneParameter);
        sprintf(OneParameter,"%d",OptionValues.TimeLimit);
        curl_mime_data(MultipartField,OneParameter,CURL_ZERO_TERMINATED);
    } else if (!strcmp(OptionValues.SubmitButton,"RunParallel")) {
        curl_mime_data(MultipartField,"RunParallel",CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"AutoMode");
        curl_mime_data(MultipartField,OptionValues.AutoMode,CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"AutoModeSystemsLimit");
        sprintf(OneParameter,"%d",OptionValues.AutoModeSystems);
        curl_mime_data(MultipartField,OneParameter,CURL_ZERO_TERMINATED);
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"AutoModeTimeLimit");
        sprintf(OneParameter,"%d",OptionValues.TimeLimit);
        curl_mime_data(MultipartField,OneParameter,CURL_ZERO_TERMINATED);
    }

//----If running systems, send file
    if (!strcmp(OptionValues.SubmitButton,"RunSelectedSystems") ||
!strcmp(OptionValues.SubmitButton,"RunParallel")) {
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"ProblemSource");
        curl_mime_data(MultipartField,OptionValues.ProblemSource,CURL_ZERO_TERMINATED);
        if (!strcmp(OptionValues.ProblemSource,"TPTP")) {
            MultipartField = curl_mime_addpart(MultipartForm);
            curl_mime_name(MultipartField,"TPTPProblem");
            curl_mime_data(MultipartField,OptionValues.Problem,CURL_ZERO_TERMINATED);
        } else if (!strcmp(OptionValues.ProblemSource,"UPLOAD")) {
            MultipartField = curl_mime_addpart(MultipartForm);
            curl_mime_name(MultipartField,"UPLOADProblem");
            if (!strcmp(OptionValues.Problem,"--")) {
                tmpnam(TemporaryFileName);
                TemporaryFILE = fopen(TemporaryFileName,"w");
                while (fgets(OneLine,STRINGLENGTH,stdin) != NULL) {
                    fputs(OneLine,TemporaryFILE);
                }
                fclose(TemporaryFILE);
                curl_mime_filedata(MultipartField,TemporaryFileName);
            } else {
                curl_mime_filedata(MultipartField,OptionValues.Problem);
            }
        }
    }

//----Force system even if it doesn't want to
    if (OptionValues.Force) {
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"ForceSystem");
        curl_mime_data(MultipartField,"-force",CURL_ZERO_TERMINATED);
    }
//----TSTP format output
    if (OptionValues.TSTPOutput) {
        MultipartField = curl_mime_addpart(MultipartForm);
        curl_mime_name(MultipartField,"X2TPTP");
        curl_mime_data(MultipartField,"-S",CURL_ZERO_TERMINATED);
    }

    return(MultipartForm);
}
//-------------------------------------------------------------------------------------------------
int main(int argc,char * argv[]) {

    OptionsType OptionValues;
    CURL * CurlHandle;
    curl_mime * MultipartForm;
    extern String TemporaryFileName;
    FILE * DataReadHandle;
    SuperString OneLine;
 
    if (!ProcessCommandLine(argc,argv,&OptionValues)) {
        printf("ERROR: Invalid command line arguments\n");
        exit(EXIT_FAILURE);
    }

    if ((CurlHandle = InitializeRemoteSoT()) == NULL) {
        return(EXIT_FAILURE);
    }

    if ((MultipartForm = BuildRemoteSoTURLParameters(OptionValues,CurlHandle)) == NULL) {
        printf("ERROR: Could not build multi-part form\n");
        return(EXIT_FAILURE);
    }

    if ((DataReadHandle = StartRemoteSoT(NULL,0,NULL,NULL,0,NULL,MultipartForm)) != NULL) {
        while (fgets(OneLine,SUPERSTRINGLENGTH,DataReadHandle) != NULL) {
            printf("%s",OneLine);
        }
        fclose(DataReadHandle);
    }
    remove(TemporaryFileName);
    if (DataReadHandle == NULL) {
        return(EXIT_FAILURE);
    } else {
        return(EXIT_SUCCESS);
    }
}
//-------------------------------------------------------------------------------------------------
String TemporaryFileName;
//-------------------------------------------------------------------------------------------------
