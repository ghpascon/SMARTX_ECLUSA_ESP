#include <Arduino.h>

#define FRAME_TIMEOUT 100 // ms para considerar fim de frame
class WIEGAND_READER
{
public:
    WIEGAND_READER(uint8_t pinD0, uint8_t pinD1, const char *name)
        : _pinD0(pinD0), _pinD1(pinD1), _name(name)
    {
        _bitCount = 0;
        _lastMillis = 0;
        lastStateD0 = HIGH;
        lastStateD1 = HIGH;
        memset(_bitBuffer, 0, sizeof(_bitBuffer));
    }

    void setup()
    {
        pinMode(_pinD0, INPUT_PULLUP);
        pinMode(_pinD1, INPUT_PULLUP);
    }

    void loop()
    {
        uint8_t stateD0 = digitalRead(_pinD0);
        uint8_t stateD1 = digitalRead(_pinD1);

        if (lastStateD0 == HIGH && stateD0 == LOW)
            handleBit('0');
        if (lastStateD1 == HIGH && stateD1 == LOW)
            handleBit('1');

        lastStateD0 = stateD0;
        lastStateD1 = stateD1;

        if (_bitCount > 0 && (millis() - _lastMillis > FRAME_TIMEOUT))
        {
            _bitBuffer[_bitCount] = '\0';

            // Envia a sequência bruta de bits
            unsigned long currentMillis = millis();
            if (currentMillis - last_close > 5000 && currentMillis - last_send > 1000)
                write_data(String("#wg:") + _name + ":" + String(_bitBuffer));
            last_send = currentMillis;

            _bitCount = 0;
            memset(_bitBuffer, 0, sizeof(_bitBuffer));
        }
    }

    bool is_reading() const
    {
        if (_bitCount == 0)
            return false;

        return (millis() - _lastMillis) <= FRAME_TIMEOUT;
    }

private:
    uint8_t _pinD0, _pinD1;
    const char *_name;

    char _bitBuffer[128]; // suporta até 128 bits (maior que Wiegand-66)
    volatile uint8_t _bitCount;
    volatile unsigned long _lastMillis;

    uint8_t lastStateD0;
    uint8_t lastStateD1;

    void handleBit(char bitChar)
    {
        if (_bitCount < sizeof(_bitBuffer) - 1)
        {
            _bitBuffer[_bitCount++] = bitChar;
        }
        _lastMillis = millis();
    }
};

// === Instâncias para 2 leitores ===
WIEGAND_READER wiegand1(PIN_D0_1, PIN_D1_1, "1");
WIEGAND_READER wiegand2(PIN_D0_2, PIN_D1_2, "2");

bool wiegand_is_reading()
{
    return wiegand1.is_reading() || wiegand2.is_reading();
}
