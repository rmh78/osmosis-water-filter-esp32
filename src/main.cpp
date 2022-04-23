#define _TASK_STD_FUNCTION
#define _TASK_THREAD_SAFE
#include <TaskScheduler.h>
#include <Arduino.h>
#include <OneButton.h>
#include <heltec.h>
#include <scale.h>
#include <relays.h>
#include <tds.h>
#include <ui.h>

// function prototypes - begin
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
void readButton(void *parameter);
void modeNormal();
void modeDisinfection();
// function prototypes - end

// initialize devices
Scale waterScale = Scale(21, 22);
Relays relayModule = Relays(27, 14, 12, 13);
TDSMeter tds = TDSMeter(36);
OneButton button1(2, true);
OneButton button2(17, true);

UI ui;
DataModel model;

Scheduler taskManager;
Task taskWater(WATER_START_DELAY, TASK_FOREVER);
Task taskPeriodicFlush(PERIODIC_FLUSH_INTERVAL, TASK_FOREVER, &waterPeriodicFlush);
Task taskDisinfection(DISINFECTION_START_DELAY, TASK_FOREVER, &waterDisinfection);
Task taskScale(SCALE_INTERVAL, TASK_FOREVER, &measureScale);
Task taskTDS(TDS_INTERVAL, TASK_FOREVER, &measureTDS);
Task taskDisplay(DISPLAY_INTERVAL, TASK_FOREVER, &updateDisplay);

bool buttonClicked = false;

void setup()
{
  Serial.begin(115200);
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  relayModule.init();
  
  // init WiFi for backend
  model.backend.init();

  taskManager.init();
  taskManager.addTask(taskScale);
  taskManager.addTask(taskTDS);
  taskManager.addTask(taskWater);
  taskManager.addTask(taskPeriodicFlush);
  taskManager.addTask(taskDisinfection);
  taskManager.addTask(taskDisplay);

  tds.init();

  taskDisplay.enable();
  ui.drawModel(model);

  button1.attachClick(modeNormal);
  button2.attachClick(modeDisinfection);
  while (buttonClicked == false)
  {
    // keep watching the buttons
    button1.tick();
    button2.tick();
    delay(50);
  }
}

void loop()
{
  taskManager.execute();
}

void modeNormal() 
{
  Serial.println("Button 1 clicked - Mode Normal");

  waterScale.init();
  model.setMode(Mode::Normal);
  taskScale.enable();
  taskPeriodicFlush.enableDelayed();

  buttonClicked = true;
}

void modeDisinfection() 
{
  Serial.println("Button 2 click - Mode Disinfection");

  model.setMode(Mode::Disinfection);
  taskDisinfection.adjust(DISINFECTION_START_DELAY);
  taskDisinfection.enableDelayed();

  buttonClicked = true;
}

void updateDisplay() 
{
  model.nextFlushTime = taskManager.timeUntilNextIteration(taskPeriodicFlush) / 1000;
  ui.drawModel(model);
}

void measureScale()
{
  model.weight = waterScale.readWeight();

  if (model.isGlassEmpty())
  {
    if (!taskWater.isEnabled())
    {
      model.setEvent(Event::GlassEmpty);
      taskWater.setCallback(model.isFlushTime() ? &waterFlushPhase1 : &waterFilterOn);
      taskWater.adjust(WATER_START_DELAY);
      taskWater.enableDelayed();
    }
  }
  else if (model.isGlassAlmostFull() && model.getEvent() == Event::GlassEmpty)
  {
    model.setEvent(Event::GlassFull);
    taskWater.disable();
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
    taskPeriodicFlush.setCallback(&waterFlushPhase1);
    taskPeriodicFlush.forceNextIteration();
  }
}

void waterFlushPhase1()
{
  relayModule.flushMembrane();
  model.setEvent(Event::FlushPhase1);

  Task* task = taskManager.getCurrentTask();
  if (task->isEnabled() && (task == &taskWater || task == &taskPeriodicFlush))
  {
    task->setCallback(&waterFlushPhase2);
    task->delay(FLUSH_PHASE1_DURATION);
  }
}

void waterFlushPhase2()
{
  relayModule.flushStandingWater();
  taskTDS.enable();
  model.setEvent(Event::FlushPhase2);

  Task* task = taskManager.getCurrentTask();
  if (task->isEnabled() && (task == &taskWater || task == &taskPeriodicFlush))
  {
    task->setCallback(&waterFlushPhase2Finished);
    task->delay(FLUSH_PHASE2_DURATION);
  }
}

void waterFlushPhase2Finished()
{
  model.lastFlushTime = millis();
  taskTDS.disable();

  Task* task = taskManager.getCurrentTask();
  if (task->isEnabled()) 
  {
    if (task == &taskWater)
    {
      // update periodic flush interval
      taskPeriodicFlush.setInterval(PERIODIC_FLUSH_INTERVAL);

      taskWater.setCallback(&waterFilterOn);
      taskWater.forceNextIteration();
    }
    else if (task == &taskPeriodicFlush)
    {
      taskPeriodicFlush.setCallback(&waterFilterOff);
      taskPeriodicFlush.forceNextIteration();
    }
  }
}

void waterFlushPhase3()
{
  relayModule.flushMembrane();
  model.setEvent(Event::FlushPhase3);

  taskWater.setCallback(&waterFilterOff);
  taskWater.delay(FLUSH_PHASE3_DURATION);
}

void waterFilterOn()
{
  relayModule.filterWater();
  model.setEvent(Event::FilterOn);

  taskWater.setCallback(&waterFilterOff);
  taskWater.delay(WATER_TIMEOUT);
}

void waterFilterOff() 
{
  relayModule.off();
  model.setEvent(Event::FilterOff);

  Task* task = taskManager.getCurrentTask();
  if (task == &taskWater)
  {
    taskWater.disable();
  }
  else if (task == &taskPeriodicFlush)
  {
    taskPeriodicFlush.setCallback(&waterPeriodicFlush);
    taskPeriodicFlush.setInterval(PERIODIC_FLUSH_INTERVAL);
  }
}

void waterDisinfection()
{
  long i = taskDisinfection.getRunCounter();
  Serial.println("iteration: " + (String)i);
  model.iteration = i;

  // filter small time amount -> wait for 15 minutes -> repeat 10 times
  if (i <= 20)
  {
    if (i % 2 == 1) 
    {
      model.setEvent(Event::FilterOn);
      relayModule.filterWater();
      taskDisinfection.delay(DISINFECTION_FILTER_DURATION);
    }
    else
    {
      model.setEvent(Event::FilterOff);
      relayModule.off();
      taskDisinfection.delay(DISINFECTION_WAIT_DURATION);
    }
  }
  // finish with flush phase 1 & 2
  else if (i == 21)
  {
    model.setEvent(Event::FlushPhase1);
    relayModule.flushMembrane();
    taskDisinfection.delay(DISINFECTION_FLUSH_PHASE1_DURATION);
  }
  else if (i == 22)
  {
    model.setEvent(Event::FlushPhase2);
    relayModule.flushStandingWater();
    taskDisinfection.delay(DISINFECTION_FLUSH_PHASE2_DURATION);
  }
  // disinfection finished
  else
  {
    model.setEvent(Event::DisinfectionFinished);
    relayModule.off();
    taskDisinfection.disable();
  }
}
