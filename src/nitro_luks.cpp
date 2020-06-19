#include <cstdio>
#include <cstring>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "nitrokey.h"

// Return status codes
#define RET_ERROR (-1)
#define RET_OK (0)

// Parameters
#define MAX_PIN_LENGTH 20

// Function prototypes
std::string read_password();

// Entry point
int main(int argc, char const *argv[])
{
    Nitrokey nitrokey{};
    std::string slot_name{"LUKS"};

    // Parse arguments
    if (2 == argc) {
        std::string arg{argv[1]};
        if ("-d" == arg) {
            // Detection only
            return nitrokey.is_present() ? RET_OK : RET_ERROR;
        } else {
            // Use argument as slot name
            slot_name = arg;
        }
    }

    // Read password from stdin and remove the trailing newline
    auto password = read_password();

    if (0 == password.length()) {
        return RET_ERROR;
    }

    if(!nitrokey.is_present()) {
        return RET_ERROR;
    }

    nitrokey.unlock_safe(password);
    if (!nitrokey.is_unlocked()) {
        return RET_ERROR;
    }

    auto slot_password = nitrokey.find_slot_content(slot_name);
    if (0 == slot_password.length()) {
        return RET_ERROR;
    }

    std::cout << slot_password;
    return RET_OK;
}

std::string read_password()
{
    struct termios saved_attributes;
    struct termios tattr;

    // Disable echo
    tcgetattr(STDIN_FILENO, &saved_attributes);
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);

    // Read password and remove trailing new line
    char password[MAX_PIN_LENGTH + 1] = {'\0'};
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    // Reset stdin
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);

    return std::string(password);
}
