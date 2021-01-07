// socket client multi clients
// File: client.c
// compile: gcc -Wall client.c -o client
// run: ./client localhost
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for read/write/close
#include <sys/types.h>  /* standard system types        */
#include <netinet/in.h> /* Internet address structures */
#include <sys/socket.h> /* socket interface functions  */
#include <netdb.h>      /* host to IP resolution            */

#define BUFLEN 100 /* maximum response size     */
#define USAGE "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>"

//This struct contain all the data to build a HTTP request.
typedef struct request_HTTP
{
    char *  _host;      // Host name
    char *  _path;      //required path;  
    char *  _command;   // Command: GET or POST
    char *  _argToList; // Arguments after -r 
    char *  _data;      // Data to post
    char *  _conLen;    //Content_length:
    char *  _port;      // Port number.
}request_HTTP;

char*   build_STR(request_HTTP* request);
void    URl_analyze(const char* str, request_HTTP* request);
void    post_request(const char* str, request_HTTP* request);
void    del_request(request_HTTP* request);
int     r_request(char **argv, request_HTTP* request, int ind);
int     check_ARGV(char *str);
int     arg_check(char * str);


void error(char *str, request_HTTP* request)
{
    perror(str);
    perror("\n");
    del_request(request);
    exit(EXIT_FAILURE);
}
//Error function for USAGE
void Usage_err(request_HTTP* request,const char *str)
{   
    fprintf(stderr,"%s\n",str);
    del_request(request);
    exit(0);
}
/**
 * This function check whatever the str contain a URL, -p or -r or trash arguments.
*/
int check_ARGV(char *str)
{
    if(strstr(str, "http://"))  return 0;
    if(strcmp(str, "-p") == 0)  return 1;
    if(strcmp(str, "-r") == 0)  return 2;
    return 3;
}

