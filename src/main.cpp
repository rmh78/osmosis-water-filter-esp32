#define _TASK_STD_FUNCTION
#define _TASK_THREAD_SAFE
#include <TaskScheduler.h>

#include <StopWatch.h>
#include <Arduino.h>
#include <heltec.h>
#include <Scale.h>
#include <Relays.h>
#include <TDS.h>
#include <UI.h>

UI ui;
DataModel model;

Scale waterScale = Scale(21, 22);
Relays relayModule = Relays(27, 14, 12, 13);
TDSMeter tds = TDSMeter(36);

void measureScale();
void measureTDS();
void waterFlushPhase1();
void waterFlushPhase2();
void waterFlushPhase2Finished();
void waterFlushPhase3();
void waterFilterOn();
void waterFilterOff();
void updateDisplay();

StopWatch sw(StopWatch::SECONDS);

const int WATER_START_DELAY = 5 * TASK_SECOND;
const int WATER_TIMEOUT = 5 * TASK_MINUTE;
/*
const int FLUSH_PHASE1_DURATION = 30 * TASK_SECOND;
const int FLUSH_PHASE2_DURATION = 2 * TASK_MINUTE;
const int FLUSH_PHASE3_DURATION = 30 * TASK_SECOND;
*/
const int FLUSH_PHASE1_DURATION = 5 * TASK_SECOND;
const int FLUSH_PHASE2_DURATION = 10 * TASK_SECOND;
const int FLUSH_PHASE3_DURATION = 15 * TASK_SECOND;


const int SCALE_INTERVAL = 500 * TASK_MILLISECOND;
const int TDS_INTERVAL = 5 * TASK_SECOND;
const int DISPLAY_INTERVAL = 500 * TASK_MILLISECOND;

Scheduler taskManager;
Task taskWater(WATER_START_DELAY, TASK_FOREVER);
Task taskScale(SCALE_INTERVAL, TASK_FOREVER, &measureScale);
Task taskTDS(TDS_INTERVAL, TASK_FOREVER, &measureTDS);
Task taskDisplay(DISPLAY_INTERVAL, TASK_FOREVER, &updateDisplay);

void setup()
{
  Serial.begin(115200);
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  taskManager.init();
  taskManager.addTask(taskScale);
  taskManager.addTask(taskTDS);
  taskManager.addTask(taskWater);
  taskManager.addTask(taskDisplay);

  tds.init();
  relayModule.init();
  waterScale.init();

  taskScale.enable();
  taskDisplay.enable();
  taskTDS.enable();
}

void loop()
{
  taskManager.execute();
}

void updateDisplay() {
  // TODO
  model.timer = sw.elapsed();
  ui.drawScale(model);
}

void measureScale()
{
  model.weight = waterScale.readWeight();

  if (model.isGlassEmpty())
  {
    if (!taskWater.isEnabled())
    {
      sw.reset();
      sw.start();

      model.setEvent(Event::GlassEmpty);
      taskWater.setCallback(model.isFlushTime() ? &waterFlushPhase1 : &waterFilterOn);
      taskWater.enableDelayed();
    }
  }
  else if (model.isGlassFull())
  {
    if (taskWater.isEnabled() && model.getEvent() == Event::FilterOn)
    {
      model.setEvent(Event::GlassFull);
      taskWater.setCallback(&waterFlushPhase3);
      taskWater.forceNextIteration();
    }
  }
  else if (model.isGlassGone())
  {
    model.setEvent(Event::GlassGone);

    if (taskWater.isEnabled())
    {
      taskWater.setCallback(&waterFilterOff);
      taskWater.forceNextIteration();
    }
  }
}

void measureTDS()
{
  model.ppm = tds.readValue();
}

void waterFlushPhase1()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase1);
  relayModule.flushMembrane();
  taskWater.setCallback(&waterFlushPhase2);
  taskWater.delay(FLUSH_PHASE1_DURATION);
}

void waterFlushPhase2()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase2);
  relayModule.flushStandingWater();
  taskWater.setCallback(&waterFlushPhase2Finished);
  taskWater.delay(FLUSH_PHASE2_DURATION);
}

void waterFlushPhase2Finished()
{
  model.lastFlushTime = millis();
  taskWater.setCallback(&waterFilterOn);
  taskWater.forceNextIteration();
}

void waterFlushPhase3()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase3);
  relayModule.flushMembrane();
  taskWater.setCallback(&waterFilterOff);
  taskWater.delay(FLUSH_PHASE3_DURATION);
}

void waterFilterOn()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FilterOn);
  relayModule.filterWater();
  taskWater.setCallback(&waterFilterOff);
  taskWater.delay(WATER_TIMEOUT);
}

void waterFilterOff() 
{
  sw.stop();
  sw.reset();

  model.setEvent(Event::FilterOff);
  relayModule.off();
  taskWater.disable();
}
