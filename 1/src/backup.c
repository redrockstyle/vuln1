// backup.c
//
// Зачётное задание 1.
//
// Маткин Илья Александрович    04.05.2015

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <linux/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>

#include "utils/files.h"

//----------------------------------------

#define BACKUP_INDEX_FILE "/index"

int glLogFd;
int glIndexFd;

//----------------------------------------



//
// Проверяет присутствие идентификатора в файле индекса.
//
static bool TFindFileInIndex (IN const char *id) {

int indexFd;
char str[MAX_STRING_SIZE + 1];
bool res = FALSE;

    indexFd = open (BACKUP_INDEX_FILE, O_RDONLY);

    while (TGetString (indexFd, str)) {
        if (!strcmp (str, id)) {
            res = TRUE;
            break;
            }
        }

    close (indexFd);

    return res;
}


//
// Возращает размер сохранённого файла.
//
static size_t TGetBackupFileSize (IN const char *id) {

char backupPath[PATH_MAX];
struct stat st;

    TGetBackupPath (id, backupPath);
    stat (backupPath, &st);

    return st.st_size;
}


//
// Копирует файл.
//
static bool TCopyFile (IN const char *dst, IN const char *src) {

unsigned char buf[100];
int dstFd;
int srcFd;
int size;

    srcFd = open (src, O_RDONLY);
    if (srcFd == -1) {
        return FALSE;
        }
    dstFd = open (dst, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (dstFd == -1) {
        close (srcFd);
        return FALSE;
        }

    while ((size = read (srcFd, buf, 100)) > 0) {
        write (dstFd, buf, size);
        }

    close (srcFd);
    close (dstFd);

    return TRUE;
}


//
// Выводит сообщение в лог.
//
static void TLogMessage (int fd, IN const char *message1, IN const char *message2) {


    write (fd, message1, strlen (message1));
    write (fd, message2, strlen (message2));
    write (fd, "\n", 1);

    return;
}


//
// Сохраняет информацию о новом файле.
//
static void TSaveBackupInfo (IN const char *id) {

char *value = NULL;
int logFd = glLogFd;
char buf[MAX_STRING_SIZE];
char path[PATH_MAX];

    // ищем в метаданных дополнительную информацию,
    // которую отобразим в логах
    value = TGetMetaParam (id, "BackupInfo", buf);

    TPutString (glIndexFd, id);
    TPutString (glIndexFd, "\n");
    TLogMessage (logFd, "Add file ", id);
    if (value) {
        TLogMessage (logFd, "Info: ", value);
        }

    return;
}


//
// Создаёт/обновляет резервную копию.
//
static void TBackupFile (IN const char *id) {

char backupPath[PATH_MAX];
char filesPath[PATH_MAX];
char fileName[MAX_STRING_SIZE + 1];

    TGetFileNameById (id, fileName);
    TGetFilesPath (fileName, filesPath);
    TGetBackupPath (id, backupPath);

    TCopyFile (backupPath, filesPath);

    return;
}


//
// Обновляет (при необходимости) резервную копию файла.
//
static void TUpdateFile (IN const char *id) {

char msg[ID_SIZE + 2 + 10 + 4 + 10 + 1]; // буфер для строки "id: prev_size -> new_size\0"
size_t newSize;
size_t prevSize;
int logFd = glLogFd;

    newSize = TGetFileSize (id);
    prevSize = TGetBackupFileSize (id);

    if (newSize != prevSize) {
        TBackupFile (id);
        sprintf (msg, "%s: %u -> %u", id, prevSize, newSize);
        TLogMessage (logFd, "Update ", msg);
        }

    return;
}


//
// Осуществляет (при необходимости) резервное копирование файлов.
//
static void TCheckFilesForBackup (void) {

DIR *dir;
char id[PATH_MAX + 1];

    dir = opendir (META_DIR);

    while (TGetNextId (dir, id)) {
        if (TFindFileInIndex (id)) {
            TUpdateFile (id);
            }
        else {
            // длина идентификатора должна иметь фиксированную длину
            if (strlen (id) == ID_SIZE) {
                TBackupFile (id);
                TSaveBackupInfo (id);
                }
            else {
                TLogMessage (glLogFd, "Error length id: ", id);
                }
            }
        }

    closedir (dir);

    return;
}



int main (unsigned int argc, char *argv[], char *envp[]) {

    // предполагается работа в фоновом режиме,
    // поэтому закрываем все лишние дескрипторы
    close (0);
    close (1);
    close (2);

    glIndexFd = open (BACKUP_INDEX_FILE, O_WRONLY | O_APPEND);
    glLogFd = open ("/dev/tty", O_WRONLY);

    TCheckFilesForBackup();

    close (glLogFd);
    close (glIndexFd);

    return 0;
}