/**
 * Analyzing the URL. Build the request_HTTP struct.
*/
void URl_analyze(const char* str, request_HTTP* request){

    //INIT flags.
    unsigned path_f = 0; //Check whatever there is '/' for path.
    unsigned port_f = 0; //Check whatever there is ':' for host.

    //temp string for host.
    char *temp_host = (char *)malloc(sizeof(char) * (strlen(str)));
    if (temp_host == NULL)
        error("allocate failed.\n", request);
    memset(temp_host, '\0', strlen(str));

    // temp string for path
    char *temp_path = (char *)malloc(sizeof(char) * (strlen(str)));
    if (temp_path == NULL)
    {
        free(temp_host);
        error("allocate failed.\n", request);
    }
    memset(temp_path, '\0', strlen(str));

    //temp string for port;
    char *temp_port = (char *)malloc(sizeof(char) * (strlen(str)));
    if (temp_port == NULL)
    {
        free(temp_host);
        free(temp_path);
        error("allocate failed.\n", request);
    }
    memset(temp_port, '\0', strlen(str));

     //INIT port, host, path indexes.
    int po = 0, ho = 0, pa = 0;
    //run over the string
    int i = 7; // Start at 7 since "http://" length = 6.
    while (i<strlen(str))
    {
        if (!path_f) // Host.
        { 
            if(port_f == 1 && str[i] != '/') //Port.
            {
                temp_port[po] = str[i];
                po++;
                i++;
                continue;
            } 
                  
            if (str[i] == '/')
            { // There is path
                path_f = 1;
                port_f = 0;
                temp_path[pa] = str[i];
                i++;
                pa++;
                continue;
            }
            // port first char
            if(str[i] == ':'){ 
                port_f = 1;
                i++;
                continue;
            }
            //host chars
            temp_host[ho] = str[i];
            i++;
            ho++;
        }
        else{ //path
            temp_path[pa] = str[i];
            i++;
            pa++;
        }
    }
    // check if path is NULL
    if(strcmp(temp_path, "") == 0)
        strcat(temp_path, "/");
    
    //Negative port.
    if(atoi(temp_port) <= 0 && strcmp(temp_port,"") != 0){
        free(temp_port);
        free(temp_host);
        free(temp_path);
        Usage_err(request, USAGE);
    }
    if (strcmp(temp_port,"") == 0)
        free(temp_port);
    
    else request->_port = temp_port;
    
    //Get request memebers point to strings.
    request->_host = temp_host;
    request->_path = temp_path;

    // if(strcmp(temp_port,"") != 0)
       
    temp_host = NULL;
    temp_path = NULL;
    temp_port = NULL;
}
/**
 * Analyzing POST request. 
*/
void post_request(const char* str, request_HTTP* request){

    // insert POST command to request.
    free(request->_command);

    //Change command to POST.
    request->_command = (char *)malloc(sizeof(char)*6);
    if(request->_command == NULL) error("allocate failed\n.", request);
    memset(request->_command, '\0', 6);
    strcpy(request->_command, "POST ");

    //Add data to request.
    request->_data = (char *)malloc(sizeof(char)*(strlen(str)+1));
    if(request->_data == NULL)error("allocate failed\n.", request);
    memset(request->_data, '\0', strlen(str)+1);
    strcpy(request->_data, str);
    
   //Content length header calculate
    int size = strlen(str);
    int count = 1;

    while(size >= 10){
        count ++;
        size = size % 10;
    }
    
    size = strlen(str);
    char *temp =(char*)malloc(sizeof(char)*count+1);
    if( temp == NULL) error("allocate failed\n.", request);
    memset(temp, '\0',count+1);
    sprintf(temp,"%d",(int)strlen(str));

    request->_conLen = (char *)malloc(sizeof(char) * count+1);
    if (request->_conLen == NULL)
    {
        free(temp);
        error("allocate failed\n.", request);
    }

    memset(request->_conLen, '\0',count+1);
    strcpy(request->_conLen, temp); 
    
    free(temp);   
}
//check if argv valid
int arg_check(char * str){
    if(     str[0] == '=' 
         || str[strlen(str)-1] == '='
         || strchr(str, '=') == NULL
         || strcmp(str, "http://") == 0
         || strcmp(str, "-r") == 0)
        return -1;
    return 0;
}
/**
 * Analyzing -r request and return argument# has passed.
*/
int r_request(char **argv, request_HTTP* request, int ind){
    //check if next to "-r" there is a number.
    char * temp = argv[ind];
    for (size_t i = 0; i <strlen(temp); i++)
        if(!(temp[i] >= 48 && temp[i] <= 57))
            Usage_err(request,USAGE);

    // check -r arguments are valids.
    int argn = atoi(argv[ind]);
    int size = argn;
    
    //if 0 after -r no arguments. 
    if (argn == 0) return 1; 
    
    for (size_t i = ind+1; i < ind+argn+1; i++)
    {   // if arv[i] is valid add it size.
        if (argv[i] == NULL)
            Usage_err(request,USAGE);

        if(arg_check(argv[i]) == 0){
            size += strlen(argv[i]);
            continue;
        }
         // invalid arg
           Usage_err(request,USAGE);

    }
    //Allocate list of arguments and start listing arguments.
    request->_argToList = (char *)malloc(sizeof(char)*size+1);
    if (request->_argToList == NULL) error("Alocate Failed\n", request);
    memset(request->_argToList, '\0', size+1);

    strcat(request->_argToList, "?");
    strcat(request->_argToList, argv[ind +1]);

    //Add arguments to list.
    for (size_t i = ind+2; i < ind + argn+1; i++)
    {
        strcat(request->_argToList, "&");
        strcat(request->_argToList, argv[i]);
    }
    
    temp = NULL;
    return argn+1;
}

/**
 * Build string to pass to the server.
*/
char* build_STR(request_HTTP* request){

    //INIT strings
    char str1[] = " HTTP/1.0\r\nHost: ";
    char str2[] = "\r\n\r\n";
    char str3[] = "\r\nContent-length:";
    
    //Calculate size for string.
    int size =  strlen(str1)+ 
                strlen(str2)+
                strlen(request->_command)+
                strlen(request->_host)+
                strlen(request->_path);
    
    if(request->_port       != NULL) size += (strlen(request->_port)+1);
    if(request->_data       != NULL) size += strlen(request->_data);
    if(request->_argToList  != NULL) size += strlen(request->_argToList); 
    if(request->_conLen     != NULL) 
        {
        size += strlen(request->_conLen);
        size += strlen(str3);
        }

    //allocate string.
    char * result = (char *)malloc(sizeof(char)*(size+1));
    if(result == NULL) error("allocate failed.\n", request);
    memset(result, '\0', size+1);
    
    //Add path
    strcat(result, request->_command);
    strcat(result, request->_path);

    //Add arguments
    if (request->_argToList != NULL)
        strcat(result, request->_argToList);
    
    //Add host and port
    strcat(result, str1);
    strcat(result, request->_host);
    if (request->_port != NULL)
    {
        strcat(result, ":");
        strcat(result, request->_port);
    }
    else{
        request->_port = (char*)malloc(sizeof(char)*3);
        if(request->_port == NULL)
            error("allocate failed.",request);
        memset(request->_port,'\0', 3);
        strcat(request->_port, "80");
    }
    
    //Add POST command data  
    if(strcmp(request->_command, "POST ") == 0){
        strcat(result, str3);
        strcat(result, request->_conLen);
        strcat(result, str2);
        strcat(result, request->_data);
    }
    else 
        strcat(result, str2);
    
    
    return result;
}
/**
 * Free all allocation on the struct. Also Freed the Struct itself.
*/
void  del_request(request_HTTP* request){
    if (request->_argToList != NULL) free(request->_argToList);
    if (request->_command   != NULL) free(request->_command);
    if (request->_conLen    != NULL) free(request->_conLen);
    if (request->_data      != NULL) free(request->_data);
    if (request->_host      != NULL) free(request->_host);
    if (request->_path      != NULL) free(request->_path);
    if (request->_port      != NULL) free(request->_port); 
    
    free(request);
}


