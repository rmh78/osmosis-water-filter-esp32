enum class Event
{
    Unknown = 0,
    GlassGone = 1,
    GlassEmpty = 2,
    GlassFull = 3,
    FlushPhase1 = 4,
    FlushPhase2 = 5,
    FlushPhase3 = 6,
    FilterOn = 7,
    FilterOff = 8,
    Disinfection = 9
};
static const String EventNames[] = 
{
    "Unknown", 
    "Glass gone", 
    "Glass empty", 
    "Glass full", 
    "Flush phase 1", 
    "Flush phase 2", 
    "Flush phase 3", 
    "Filter on", 
    "Filter off",
    "Disinfection"
};

const int WATER_START_DELAY = 5 * TASK_SECOND;
const int WATER_TIMEOUT = 5 * TASK_MINUTE;
/*
const int FLUSH_PHASE1_DURATION = 30 * TASK_SECOND;
const int FLUSH_PHASE2_DURATION = 2 * TASK_MINUTE;
const int FLUSH_PHASE3_DURATION = 30 * TASK_SECOND;
const int PERIODIC_FLUSH_INTERVAL = 2 * TASK_HOURS;
*/
const int FLUSH_PHASE1_DURATION = 5 * TASK_SECOND;
const int FLUSH_PHASE2_DURATION = 10 * TASK_SECOND;
const int FLUSH_PHASE3_DURATION = 15 * TASK_SECOND;
const int PERIODIC_FLUSH_INTERVAL = 2 * TASK_MINUTE;

const int DISINFECTION_START_DELAY = 5 * TASK_SECOND;
const int DISINFECTION_FILTER_DURATION = 5 * TASK_SECOND;
const int DISINFECTION_WAIT_DURATION = 15 * TASK_MINUTE;

const int SCALE_INTERVAL = 500 * TASK_MILLISECOND;
const int TDS_INTERVAL = 5 * TASK_SECOND;
const int DISPLAY_INTERVAL = 500 * TASK_MILLISECOND;

const int GLASS_EMPTY_MIN_WEIGHT_GRAM = -20;
//const int GLASS_EMPTY_MAX_WEIGHT_GRAM = 500;
//const int GLASS_FULL_WEIGHT_GRAM = 1200;
const int GLASS_EMPTY_MAX_WEIGHT_GRAM = 100;
const int GLASS_FULL_WEIGHT_GRAM = 280;

class DataModel
{
private:
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
    bool result = lastFlushTime == 0 || (lastFlushTime + PERIODIC_FLUSH_INTERVAL < millis());
    String flushText = result ? "Flush needed" : "Flush NOT needed";
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