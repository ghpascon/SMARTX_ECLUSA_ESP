#pragma once

#include <Arduino.h>
#include <ctype.h>

#include "vars.h"

bool open_door(int id);
void write_data(String data);

static const uint32_t COMMAND_ACTIVITY_RESET_MS = 60000;
static const uint32_t COMMAND_ACTIVITY_TIMEOUT_MS = 5000;
static const uint32_t COMMAND_STALE_INPUT_MS = 3000;
static const size_t MAX_COMMAND_LENGTH = 256;

unsigned long last_command_receive_ms = 0;
String serial_command_buffer = "";
String usb_command_buffer = "";
unsigned long serial_last_byte_ms = 0;
unsigned long usb_last_byte_ms = 0;

inline bool parse_strict_int(const String &text, int min_value, int max_value, int &out)
{
    if (text.length() == 0)
        return false;

    for (size_t i = 0; i < text.length(); ++i)
    {
        if (!isdigit((unsigned char)text[i]))
            return false;
    }

    long parsed = text.toInt();
    if (parsed < min_value || parsed > max_value)
        return false;

    out = (int)parsed;
    return true;
}

inline String sanitize_command(const String &raw)
{
    String sanitized = "";
    sanitized.reserve(raw.length());

    for (size_t i = 0; i < raw.length(); ++i)
    {
        char c = raw[i];
        if (c == '\t')
            c = ' ';

        if (c >= 32 && c <= 126)
            sanitized += c;
    }

    sanitized.trim();
    return sanitized;
}

inline void mark_command_activity()
{
    last_command_receive_ms = millis();
    is_connected = true;
}

inline void update_command_connection_state()
{
    if (last_command_receive_ms == 0)
    {
        is_connected = false;
        return;
    }

    const unsigned long now = millis();
    is_connected = (now - last_command_receive_ms) <= COMMAND_ACTIVITY_TIMEOUT_MS;
    if (now - last_command_receive_ms >= COMMAND_ACTIVITY_RESET_MS)
    {
        ESP.restart();
    }
}

inline void command_error(const String &origin, const String &reason)
{
    write_data("#ERROR:" + reason);
}

inline bool process_command(const String &raw_command, const String &origin)
{
    String cmd = sanitize_command(raw_command);
    if (cmd.length() == 0)
        return false;

    if (cmd.length() > MAX_COMMAND_LENGTH)
    {
        command_error(origin, "CMD_TOO_LONG");
        return false;
    }

    String cmd_lc = cmd;
    cmd_lc.toLowerCase();
    mark_command_activity();

    if (cmd_lc == "#ping")
    {
        write_data("#pong");
        return true;
    }

    if (cmd_lc == "#status")
    {
        String status = "#STATUS:connected=" + String(is_connected ? "1" : "0") +
                        ",tags=" + String(tags_qty) +
                        ",authorized=" + String(authorized);
        write_data(status);
        return true;
    }

    if (cmd_lc.startsWith("#open:"))
    {
        if (emg_active)
        {
            command_error(origin, "EMG_ACTIVE");
            return false;
        }

        int sep = cmd_lc.indexOf(':');
        String door_text = cmd_lc.substring(sep + 1);
        door_text.trim();

        int door = 0;
        if (!parse_strict_int(door_text, 1, 2, door))
        {
            command_error(origin, "INVALID_DOOR");
            return false;
        }

        if (!open_door(door))
        {
            command_error(origin, "DOOR_BUSY");
            return false;
        }

        write_data("#OPEN:" + String(door));
        return true;
    }

    if (cmd_lc.startsWith("#authorized:") || cmd_lc.startsWith("#denied:"))
    {
        const bool approved = cmd_lc.startsWith("#authorized:");
        const String prefix = approved ? "#authorized:" : "#denied:";
        String payload = cmd.substring(prefix.length());
        payload.trim();

        if (payload.length() == 0)
        {
            command_error(origin, "MISSING_CARD");
            return false;
        }

        String door_text = "";
        String card_id = payload;
        int sep = payload.indexOf(':');
        bool has_door = sep >= 0;

        if (has_door)
        {
            door_text = payload.substring(0, sep);
            card_id = payload.substring(sep + 1);
            door_text.trim();
            card_id.trim();

            if (card_id.length() == 0)
            {
                command_error(origin, "MISSING_CARD");
                return false;
            }
        }

        int door = 0;
        if (has_door && !parse_strict_int(door_text, 1, 2, door))
        {
            command_error(origin, "INVALID_DOOR");
            return false;
        }

        identification = card_id;
        authorized = approved ? 1 : 2;

        if (approved && has_door)
        {
            if (!open_door(door))
            {
                command_error(origin, "DOOR_BUSY");
                return false;
            }
        }

        write_data(approved ? "#AUTHORIZED" : "#DENIED");
        if (has_door)
            write_data("#DOOR:" + String(door));
        write_data("#CARD:" + card_id);
        return true;
    }

    if (cmd_lc.startsWith("#tags_qty:"))
    {
        int sep = cmd_lc.indexOf(':');
        String qty_text = cmd_lc.substring(sep + 1);
        qty_text.trim();

        int qty = 0;
        if (!parse_strict_int(qty_text, 0, 1000000, qty))
        {
            command_error(origin, "INVALID_TAGS_QTY");
            return false;
        }

        tags_qty = qty;
        write_data("#TAGS_QTY:" + String(tags_qty));
        return true;
    }

    command_error(origin, "INVALID_CMD");
    return false;
}

inline bool collect_stream_command(Stream &stream, String &buffer, unsigned long &last_byte_ms, String &out_command)
{
    while (stream.available())
    {
        int raw = stream.read();
        if (raw < 0)
            break;

        char c = (char)raw;

        if (c == '\b' || c == 0x7F)
        {
            if (buffer.length() > 0)
                buffer.remove(buffer.length() - 1);
            continue;
        }

        if (c == '\r' || c == '\n')
        {
            out_command = sanitize_command(buffer);
            buffer = "";
            last_byte_ms = 0;
            if (out_command.length() > 0)
                return true;
            continue;
        }

        if (c == '\0')
            continue;

        if (c == '\t')
            c = ' ';

        if (c < 32 || c > 126)
            continue;

        if (buffer.length() >= MAX_COMMAND_LENGTH)
        {
            buffer = "";
            last_byte_ms = 0;
            return false;
        }

        buffer += c;
        last_byte_ms = millis();
    }

    if (buffer.length() > 0 && last_byte_ms > 0)
    {
        if (millis() - last_byte_ms >= COMMAND_STALE_INPUT_MS)
        {
            buffer = "";
            last_byte_ms = 0;
        }
    }

    return false;
}

inline void process_stream_commands(Stream &stream, String &buffer, unsigned long &last_byte_ms, const String &origin)
{
    while (true)
    {
        String command_line;
        if (!collect_stream_command(stream, buffer, last_byte_ms, command_line))
            break;

        process_command(command_line, origin);
    }
}
