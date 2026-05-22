#include "gpio.hpp"
#include "nrf24.hpp"
#include "rcc.hpp"
#include "spi.hpp"
#include <cstdint>
#include <stdint.h>

extern "C" int main(void) {
  hal::RCC rcc;
  rcc.clock_enable(hal::RCC::Periph::GPIOA);
  rcc.clock_enable(hal::RCC::Periph::SPI1);

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

  rf.set_rf_channel(76);
  [[maybe_unused]] volatile uint8_t channel =
      rf.read_reg(driver::nrf24_cmd_reg::RF_CH);

  while (1) {
  }
}
