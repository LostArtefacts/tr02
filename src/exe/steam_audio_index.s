    .386
    .model flat

    extern _AudioIndexPath:byte
    extern _CD_CommandBuffer:dword
    extern _CD_AudioIndex:dword
    extern _malloc:proc
    extern __imp__CreateFileA@28:dword
    extern __imp__GetFileSize@8:dword
    extern __imp__ReadFile@20:dword
    extern __imp__CloseHandle@4:dword

    .code

_CD_LoadAudioIndex proc public
    sub esp, 4
    xor ebx, ebx
    push ebx
    push 80h
    push 3
    push ebx
    push ebx
    push 80000000h
    push offset _AudioIndexPath
    call dword ptr [__imp__CreateFileA@28]
    mov esi, eax
    cmp esi, -1
    je short allocation_result
    push ebx
    push esi
    call dword ptr [__imp__GetFileSize@8]
    mov edi, eax
    add eax, 50
    push eax
    call _malloc
    add esp, 4
    test eax, eax
allocation_result:
    je short index_failed
    mov _CD_AudioIndex, eax
    push ebx
    lea ecx, [esp+4]
    push ecx
    push edi
    push eax
    add eax, edi
    mov _CD_CommandBuffer, eax
    push esi
    call dword ptr [__imp__ReadFile@20]
    mov ebx, eax
    push esi
    call dword ptr [__imp__CloseHandle@4]
    test ebx, ebx
    je short index_failed
    mov esi, _CD_AudioIndex
    mov edi, esi
index_entry:
    xor eax, eax
    add esi, 3
index_field:
    xor ecx, ecx
    mov cl, 8
    rep movsb
    mov [edi], cl
    inc esi
    inc edi
    nop
    nop
    test eax, eax
    jne short index_line_end
    inc eax
    jmp short index_field
index_line_end:
    cmp esi, _CD_CommandBuffer
    je short index_success
    mov cx, [esi]
    cmp cx, 0a0dh
    je short index_next_line
    inc esi
    jmp short index_line_end
index_next_line:
    add esi, 2
    jmp short index_entry
    xor eax, eax
index_success:
    mov al, 1
    jmp short index_done
index_failed:
    xor eax, eax
index_done:
    add esp, 4
    ret
_CD_LoadAudioIndex endp

    end
