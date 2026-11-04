    .386
    .model flat

    extern _WinVidExitMessage:proc
    extern _GhInstance:dword
    extern _ClassName:byte
    extern __imp__UnregisterClassA@8:dword

    .code

_WinVidFreeWindow proc public
    call _WinVidExitMessage
    jmp near ptr unregister_window
    nop
    nop
    nop
    nop
    nop
    nop
unregister_window:
    mov eax, _GhInstance
    push eax
    push offset _ClassName
    call dword ptr [__imp__UnregisterClassA@8]
    ret
_WinVidFreeWindow endp

    end
