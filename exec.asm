use32

    xor eax, eax
    push eax
    ;mov eax, "rld!"
    ;push eax
    mov eax, "ute!"
    push eax
    mov eax, "exec"
    push eax
    xor eax,eax
    mov al,4
    xor ebx,ebx
    inc ebx
    mov ecx, esp
    xor edx,edx
    mov dl,13
    int 0x80

    xor eax,eax
    mov ebx,eax
    mov al,1
    int 0x80