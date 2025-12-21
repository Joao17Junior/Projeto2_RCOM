#include "../include/download.h"

/**
 * Parse do URL usando regex
 */
int parseURL(const char *input, URL *url) {
    regex_t regex;
    
    // Verificar se tem barra
    regcomp(&regex, BAR, 0);
    if (regexec(&regex, input, 0, NULL, 0)) {
        fprintf(stderr, "Erro: URL inválido\n");
        return -1;
    }
    
    // Verificar se tem autenticação (@)
    regcomp(&regex, AT, 0);
    if (regexec(&regex, input, 0, NULL, 0) != 0) {
        // Formato: ftp://<host>/<url-path>
        sscanf(input, HOST_REGEX, url->host);
        strcpy(url->user, DEFAULT_USER);
        strcpy(url->password, DEFAULT_PASSWORD);
    } else {
        // Formato: ftp://[<user>:<password>@]<host>/<url-path>
        sscanf(input, HOST_AT_REGEX, url->host);
        sscanf(input, USER_REGEX, url->user);
        sscanf(input, PASS_REGEX, url->password);
    }
    
    // Extrair resource e filename
    // Encontrar o caminho após o host
    const char *pathStart = strstr(input, "://");
    if (pathStart != NULL) {
        pathStart += 3;  // Pular "://"
        pathStart = strchr(pathStart, '/');  // Encontrar primeira barra após host
        if (pathStart != NULL) {
            strcpy(url->resource, pathStart);  // Copiar o caminho completo (com a barra inicial)
        } else {
            strcpy(url->resource, "/");
        }
    } else {
        strcpy(url->resource, "/");
    }
    
    // Extrair nome do ficheiro
    char *lastSlash = strrchr(url->resource, '/');
    if (lastSlash != NULL && strlen(lastSlash) > 1) {
        strcpy(url->file, lastSlash + 1);
    } else {
        strcpy(url->file, "downloaded_file");
    }
    
    // Resolver hostname para IP
    struct hostent *h;
    if (strlen(url->host) == 0) {
        fprintf(stderr, "Erro: Host vazio\n");
        return -1;
    }
    
    if ((h = gethostbyname(url->host)) == NULL) {
        herror("gethostbyname()");
        return -1;
    }
    
    strcpy(url->ip, inet_ntoa(*((struct in_addr *)h->h_addr)));
    
    // Validar se todos os campos foram preenchidos
    if (strlen(url->host) == 0 || strlen(url->user) == 0 || 
        strlen(url->password) == 0 || strlen(url->resource) == 0 || 
        strlen(url->file) == 0) {
        fprintf(stderr, "Erro: Parse incompleto\n");
        return -1;
    }
    
    return 0;
}

/**
 * Cria socket e conecta ao servidor
 */
