extern CONNECTION connection;

String readLine(Stream &stream)
{
    String cmd = "";
    char c;
    while (stream.available())
    {
        c = stream.read();
        if (c == '\r' || c == '\n')
            break;
        cmd += c;
    }
    return cmd;
}

void write_data(String data)
{
    Serial.println(data);
    my_usb.println(data);
    connection.telnet_write(data);
}