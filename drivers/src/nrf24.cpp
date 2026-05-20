#include "nrf24.hpp"
#include <cstdint>

namespace driver {

driver::NRF24::NRF24(hal::SPI &spi, hal::GPIO &csn, hal::GPIO &ce)
    : spi_(spi), csn_(csn), ce_(ce) {
  csn_.set();
  ce.reset();
}

uint8_t driver::NRF24::get_status() {
  chip_select();
  uint8_t status = spi_.transfer_data(driver::nrf24_cmd_reg::NOP);
  chip_deselect();
  return status;
}

uint8_t driver::NRF24::read_reg(uint8_t reg_addr) {
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::R_REGISTER | reg_addr);
  uint8_t val = spi_.transfer_data(driver::nrf24_cmd_reg::NOP);
  chip_deselect();

  return val;
}

void driver::NRF24::write_reg(uint8_t reg_addr, uint8_t data) {
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::W_REGISTER | reg_addr);
  spi_.transfer_data(data);
  chip_deselect();
}

void driver::NRF24::set_rf_channel(uint8_t channel) {
  write_reg(driver::nrf24_cmd_reg::RF_CH, channel);
}

void driver::NRF24::chip_select() { csn_.reset(); }

void driver::NRF24::chip_deselect() { csn_.set(); }
} // namespace driver
