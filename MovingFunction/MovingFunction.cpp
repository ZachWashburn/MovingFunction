
/*

    11/16/2021 - Zachary Washburn - zachary.washburn.business@gmail.com
    
    This is a simple function that will move itself around in memory by allocating new memory, copying itself over,
    and running itself there. DEBUG mode will print out the new base address of the function, and you can make it
    free its previous instances by defining FREE_PREVIOUS_MEMORY.

    Everything is calculated using rel offsets so you should be able to add whatever code you would like inbetween it as
    long as you preserver the register and stack!

    You will need to set the .text section to have PAGE_EXECUTE_READWRITE rights, aswell as turn of Data Execution Prevention
    for this code to work. with a bit of changes you could just not modify the first instance (no .text modification) and
    call VirtualProtect to not have these requirements.

    For unknown reasons to me, attaching the VS debugger breaks the code? 
*/
#include <iostream>
#include <Windows.h>
#include <vector>

//#define FREE_PREVIOUS_MEMORY


// Eventually malloc will fail with error (not enough memory) if we aren't freeing it
void* __cdecl OurMalloc(size_t nRequestedMemorySize);
void FreeAllMemory();

const char* szDebugPrint = "New Instance at : %p\n";

#pragma optimize("", off)
static void __declspec(naked) MovingFunction()
{
    // ebp + 4 = malloc
    // ebp + 8 = free
    // ebp + 12 = memcpy
    // ebp + 16 = base address
    // ebp + 20 = previous occurance
    // ebp + 24 = function size
    // ebp + 28 = printf (debug only)
    _asm {
start_of_function:
        nop
        mov ebp, esp
        mov eax, ebp
        add eax, 20
        cmp[eax], 0    // previous occurance?
        je no_free      // should we attempt to free (is this the first iteration?)
#ifdef FREE_PREVIOUS_MEMORY
        mov ebx, ebp
        add ebx, 8
        push[eax]
        call[ebx]       // call free
        add esp, 4
#endif
        jmp after_free
no_free :
        _emit 0xE8      // call +5 so we can fetch ebp!
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        pop ebx         // get ebp

        // get start of function (no base address on stack, so we need to get it)
        // passing it in doesn't work well if you use compile time-linking functions
        lea eax, start_of_function
        lea edx, no_free
        sub edx, eax    // calculate offset to start
        sub ebx, edx    // calculate start addr
        sub ebx, 5      // remove the call!

        mov eax, ebp
        add eax, 16
        cmp[eax], 0
        mov[eax], ebx   // set the base address
        je after_free   // don't write when 0 in base address
        // lets just do our modifying code here
        // I was doing it at the bottom of the function
        // but it was causing the occasional issue
        // and it will take less code to do it here!
        lea ebx, cmp_operand
        lea eax, start_of_function
        sub ebx, eax    // calc offset
        mov eax, ebp
        add eax, 16
        mov eax, [eax]  // add offset to our base
        add eax, ebx
        add eax, 2
        xor ebx, ebx    // clear reg
        mov bl, [eax]   // move bye over
        inc bl          // inc byte
        mov[eax], bl    // write 
after_free :
        mov eax, ebp
        add eax, 4      // malloc
        mov ebx, ebp
        add ebx, 24     // function size
        push ebx
        push[ebx]      // function arg
        call[eax]      // call malloc
        add esp, 4
        // eax is our new function base address
        pop ebx
        push[ebx]       // push size operand 
        sub ebx, 8      // Base Address (src)
        push[ebx]       // push base address (src)
        push eax        // push memory address
        sub ebx, 4      // fetch memcpy 
        call[ebx]       // call memcpy
        add esp, 12     // fix stack 3 for memcpy
        mov ebx, ebp
        add ebx, 16     // fetch base address
        mov edx, [ebx]  // store our old base address
        mov[ebx], eax   // copy new base address over
        push eax        // save our next address rq
        mov eax, 2
cmp_operand:
        cmp eax, 0      // do we bother?
        jne jump_to_function // so the first call (no-malloc'd memory) doesn't try and get free'd
        add ebx, 4      // get the previous occurance
        mov[ebx], edx   // copy our base to the previous occurnace
#ifdef _DEBUG           // call printf so we can see where we are in memory
        push [esp]
        push szDebugPrint
        mov eax, ebp
        add eax, 28
        call [eax]
        add esp, 8
#endif
        jmp jump_to_function
jump_to_function :
        pop eax
        jmp eax         // jump to the new base address
    }
}

void __declspec(noinline) CreateMovingFunction()
{

    _asm {
#ifdef _DEBUG
        push printf
#endif
        push 300
        push 0
        push 0
        push memcpy
        push free
#ifdef FREE_PREVIOUS_MEMORY
        push malloc
#else
        push OurMalloc
#endif
        call MovingFunction
        // never will get here!
#ifdef _DEBUG
        add esp, 28
#else 
        add esp, 24
#endif
    }
}

// Keep track of the memory
std::vector<void*> g_vecAllocatedMemory;
void* __cdecl OurMalloc(size_t nRequestedMemorySize)
{
    void* pMemory = malloc(nRequestedMemorySize);
    if (pMemory)
        g_vecAllocatedMemory.push_back(pMemory);
    return pMemory;
}

// Free all the memory we asked for
void FreeAllMemory()
{
    printf("%d Intances Allocated on last run!\n", g_vecAllocatedMemory.size());

    for (void* pMem : g_vecAllocatedMemory)
        free(pMem);

    g_vecAllocatedMemory.clear();
}

int main()
{
    
    __try {
        CreateMovingFunction();
    } __except(ExceptionContinueSearch)
    {
        printf("Exception Thrown! Probably out of memory!, Freeing!\n");
        FreeAllMemory(); // free all memory
        _asm {
            add esp, 8 // give room for our ret address on stack (no stack overflow!)
        }
        main();
    }
}
#pragma optimize("", on)

