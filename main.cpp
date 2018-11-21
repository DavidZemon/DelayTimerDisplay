#include <PropWare/hmi/output/printer.h>
#include <PropWare/serial/uart/uarttx.h>
#include <PropWare/hmi/output/ws2812.h>
#include <PropWare/memory/eeprom.h>

using PropWare::Printer;
using PropWare::UARTTX;
using PropWare::WS2812;
using PropWare::Eeprom;
using PropWare::Pin;

static const Pin::Mask RELAY_OUTPUT_MASK     = Pin::Mask::P0;
static const Pin::Mask RELAY_INPUT_MASK      = Pin::Mask::P1;
static const Pin::Mask CANCEL_INPUT_MASK     = Pin::Mask::P2;
static const Pin::Mask INCREMENT_BUTTON_MASK = Pin::Mask::P3;
static const Pin::Mask DECREMENT_BUTTON_MASK = Pin::Mask::P4;

#ifdef DEBUG
static const Pin::Mask    SERIAL_OUT_MASK  = Pin::to_mask(_cfg_txpin);
static const unsigned int SERIAL_BAUD_RATE = _cfg_baudrate;
#else
static const Pin::Mask    SERIAL_OUT_MASK  = Pin::Mask::P5;
static const unsigned int SERIAL_BAUD_RATE = 19200;
#endif

static const Pin::Mask    LED_OUT_MASK   = Pin::Mask::P6;
static const uint8_t      LED_INTENSITY  = 10;
static const unsigned int ACTIVE_COLOR   = WS2812::to_color(LED_INTENSITY, 0, 0);
static const unsigned int INACTIVE_COLOR = WS2812::to_color(0, LED_INTENSITY, 0);
static const unsigned int WARNING_COLOR  = WS2812::to_color(LED_INTENSITY, LED_INTENSITY, 0);
static const unsigned int ERROR_COLOR    = WS2812::to_color(LED_INTENSITY, 0, LED_INTENSITY);

static const unsigned int DEFAULT_DELAY_MILLIS     = 7500;
static const unsigned int MINIMUM_DELAY_MILLIS     = 100;
static const unsigned int MAXIMUM_DELAY_MILLIS     = 50000;
static const unsigned int ADJUSTMENT_VALUE         = 100;
static const unsigned int DELAY_WIGGLE_ROOM_MICROS = 500;

class RelayActivator {
    public:
        static const char FORM_FEED = 12;

    public:
        RelayActivator ()
                : m_relayOutput(RELAY_OUTPUT_MASK, Pin::Dir::OUT),
                  m_relayInput(RELAY_INPUT_MASK, Pin::Dir::IN),
                  m_cancelInput(CANCEL_INPUT_MASK, Pin::Dir::IN),
                  m_increment(INCREMENT_BUTTON_MASK, Pin::Dir::IN),
                  m_decrement(DECREMENT_BUTTON_MASK, Pin::Dir::IN),
                  m_statusLed(LED_OUT_MASK, WS2812::Type::GRB),
                  m_uart(SERIAL_OUT_MASK),
                  m_printer(this->m_uart, false) {
            this->m_relayOutput.clear();
            this->m_uart.set_baud_rate(SERIAL_BAUD_RATE);
        }

        void run () const {
            unsigned int delayMillis = DEFAULT_DELAY_MILLIS;

            while (1) {
                if (this->m_relayInput.read()) {
                    this->activateRelay(delayMillis);
                }

                if (this->m_increment.read()) {
                    delayMillis += ADJUSTMENT_VALUE;
                    this->updateDefaultDelay(delayMillis);
                }

                if (this->m_decrement.read()) {
                    delayMillis -= ADJUSTMENT_VALUE;
                    this->updateDefaultDelay(delayMillis);
                }
            }
        }

        void activateRelay (const unsigned int delayMillis) const {
            this->m_statusLed.send(ACTIVE_COLOR);
            this->m_relayOutput.set();
            const auto timeoutValue    = CNT + delayMillis * MILLISECOND;
            const auto minTimeoutValue = timeoutValue - DELAY_WIGGLE_ROOM_MICROS * MICROSECOND;
            const auto maxTimeoutValue = timeoutValue + DELAY_WIGGLE_ROOM_MICROS * MICROSECOND;
            while (!this->m_cancelInput.read() && minTimeoutValue < CNT && CNT < maxTimeoutValue);
            this->m_relayOutput.clear();

            if (this->m_cancelInput.read())
                this->blinkLed(ERROR_COLOR);

            this->m_statusLed.send(INACTIVE_COLOR);
        }

        void updateDefaultDelay (const unsigned int delayMillis) const {
            if (MINIMUM_DELAY_MILLIS <= delayMillis && delayMillis <= MAXIMUM_DELAY_MILLIS) {
                // Double cast necessary to avoid `-f permissive` error
                this->m_eeprom.put((uint16_t) (uint32_t) &DEFAULT_DELAY_MILLIS, (uint8_t *) &delayMillis,
                                   sizeof(delayMillis));

                const unsigned int delaySeconds = delayMillis / 1000;
                const unsigned int delayTenths  = (delayMillis % 1000) / 100;
                this->m_printer << FORM_FEED << delaySeconds << '.' << delayTenths << 's';
            } else
                this->blinkLed(WARNING_COLOR);
        }

        void blinkLed (const unsigned int color) const {
            for (unsigned int i = 0; i < 5; ++i) {
                this->m_statusLed.send(color);
                waitcnt(CNT * 100 * MILLISECOND);
                this->m_statusLed.send(WS2812::BLACK);
                waitcnt(CNT * 100 * MILLISECOND);
            }
            this->m_statusLed.send(INACTIVE_COLOR);
        }

    protected:
        const Eeprom  m_eeprom;
        const Pin     m_relayOutput;
        const Pin     m_relayInput;
        const Pin     m_cancelInput;
        const Pin     m_increment;
        const Pin     m_decrement;
        const WS2812  m_statusLed;
        UARTTX        m_uart;
        const Printer m_printer;
};

int main () {
    const RelayActivator relayActivator;
    relayActivator.run();
    return 0;
}
