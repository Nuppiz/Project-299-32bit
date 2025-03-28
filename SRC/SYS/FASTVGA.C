void fast_vga_memcpy(void __far *dest, void __far *src, unsigned int count)
{
    _asm {
        push    esi              // Save source index
        push    edi              // Save destination index
        
        mov     edi, dword ptr [dest]    // Load destination ptr
        mov     esi, dword ptr [src]     // Load source ptr
        mov     ecx, dword ptr [count]   // Load count
        
        // Check if count is small
        cmp     ecx, 16
        jbe     small_copy
        
        // Handle alignment
        mov     eax, edi
        and     eax, 3           // Check lower 2 bits
        jz      aligned          // If already aligned, skip
        
        mov     edx, 4
        sub     edx, eax         // Bytes needed for alignment
        sub     ecx, edx         // Adjust count
        
        // Copy alignment bytes
    align_loop:
        mov     al, [esi]
        mov     [edi], al
        inc     esi
        inc     edi
        dec     edx
        jnz     align_loop
        
    aligned:
        // Save remaining bytes count
        mov     eax, ecx
        shr     ecx, 2           // ECX = DWORDs to copy
        
        // Copy DWORDs
        cld
        rep     movsd
        
        // Handle remaining bytes
        mov     ecx, eax
        and     ecx, 3
        rep     movsb
        jmp     done
        
    small_copy:
        // For small transfers
        cld
        rep     movsb
        
    done:
        pop     edi
        pop     esi
    }
}
