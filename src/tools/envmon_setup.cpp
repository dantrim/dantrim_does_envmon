//std/stl
#include <iostream>
#include <chrono>
#include <thread> // std::this_thread

//labRemote
#include "FT232H.h"
#include "I2CDevCom.h"
#include "I2CFTDICom.h"

#include "SHT85.h"
#include "AD799X.h"
#include "PCA9548ACom.h"

#include "Logger.h"

int main(int argc, char* argv[]) {

    // first we need to setup the FT232H device for arbitrating our I2C communication
    std::shared_ptr<FT232H> ft232;
    try {
        ft232 = std::make_shared<FT232H>(MPSSEChip::Protocol::I2C,
                                            MPSSEChip::Speed::FOUR_HUNDRED_KHZ,
                                            MPSSEChip::Endianness::MSBFirst);
        logger(logINFO) << "Initialized FT232H device: " << ft232->to_string();
    } catch(std::exception& e) {
        logger(logERROR) << "Failed to initialize FT232H device: " << e.what();
        return 1;
    }

    //
    // The FT232H device on the EnvMon board is directly connected to:
    //   - Two TCA9548 I2C mux devices
    //   - One AD7998 12-bit ADC
    // On the EnvMon board, the two TCA9548 devices are used to communicate
    // to the I2C devices connected to the QWIIC/Stemma connectors, and so
    // when we initialize these devices we will do so with the I2C communication
    // mediated through the corresponding TCA9548.
    //
    // The AD7998 device can be communicated with directly and so does not need
    // to  have it's communication wrapped in the TCA9548.
    //

    //
    // First let's setup the AD7998 device.
    // The default I2C address of the AD7998 ADC is 0x21.
    // On the EnvMon board, the voltage reference for the AD7998 device
    // is 2.5 volts.
    //
    std::shared_ptr<I2CFTDICom> adc_com(new I2CFTDICom(ft232, 0x21));
    std::unique_ptr<AD799X> adc(new AD799X(2.5, AD799X::Model::AD7998, adc_com));

    //
    // Now let's assume that we have a few SHT85 devices connected to the QWIIC/Stemma
    // connectors on the EnvMon board. Let's assume that we have two SHT85 devices:
    // sht85_0 and sht85_1.
    //
    // The SHT85 has the default I2C address of 0x44.
    //
    uint8_t sht85_device_address = 0x44;

    //
    // Let's assume that the device sht85_0 is connected to the TCA9548 with ID = 000
    // on the EnvMon board and that the device sht85_1 is connected to the TCA9548
    // device with ID = 001 on the EnvMon board.
    //

    // create the device for TCA9548 with ID = 000, that we communicate with via the
    // FT232H device already created in the lines above
    uint8_t address_mux0 = 0x0; // i.e. 000
    uint8_t sht85_0_mux_channel = 0; // assume that sht85_0 is connected to QWIIC/Stemma at ch0
    std::shared_ptr<I2CFTDICom> com_mux0(new I2CFTDICom(ft232, address_mux0));
    std::unique_ptr<SHT85> sht85_0(new SHT85(std::make_shared<PCA9548ACom>(sht85_device_address, sht85_0_mux_channel, com_mux0)));

    uint8_t address_mux1 = 0x1; // TCA9648 with ID = 001
    uint8_t sht85_1_mux_channel = 5; // assume that sht85_1 is connected to QWIIC/Stemma at ch5
    std::shared_ptr<I2CFTDICom> com_mux1(new I2CFTDICom(ft232, address_mux1));
    std::unique_ptr<SHT85> sht85_1(new SHT85(std::make_shared<PCA9548ACom>(sht85_device_address, sht85_1_mux_channel, com_mux1)));

    //
    // Now we have communication established between all the devices on our EnvMon board:
    //    - The AD7998 ADC (part number U1 on the EnvMon board silk screen)
    //    - The SHT85 sensor on MUX ID = 000 on mux channel 0
    //    - The SHT85 sensor on MUX ID = 001 on mux channel 5
    // Let's take some dummy measurements.
    //

    while(true) {
        logger(logINFO) << "-----------------------------------------------";

        // Take measurements on a random channel on the 8-channel AD7998 ADC
        uint8_t adc_measurement_channel = 4;
        uint16_t adc_counts = adc->readCount(adc_measurement_channel); // raw 12-bit counts
        double adc_val = adc->read(adc_measurement_channel); // value converted to Volts

        // Take measurements from the two SHT85 sensors

        // sht85_0
        sht85_0->read();
        float temp_0 = sht85_0->temperature();
        float humidity_0 = sht85_0->humidity();

        // sht85_1
        sht85_1->read();
        float temp_1 = sht85_1->temperature();
        float humidity_1 = sht85_1->humidity();

        // print out the measurements
        logger(logINFO) << " ADC[channel " << adc_measurement_channel << "]: " << adc_counts << " (" << adc_val << " Volts)";
        logger(logINFO) << " sht85_0  : Temp = " << temp_0 << " deg-C, humidity = " << humidity_0;
        logger(logINFO) << " sht85_1  : Temp = " << temp_1 << " deg-C, humidity = " << humidity_1;

        // delay to slow the loop down
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }

    return 0;
}
