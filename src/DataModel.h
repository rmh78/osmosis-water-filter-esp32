class DataModel
{
    private:
        const int GLASS_EMPTY_MIN_WEIGHT_GRAM = -20;
        const int GLASS_EMPTY_MAX_WEIGHT_GRAM = 500;
        const int GLASS_FULL_WEIGHT_GRAM = 1200;
        const long FLUSH_INTERVAL_MILLIS = 2 * 60 * 60 * 1000;

    public:
        int weight = 0;
        int ppm = 0;
        long timer = 0;
        String event = "water stop";
        long lastFlushTime = 0;
        bool isGlassEmpty();
        bool isGlassFull();
        bool isGlassGone();
        bool isFlushTime();
};

bool DataModel::isGlassEmpty()
{
    return weight >= GLASS_EMPTY_MIN_WEIGHT_GRAM && weight <= GLASS_EMPTY_MAX_WEIGHT_GRAM;
}

bool DataModel::isGlassFull()
{
    return weight >= GLASS_FULL_WEIGHT_GRAM;
}

bool DataModel::isGlassGone()
{
    return weight < GLASS_EMPTY_MIN_WEIGHT_GRAM;
}

bool DataModel::isFlushTime()
{
    return lastFlushTime + FLUSH_INTERVAL_MILLIS < millis();
}