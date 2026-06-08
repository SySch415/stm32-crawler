#include "adc.hpp"
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
  rcc.clock_enable(hal::RCC::Periph::ADC1);

  hal::GPIO joystick_y(hal::GPIO::Port::A, 0, hal::GPIO::Mode::Analog);
  hal::GPIO joystick_x(hal::GPIO::Port::A, 1, hal::GPIO::Mode::Analog);

  hal::ADC adc;
  adc.set_sample_t(hal::ADC::CH::CH0, hal::ADC::SAMPLE_T::CYCLE_84);
  adc.set_sample_t(hal::ADC::CH::CH1, hal::ADC::SAMPLE_T::CYCLE_84);

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

  uint8_t addr[]{0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  rf.set_rf_channel(76);
  rf.set_tx_addr(addr, sizeof(addr));
  rf.set_payload_size(0, 5);

  uint16_t throttle{0};
  uint16_t steering{0};

  rf.flush_tx();
  rf.clear_flags();

  // DEBUG: disable auto_ack test
  // rf.write_reg(0x01, 0x00);
  // rf.write_reg(0x04, 0x00);

  while (1) {
    [[maybe_unused]] uint8_t tx_status_debug{0};
    uint8_t payload[5] /*{0xAB, 0xCD, 0xDF, 0xCC, 0xDD}*/;
    throttle = adc.read(hal::ADC::CH::CH0);
    steering = adc.read(hal::ADC::CH::CH1);

    payload[0] = throttle & 0xFF;
    payload[1] = throttle >> 8;
    payload[2] = steering & 0xFF;
    payload[3] = steering >> 8;
    payload[4] = 0;

    tx_status_debug =
        rf.transmit_data(/*reinterpret_cast<uint8_t *>(&vr_x_val)*/ payload, 5);
    /*
    for (int i{0}; i < 500000; i++) {
    }
  */
  }
}
