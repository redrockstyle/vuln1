// task1.c
//
// Зачётное задание 1.
//
// Маткин Илья Александрович    07.04.2015


#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
//#include <linux/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "utils/files.h"

//----------------------------------------

void TListen (unsigned int port);
int TProcessing (int sock);

bool TPutFile (int sock, IN char *fileName);
bool TIsFileExist (int sock, IN char *fileName);
void TPrintFullMetaFile (int sock, IN char *id, OUT char *name, OUT char *value);

void TGetFile (int sock, IN char *fileName);
bool TSendFile (int sock, IN char *fileName, IN char *id);

void TPrintMetaFile (int sock, IN char *id);
int TListFiles (int sock);

//----------------------------------------


//
// Прослушивает порт и порождает обрабатывающий 
// процесс на каждое входящее соединение.
//
void TListen (unsigned int port) {

struct sockaddr_in addr;
struct sockaddr_in addr2;
socklen_t size = sizeof(addr2);
int sock;
int acceptSock;
pid_t pid;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    
    memset (&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    addr.sin_addr.s_addr = 0;

    bind (sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));

    listen (sock, 100);

    while ((acceptSock = accept (sock, (struct sockaddr*)&addr2, &size)) != -1) {

        pid = fork();

        if (pid == 0) {
            // понижаем права
            setresgid (10000, 10000, 10000);
            setresuid (10000, 10000, 10000);

            close (sock);

            // вызываем основной обработчик входящего соединения
            TProcessing (acceptSock);
            
            close (acceptSock);
            return;
            }
        else {
            int status;
            close (acceptSock);
            waitpid (pid, &status, 0);
            }
        }

    return;
}


//
// Основной обработчик входящего соединения.
//
int TProcessing (int sock) {

char request[MAX_STRING_SIZE + 1];
int fd;
char *action;
char *value;

    if (!TGetString (sock, request)) {
        return 0;
        }

    printf ("%s\n", request);

    action = request;
    value = TParseParam (request);

    if (!strcmp (action, "PUT")) {
        TPutFile (sock, value);
        return 1;
        }

    if (!strcmp (action, "GET")) {
        TGetFile (sock, value);
        return 1;
        }

    if (!strcmp (action, "LIST")) {
        TListFiles (sock);
        return 1;
        }

    TPutString (sock, "unknown command\n");
    return 0;
}


//
// Обработчик команды сохранения файла.
//
bool TPutFile (int sock, IN char *fileName) {

char id[ID_SIZE + 1];
char param[MAX_STRING_SIZE + 1];
char origParam[MAX_STRING_SIZE + 1];
char metaPath[PATH_MAX];
char filesPath[PATH_MAX];
int metaFd;
int contentFd;
bool fileOK = FALSE;

    // проверка имени файла на отсутствие недопустимых символов
    if (!TCheckString (fileName)) {
        TPutString (sock, "Error symbol in file name\n");
        return FALSE;
        }

    // проверка, что такого имени файла не существует
    if (TIsFileExist (sock, fileName)) {
        return FALSE;
        }

    // генерируем случайный идентификатор, который
    // используется в качестве имени файла метаданных
    TGenerateId (id);

    TGetMetaPath (id, metaPath);
    TGetFilesPath (fileName, filesPath);

    // создаем файлы для хранения метаданных и содержимого файла
    metaFd = open (metaPath, O_CREAT | O_TRUNC | O_RDWR, 0644);
    contentFd = open (filesPath, O_CREAT | O_TRUNC | O_RDWR, 0644);

    // в начале файла метаданных сохраняем имя файла
    write (metaFd, fileName, strlen (fileName));
    write (metaFd, "\n", 1);

    // получаем метаданные файла
    while (TGetString (sock, origParam)) {

        strcpy (param, origParam);
        char *value = TParseParam (param);

        write (metaFd, origParam, strlen (origParam));
        write (metaFd, "\n", 1);

        // после метаданных с длиной передаётся содержимое файла
        if (!strcmp (param, "Size")) { //rename
            size_t length = atoi (value);
            unsigned char *buf = (unsigned char*) malloc (length);

            if (recv (sock, buf, length, MSG_WAITALL) == length) {
                write (contentFd, buf, length);
                fileOK = TRUE;
                }

            free (buf);
            break;
            }
        }

    close (metaFd);
    close (contentFd);

    // если передана не полная информация, удаляем все файлы
    if (!fileOK) {
        unlink (metaPath);
        unlink (filesPath);
        }

    return fileOK;
}


