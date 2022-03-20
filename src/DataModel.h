enum class Event { Unknown = 0, GlassGone = 1, GlassEmpty = 2, GlassFull = 3, FlushPhase1 = 4, FlushPhase2 = 5, FlushPhase3 = 6, FilterOn = 7, FilterOff = 8 };
static const String EventNames[] = { "UNKNOWN", "GLASS GONE", "GLASS EMPTY", "GLASS FULL", "FLUSH PHASE 1", "FLUSH PHASE 2", "FLUSH PHASE 3", "FILTER ON", "FILTER OFF" };

class DataModel
{
    private:
        const int GLASS_EMPTY_MIN_WEIGHT_GRAM = -20;
        const int GLASS_EMPTY_MAX_WEIGHT_GRAM = 500;
        const int GLASS_FULL_WEIGHT_GRAM = 1200;
        const long FLUSH_INTERVAL_MILLIS = 2 * 60 * 60 * 1000;
        Event event = Event::Unknown;

    public:
        int weight = 0;
        int ppm = 0;
        long timer = 0;
        long lastFlushTime = 0;
        void setEvent(Event e);
        Event getEvent();
        String getEventText();
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
    bool result = lastFlushTime == 0 || (lastFlushTime + FLUSH_INTERVAL_MILLIS < millis());
    String flushText = result ? "flush needed" : "flush NOT needed";
    Serial.println((String)millis() + " - " + flushText);
    return result;
}

void DataModel::setEvent(Event e)
{
    if (event == e) 
    {
        return;
    }
    event = e;
    Serial.println((String)millis() + " - " + EventNames[(int)event]);
}

Event DataModel::getEvent()
{
    return event;
}

String DataModel::getEventText()
{
    return EventNames[(int)event];
}