// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

typedef struct {
    void    *font;
    uint32_t timer;
} fw_mismatch_args_t;

static fw_mismatch_args_t fw_mismatch_state = {0};

bool fw_mismatch_init(ui_node_t *self) {
    painter_font_handle_t font = get_font_by_name("fira_code");
    if (font == NULL) {
        return false;
    }

    fw_mismatch_state.font = font;
    self->args             = &fw_mismatch_state;

    return true;
}

void fw_mismatch_render(const ui_node_t *self, painter_device_t display) {
    fw_mismatch_args_t *args = self->args;

    if ((2 * args->font->line_height) > self->size.y) {
        return;
    }

    if (!task_should_draw(&args->timer, SECONDS(1))) {
        return;
    }

    // print build id

    const u128 id = get_build_id();

    string_t str = str_new(50);
    str_append(&str, "0x");

    // id is too long, copy only first few bytes
    for (size_t i = 0; i < 3; ++i) {
        str_printf(&str, "%x", id.bytes[i]);
    }
    str_append(&str, "...");

    const uint16_t id_width = qp_textwidth(args->font, str.ptr);
    if (id_width != 0 && id_width <= self->size.x) {
        const uint16_t x = self->start.x + (self->size.x - id_width) / 2;
        const uint16_t y = self->start.y + (self->size.y - (2 * args->font->line_height)) / 2;
        qp_drawtext(display, x, y, args->font, str.ptr);
    }

    // check master-slave matching ids

    u128 slave_id;
    if (!get_slave_build_id(&slave_id)) {
        return;
    }

    const bool fw_mismatch = memcmp(&id, &slave_id, sizeof(u128)) != 0;

    str_reset(&str);
    str_printf(&str, "FW:%s match" : (fw_mismatch) ? " doesn't" : "");

    const uint16_t match_width = qp_textwidth(args->font, str.ptr);
    if (match_width != 0 && match_width <= self->size.x) {
        const hsv_t fg = (fw_mismatch) ? (hsv_t){HSV_RED} : (hsv_t){HSV_GREEN};

        const uint16_t x = self->start.x + (self->size.x - match_width) / 2;
        const uint16_t y = self->start.y + (self->size.y - (2 * args->font->line_height)) / 2;
        qp_drawtext_recolor(display, x, y + args->font->line_height, args->font, str.ptr, fg.h, fg.s, fg.v, HSV_BLACK);
    }
}
