#include <Windows.h>
#include <iostream>

DWORD returnAddressAmmo;
DWORD returnAddressTbot;

INPUT input{};
DWORD isLookingAtPlayer;
DWORD renderTextFunc = 0x004607C0;


bool HookFunction(void* dest, void* ourFunction, int length) {
    if (length < 5) {
        return false;
    }
    DWORD oldProtect;
    VirtualProtect(dest, length, PAGE_EXECUTE_READWRITE, &oldProtect);
    memset(dest, 0x90, length);
    DWORD RelativeAddress = ((DWORD)ourFunction - (DWORD)dest - 5);
    *(BYTE*)dest = 0xE9;
    *(DWORD*)((DWORD)dest + 1) = RelativeAddress;
    DWORD buffer;
    VirtualProtect(dest, length, oldProtect, &buffer);

    return true;

}

void __declspec(naked) AmmoFunc() {
    __asm {
        pushad
    }

    __asm {
        popad
        mov esi, dword ptr ds:[esi+14]
        jmp[returnAddressAmmo]
    }

}

void __declspec(naked) TbotFunc() {
    __asm {
        call [renderTextFunc]
        pushad
        mov isLookingAtPlayer, eax
    }

    if (isLookingAtPlayer != 0) {
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }
    else {
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));

    }

    __asm {
        popad
        jmp [returnAddressTbot]
    }

}

DWORD WINAPI InternalThread(HMODULE hModule){
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "<--- Assault Cube Trainer -->" << std::endl;
    std::cout << "[NUMPAD1]Set Teleport Coords" << std::endl;
    std::cout << "[NUMPAD2]Teleport" << std::endl;
    std::cout << "[NUMPAD3]Exit" << std::endl;

    DWORD moduleBaseAddress = (DWORD)GetModuleHandle(L"ac_client.exe");
    DWORD playerObjBasePointer = moduleBaseAddress + 0x109B74;
    DWORD localPlayerObj = *(DWORD*)(playerObjBasePointer);

    DWORD ammoHookLocation = 0x004637E6;
    DWORD tbotHookLocation = 0x0040AD9D;
    int hL = 5;
    returnAddressAmmo = ammoHookLocation + hL;
    returnAddressTbot = tbotHookLocation + hL;
    HookFunction((void*)ammoHookLocation, AmmoFunc, hL);
    HookFunction((void*)tbotHookLocation, TbotFunc, hL);

    float xPos, zPos, yPos = 0.0;




    while (true){
        *(DWORD*)(localPlayerObj + 0xF8) = 500;


        if (GetAsyncKeyState(VK_NUMPAD3) & 1){
            break;
        }

        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            xPos = *(float*)(localPlayerObj + 0x34);
            zPos = *(float*)(localPlayerObj + 0x38);
            yPos = *(float*)(localPlayerObj + 0x3C);
        }

        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            *(float*)(localPlayerObj + 0x34) = xPos;
            *(float*)(localPlayerObj + 0x38) = zPos;
            *(float*)(localPlayerObj + 0x3C) = yPos;

        }

    }

    FreeConsole();
    if (f) { fclose(f); };
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InternalThread, hModule, 0, 0));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

