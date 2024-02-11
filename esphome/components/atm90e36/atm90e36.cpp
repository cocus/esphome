#include "atm90e36.h"
#include "atm90e36_reg.h"
#include "esphome/core/log.h"
#include <cinttypes>

namespace esphome {
namespace atm90e36 {

static const char *const TAG = "atm90e36";

void ATM90E36Component::update() {
  if (this->chip_temperature_sensor_ != nullptr) {
    this->chip_temperature_sensor_->publish_state(this->get_chip_temperature_());
  }
  if (this->phase_[0].voltage_sensor_ != nullptr) {
    this->phase_[0].voltage_sensor_->publish_state(this->get_line_voltage_a_());
  }
  if (this->phase_[1].voltage_sensor_ != nullptr) {
    this->phase_[1].voltage_sensor_->publish_state(this->get_line_voltage_b_());
  }
  if (this->phase_[2].voltage_sensor_ != nullptr) {
    this->phase_[2].voltage_sensor_->publish_state(this->get_line_voltage_c_());
  }
  if (this->phase_[0].current_sensor_ != nullptr) {
    this->phase_[0].current_sensor_->publish_state(this->get_line_current_a_());
  }
  if (this->phase_[1].current_sensor_ != nullptr) {
    this->phase_[1].current_sensor_->publish_state(this->get_line_current_b_());
  }
  if (this->phase_[2].current_sensor_ != nullptr) {
    this->phase_[2].current_sensor_->publish_state(this->get_line_current_c_());
  }
  if (this->phase_[0].power_sensor_ != nullptr) {
    this->phase_[0].power_sensor_->publish_state(this->get_active_power_a_());
  }
  if (this->phase_[1].power_sensor_ != nullptr) {
    this->phase_[1].power_sensor_->publish_state(this->get_active_power_b_());
  }
  if (this->phase_[2].power_sensor_ != nullptr) {
    this->phase_[2].power_sensor_->publish_state(this->get_active_power_c_());
  }
  if (this->phase_[0].reactive_power_sensor_ != nullptr) {
    this->phase_[0].reactive_power_sensor_->publish_state(this->get_reactive_power_a_());
  }
  if (this->phase_[1].reactive_power_sensor_ != nullptr) {
    this->phase_[1].reactive_power_sensor_->publish_state(this->get_reactive_power_b_());
  }
  if (this->phase_[2].reactive_power_sensor_ != nullptr) {
    this->phase_[2].reactive_power_sensor_->publish_state(this->get_reactive_power_c_());
  }
  if (this->phase_[0].power_factor_sensor_ != nullptr) {
    this->phase_[0].power_factor_sensor_->publish_state(this->get_power_factor_a_());
  }
  if (this->phase_[1].power_factor_sensor_ != nullptr) {
    this->phase_[1].power_factor_sensor_->publish_state(this->get_power_factor_b_());
  }
  if (this->phase_[2].power_factor_sensor_ != nullptr) {
    this->phase_[2].power_factor_sensor_->publish_state(this->get_power_factor_c_());
  }
  if (this->phase_[0].forward_active_energy_sensor_ != nullptr) {
    this->phase_[0].forward_active_energy_sensor_->publish_state(this->get_forward_active_energy_a_());
  }
  if (this->phase_[1].forward_active_energy_sensor_ != nullptr) {
    this->phase_[1].forward_active_energy_sensor_->publish_state(this->get_forward_active_energy_b_());
  }
  if (this->phase_[2].forward_active_energy_sensor_ != nullptr) {
    this->phase_[2].forward_active_energy_sensor_->publish_state(this->get_forward_active_energy_c_());
  }
  if (this->phase_[0].reverse_active_energy_sensor_ != nullptr) {
    this->phase_[0].reverse_active_energy_sensor_->publish_state(this->get_reverse_active_energy_a_());
  }
  if (this->phase_[1].reverse_active_energy_sensor_ != nullptr) {
    this->phase_[1].reverse_active_energy_sensor_->publish_state(this->get_reverse_active_energy_b_());
  }
  if (this->phase_[2].reverse_active_energy_sensor_ != nullptr) {
    this->phase_[2].reverse_active_energy_sensor_->publish_state(this->get_reverse_active_energy_c_());
  }
  if (this->freq_sensor_ != nullptr) {
    this->freq_sensor_->publish_state(this->get_frequency_());
  }

  ESP_LOGD(TAG, "EnStatus1 = %04x, voltA: %d currA: %d", this->read16_(ATM90E36_REGISTER_ENSTATUS1), this->read16_(ATM90E36_REGISTER_URMSA), this->read16_(ATM90E36_REGISTER_IRMSA) );

  this->status_clear_warning();
}

void ATM90E36Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ATM90E36 Component...");
  this->spi_setup();

