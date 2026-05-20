#pragma once
#include "../../hal_lib/include/gpio.hpp"
#include "../../hal_lib/include/spi.hpp"
#include <cstdint>

namespace driver {

class NRF24 {
public:
  NRF24(hal::SPI &spi, hal::GPIO &csn, hal::GPIO &ce);

  uint8_t get_status();
  uint8_t read_reg(uint8_t reg_addr);
  void write_reg(uint8_t reg_addr, uint8_t val);

  void set_rf_channel(uint8_t channel);
  void set_tx_addr(const uint8_t *addr, uint8_t len);
  void set_rx_addr(uint8_t pipe, const uint8_t *addr, uint8_t len);

  void transmit_data(const uint8_t *data, uint8_t len);
  bool data_ready();
  uint8_t receive_data(uint8_t *data, uint8_t len);

private:
  hal::SPI &spi_;
  hal::GPIO &csn_;
  hal::GPIO &ce_;

  void chip_select();
  void chip_deselect();
};

namespace nrf24_cmd_reg {

constexpr uint8_t R_REGISTER = 0x00;   // read cmd and status registers
constexpr uint8_t W_REGISTER = 0x20;   // write to cmd and status registers
constexpr uint8_t R_TX_PAYLOAD = 0x61; // read rx-payload
constexpr uint8_t W_TX_PAYLOAD = 0xA0; // write tx payload
constexpr uint8_t FLUSH_TX = 0xE1;     // flush tx fifo, used in tx mode
constexpr uint8_t FLUSH_RX = 0xE2;     // flush rx fifo, used in rx mode
constexpr uint8_t REUSE_TX_PL = 0xE3;  // reuse last transmitted payload
constexpr uint8_t R_RX_PL_WID =
    0x60; // read rx payload width for the top r_rx_payload in rx fifo
constexpr uint8_t W_ACK_PAYLOAD =
    0xA8; // write payload to be trannsmitted together with ack packet
constexpr uint8_t W_TX_PAYLOAD_NO_ACK =
    0xB0; // disables autoack o this specific packet
constexpr uint8_t NOP = 0xFF;

constexpr uint8_t CONFIG = 0x00;    // config register
constexpr uint8_t EN_AA = 0x01;     // enable auto ack function disable
constexpr uint8_t EN_RXADDR = 0x02; // enabled rx addresses
constexpr uint8_t SETUP_AW =
    0x03; // setup of address widths (common for all data pipes)
constexpr uint8_t SETUP_RETR = 0x04; // setup of automatic retransmission
constexpr uint8_t RF_CH = 0x05;      // rf channel
constexpr uint8_t RF_SETUP = 0x06;   // rf setup register
constexpr uint8_t STATUS =
    0x07; // status register (in paralell to spi command word applied on mosi
          // pin, the status register is shifted serially out on the miso pin)
constexpr uint8_t OBSERVE_TX = 0x08; // transmit observe register
constexpr uint8_t RPD = 0x09;        // recieved power deteictor
constexpr uint8_t RX_ADDR_P0 = 0x0A; // recieve address date pipe 0

}; // namespace nrf24_cmd_reg
}; // namespace driver
