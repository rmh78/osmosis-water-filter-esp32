#include "HX711.h"

class Scale 
{
	private:
        // the scale value obtained by the calibrate method
        float SCALE_VALUE = 102.6f;
        int DOUT_PIN;
        int SCK_PIN;

    public:
        Scale(int doutPin, int sckPin);
        float calibrate();
        void init();
        float readWeight();
};

HX711 scale;

Scale::Scale(int doutPin, int sckPin)
{
    DOUT_PIN = doutPin;
    SCK_PIN = sckPin;
}

float Scale::calibrate()
{
    Serial.println("scale calibration started");

    scale.begin(DOUT_PIN, SCK_PIN);
    scale.set_scale();
    scale.tare();

    Serial.println("place 1kg on the scale");
    delay(5000);

    float result = 0.0;
    for (int i = 0; i <= 10; i++)
    {
        float tmp = scale.get_units(10);
        result = result + (tmp / 1000);
    }
    result = result / 10;
    Serial.print("scale: ");
    Serial.println(result);

    Serial.println("scale calibration finished");

    return result;
}

void Scale::init()
{
    Serial.println("Initializing the scale");
    scale.begin(DOUT_PIN, SCK_PIN);

    // set the scale value (obtained by the calibrate method)
    scale.set_scale(SCALE_VALUE);
    // reset the scale to 0
    scale.tare();

    // print a raw reading from the ADC
    Serial.print("read: \t\t");
    Serial.println(scale.read());

    // print the average of 20 readings from the ADC
    Serial.print("read average: \t\t");
    Serial.println(scale.read_average(20));

    // print the average of 5 readings from the ADC minus the tare weight, set with tare()
    Serial.print("get value: \t\t");
    Serial.println(scale.get_value(5));

    // print the average of 5 readings from the ADC minus tare weight, divided
    // by the SCALE parameter set with set_scale
    Serial.print("get units: \t\t");
    Serial.println(scale.get_units(5), 1);
}

float Scale::readWeight()
{
    scale.power_up();
    float result = scale.get_units(2);

    Serial.print("read weight: ");
    Serial.println(result, 1);

    scale.power_down();
    return result;
}