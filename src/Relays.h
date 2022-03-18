class Relays 
{
	private:
        int PINS[4];
        void set(byte relay1, byte relay2, byte relay3, byte relay4);

    public:
        Relays(int relay1Pin, int relay2Pin, int relay3Pin, int relay4Pin);
        void init();
        void flushMembrane();
        void flushStandingWater();
        void filterWater();
        void off();
};

Relays::Relays(int relay1Pin, int relay2Pin, int relay3Pin, int relay4Pin)
{
    PINS[0] = relay1Pin;
    PINS[1] = relay2Pin;
    PINS[2] = relay3Pin;
    PINS[3] = relay4Pin;
}

void Relays::init()
{
    for (int i = 0; i < 4; i++) {
        pinMode(PINS[i], OUTPUT);
    }
}

void Relays::set(byte relay1, byte relay2, byte relay3, byte relay4)
{
    digitalWrite(PINS[0], relay1);
    digitalWrite(PINS[1], relay2);
    digitalWrite(PINS[2], relay3);
    digitalWrite(PINS[3], relay4);
}

void Relays::flushMembrane()
{
    set(HIGH, HIGH, LOW, LOW);
}

void Relays::flushStandingWater()
{
    set(HIGH, LOW, HIGH, LOW);
}

void Relays::filterWater()
{
    set(HIGH, LOW, LOW, HIGH);
}

void Relays::off() 
{
    set(LOW, LOW, LOW, LOW); 
}