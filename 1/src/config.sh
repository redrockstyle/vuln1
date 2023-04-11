cd bin
mkdir meta
chown user1:user1 meta
chmod 777 meta
mkdir files
chown user1:user1 files
chmod 777 files
mkdir backup_files
chown user1:user1 backup_files
chmod 755 backup_files
>index
chown user1:user1 index
chmod 600 index
chown user1:user1 key
chmod 400 key

chown user1:user1 backup
chmod 755 backup task1
chmod u+s backup

mkdir dev
mknod dev/full c 1 7
mknod dev/tty c 5 0
mknod dev/urandom c 1 9
chmod -R 777 dev

chmod 755 lib
chmod 755 lib/*

