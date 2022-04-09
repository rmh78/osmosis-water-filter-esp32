#include "HX711.h"

class Scale 
{
	private:
        // the scale value obtained by the calibrate method
        //float SCALE_VALUE = 102.6f; // small scale
        float SCALE_VALUE = 236.46; // big scale
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
    Serial.println("Init the scale");

    scale.begin(DOUT_PIN, SCK_PIN);
    // set the scale value (obtained by the calibrate method)
    scale.set_scale(SCALE_VALUE);
    // reset the scale to 0
    scale.tare();
}

float Scale::readWeight()
{
    scale.power_up();
    float result = scale.get_units(2);

    //Serial.print("read weight: ");
    //Serial.println(result, 1);

    scale.power_down();
    return result;
}