  uint16_t mmode0 = 0x87;  // 3P4W 50Hz
  if (line_freq_ == 60) {
    mmode0 |= 1 << 12;  // sets 12th bit to 1, 60Hz
  }

  if (current_phases_ == 2) {
    mmode0 |= 1 << 8;  // sets 8th bit to 1, 3P3W
    //mmode0 |= 0 << 1;  // sets 1st bit to 0, phase b is not counted into the all-phase sum energy/power (P/Q/S)
  }

  this->write16_(ATM90E36_REGISTER_SOFTRESET, 0x789A);    // Perform soft reset

  this->write16_(ATM90E36_REGISTER_CONFIGSTART, 0x5678);  // Metering calibration startup

  /*this->write16_(ATM90E36_REGISTER_METEREN, 0x0001);      // Enable Metering
  if (this->read16_(ATM90E36_REGISTER_LASTSPIDATA) != 0x0001) {
    ESP_LOGW(TAG, "Could not initialize ATM90E36 IC, check SPI settings");
    this->mark_failed();
    return;
  }*/

  uint16_t mmode1 = 0;
  mmode1 |= this->phase_[0].current_pga_gain_ << 0; // I1
  mmode1 |= this->phase_[1].current_pga_gain_ << 2; // I2
  mmode1 |= this->phase_[2].current_pga_gain_ << 4; // I3
  mmode1 |= this->phase_[3].current_pga_gain_ << 6; // I4

  mmode1 |= this->phase_[0].volt_pga_gain_ << 8; // V1
  mmode1 |= this->phase_[1].volt_pga_gain_ << 10; // V2
  mmode1 |= this->phase_[2].volt_pga_gain_ << 12; // V3

  this->write16_(ATM90E36_REGISTER_PLCONSTH, 0x0861);   // PL Constant MSB (default) = 140625000
  this->write16_(ATM90E36_REGISTER_PLCONSTL, 0xC468);   // PL Constant LSB (default)
  this->write16_(ATM90E36_REGISTER_ZXCONFIG, 0xD654);   // ZX2, ZX1, ZX0 pin config
  this->write16_(ATM90E36_REGISTER_MMODE0, mmode0);     // Mode Config (frequency set in main program)
  this->write16_(ATM90E36_REGISTER_MMODE1, mmode1);     // PGA Gain Configuration for Current Channels
  this->write16_(ATM90E36_REGISTER_PSTARTTH, 0x1D4C);   // All Active Startup Power Threshold - 0.02A/0.00032 = 7500
  this->write16_(ATM90E36_REGISTER_QSTARTTH, 0x1D4C);   // All Reactive Startup Power Threshold - 50%
  this->write16_(ATM90E36_REGISTER_PPHASETH, 0x02EE);   // Each Phase Active Phase Threshold - 0.002A/0.00032 = 750
  this->write16_(ATM90E36_REGISTER_QPHASETH, 0x02EE);   // Each phase Reactive Phase Threshold - 10%
  this->write16_(ATM90E36_REGISTER_CSZERO, 0x4741);      // Checksum 0


  this->write16_(ATM90E36_REGISTER_ADJSTART, 0x5678); // Measurement calibration
  this->write16_(ATM90E36_REGISTER_UGAINA, this->phase_[0].volt_gain_);  // A Voltage rms gain
  this->write16_(ATM90E36_REGISTER_IGAINA, this->phase_[0].ct_gain_);    // A line current gain
  this->write16_(ATM90E36_REGISTER_UGAINB, this->phase_[1].volt_gain_);  // B Voltage rms gain
  this->write16_(ATM90E36_REGISTER_IGAINB, this->phase_[1].ct_gain_);    // B line current gain
  this->write16_(ATM90E36_REGISTER_UGAINC, this->phase_[2].volt_gain_);  // C Voltage rms gain
  this->write16_(ATM90E36_REGISTER_IGAINC, this->phase_[2].ct_gain_);    // C line current gain

