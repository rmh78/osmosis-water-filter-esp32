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
void waterFlushMembrane();
void waterFlushMembraneAfterFilter();
void waterFlushStandingWater();
void waterFlushFinished();
void waterFilterStart();
void waterFilterTimeout();
void waterStop();
void updateDisplay();

StopWatch sw(StopWatch::SECONDS);

const int WATER_START_DELAY = 5 * TASK_SECOND;
const int WATER_TIMEOUT = 5 * TASK_MINUTE;
const int FLUSH_MEMBRANE_DURATION = 30 * TASK_SECOND;
const int FLUSH_STANDING_WATER_DURATION = 2 * TASK_MINUTE;
const int SCALE_INTERVAL = 500 * TASK_MILLISECOND;
const int TDS_INTERVAL = 5 * TASK_SECOND;
const int DISPLAY_INTERVAL = 500 * TASK_MILLISECOND;

Scheduler taskManager;
Task taskWater(WATER_START_DELAY, TASK_FOREVER);
Task taskFlushAfterFilter(WATER_START_DELAY, TASK_FOREVER, &waterFlushMembraneAfterFilter);
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

  taskWater.setOnDisable(&waterStop);
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

      model.event = "glass empty";
      taskWater.setCallback(model.isFlushTime() ? &waterFlushMembrane : &waterFilterStart);
      taskWater.enableDelayed();
    }
  }
  else if (model.isGlassFull())
  {
    if (taskWater.isEnabled())
    {
      model.event = "glass full";
      taskWater.cancel();
      taskFlushAfterFilter.enable();
    }
  }
  else if (model.isGlassGone())
  {
    if (taskWater.isEnabled())
    {
      model.event = "glass gone";
      taskWater.cancel();
    }
  }
}

void measureTDS()
{
  model.ppm = tds.readValue();
}

void waterFlushMembrane()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.event = "flush membrane";
  relayModule.flushMembrane();
  taskWater.setCallback(&waterFlushStandingWater);
  taskWater.delay(FLUSH_MEMBRANE_DURATION);
}

void waterFlushMembraneAfterFilter()
{
  model.event = "flush membrane";
  relayModule.flushMembrane();
  taskWater.setCallback(&waterStop);
  taskWater.delay(FLUSH_MEMBRANE_DURATION);
}

void waterFlushStandingWater()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.event = "flush standing water";
  relayModule.flushStandingWater();
  taskWater.setCallback(&waterFlushFinished);
  taskWater.delay(FLUSH_STANDING_WATER_DURATION);
}

void waterFlushFinished()
{
  model.lastFlushTime = millis();
  taskWater.setCallback(&waterFilterStart);
  taskWater.forceNextIteration();
}

void waterFilterStart()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.event = "filter run";
  relayModule.filterWater();
  taskWater.setCallback(&waterFilterTimeout);
  taskWater.delay(WATER_TIMEOUT);
}

void waterFilterTimeout()
{
  taskWater.disable();
}

void waterStop() 
{
  sw.stop();
  relayModule.off();
}