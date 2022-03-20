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
void waterPeriodicFlush();
void waterDisinfection();
void waterFlushPhase1();
void waterFlushPhase2();
void waterFlushPhase2Finished();
void waterFlushPhase3();
void waterFilterOn();
void waterFilterOff();
void updateDisplay();

StopWatch sw(StopWatch::SECONDS);

Scheduler taskManager;
Task taskWater(WATER_START_DELAY, TASK_FOREVER);
Task taskPeriodicFlush(PERIODIC_FLUSH_INTERVAL, TASK_FOREVER, &waterPeriodicFlush);
Task taskDisinfection(DISINFECTION_START_DELAY, TASK_FOREVER, &waterDisinfection);
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
  taskManager.addTask(taskPeriodicFlush);
  taskManager.addTask(taskDisinfection);
  taskManager.addTask(taskDisplay);

  tds.init();
  relayModule.init();
  waterScale.init();

  taskDisplay.enable();
  taskScale.enable();
  taskTDS.enable();
  taskPeriodicFlush.enableDelayed();
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

void waterPeriodicFlush()
{
  if (model.isFlushTime() && model.getEvent() == Event::FilterOff)
  {
    Task* task = taskManager.getCurrentTask();
    task->setCallback(&waterFlushPhase1);
    task->forceNextIteration();
  }
}

void waterFlushPhase1()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase1);
  relayModule.flushMembrane();

  Task* task = taskManager.getCurrentTask();
  task->setCallback(&waterFlushPhase2);
  task->delay(FLUSH_PHASE1_DURATION);
}

void waterFlushPhase2()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase2);
  relayModule.flushStandingWater();

  Task* task = taskManager.getCurrentTask();
  task->setCallback(&waterFlushPhase2Finished);
  task->delay(FLUSH_PHASE2_DURATION);
}

void waterFlushPhase2Finished()
{
  model.lastFlushTime = millis();

  Task* task = taskManager.getCurrentTask();
  if (task == &taskWater)
  {
    task->setCallback(&waterFilterOn);
  
    // update periodic flush interval
    taskPeriodicFlush.setInterval(PERIODIC_FLUSH_INTERVAL);
  }
  else
  {
    task->setCallback(&waterFilterOff);
  }
  task->forceNextIteration();
}

void waterFlushPhase3()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FlushPhase3);
  relayModule.flushMembrane();

  Task* task = taskManager.getCurrentTask();
  task->setCallback(&waterFilterOff);
  task->delay(FLUSH_PHASE3_DURATION);
}

void waterFilterOn()
{
  sw.stop();
  sw.reset();
  sw.start();

  model.setEvent(Event::FilterOn);
  relayModule.filterWater();

  Task* task = taskManager.getCurrentTask();
  task->setCallback(&waterFilterOff);
  task->delay(WATER_TIMEOUT);
}

void waterFilterOff() 
{
  sw.stop();
  sw.reset();

  model.setEvent(Event::FilterOff);
  relayModule.off();

  Task* task = taskManager.getCurrentTask();
  if (task == &taskWater)
  {
    task->disable();
  }
}

// TODO start on button press
void startDisinfection()
{
  taskDisinfection.enableDelayed();
}

void waterDisinfection()
{
  Task* task = taskManager.getCurrentTask();
  long i = task->getRunCounter();
  Serial.println("iteration: " + (String)i);

  // filter small time amount -> wait for 15 minutes -> repeat 10 times
  if (i <= 20)
  {
    if (i % 2 == 1) 
    {
      model.setEvent(Event::FilterOn);
      relayModule.filterWater();
      task->delay(DISINFECTION_FILTER_DURATION);
    }
    else
    {
      model.setEvent(Event::FilterOff);
      relayModule.off();
      task->delay(DISINFECTION_WAIT_DURATION);
    }
  }
  // finish with flush phase 1 & 2
  else if (i == 16)
  {
    model.setEvent(Event::FlushPhase1);
    relayModule.flushMembrane();
    task->delay(FLUSH_PHASE1_DURATION);
  }
  else if (i == 17)
  {
    model.setEvent(Event::FlushPhase2);
    relayModule.flushStandingWater();
    task->delay(FLUSH_PHASE2_DURATION);
  }
  // disinfection finished
  else
  {
    model.setEvent(Event::FilterOff);
    relayModule.off();
    task->disable();
  }
}