int createSocket(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // Criar socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    
    // Configurar endereço do servidor
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    
    // Conectar
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

/**
 * Lê resposta do servidor usando máquina de estados
 */
int readResponse(const int socket, char *buffer) {
    char byte;
    int index = 0, responseCode = 0;
    ResponseState state = START;
    char code[4] = {0};  // Para armazenar os 3 dígitos do código
    int codeIndex = 0;
    
    memset(buffer, 0, MAX_LENGTH);
    
    while (state != END) {
        if (read(socket, &byte, 1) <= 0) {
            break;
        }
        
        switch (state) {
            case START:
                if (codeIndex < 3 && byte >= '0' && byte <= '9') {
                    // Ler os primeiros 3 dígitos do código
                    code[codeIndex++] = byte;
                } else if (byte == ' ') {
                    state = SINGLE;  // Resposta de linha única
                } else if (byte == '-') {
                    state = MULTIPLE;  // Resposta multi-linha
                } else if (byte == '\n') {
                    state = END;
                }
                break;
                
            case SINGLE:
                if (byte == '\n') {
                    state = END;
                } else {
                    buffer[index++] = byte;
                }
                break;
                
            case MULTIPLE:
                if (byte == '\n') {
                    // Nova linha em resposta multi-linha
                    // Verificar se a próxima linha começa com o código seguido de espaço
                    char nextCode[4] = {0};
                    int tempIndex = 0;
                    char tempByte;
                    
                    // Ler próximos 3 caracteres
                    for (int i = 0; i < 3; i++) {
                        if (read(socket, &tempByte, 1) > 0) {
                            nextCode[tempIndex++] = tempByte;
                        }
                    }
                    
                    // Ler o 4º caractere (espaço ou hífen)
                    if (read(socket, &tempByte, 1) > 0) {
                        if (strcmp(code, nextCode) == 0 && tempByte == ' ') {
                            // Fim da resposta multi-linha
                            state = SINGLE;
                        } else {
                            // Continuar lendo a linha
                            // (descartar o que foi lido)
                        }
                    }
                } else {
                    // Ignorar o conteúdo da resposta multi-linha
                }
                break;
                
            case END:
                break;
                
            default:
                break;
        }
    }
    
    // Converter código para inteiro
    if (strlen(code) == 3) {
        responseCode = atoi(code);
    }
    
    printf("< %d %s\n", responseCode, buffer);
    
    return responseCode;
}

/**
 * Envia comando FTP (COM \r\n que é o correto!)
 */
int sendCommand(const int socket, const char *cmd, const char *arg) {
    char command[MAX_LENGTH];
    
    if (arg != NULL && strlen(arg) > 0) {
        snprintf(command, sizeof(command), "%s %s\r\n", cmd, arg);
    } else {
        snprintf(command, sizeof(command), "%s\r\n", cmd);
    }
    
    printf("> %s", command);
    
    if (write(socket, command, strlen(command)) < 0) {
        perror("write()");
        return -1;
    }
    
    return 0;
}

/**
 * Parse da resposta PASV
 */
int parsePasv(const char *response, char *ip, int *port) {
    int ip1, ip2, ip3, ip4, port1, port2;
    
    if (sscanf(response, PASSIVE_REGEX, &ip1, &ip2, &ip3, &ip4, &port1, &port2) != 6) {
        return -1;
    }
    
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    *port = port1 * 256 + port2;
    
    return 0;
}

/**
 * Autentica no servidor
 */
int ftpLogin(const int socket, const char *user, const char *password) {
    char buffer[MAX_LENGTH];
    int code;
    
    // Enviar USER
    if (sendCommand(socket, "USER", user) < 0) {
        return -1;
    }
    
    code = readResponse(socket, buffer);
    if (code != SV_READY_FOR_PASS && code != SV_LOGIN_SUCCESS) {
        fprintf(stderr, "Erro: USER falhou (código %d)\n", code);
        return code;
    }
    
    // Se precisar de password
    if (code == SV_READY_FOR_PASS) {
        if (sendCommand(socket, "PASS", password) < 0) {
            return -1;
        }
        
        code = readResponse(socket, buffer);
        if (code != SV_LOGIN_SUCCESS) {
            fprintf(stderr, "Erro: PASS falhou (código %d)\n", code);
            return code;
        }
    }
    
    return code;
}

/**
 * Entra em modo passivo
 */
int ftpPassiveMode(const int socket, char *ip, int *port) {
    char buffer[MAX_LENGTH];
    
    if (sendCommand(socket, "PASV", NULL) < 0) {
        return -1;
    }
    
    int code = readResponse(socket, buffer);
    if (code != SV_PASSIVE_MODE) {
        fprintf(stderr, "Erro: PASV falhou (código %d)\n", code);
        return code;
    }
    
    if (parsePasv(buffer, ip, port) < 0) {
        fprintf(stderr, "Erro: Parse da resposta PASV falhou\n");
        return -1;
    }
    
    return code;
}

/**
 * Requisita ficheiro
 */
int ftpRequestFile(const int socket, const char *resource) {
    char buffer[MAX_LENGTH];
    
    if (sendCommand(socket, "RETR", resource) < 0) {
        return -1;
    }
    
    return readResponse(socket, buffer);
}

/**
 * Descarrega ficheiro
 */
int ftpDownloadFile(const int control_socket, const int data_socket, const char *filename) {
    char buffer[MAX_LENGTH];
    
    // Abrir ficheiro para escrita
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen()");
        return -1;
    }
    
    // Receber dados
    printf("\nA transferir ficheiro '%s'...\n", filename);
    int total_bytes = 0;
    int bytes;
    
    while ((bytes = read(data_socket, buffer, MAX_LENGTH)) > 0) {
        if (fwrite(buffer, bytes, 1, file) < 0) {
            perror("fwrite()");
            fclose(file);
            return -1;
        }
        total_bytes += bytes;
        printf("\rRecebidos: %d bytes", total_bytes);
        fflush(stdout);
    }
    
    printf("\n");
    fclose(file);
    close(data_socket);  // Fechar socket de dados antes de ler resposta final
    
    // Ler resposta final do servidor
    char response[MAX_LENGTH];
    int code = readResponse(control_socket, response);
    
    printf("✓ Ficheiro '%s' transferido com sucesso! (%d bytes)\n", filename, total_bytes);
    
    return code;
}

