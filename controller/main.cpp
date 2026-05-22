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

  uint8_t addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  rf.set_rf_channel(76);
  rf.set_tx_addr(addr, sizeof(addr));
  rf.set_payload_size(0, 1);

  while (1) {
    uint8_t msg = 0xAB;

    rf.transmit_data(&msg, 1);
    for (int i{0}; i < 500000; i++) {
    }
  }
}
