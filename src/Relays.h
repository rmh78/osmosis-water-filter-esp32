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
    PINS[0] = relay1Pin; // M1
    PINS[1] = relay2Pin; // M2
    PINS[2] = relay3Pin; // M3
    PINS[3] = relay4Pin; // M4
}

void Relays::init()
{
    for (int i = 0; i < 4; i++) {
        pinMode(PINS[i], OUTPUT);
    }
    this->off();
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
    set(LOW, HIGH, LOW, HIGH);
}

void Relays::filterWater()
{
    set(LOW, HIGH, HIGH, LOW);
}

void Relays::off() 
{
    set(LOW, LOW, LOW, LOW); 
}