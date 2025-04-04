#ifndef MISC_H
#define MISC_H

#include "../game/sdk.h"
#include "config.h"
#include "../overlay/overlay.h"

using namespace std::chrono_literals;

typedef ULONG(NTAPI* NtUserSendInput_t)(
    _In_ ULONG cInputs,
    _In_ LPINPUT pInputs,
    _In_ LONG cbSize
    );


const uint32_t STANDING = 65665;
const uint32_t CROUCHING = 65667;
const uint32_t PLUS_JUMP = 65537;
const uint32_t MINUS_JUMP = 256;


inline class _misc
{
public:
    NtUserSendInput_t NtUserSendInput;

    void think();
}misc;

#endif