#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "text.h" /* Created by rpcgen */

struct auxstat{
    int pingnum;
    int swapnum;
    int hashnum;
    int checknum;
    int statnum;
} ;

struct auxstat st;



bool_t pingproc_1_svc(pingrequest * request,pingresponse *response, struct svc_req * rqstp)
{
    st.pingnum++;
    char *clieAddr = malloc(request->clientId.clientId_len);
    clieAddr = request->clientId.clientId_val;
    printf("s> %s ping\n",clieAddr);

    response->output =  0;
    return (TRUE);
}

bool_t statproc_1_svc(statrequest * request,struct statresponse *response, struct svc_req * rqstp)
{
    st.statnum++;
    char *clieAddr = malloc(request->clientId.clientId_len);
    clieAddr = request->clientId.clientId_val;
    printf("s> %s stat\n",clieAddr);


    response->pingnum =  st.pingnum;
    response->swapnum = st.swapnum;
    response->hashnum = st.hashnum;
    response->checknum = st.checknum;
    response->statnum = st.statnum;
    printf(" ping  %d\n swap  %d\n hash  %d\n check %d\n stat  %d\n", st.pingnum,st.swapnum,st.hashnum,st.checknum,st.statnum);

    return (TRUE);
}

bool_t swapproc_1_svc(swaprequest * request,swapresponse * response, struct svc_req * rqstp)
{
    st.swapnum++;
    //response->swapno = request->textlength;
    if(request->part == 1) printf("s> %s init swap %d\n",request->clientId.clientId_val,request->filesize);
    char *buffer = NULL;
    buffer = malloc(sizeof(char)*request->textin.textin_len);
    strncpy(buffer,request->textin.textin_val,request->textin.textin_len);
    //buffer[request->textin.textin_len] = '\0';
    //printf("BRUTE %s\n",buffer);
    /////////////// CONVERT TO UPPERCASE /////////////////////////
    char aux;
    char c = 'g';
    int i = 0;
    char conv_s[1024];
    int converted = 0;
    while(c != '\0')
    {
         int changed = 0;
         c = buffer[i];
         aux = toupper(c);
         if(aux != c) changed = 1;
         if(changed == 0)
         {
             aux = tolower(c);
             if(aux != c)changed = 1;
         }
         if(changed == 1)
         {
             converted++;
             conv_s[i] = aux;
         }
         else
         {
             conv_s[i] = c;
         }
         i++;
         if(converted > request->textin.textin_len)
         {
             converted--;
             break;
         }
    }

            /////////////////////////////////////////////////////////////////////
    //printf("CONVERTED %s\n",conv_s);

    response->out.out_len = request->textin.textin_len;
    response->out.out_val = malloc(sizeof(char) * request->textin.textin_len);
    response->out.out_val = conv_s;


    response->swapno = converted + request->swapno;
    if(request->isFinished == 1) 
    {
        st.swapnum++;
        printf("s> swap %s = %d\n",request->clientId.clientId_val, response->swapno);
    }

    return (TRUE);
}

bool_t hashproc_1_svc(hashrequest * request,hashresponse *response, struct svc_req * rqstp)
{
    
    
    char *buffer = malloc(sizeof(char) * request->textin.textin_len);

    buffer = request->textin.textin_val;

    int currentHash = 0;


    if(request->part == 1) printf("s> %s init hash %d\n", request->clientId.clientId_val, request->filesize);
    
    /////////////// OBTAIN PART HASH ///////////////////////////////////////////
    int i = 0;
    for(i = 0; i < request->textin.textin_len; i++)
    {
        currentHash = (currentHash + buffer[i])%1000000000;
    }
    ///////////////////////////////////////////////////////////////////////

    //////////////// OBTAIN FULL HASH ///////////////////////////////////////////
    if(currentHash<0)currentHash += 1000000000;//Normalize hash value
    currentHash = (currentHash + request->hash)%1000000000; //Obtain hash with previous chunks
    if(currentHash<0)currentHash += 1000000000;//Normalize hash value
    /////////////////////////////////////////////////////////////////////////////

    response->hash = currentHash; //Assign output

    if(request->isFinished == 1)
    {
        st.hashnum++;
        printf("s> %s hash = %d\n",request->clientId.clientId_val,currentHash);
    }
    return (TRUE);
}

bool_t checkproc_1_svc(checkrequest * request,checkresponse *response, struct svc_req * rqstp)
{
    

    char *buffer = malloc(sizeof(char) * request->textin.textin_len);

    buffer = request->textin.textin_val;

    int currentHash = 0;

    if(request->part == 1) printf("s> %s init check %d %d\n", request->clientId.clientId_val, request->filesize, request->clieHash);


    /////////////// OBTAIN PART HASH ///////////////////////////////////////////
    int i = 0;
    for(i = 0; i < request->textin.textin_len; i++)
    {
        currentHash = (currentHash + buffer[i])%1000000000;
    }
    ///////////////////////////////////////////////////////////////////////

    //////////////// OBTAIN FULL HASH ///////////////////////////////////////////
    if(currentHash<0)currentHash += 1000000000;//Normalize hash value
    currentHash = (currentHash + request->servHash)%1000000000; //Obtain hash with previous chunks
    if(currentHash<0)currentHash += 1000000000;//Normalize hash value
    /////////////////////////////////////////////////////////////////////////////

    response->hashAcum = currentHash; //Assign output

    if(request->isFinished == 1)
    {
        st.checknum++;
        char *result = (currentHash == request->clieHash) ? "OK" : "FAIL";
        response->isCorrect[0] = (currentHash == request->clieHash) ? 1 : 0;
        //response->isCorrect = 1;
        printf("s> %s check = %s\n",request->clientId.clientId_val,result);
    }
    return (TRUE);
}

int server_1_freeresult (SVCXPRT *arg0, xdrproc_t arg1, caddr_t arg2){

    //xdr_free(arg1, arg2);
    return TRUE; //1
}