  this->write16_(ATM90E36_REGISTER_IOFFSETA, this->phase_[0].offset_current_);    // A line current gain
  this->write16_(ATM90E36_REGISTER_UOFFSETA, this->phase_[0].offset_voltage_);    // A line current gain
  this->write16_(ATM90E36_REGISTER_IOFFSETB, this->phase_[1].offset_current_);    // B line current gain
  this->write16_(ATM90E36_REGISTER_UOFFSETB, this->phase_[1].offset_voltage_);    // B line current gain
  this->write16_(ATM90E36_REGISTER_IOFFSETC, this->phase_[2].offset_current_);    // C line current gain
  this->write16_(ATM90E36_REGISTER_UOFFSETC, this->phase_[2].offset_voltage_);    // C line current gain

  //CommEnergyIC(WRITE, IgainN, 0xFD7F); // D line current gain
  this->write16_(ATM90E36_REGISTER_CSTHREE, 0x02F6); // Checksum 3
}

void ATM90E36Component::dump_config() {
  ESP_LOGCONFIG("", "ATM90E36:");
  LOG_PIN("  CS Pin: ", this->cs_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with ATM90E36 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Voltage A", this->phase_[0].voltage_sensor_);
  LOG_SENSOR("  ", "Current A", this->phase_[0].current_sensor_);
  LOG_SENSOR("  ", "Power A", this->phase_[0].power_sensor_);
  LOG_SENSOR("  ", "Reactive Power A", this->phase_[0].reactive_power_sensor_);
  LOG_SENSOR("  ", "PF A", this->phase_[0].power_factor_sensor_);
  LOG_SENSOR("  ", "Active Forward Energy A", this->phase_[0].forward_active_energy_sensor_);
  LOG_SENSOR("  ", "Active Reverse Energy A", this->phase_[0].reverse_active_energy_sensor_);
  LOG_SENSOR("  ", "Voltage B", this->phase_[1].voltage_sensor_);
  LOG_SENSOR("  ", "Current B", this->phase_[1].current_sensor_);
  LOG_SENSOR("  ", "Power B", this->phase_[1].power_sensor_);
  LOG_SENSOR("  ", "Reactive Power B", this->phase_[1].reactive_power_sensor_);
  LOG_SENSOR("  ", "PF B", this->phase_[1].power_factor_sensor_);
  LOG_SENSOR("  ", "Active Forward Energy B", this->phase_[1].forward_active_energy_sensor_);
  LOG_SENSOR("  ", "Active Reverse Energy B", this->phase_[1].reverse_active_energy_sensor_);
  LOG_SENSOR("  ", "Voltage C", this->phase_[2].voltage_sensor_);
  LOG_SENSOR("  ", "Current C", this->phase_[2].current_sensor_);
  LOG_SENSOR("  ", "Power C", this->phase_[2].power_sensor_);
  LOG_SENSOR("  ", "Reactive Power C", this->phase_[2].reactive_power_sensor_);
  LOG_SENSOR("  ", "PF C", this->phase_[2].power_factor_sensor_);
  LOG_SENSOR("  ", "Active Forward Energy C", this->phase_[2].forward_active_energy_sensor_);
  LOG_SENSOR("  ", "Active Reverse Energy C", this->phase_[2].reverse_active_energy_sensor_);
  LOG_SENSOR("  ", "Frequency", this->freq_sensor_);
  LOG_SENSOR("  ", "Chip Temp", this->chip_temperature_sensor_);
}
float ATM90E36Component::get_setup_priority() const { return setup_priority::DATA; }

uint16_t ATM90E36Component::read16_(uint16_t a_register) {
  uint8_t addrh = (1 << 7) | ((a_register >> 8) & 0x03);
  uint8_t addrl = (a_register & 0xFF);
  uint8_t data[2];
  uint16_t output;

  this->enable();
  delayMicroseconds(10);
  this->write_byte(addrh);
  this->write_byte(addrl);
  delayMicroseconds(4);
  this->read_array(data, 2);
  this->disable();

  output = (uint16_t(data[0] & 0xFF) << 8) | (data[1] & 0xFF);
  ESP_LOGVV(TAG, "read16_ 0x%04" PRIX16 " output 0x%04" PRIX16, a_register, output);
  return output;
}

int ATM90E36Component::read32_(uint16_t addr_h, uint16_t addr_l) {
  uint16_t val_h = this->read16_(addr_h);
  uint16_t val_l = this->read16_(addr_l);
  int32_t val = (val_h << 16) | val_l;

  ESP_LOGVV(TAG,
            "read32_ addr_h 0x%04" PRIX16 " val_h 0x%04" PRIX16 " addr_l 0x%04" PRIX16 " val_l 0x%04" PRIX16
            " = %" PRId32,
            addr_h, val_h, addr_l, val_l, val);

  return val;
}

float ATM90E36Component::read32_float_(uint16_t addr_h, uint16_t addr_l, float k) {
  int16_t val_h = this->read16_(addr_h);
  int16_t val_l = this->read16_(addr_l);
  float result = val_l;
  result /= k;
  result += val_h;

  ESP_LOGVV(TAG,
            "read32_float_ addr_h 0x%04" PRIX16 " val_h 0x%04" PRIX16 " addr_l 0x%04" PRIX16 " val_l 0x%04" PRIX16
            " = %f",
            addr_h, val_h, addr_l, val_l, result);

  return result;
}

void ATM90E36Component::write16_(uint16_t a_register, uint16_t val) {
  uint8_t addrh = (a_register >> 8) & 0x03;
  uint8_t addrl = (a_register & 0xFF);

  ESP_LOGVV(TAG, "write16_ 0x%04" PRIX16 " val 0x%04" PRIX16, a_register, val);
  this->enable();
  delayMicroseconds(10);
  this->write_byte(addrh);
  this->write_byte(addrl);
  delayMicroseconds(4);
  this->write_byte((val >> 8) & 0xff);
  this->write_byte(val & 0xFF);
  this->disable();
}

float ATM90E36Component::get_line_voltage_a_() {
  float voltage = this->read32_float_(ATM90E36_REGISTER_URMSA, ATM90E36_REGISTER_URMSALSB);
  return (float) voltage / 100;
}
float ATM90E36Component::get_line_voltage_b_() {
  float voltage = this->read32_float_(ATM90E36_REGISTER_URMSB, ATM90E36_REGISTER_URMSBLSB);
  return (float) voltage / 100;
}
float ATM90E36Component::get_line_voltage_c_() {
  float voltage = this->read32_float_(ATM90E36_REGISTER_URMSC, ATM90E36_REGISTER_URMSCLSB);
  return (float) voltage / 100;
}
float ATM90E36Component::get_line_current_a_() {
  float current = this->read32_float_(ATM90E36_REGISTER_IRMSA, ATM90E36_REGISTER_IRMSALSB);
  return (float) current / 1000;
}
float ATM90E36Component::get_line_current_b_() {
  float current = this->read32_float_(ATM90E36_REGISTER_IRMSB, ATM90E36_REGISTER_IRMSBLSB);
  return (float) current / 1000;
}
float ATM90E36Component::get_line_current_c_() {
  float current = this->read32_float_(ATM90E36_REGISTER_IRMSC, ATM90E36_REGISTER_IRMSCLSB);
  return (float) current / 1000;
}
float ATM90E36Component::get_active_power_a_() {
  float val = this->read32_float_(ATM90E36_REGISTER_PMEANA, ATM90E36_REGISTER_PMEANALSB);
  return val;
}
float ATM90E36Component::get_active_power_b_() {
  float val = this->read32_float_(ATM90E36_REGISTER_PMEANB, ATM90E36_REGISTER_PMEANBLSB);
  return val;
}
float ATM90E36Component::get_active_power_c_() {
  float val = this->read32_float_(ATM90E36_REGISTER_PMEANC, ATM90E36_REGISTER_PMEANCLSB);
  return val;
}
float ATM90E36Component::get_reactive_power_a_() {
  float val = this->read32_float_(ATM90E36_REGISTER_QMEANA, ATM90E36_REGISTER_QMEANALSB);
  return val;
}
float ATM90E36Component::get_reactive_power_b_() {
  float val = this->read32_float_(ATM90E36_REGISTER_QMEANB, ATM90E36_REGISTER_QMEANBLSB);
  return val;
}
float ATM90E36Component::get_reactive_power_c_() {
  float val = this->read32_float_(ATM90E36_REGISTER_QMEANC, ATM90E36_REGISTER_QMEANCLSB);
  return val;
}
float ATM90E36Component::get_power_factor_a_() {
  int16_t pf = this->read16_(ATM90E36_REGISTER_PFMEANA);
  return (float) pf / 1000;
}
float ATM90E36Component::get_power_factor_b_() {
  int16_t pf = this->read16_(ATM90E36_REGISTER_PFMEANB);
  return (float) pf / 1000;
}
float ATM90E36Component::get_power_factor_c_() {
  int16_t pf = this->read16_(ATM90E36_REGISTER_PFMEANC);
  return (float) pf / 1000;
}
float ATM90E36Component::get_forward_active_energy_a_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_APENERGYA);
  if ((UINT32_MAX - this->phase_[0].cumulative_forward_active_energy_) > val) {
    this->phase_[0].cumulative_forward_active_energy_ += val;
  } else {
    this->phase_[0].cumulative_forward_active_energy_ = val;
  }
  return ((float) this->phase_[0].cumulative_forward_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_forward_active_energy_b_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_APENERGYB);
  if ((UINT32_MAX - this->phase_[1].cumulative_forward_active_energy_) > val) {
    this->phase_[1].cumulative_forward_active_energy_ += val;
  } else {
    this->phase_[1].cumulative_forward_active_energy_ = val;
  }
  return ((float) this->phase_[1].cumulative_forward_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_forward_active_energy_c_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_APENERGYC);
  if ((UINT32_MAX - this->phase_[2].cumulative_forward_active_energy_) > val) {
    this->phase_[2].cumulative_forward_active_energy_ += val;
  } else {
    this->phase_[2].cumulative_forward_active_energy_ = val;
  }
  return ((float) this->phase_[2].cumulative_forward_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_reverse_active_energy_a_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_ANENERGYA);
  if (UINT32_MAX - this->phase_[0].cumulative_reverse_active_energy_ > val) {
    this->phase_[0].cumulative_reverse_active_energy_ += val;
  } else {
    this->phase_[0].cumulative_reverse_active_energy_ = val;
  }
  return ((float) this->phase_[0].cumulative_reverse_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_reverse_active_energy_b_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_ANENERGYB);
  if (UINT32_MAX - this->phase_[1].cumulative_reverse_active_energy_ > val) {
    this->phase_[1].cumulative_reverse_active_energy_ += val;
  } else {
    this->phase_[1].cumulative_reverse_active_energy_ = val;
  }
  return ((float) this->phase_[1].cumulative_reverse_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_reverse_active_energy_c_() {
  uint16_t val = this->read16_(ATM90E36_REGISTER_ANENERGYC);
  if (UINT32_MAX - this->phase_[2].cumulative_reverse_active_energy_ > val) {
    this->phase_[2].cumulative_reverse_active_energy_ += val;
  } else {
    this->phase_[2].cumulative_reverse_active_energy_ = val;
  }
  return ((float) this->phase_[2].cumulative_reverse_active_energy_ * 10 / 3200);
}
float ATM90E36Component::get_frequency_() {
  uint16_t freq = this->read16_(ATM90E36_REGISTER_FREQ);
  return (float) freq / 100;
}
float ATM90E36Component::get_chip_temperature_() {
  uint16_t ctemp = this->read16_(ATM90E36_REGISTER_TEMP);
  return (float) ctemp;
}
}  // namespace atm90e36
}  // namespace esphome
