#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <termios.h>
#include <unistd.h>
#include <libnitrokey/NK_C_API.h>

// return status codes
#define ERROR 1
#define RETRY_PASSWORD 2
#define PASSWORD_OK 0

// libnitrokey status codes
#define STATUS_OK 0
#define NITROKEY_FOUND 1

// parameters
#define SLOT_COUNT 16
#define MAX_PIN_LENGTH 20
#define NITROKEY_WAIT_TIMEOUT_MS 5000
#define NITROKEY_WAIT_RETRY_INTERVAL_MS 250
#define NITROKEY_MAX_RETRY_COUNT (NITROKEY_WAIT_TIMEOUT_MS / NITROKEY_WAIT_RETRY_INTERVAL_MS)

struct termios saved_attributes;

int error(char const *msg)
{
    fprintf(stderr, "%s \n*** Falling back to default LUKS password entry.\n", msg);
    return ERROR;
}

void disable_echo(void)
{
  struct termios tattr;
  tcgetattr(STDIN_FILENO, &saved_attributes);

  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

void reset_input_mode(void)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

int main(int argc, char const *argv[])
{
    // Disable debug messages
    NK_set_debug(false);
    
    // Check for nitrokey
    int login_status = !NITROKEY_FOUND;
    for (int retry = 0; retry < NITROKEY_MAX_RETRY_COUNT; ++retry) {
        login_status = NK_login_auto();
        if (login_status == NITROKEY_FOUND) {
        const char* serial = NK_device_serial_number();
        fprintf(stderr, "*** Nitrokey : %s found!\n", serial);
            break;
    }
        usleep(NITROKEY_WAIT_RETRY_INTERVAL_MS*1000);
    }
    if (login_status != NITROKEY_FOUND) {
        return error("*** No nitrokey detected.");
    }

    // Check if user pin is locked
    uint8_t retry_count = NK_get_user_retry_count();
    if (retry_count == 0) {
        return error("*** User PIN locked.");
    }

    // Read password from stdin and remove the trailing newline
    char password[MAX_PIN_LENGTH + 1] = {'\0'};
        disable_echo();
        fgets(password, sizeof(password), stdin);
        reset_input_mode();
        password[strcspn(password, "\n")] = 0;

    // Check password length
    if (strlen(password) == 0) {
        return error("*** Empty password, falling back to LUKS passphrase");
        }

    // Unlock password safe
    int password_safe_status = NK_enable_password_safe(password);
    if (password_safe_status != STATUS_OK) {
        fprintf(stderr, "*** Error while accessing password safe.\n");
        return RETRY_PASSWORD;
    }

    fprintf(stderr, "*** Scanning the nitrokey slots...\n");
    slots = NK_get_password_safe_slot_status();
    for (uint8_t slot = 0; slot < SLOT_COUNT; ++slot) 
    {
        if (slots[slot] == 1)
        {
            const char* slotname = NK_get_password_safe_slot_name(slot);
            if(strcmp(slotname, "LUKS") == 0)
                slot_number = slot;
        } 
        else 
        {
            empty_slots++;
        }
    }

    if (empty_slots == SLOT_COUNT)
    {
        return error("*** No slots enabled.\n");
    } 
    else if(slot_number == 255) 
    {
        return error("*** No slot configured by name LUKS.\n");
    }

    // At this point we found a valid slot, go ahead and fetch the password.
    LUKS_password = NK_get_password_safe_slot_password(slot_number);
    
    // print password to stdout
    fprintf(stdout, "%s", LUKS_password);

    // Close the device and disconnect
    NK_lock_device();
    NK_logout();

    return PASSWORD_OK;
}
