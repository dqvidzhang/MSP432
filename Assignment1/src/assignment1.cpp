#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <cstdint>

struct gpio_port
{
    volatile uint16_t IN;
    volatile uint16_t OUT;
    volatile uint16_t DIR;
    volatile uint16_t REN;
    volatile uint16_t DS; // drive strength
    volatile uint16_t SEL0;
    volatile uint16_t SEL1;
};

gpio_port *gpioREG =
    reinterpret_cast<gpio_port *>(0x40004C00u);

constexpr uint16_t ledMASK = 1u << 0;
constexpr uint16_t buttonMASK = 1u << 1;

struct timer_A_registers
{
    volatile uint16_t CTL;     // 0x00
    volatile uint16_t CCTL[5]; // 0x02–0x0A
    uint16_t RESERVED0[2];     // 0x0C–0x0E
    volatile uint16_t R;       // 0x10
    volatile uint16_t CCR[5];  // 0x12–0x1A
    uint16_t RESERVED1[9];     // 0x1C–0x2C
    volatile uint16_t IV;      // 0x2E
};

timer_A_registers *timerA0 =
    reinterpret_cast<timer_A_registers *>(0x40000000u);

// setup our TIMER

constexpr uint32_t defclkHZ = 32768;
constexpr uint32_t amountofBlinks = 2; // how many hz?
constexpr uint32_t amountofToggle = amountofBlinks * 2;

constexpr uint32_t ToggleIntervalTicks = defclkHZ / amountofToggle;

int main()
{
    MAP_WDT_A_holdTimer();

    gpioREG->SEL0 &= ~ledMASK;
    gpioREG->SEL1 &= ~ledMASK;
    gpioREG->DIR |= ledMASK;
    gpioREG->OUT &= ~ledMASK;

    gpioREG->SEL0 &= ~buttonMASK;
    gpioREG->SEL1 &= ~buttonMASK;
    gpioREG->REN |= buttonMASK;
    gpioREG->DIR &= ~buttonMASK;
    gpioREG->OUT |= buttonMASK; // needed because if not will be floating

    timerA0->CCR[0] = static_cast<uint16_t>(ToggleIntervalTicks - 1); // puts the IntervalTicks we want to the compare reg
    timerA0->CCTL[0] = 0;

    timerA0->CTL =
        (1u << 8) | // ACLK
        (1u << 4) | // Up mode, divide by 1
        (1u << 2);  // Clear timer count

    while (1)
    {
        // Poll S1: LOW means pressed.
        if ((gpioREG->IN & buttonMASK) == 0)
        {
            // Toggle when the timer reaches CCR0.
            if ((timerA0->CCTL[0] & (1u << 0)) != 0)
            {
                // Clear compare flag.
                timerA0->CCTL[0] &= ~(1u << 0);

                // Toggle LED.
                gpioREG->OUT ^= ledMASK;
            }
        }
        else
        {
            // Released: LED OFF.
            gpioREG->OUT &= ~ledMASK;

            // Clear any pending compare flag.
            timerA0->CCTL[0] &= ~(1u << 0);
        }
    }
}