/**
 * Fecha conexão FTP
 */
int ftpQuit(const int control_socket) {
    char buffer[MAX_LENGTH];
    
    if (sendCommand(control_socket, "QUIT", NULL) < 0) {
        return -1;
    }
    
    int code = readResponse(control_socket, buffer);
    if (code != SV_GOODBYE) {
        fprintf(stderr, "Aviso: QUIT retornou código %d\n", code);
    }
    
    close(control_socket);
    
    return (code == SV_GOODBYE) ? 0 : -1;
}

/**
 * Main
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s ftp://demo:password@test.rebex.net/readme.txt\n", argv[0]);
        return 1;
    }
    
    URL url;
    memset(&url, 0, sizeof(url));
    
    printf("=== Cliente FTP ===\n\n");
    
    // 1. Parse do URL
    printf("1. A fazer parse do URL...\n");
    if (parseURL(argv[1], &url) != 0) {
        fprintf(stderr, "Erro no parse do URL\n");
        return 1;
    }
    
    printf("   Host: %s\n", url.host);
    printf("   IP: %s\n", url.ip);
    printf("   User: %s\n", url.user);
    printf("   Resource: %s\n", url.resource);
    printf("   File: %s\n\n", url.file);
    
    // 2. Conectar ao servidor FTP (porta 21)
    printf("2. A conectar ao servidor %s:%d...\n", url.ip, FTP_PORT);
    int control_socket = createSocket(url.ip, FTP_PORT);
    if (control_socket < 0) {
        fprintf(stderr, "Erro ao conectar ao servidor\n");
        return 1;
    }
    printf("✓ Conectado!\n\n");
    
    // 3. Ler mensagem de boas-vindas
    printf("3. A ler mensagem de boas-vindas...\n");
    char buffer[MAX_LENGTH];
    int code = readResponse(control_socket, buffer);
    if (code != SV_READY_FOR_AUTH) {
        fprintf(stderr, "Erro: Mensagem de boas-vindas inválida (código %d)\n", code);
        close(control_socket);
        return 1;
    }
    printf("\n");
    
    // 4. Autenticar
    printf("4. A autenticar...\n");
    code = ftpLogin(control_socket, url.user, url.password);
    if (code != SV_LOGIN_SUCCESS) {
        fprintf(stderr, "Erro na autenticação\n");
        close(control_socket);
        return 1;
    }
    printf("✓ Login com sucesso!\n\n");
    
    // 5. Entrar em modo passivo
    printf("5. A entrar em modo passivo...\n");
    char data_ip[MAX_LENGTH];
    int data_port;
    code = ftpPassiveMode(control_socket, data_ip, &data_port);
    if (code != SV_PASSIVE_MODE) {
        fprintf(stderr, "Erro no modo passivo\n");
        close(control_socket);
        return 1;
    }
    printf("✓ Modo passivo: %s:%d\n\n", data_ip, data_port);
    
    // 6. Conectar ao socket de dados
    printf("6. A conectar ao socket de dados...\n");
    int data_socket = createSocket(data_ip, data_port);
    if (data_socket < 0) {
        fprintf(stderr, "Erro ao conectar ao socket de dados\n");
        close(control_socket);
        return 1;
    }
    printf("✓ Conectado ao socket de dados!\n\n");
    
    // 7. Requisitar ficheiro
    printf("7. A requisitar ficheiro '%s'...\n", url.resource);
    code = ftpRequestFile(control_socket, url.resource);
    if (code != SV_READY_FOR_TRANSFER && code != SV_CONN_ALREADY_OPEN) {
        fprintf(stderr, "Erro ao requisitar ficheiro (código %d)\n", code);
        close(control_socket);
        close(data_socket);
        return 1;
    }
    
    // 8. Descarregar ficheiro
    char output_path[MAX_LENGTH * 2];
    snprintf(output_path, sizeof(output_path), "test_folder/%s", url.file);
    
    code = ftpDownloadFile(control_socket, data_socket, output_path);
    if (code != SV_TRANSFER_COMPLETE) {
        fprintf(stderr, "Aviso: Transferência pode estar incompleta (código %d)\n", code);
    }
    
    // 9. Fechar conexão
    printf("\n9. A fechar conexão...\n");
    ftpQuit(control_socket);
    
    printf("\n=== Download concluído com sucesso! ===\n");
    
    return 0;
}
