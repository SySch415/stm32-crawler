#include "gpio.hpp"
#include "nrf24.hpp"
#include "rcc.hpp"
#include "spi.hpp"
#include "timer.hpp"
#include <cstdint>
#include <stdint.h>

extern "C" int main(void) {
  hal::RCC rcc;
  rcc.clock_enable(hal::RCC::Periph::GPIOA);
  rcc.clock_enable(hal::RCC::Periph::GPIOB);
  rcc.clock_enable(hal::RCC::Periph::SPI1);
  rcc.clock_enable(hal::RCC::Periph::TIM4);

  hal::TIMX<hal::TimSize::BIT16> pwm_timer(hal::TimType::TIM4);

  hal::GPIO pwm_out(hal::GPIO::Port::B, 6, hal::GPIO::Mode::AF);
  hal::GPIO ain1_out(hal::GPIO::Port::B, 7, hal::GPIO::Mode::Output);
  hal::GPIO ain2_out(hal::GPIO::Port::B, 8, hal::GPIO::Mode::Output);

  pwm_out.set_alt_func(2);

  pwm_timer.set_PSC(0);
  pwm_timer.set_ARR(15999);
  pwm_timer.set_PWM_mode_1(1);
  pwm_timer.set_CCR(1, 8000);
  pwm_timer.enable_CCER(1);
  pwm_timer.enable_Counter();
  pwm_timer.set_CNT(0);

  ain1_out.set();

  hal::GPIO led(hal::GPIO::Port::A, 0, hal::GPIO::Mode::Output);

  hal::GPIO sck(hal::GPIO::Port::A, 5, hal::GPIO::Mode::AF);
  hal::GPIO miso(hal::GPIO::Port::A, 6, hal::GPIO::Mode::AF);
  hal::GPIO mosi(hal::GPIO::Port::A, 7, hal::GPIO::Mode::AF);
  hal::GPIO csn(hal::GPIO::Port::A, 4, hal::GPIO::Mode::Output);
  hal::GPIO ce(hal::GPIO::Port::A, 3, hal::GPIO::Mode::Output);

  sck.set_alt_func(5);
  miso.set_alt_func(5);
  mosi.set_alt_func(5);

  hal::SPI spi(hal::SPI::ID::SPI1, hal::SPI::PSr::D256);
  driver::NRF24 rf(spi, csn, ce);

  [[maybe_unused]] uint8_t status = rf.get_status();

  uint8_t addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

  rf.set_rf_channel(76);
  rf.set_rx_addr(0, addr, sizeof(addr));
  rf.set_payload_size(0, 3);
  rf.write_reg(driver::nrf24_cmd_reg::EN_RXADDR, driver::nrf24_cmd_reg::EN_AA);
  rf.pw_up_rx();

  rf.flush_rx();
  rf.clear_flags();

  // rf.write_reg(0x01, 0x00);

  volatile uint8_t debug_status{0};
  while (1) {
    debug_status = rf.get_status();
    if (rf.data_ready()) {
      uint8_t recieved[3];
      rf.receive_data(/*reinterpret_cast<uint8_t *>(&recieved)*/ recieved, 3);

      if (recieved[0] == 0xAB)
        led.set();
    }
  }
}
