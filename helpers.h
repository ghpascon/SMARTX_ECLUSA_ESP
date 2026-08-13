extern CONNECTION connection;

void write_data(String data)
{
    Serial.println(data);
    connection.telnet_write(data);
}