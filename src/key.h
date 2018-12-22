#ifndef CG_SEM5_KEY_H
#define CG_SEM5_KEY_H

struct Key
{
    enum class Modifiers : unsigned int
    {
        NONE    = 0x0,
        COMMAND = 0x1
    } modifiers;

    enum class State : unsigned int
    {
        PRESSED,
        RELEASED
    } state;

    unsigned int code;
};

#endif // CG_SEM5_KEY_H