//
// Проверяет существование файла.
//
bool TIsFileExist (int sock, IN char *fileName) {

bool ok = FALSE;
                                     // строка с параметром имеет вид: имя_параметра:значение_параметра
char value[MAX_STRING_SIZE + 1 - 2]; // максимальный размер значения параметра на 2 меньше максимальной длины строки
char name[MAX_STRING_SIZE + 1 - 2];  // максимальный размер имени параметра на 2 меньше максимальной длины строки
char id[ID_SIZE + 1];

    if (TGetIdByFileName (fileName, id)) {
        ok = TRUE;

        TPutString (sock, "File exist:\n");
        TPrintFullMetaFile (sock, id, name, value);
        // В буферы name и value попадут данные последнего параметра файла
        // (должна быть информация о длине файла).
        // Выведем информацию в лог программы.
        printf ("Second request for file %s\t%s:%s\n", fileName, name, value);
        }

    return ok;
}


//
// Выводит метаданные файла.
//
void TPrintFullMetaFile (int fd, IN char *id, OUT char *name, OUT char *value) {

int metaFd;
char path[PATH_MAX];
char buf[MAX_STRING_SIZE + 1];

    TGetMetaPath (id, path);
    metaFd = open (path, O_RDONLY);
    TGetString (metaFd, buf);

    while (TGetString (metaFd, buf)) {
        char *v = TParseParam (buf);
        if (v == buf) {
            continue;
            }
        strcpy (value, v);
        strcpy (name, buf);
        TPutString (fd, name);
        TPutString (fd, " = ");
        TPutString (fd, value);
        TPutString (fd, "\n");
        }

    close (metaFd);

    return;
}


//
// Обработчик команды запроса содержимого файла.
//
void TGetFile (int sock, IN char *fileName) {

char id[PATH_MAX];

    if (TGetIdByFileName (fileName, id)) {
        TSendFile (sock, fileName, id);
        }

    return;
}


//
// Отправка файла пользователю.
//
bool TSendFile (int sock, IN char *fileName, IN char *id) {

unsigned char buf[111];
char path[PATH_MAX];
size_t size;
int fd;

    // получаем размер файла из метаданных
    size = TGetFileSize (id);

    // слишком большие файлы не пересылаем
    if (size > 111) {
        TPutString (sock, "file too big\n");
        return FALSE;
        }

    TGetFilesPath (fileName, path);

    fd = open (path, O_RDONLY);
    TGetString (fd, buf);
    close (fd);

    send (sock, buf, size, 0);
    TPutString (sock, "\n");

    return TRUE;
}


//
// Обработчик команды вывода списка сохранённых файлов.
//
int TListFiles (int sock) {

DIR *dir;
char id[PATH_MAX];

    dir = opendir (META_DIR);

    while (TGetNextId (dir, id)) {
        TPrintMetaFile (sock, id);
        }

    closedir (dir);

    return 0;
}


//
// Выводит идентификатор, имя и размер файла.
//
void TPrintMetaFile (int sock, IN char *id) {

char fileName[MAX_STRING_SIZE + 1];
char fileSize[16];

    TGetFileNameById (id, fileName);

    sprintf (fileSize, "%d", TGetFileSize (id));

    TPutString (sock, id);
    TPutString (sock, ": ");
    TPutString (sock, fileName);
    TPutString (sock, " - ");
    TPutString (sock, fileSize);
    TPutString (sock, "\n");

    return;
}


int main (unsigned int argc, char *argv[], char *envp[]) {

    signal (SIGPIPE, SIG_IGN);

    TListen (5555);

    return 0;
}

