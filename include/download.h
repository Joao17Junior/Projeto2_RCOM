#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <regex.h>

// ============== Constantes ==============

#define MAX_LENGTH 500
#define FTP_PORT 21

// Server response codes (RFC 959)
#define SV_READY_FOR_AUTH       220
#define SV_READY_FOR_PASS       331
#define SV_LOGIN_SUCCESS        230
#define SV_PASSIVE_MODE         227
#define SV_READY_FOR_TRANSFER   150
#define SV_TRANSFER_COMPLETE    226
#define SV_GOODBYE              221

// Parser regular expressions
#define AT              "@"
#define BAR             "/"
#define HOST_REGEX      "%*[^/]//%[^/]"
#define HOST_AT_REGEX   "%*[^/]//%*[^@]@%[^/]"
#define RESOURCE_REGEX  "%*[^/]//%*[^/]/%s"
#define USER_REGEX      "%*[^/]//%[^:/]"
#define PASS_REGEX      "%*[^/]//%*[^:]:%[^@\n$]"
#define RESPCODE_REGEX  "%d"
#define PASSIVE_REGEX   "%*[^(](%d,%d,%d,%d,%d,%d)%*[^\n$)]"

// Default credentials for anonymous login
#define DEFAULT_USER     "anonymous"
#define DEFAULT_PASSWORD "anonymous@"

// ============== Estruturas ==============

/**
 * Estrutura para guardar informação do URL
 * Formato: ftp://[user:password@]host/path
 */
typedef struct {
    char host[MAX_LENGTH];      // 'ftp.up.pt'
    char resource[MAX_LENGTH];  // 'pub/files/test.txt'
    char file[MAX_LENGTH];      // 'test.txt'
    char user[MAX_LENGTH];      // 'username'
    char password[MAX_LENGTH];  // 'password'
    char ip[MAX_LENGTH];        // '193.137.29.15'
} URL;

/**
 * Estados da máquina que processa respostas do servidor
 */
typedef enum {
    START,      // Início da leitura
    SINGLE,     // Resposta de linha única (código + espaço)
    MULTIPLE,   // Resposta multi-linha (código + hífen)
    END         // Fim da resposta
} ResponseState;

// ============== Funções de Parsing ==============

/**
 * Parse do URL fornecido pelo utilizador
 * @param input String com o URL (ftp://...)
 * @param url Estrutura que será preenchida
 * @return 0 se sucesso, -1 se erro
 */
int parseURL(const char *input, URL *url);

/**
 * Parse da resposta PASV para obter IP e porta
 * Formato: 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
 * @param response Resposta do servidor
 * @param ip String que será preenchida com o IP
 * @param port Ponteiro para inteiro que será preenchido com a porta
 * @return 0 se sucesso, -1 se erro
 */
int parsePasv(const char *response, char *ip, int *port);

// ============== Funções de Rede ==============

/**
 * Cria socket TCP e conecta ao servidor
 * @param ip Endereço IP do servidor
 * @param port Porta do servidor
 * @return Socket descriptor se sucesso, -1 se erro
 */
int createSocket(const char *ip, int port);

/**
 * Lê resposta do servidor FTP usando máquina de estados
 * @param socket Descriptor do socket
 * @param buffer Buffer que será preenchido com a resposta
 * @return Código de resposta (220, 331, etc)
 */
int readResponse(const int socket, char *buffer);

/**
 * Envia comando para o servidor FTP
 * @param socket Descriptor do socket
 * @param cmd Comando (ex: "USER", "PASS", "RETR")
 * @param arg Argumento do comando (pode ser NULL)
 * @return 0 se sucesso, -1 se erro
 */
int sendCommand(const int socket, const char *cmd, const char *arg);

// ============== Funções FTP ==============

/**
 * Autentica no servidor FTP
 * @param socket Descriptor do socket de controlo
 * @param user Nome de utilizador
 * @param password Password
 * @return Código de resposta do servidor
 */
int ftpLogin(const int socket, const char *user, const char *password);

/**
 * Entra em modo passivo
 * @param socket Descriptor do socket de controlo
 * @param ip String que será preenchida com IP para conexão de dados
 * @param port Ponteiro que será preenchido com porta para conexão de dados
 * @return Código de resposta do servidor
 */
int ftpPassiveMode(const int socket, char *ip, int *port);

/**
 * Requisita um ficheiro ao servidor
 * @param socket Descriptor do socket de controlo
 * @param resource Caminho do ficheiro no servidor
 * @return Código de resposta do servidor
 */
int ftpRequestFile(const int socket, const char *resource);

/**
 * Descarrega o ficheiro do servidor
 * @param control_socket Socket de controlo
 * @param data_socket Socket de dados
 * @param filename Nome do ficheiro a criar localmente
 * @return Código de resposta do servidor
 */
int ftpDownloadFile(const int control_socket, const int data_socket, const char *filename);

/**
 * Fecha a conexão FTP
 * @param control_socket Socket de controlo
 * @param data_socket Socket de dados
 * @return 0 se sucesso, -1 se erro
 */
int ftpQuit(const int control_socket, const int data_socket);

#endif // DOWNLOAD_H