// Модуль files.c
//
// Зачётное задание 1.
//
// Маткин Илья Александрович    04.05.2015


#include "files.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <malloc.h>
//#include <linux/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>



//----------------------------------------


//
// Чтение строки из дескриптора. Не больше MAX_STRING_SIZE байт.
//
size_t TGetString (int fd, OUT char *dst) {

unsigned int i = 0;
char c;

    while (i < MAX_STRING_SIZE) {
        if (read (fd, &c, 1) > 0) {
            if (c == '\n' || c == '\0') {
                dst[i] = 0;
                break;
                }
            dst[i] = c;
            ++i;
            }
        else {
            dst[i] = 0;
            break;
            }
        }

    if (i == MAX_STRING_SIZE) {
        dst[i] = 0;
        TFlushString (fd);
        }

    return i;
}


//
// Вывод строки в дескриптор.
//
void TPutString (int fd, IN const char *src) {


    write (fd, src, strlen (src));

    return;
}


//
// Разбор строки вида имя:значение на две части.
// Вместо символа ':' записывается 0.
// Возвращает указатель на значение.
// Возможен формат только с именем (например в команде LIST).
// В этом случае возращаемое значение игнорируется.
//
char * TParseParam (IN OUT char *str) {

unsigned int i = 0;
unsigned int valueIndex = 0;
char *p = str;

    while (str[i]) {
        if (str[i] == ':') {
            str[i] = 0;
            valueIndex = i + 1; // значение начинается со следующего индекса
            break;
            }
        ++i;
        }

    return p + valueIndex;
}


//
// Сбрасывает данные до следующей строки.
//
void TFlushString (int fd) {

unsigned char tmp;

    while (read (fd, &tmp, 1) == 1) {
        if (tmp == '\n') {
            break;
            }
        }

    return;
}


//
// Проверка строки на отсутствие недопустимых символов.
//
int TCheckString (IN const char *str) {

size_t i;

    for (i = 0; i < strlen (str); ++i) {
        if (!isalnum (str[i])) {
            return 0;
            }
        }

    return 1;
}


//
// Возвращает идентификатор по имени файла.
//
bool TGetIdByFileName (IN const char *fileName, OUT char *dstId) {

DIR *dir;
char id[PATH_MAX];

    dir = opendir (META_DIR);

    // перебираем все идентификаторы
    while (TGetNextId (dir, id)) {
        char buf[MAX_STRING_SIZE + 1];

        // получаем имя файла для очередного идентификатора
        TGetFileNameById (id, buf);

        if (!strcmp (fileName, buf)) {
            strcpy (dstId, id);
            return TRUE;
            }
        }

    closedir (dir);

    return FALSE;
}


//
// Получает имя файла по идентификатору.
//
void TGetFileNameById (IN const char *id, OUT char *dstFileName) {

int fd;
char path[MAX_STRING_SIZE + 1];

    TGetMetaPath (id, path);

    fd = open (path, O_RDONLY);
    TGetString (fd, dstFileName);
    close (fd);

    return;
}


//
// Возвращает значение параметра из метаданных файла.
//
char * TGetMetaParam (IN const char *id, IN const char *paramName, OUT char *buf) {

int fd;
char path[PATH_MAX];

    TGetMetaPath (id, path);

    fd = open (path, O_RDONLY);
    if (fd == -1) {
        return NULL;
        }

    // Пропускаем имя файла
    if (!TGetString (fd, buf)) {
        close (fd);
        return NULL;
        }

    while (TGetString (fd, buf)) {
        char *value = TParseParam (buf);

        if (!strcmp (buf, paramName)) {
            close (fd);
            return value;
            }
        }

    close (fd);
    return NULL;
}


//
// Возвращает размер файла из метаданных.
//
size_t TGetFileSize (IN const char *id) {

char buf[MAX_STRING_SIZE];
size_t size = -1;
char *value;

    value = TGetMetaParam (id, "Size", buf); //rename (before Length)
    if (value) {
        size = atoi (value);
        }

    return size;
}


//
// Возвращает путь до файла метаданных по идентификатору.
//
void TGetMetaPath (IN const char *id, OUT char *dstPath) {

    strcpy (dstPath, META_DIR);
    strcat (dstPath, id);

    return;
}


//
// Возвращает путь до файла с содержимым по идентификатору.
//
void TGetFilesPath (IN const char *fileName, OUT char *dstPath) {


    strcpy (dstPath, FILES_DIR);
    strcat (dstPath, fileName);

    return;
}


//
// Возвращает путь до файла в каталоге резервных копий.
//
void TGetBackupPath (IN const char *fileName, OUT char *dstPath) {


    strcpy (dstPath, BACKUP_DIR);
    strcat (dstPath, fileName);

    return;
}


//
// Генерирует случайный идентификатор.
//
void TGenerateId (OUT char *dstId) {


    TGenerateRandomString (ID_SIZE, dstId);
    dstId[ID_SIZE] = 0;

    return;
}


//
// Генерирует случайную строку.
//
void TGenerateRandomString (unsigned int count, OUT char *dst) {

unsigned int i;
int fd;

    fd = open ("/dev/urandom", O_RDONLY);

    for (i = 0; i < count; ++i) {
        size_t rnd = 0;
        read (fd, &rnd, 1);
        dst[i] = 'a' + (rnd % 26);
        }

    close (fd);

    return;
}


//
// По дескриптору директории возвращает следующий существующий идентификатор.
//
bool TGetNextId (IN DIR *dir, OUT char *outId) {

struct stat statFile;
char path[PATH_MAX];
struct dirent *ent;

    // читаем записи директории
    while ( (ent = readdir (dir)) != NULL) {

        TGetMetaPath (ent->d_name, path);
        stat (path, &statFile);

        // если обычный файл, значит это идентификатор
        if (S_ISREG (statFile.st_mode)) {
            strcpy (outId, ent->d_name);
            return TRUE;
            }
        }

    return FALSE;
}

