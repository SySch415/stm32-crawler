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

void driver::NRF24::read_reg(uint8_t reg_addr, uint8_t *data, uint8_t len) {
  chip_select();

  spi_.transfer_data(driver::nrf24_cmd_reg::R_REGISTER | reg_addr);
  for (uint8_t i{0}; i < len; i++) {
    data[i] = spi_.transfer_data(driver::nrf24_cmd_reg::NOP);
  }
  chip_deselect();
}

void driver::NRF24::write_reg(uint8_t reg_addr, const uint8_t *data,
                              uint8_t len) {
  chip_select();

  spi_.transfer_data(driver::nrf24_cmd_reg::W_REGISTER | reg_addr);
  for (uint8_t i{0}; i < len; i++) {
    spi_.transfer_data(data[i]);
  }
  chip_deselect();
}

uint8_t driver::NRF24::read_reg(uint8_t reg_addr) {
  uint8_t data;
  read_reg(reg_addr, &data, 1);

  return data;
}

void driver::NRF24::write_reg(uint8_t reg_addr, uint8_t data) {
  write_reg(reg_addr, &data, 1);
}

void driver::NRF24::set_rf_channel(uint8_t channel) {
  write_reg(driver::nrf24_cmd_reg::RF_CH, channel);
}

void driver::NRF24::set_tx_addr(const uint8_t *addr, uint8_t len) {
  write_reg(driver::nrf24_cmd_reg::TX_ADDR, addr, len);
  write_reg(driver::nrf24_cmd_reg::RX_ADDR_P0, addr, len);
}
void driver::NRF24::set_rx_addr(uint8_t pipe, const uint8_t *addr,
                                uint8_t len) {
  write_reg(driver::nrf24_cmd_reg::RX_ADDR_P0 + pipe, addr, len);
}

void driver::NRF24::set_payload_size(uint8_t pipe, uint8_t size) {
  write_reg(driver::nrf24_cmd_reg::RX_PW_P0 + pipe, size);
}

void driver::NRF24::pw_up_tx() { write_reg(driver::nrf24_cmd_reg::CONFIG, 14); }

void driver::NRF24::flush_tx() {
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::FLUSH_TX);
  chip_deselect();
}

void driver::NRF24::pw_up_rx() {
  write_reg(driver::nrf24_cmd_reg::CONFIG, 15);
  ce_.set();
}

void driver::NRF24::flush_rx() {
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::FLUSH_RX);
  chip_deselect();
}

void driver::NRF24::clear_flags() {
  write_reg(driver::nrf24_cmd_reg::STATUS, 0x70);
}

void driver::NRF24::transmit_data(const uint8_t *data, uint8_t len) {
  pw_up_tx();
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::W_TX_PAYLOAD);
  for (uint8_t i{0}; i < len; i++) {
    spi_.transfer_data(data[i]);
  }
  chip_deselect();

  ce_.set();
  for (volatile int i{0}; i < 10000; i++) {
  }
  ce_.reset();

  [[maybe_unused]] uint8_t status{get_status()};
  while (!(status & (1 << 5)) && !(status & (1 << 4))) {
    status = get_status();
  }
  clear_flags();
  flush_tx();
}

void driver::NRF24::receive_data(uint8_t *data, uint8_t len) {
  pw_up_rx();
  chip_select();
  spi_.transfer_data(driver::nrf24_cmd_reg::R_RX_PAYLOAD);
  for (uint8_t i{0}; i < len; i++) {
    data[i] = spi_.transfer_data(driver::nrf24_cmd_reg::NOP);
  }
  chip_deselect();

  clear_flags();
}

bool driver::NRF24::data_ready() {
  uint8_t status{get_status()};
  return status & (1 << 6);
}

void driver::NRF24::chip_select() { csn_.reset(); }

void driver::NRF24::chip_deselect() { csn_.set(); }
} // namespace driver
