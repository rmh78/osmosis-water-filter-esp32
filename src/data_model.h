#include <backend.h>
#include <StopWatch.h>

enum class Mode
{
    Waiting = 0,
    Normal = 1,
    Disinfection = 2
};

enum class Event
{
    Waiting = 0,
    GlassGone = 1,
    GlassEmpty = 2,
    GlassFull = 3,
    FlushPhase1 = 4,
    FlushPhase2 = 5,
    FlushPhase3 = 6,
    FilterOn = 7,
    FilterOff = 8,
    DisinfectionFinished = 9
};
static const String EventNames[] =
    {
        "Waiting",
        "Glass gone",
        "Glass empty",
        "Glass full",
        "Flush phase 1",
        "Flush phase 2",
        "Flush phase 3",
        "Filter on",
        "Filter off",
        "Disinfection finished"
    };

const int WATER_START_DELAY = 3 * TASK_SECOND;
const int WATER_TIMEOUT = 15 * TASK_MINUTE;

const int FLUSH_PHASE1_DURATION = 30 * TASK_SECOND;
const int FLUSH_PHASE2_DURATION = 2 * TASK_MINUTE;
const int FLUSH_PHASE3_DURATION = 30 * TASK_SECOND;
const int PERIODIC_FLUSH_INTERVAL = 2 * TASK_HOUR;

const int DISINFECTION_START_DELAY = 3 * TASK_SECOND;
const int DISINFECTION_FILTER_DURATION = 5 * TASK_SECOND;
const int DISINFECTION_WAIT_DURATION = 15 * TASK_MINUTE;
const int DISINFECTION_FLUSH_PHASE1_DURATION = 5 * TASK_MINUTE;
const int DISINFECTION_FLUSH_PHASE2_DURATION = 5 * TASK_MINUTE;

const int SCALE_INTERVAL = 500 * TASK_MILLISECOND;
const int TDS_INTERVAL = 5 * TASK_SECOND;
const int DISPLAY_INTERVAL = 500 * TASK_MILLISECOND;

const int GLASS_EMPTY_MIN_WEIGHT_GRAM = -20;
const int GLASS_EMPTY_MAX_WEIGHT_GRAM = 800;
const int GLASS_FULL_WEIGHT_GRAM = 1150;

StopWatch sw(StopWatch::SECONDS);

struct FilterRun
{
    int startWeight;
    int endWeight;
    int duration;
    int ppm;
};

class DataModel
{
private:
    Backend backend;
    FilterRun filterRun;
    Mode mode = Mode::Waiting;
    Event event = Event::Waiting;
    void sendDataToBackend();

public:
    int weight = 0;
    int ppm = 0;
    long lastFlushTime = 0;
    long nextFlushTime;
    long iteration = 0;
    void setMode(Mode m);
    Mode getMode();
    void setEvent(Event e);
    Event getEvent();
    String getEventText();
    bool isGlassEmpty();
    bool isGlassAlmostFull();
    bool isGlassFull();
    bool isGlassGone();
    bool isFlushTime();
    long getElapsedTime();
};

long DataModel::getElapsedTime()
{
    return sw.elapsed();
}

bool DataModel::isGlassEmpty()
{
    return weight >= GLASS_EMPTY_MIN_WEIGHT_GRAM && weight <= GLASS_EMPTY_MAX_WEIGHT_GRAM;
}

bool DataModel::isGlassAlmostFull()
{
    return weight > GLASS_EMPTY_MAX_WEIGHT_GRAM && weight < GLASS_FULL_WEIGHT_GRAM;
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

void DataModel::setMode(Mode m)
{
    sw.reset();
    sw.start();

    this->mode = m;
    this->setEvent(Event::Waiting);
    this->iteration = 0;
}

Mode DataModel::getMode()
{
    return this->mode;
}

void DataModel::setEvent(Event e)
{
    if (this->event == e)
    {
        return;
    }

    if (this->mode == Mode::Normal)
    {
        if (e == Event::FilterOn)
        {
            this->filterRun.ppm = this->ppm;
            this->filterRun.startWeight = this->weight;
        }
        else if (this->event == Event::FilterOn && e == Event::GlassFull)
        {
            this->filterRun.endWeight = this->weight;
            this->filterRun.duration = this->getElapsedTime();
        }
        else if (this->event == Event::FlushPhase3 && e == Event::FilterOff)
        {
            this->sendDataToBackend();
        }
    }

    sw.reset();
    sw.start();

    this->event = e;
    Serial.println((String)millis() + " - " + EventNames[(int)this->event]);
}

Event DataModel::getEvent()
{
    return event;
}

String DataModel::getEventText()
{
    return EventNames[(int)event];
}

void DataModel::sendDataToBackend()
{
    if (backend.connectWiFi())
    {
        backend.createFilterEvent(
            this->filterRun.startWeight,
            this->filterRun.endWeight,
            this->filterRun.duration,
            this->filterRun.ppm);
    }
    backend.disconnectWiFi();
}