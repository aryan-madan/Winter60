// Copyright 2026 aryanworks
// SPDX-License-Identifier: GPL-3.0-or-later

#include QMK_KEYBOARD_H

#if defined(OLED_ENABLE)

static uint8_t  pet_hunger    = 20;
static uint8_t  pet_happiness = 80;
static uint8_t  pet_energy    = 80;
static uint16_t pet_keys      = 0;
static uint32_t pet_timer     = 0;
static uint32_t pet_activity  = 0;

static uint8_t pet_clamp_add(uint8_t value, uint8_t amount) {
    return value > 255 - amount ? 255 : value + amount;
}

static uint8_t pet_clamp_sub(uint8_t value, uint8_t amount) {
    return value < amount ? 0 : value - amount;
}

static void pet_tick(void) {
    if (timer_elapsed32(pet_timer) < 10000) {
        return;
    }

    pet_timer     = timer_read32();
    pet_hunger    = pet_clamp_add(pet_hunger, 4);
    pet_happiness = pet_clamp_sub(pet_happiness, pet_hunger > 75 ? 5 : 2);
    pet_energy    = pet_clamp_sub(pet_energy, 2);
}

static void pet_write_bar(uint8_t value, bool inverted) {
    const uint8_t filled = value / 20;

    oled_write_char('[', false);
    for (uint8_t i = 0; i < 5; i++) {
        oled_write_char(i < filled ? '#' : '-', inverted);
    }
    oled_write_char(']', false);
}

static const char *pet_face(void) {
    if (pet_hunger > 85) {
        return "x_x hungry";
    }
    if (pet_happiness < 25) {
        return ";_; bored ";
    }
    if (pet_energy < 25) {
        return "-_- sleepy";
    }
    if (timer_elapsed32(pet_activity) < 1500) {
        return "o_o happy ";
    }
    return "^_^ cozy  ";
}

void keyboard_post_init_kb(void) {
    pet_timer    = timer_read32();
    pet_activity = pet_timer;
    keyboard_post_init_user();
}

void post_process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        pet_activity  = timer_read32();
        pet_happiness = pet_clamp_add(pet_happiness, 1);
        pet_energy    = pet_clamp_sub(pet_energy, 1);
        pet_keys++;

        if (keycode == KC_SPC || keycode == KC_ENT) {
            pet_hunger = pet_clamp_sub(pet_hunger, 4);
        } else if ((pet_keys % 16) == 0) {
            pet_hunger = pet_clamp_sub(pet_hunger, 1);
        }
    }

    post_process_record_user(keycode, record);
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }

    pet_tick();

    oled_set_cursor(0, 0);
    oled_write_P(PSTR("Winter60 Tama "), false);
    oled_write(pet_face(), false);

    oled_set_cursor(0, 1);
    oled_write_P(PSTR("Food "), false);
    pet_write_bar(100 - pet_hunger, pet_hunger > 75);
    oled_write_P(PSTR(" L"), false);
    oled_write_char('0' + get_highest_layer(layer_state), false);

    oled_set_cursor(0, 2);
    oled_write_P(PSTR("Mood "), false);
    pet_write_bar(pet_happiness, pet_happiness < 30);
    oled_write_P(PSTR(" Vol"), false);

    oled_set_cursor(0, 3);
    oled_write_P(PSTR("Rest "), false);
    pet_write_bar(pet_energy, pet_energy < 30);
    oled_write_P(PSTR(" Knob"), false);

    return false;
}

#endif
