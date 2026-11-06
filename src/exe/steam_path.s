    .386
    .model flat

    extern _DriveLetter:byte
    extern _FullPathBuffer:byte
    extern _CDPathFormat:byte
    extern __imp__wsprintfA:dword

    .code

_GetFullPath proc public
    movsx ecx, _DriveLetter
    mov eax, [esp+4]
    push eax
    nop
    push offset _CDPathFormat+4
    push offset _FullPathBuffer
    call dword ptr [__imp__wsprintfA]
    add esp, 12
    mov eax, offset _FullPathBuffer
    ret
_GetFullPath endp

    end
