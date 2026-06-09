#include "gpio.hpp"
#include "nrf24.hpp"
#include "rcc.hpp"
#include "spi.hpp"
#include "timer.hpp"
#include <cstdint>
#include <stdint.h>

constexpr uint16_t CENTER{2048};
constexpr uint16_t DEAD_BAND{100};
constexpr uint16_t ADC_MAX{4095};
constexpr uint16_t MOTOR_ARR{15999};

void update_throttle(uint16_t throttle,
                     hal::TIMX<hal::TimSize::BIT16> &pwm_timer,
                     hal::GPIO &ain1_out, hal::GPIO &ain2_out) {

  if (throttle > (CENTER + DEAD_BAND)) {
    ain1_out.set();
    ain2_out.reset();

    uint16_t mag{static_cast<uint16_t>(throttle - (CENTER + DEAD_BAND))};
    uint16_t duty{static_cast<uint16_t>((uint32_t)mag * MOTOR_ARR /
                                        (ADC_MAX - (CENTER - DEAD_BAND)))};
    pwm_timer.set_CCR(1, duty);
  } else if (throttle < (CENTER - DEAD_BAND)) {
    ain1_out.reset();
    ain2_out.set();

    uint16_t mag{static_cast<uint16_t>((CENTER - DEAD_BAND) - throttle)};
    uint16_t duty{static_cast<uint16_t>((uint32_t)mag * MOTOR_ARR /
                                        (CENTER - DEAD_BAND))};
    pwm_timer.set_CCR(1, duty);
  } else {
    ain1_out.set();
    ain2_out.set();
    pwm_timer.set_CCR(1, 0);
  }
}

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
  pwm_timer.set_ARR(MOTOR_ARR);
  pwm_timer.set_PWM_mode_1(1);
  pwm_timer.enable_CCER(1);
  pwm_timer.enable_Counter();
  pwm_timer.set_CNT(0);

  hal::GPIO led_throttle_up(hal::GPIO::Port::A, 0, hal::GPIO::Mode::Output);
  hal::GPIO led_throttle_down(hal::GPIO::Port::A, 1, hal::GPIO::Mode::Output);
  hal::GPIO led_steering_left(hal::GPIO::Port::A, 11, hal::GPIO::Mode::Output);
  hal::GPIO led_steering_right(hal::GPIO::Port::A, 12, hal::GPIO::Mode::Output);

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
  rf.set_payload_size(0, 5);
  rf.write_reg(driver::nrf24_cmd_reg::EN_RXADDR, driver::nrf24_cmd_reg::EN_AA);
  rf.pw_up_rx();

  rf.flush_rx();
  rf.clear_flags();

  // rf.write_reg(0x01, 0x00);

  volatile uint8_t debug_status{0};
  while (1) {
    debug_status = rf.get_status();
    if (rf.data_ready()) {
      uint8_t recieved[5];
      rf.receive_data(/*reinterpret_cast<uint8_t *>(&recieved)*/ recieved, 5);

      uint16_t throttle{
          static_cast<uint16_t>(recieved[0] | (recieved[1] << 8))};
      uint16_t steering{
          static_cast<uint16_t>(recieved[2] | (recieved[3] << 8))};

      if (!(throttle < (CENTER + DEAD_BAND) &&
            throttle > (CENTER - DEAD_BAND)) &&
          throttle > (CENTER + DEAD_BAND)) {
        led_throttle_down.reset();
        led_throttle_up.set();
        update_throttle(throttle, pwm_timer, ain1_out, ain2_out);
      } else if (!(throttle < (CENTER + DEAD_BAND) &&
                   throttle > (CENTER - DEAD_BAND)) &&
                 throttle < (CENTER - DEAD_BAND)) {
        led_throttle_up.reset();
        led_throttle_down.set();
        update_throttle(throttle, pwm_timer, ain1_out, ain2_out);
      } else {
        led_throttle_down.reset();
        led_throttle_up.reset();
        update_throttle(throttle, pwm_timer, ain1_out, ain2_out);
      }

      if (!(steering < 2248 && steering > 1948) && steering > 2248) {
        led_steering_left.reset();
        led_steering_right.set();
      } else if (!(steering < 2248 && steering > 1948) && steering < 1948) {
        led_steering_right.reset();
        led_steering_left.set();
      } else {
        led_steering_left.reset();
        led_steering_right.reset();
      }
    }
  }
}
