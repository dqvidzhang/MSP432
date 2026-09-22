#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <cstdint>

struct gpio_port
{
    volatile uint16_t IN;
    volatile uint16_t OUT;
    volatile uint16_t DIR;
    volatile uint16_t REN;
    volatile uint16_t DS;
    volatile uint16_t SEL0;
    volatile uint16_t SEL1;
};

struct timer_a_registers
{
    volatile uint16_t CTL;     // 0x00            //control
    volatile uint16_t CCTL[5]; // 0x02–0x0A       //capture and compare
    uint16_t RESERVED0[2];     // 0x0C–0x0E
    volatile uint16_t R;       // 0x10
    volatile uint16_t CCR[5];  // 0x12–0x1A
    uint16_t RESERVED1[9];     // 0x1C–0x2C
    volatile uint16_t IV;      // 0x2E
};

timer_a_registers *timerA0 =
    reinterpret_cast<timer_a_registers *>(0x40000000u);

gpio_port *portA =
    reinterpret_cast<gpio_port *>(0x40004C00u);

constexpr uint16_t ledMask = 1u << 0;    // P1.0 red LED
constexpr uint16_t buttonMask = 1u << 1; // P1.1 S1

// Assumes ACLK = 32768 Hz
constexpr uint32_t clockHz = 32768;
constexpr uint32_t blinkHz = 2; // we want 2 blinks a second

// Two toggles make one full blink.
constexpr uint32_t toggleHz = blinkHz * 2; // 2hz is 2 ticks per second, so led should blink twice. So on off on off, 4 needed to blink twice

constexpr uint32_t timerCounts =
    clockHz / (toggleHz);

int main()
{
    MAP_WDT_A_holdTimer();

    // Select GPIO mode.
    portA->SEL0 &= ~ledMask;
    portA->SEL1 &= ~ledMask;

    portA->OUT &= ~ledMask;
    portA->DIR |= ledMask;

    // Select GPIO mode.
    portA->SEL0 &= ~buttonMask;
    portA->SEL1 &= ~buttonMask;

    // Clear DIR bit: configure P1.1 as an INPUT.
    portA->DIR &= ~buttonMask;

    // Internal pull-up keeps the released input HIGH.
    // Pressing S1 connects the pin to ground, reading LOW.
    portA->OUT |= buttonMask; // Select pull-up
    portA->REN |= buttonMask; // Enable resistor

    // ---------- Timer setup ----------

    // Stop and clear the timer.
    timerA0->CTL = (1u << 2);

    // CCR0 = 32768 / (1 × 2 × 2) - 1 = 8191.
    // Toggle interval = 250 ms.
    timerA0->CCR[0] =
        static_cast<uint16_t>(timerCounts - 1u);

    // Clear compare flag; leave interrupts disabled.
    timerA0->CCTL[0] = 0;

    // Start timer; keep it running continuously.
    timerA0->CTL =
        (1u << 8) | // ACLK
        (1u << 4) | // Up mode, divide by 1
        (1u << 2);  // Clear timer count

    while (1)
    {
        // Poll S1: LOW means pressed.
        if ((portA->IN & buttonMask) == 0)
        {
            // Toggle when the timer reaches CCR0.
            if (timerA0->CCTL[0] & (1u << 0))
            {
                // Clear compare flag.
                timerA0->CCTL[0] &= ~(1u << 0);

                // Toggle LED.
                portA->OUT ^= ledMask;
            }
        }
        else
        {
            // Released: LED OFF.
            portA->OUT &= ~ledMask;

            // Clear any pending compare flag.
            timerA0->CCTL[0] &= ~(1u << 0);
        }
    }
}