// SEARCH ARGV FOR URL, -P -R and so on.
int main(int argc, char *argv[])
{
    //No arguments. 
    if(argc == 1){
        printf("argc\n");
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }
    //INIT variables, strings and structs.
    int             rc = 0; /* system calls return value storage */
    int             sockfd = 0;
    int             argn = 0; //Number of arguments with -r request.
    int             bytes_sum = 0;//count bytes received from server.
    char*           request_STR = NULL; // string request for sent to the server.
    char            rbuf[BUFLEN] = {0};
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    //Open struct to constructe the request.
    struct request_HTTP *request = (request_HTTP *)malloc(sizeof(request_HTTP));
    if (request == NULL) {
        perror("Failed to create request.\n");
        exit(EXIT_FAILURE);
    }
    //Init struct memebers.
    request->_host      = NULL;
    request->_path      = NULL;
    request->_argToList = NULL;
    request->_data      = NULL;
    request->_port      = NULL;
    request->_conLen    = NULL;

    //Set Default commant to GET.
    request->_command = (char *)malloc(sizeof(char)*5);
    if(request->_command == NULL) error("Request Failed\n.",request);
    strcpy(request->_command, "GET ");

    //Start analyzing the arguments.
    for (size_t i = 1; i < argc; i++) 
    {
        switch (check_ARGV(argv[i]))
        {
        case 0: // URL
            if(request->_host != NULL) // Not the first URL. USAGE failed.  
                Usage_err(request,USAGE);
            URl_analyze(argv[i], request);// Analyzing the URL
            break;
        case 1: // -p POST
            if(argv[i+1] == NULL || request->_data != NULL) // not the first -p, or nothing to post.
                Usage_err(request,USAGE);
            
            post_request(argv[i+1], request);
            i++; //After -p must come text, so avoid next argument.
            break;
        case 2: // -r List of arguments.
            if(request->_argToList != NULL || argv[i+1] == NULL) // not the first -r, or nothing after -r. 
                Usage_err(request,USAGE);
            
            argn = r_request(argv, request, i+1);
            i += (argn); // go after -r arguments.

            break;
        default: // IF we found a arguments not related to -r/-p/URL, then we have trash argument.
            Usage_err(request, USAGE);
            break;
        }
    }
    //NO host.
    if(request->_host == NULL) 
        Usage_err(request,USAGE);
    
    //construct the request string
    request_STR = build_STR(request);
  
    //Init socket client side
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("socket failed", request);
    
    // connect to server
    server = gethostbyname(request->_host);
    if (server == NULL){
        herror("ERROR, no such host\n");
        del_request(request);
        free(request_STR);
        exit(EXIT_FAILURE);
    }
 
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(request->_port));

    rc = connect(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rc < 0){
        free(request_STR);
        error("connect failed:", request);
    }
    
    //Required print as demand.
    printf("HTTP request =\n%s\nLEN = %d\n", request_STR, (int)strlen(request_STR));

    // send and then receive messages from the server
    if(write(sockfd, request_STR, strlen(request_STR)) == 0) error("write failed: ", request);

    // Read from socket and print. 
    while (rc >= 0)
    {
        memset(rbuf, '\0', BUFLEN);
        rc = read(sockfd, rbuf, BUFLEN-1);
        if (rc > 0){
            printf("%s", rbuf);
            bytes_sum += rc;
        }
        else if(rc == 0)
            break;
        else error("read failed: ", request);
    }

    close(sockfd);
    printf("\n Total received response bytes: %d\n", bytes_sum);

    //Free allocations.
    del_request(request);
    free(request_STR);
    
    return EXIT_SUCCESS;
}
