#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <libnitrokey/NK_C_API.h>

class Nitrokey
{
private:
    const std::size_t slot_count = 16;
    bool present = false;
    bool unlocked = false;

    bool login() { return (1 == NK_login_auto()); }
    void logout() { NK_logout(); }
    bool unlock(const char* password) { return (0 == NK_enable_password_safe(password)); }
    void lock() { NK_lock_device(); }
    void set_debug(bool value) { NK_set_debug(value); }
    bool is_user_pin_locked() { return (0 == NK_get_user_retry_count()); }

    const std::vector<uint8_t> get_slot_status() {
        uint8_t *slots_ptr = NK_get_password_safe_slot_status();
        std::vector<uint8_t> slots;
        for (uint8_t slot = 0; slot < this->slot_count; ++slot) {
            slots.push_back(slots_ptr[slot]);
        }
        delete[] slots_ptr;
        return slots;
    }

    std::string get_slot_name(uint8_t slot_number) {
        const char* name_ptr = NK_get_password_safe_slot_name(slot_number);
        std::string name(name_ptr);
        if (name_ptr) {
            free(const_cast<char*>(name_ptr));
        }
        return name;
    }

    std::string get_slot_password(uint8_t slot_number) {
        const char* password_ptr = NK_get_password_safe_slot_password(slot_number);
        std::string password{password_ptr};
        if (password_ptr) {
            free(const_cast<char*>(password_ptr));
        }
        return password;
    }

    void detect(std::size_t detect_timeout_ms, std::size_t detect_interval_ms) {
        std::size_t detect_retry_count = detect_timeout_ms / detect_interval_ms;

        for (std::size_t retry = 0; retry < detect_retry_count; ++retry) {
            if (this->login()) {
                std::cerr << "[INFO] Nitrokey found" << std::endl;
                this->present = is_user_pin_locked() ? false : true;
                return;
            }
            usleep(detect_interval_ms * 1000);
        }

        this->present = false;
        std::cerr << "[INFO] Nitrokey not found" << std::endl;
    }

public:
    Nitrokey(std::size_t detect_timeout_ms = 5000, std::size_t detect_interval_ms = 250) {
        this->set_debug(false);
        this->detect(detect_timeout_ms, detect_interval_ms);
    }

    ~Nitrokey() {
        this->lock();
        this->logout();
    }

    bool is_present() { return this->present; }
    bool is_unlocked() { return this->unlocked; }

    void unlock_safe(std::string password) {
        this->unlocked = this->unlock(password.c_str());
        std::cerr << "[INFO] Nitrokey unlock " << (this->unlocked ? "successful" : "failed") << std::endl;
    }

    std::string find_slot_content(std::string const slot_name) {
        if (!this->unlocked) {
            return "";
        }

        auto slots = this->get_slot_status();
        for (std::size_t slot = 0; slot < slots.size(); ++slot) {
            if (slots[slot] == 1) {
                if(slot_name == this->get_slot_name(slot)) {
                    return this->get_slot_password(slot);
                }
            }
        }

        std::cerr << "[INFO] Slot not found" << std::endl;
        return "";
    }
};
