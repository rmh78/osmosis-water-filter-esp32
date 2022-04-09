#include <heltec.h>
#include <data_model.h>

class UI
{
private:
  DataModel model;
  void drawHelp();
  void drawScale();
  void drawDisinfection();

public:
  void drawModel(DataModel model);
};

void UI::drawModel(DataModel model)
{
  this->model = model;
  if (model.getMode() == Mode::Waiting) 
  {
    this->drawHelp();
  }
  else if (model.getMode() == Mode::Normal) 
  {
    this->drawScale();
  }
  else if (model.getMode() == Mode::Disinfection)
  {
    this->drawDisinfection();
  }
}

void UI::drawHelp()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "Press a button!");
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 25, "GREEN - normal mode");
  Heltec.display->drawString(0, 40, "RED - disinfection mode");
  Heltec.display->display();
}

void UI::drawScale()
{
  bool showTimer = model.getEvent() == Event::FilterOn 
    || model.getEvent() == Event::FlushPhase1 
    || model.getEvent() == Event::FlushPhase2 
    || model.getEvent() == Event::FlushPhase3 
    || model.getEvent() == Event::GlassEmpty;

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "Scale: " + (String)model.weight + " g");
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 20, "Event: " + model.getEventText());
  if (showTimer)
  {
    Heltec.display->drawString(0, 30, "Timer: " + (String)model.getElapsedTime() + " s");
  }
  Heltec.display->drawString(0, 40, "TDS: " + (String)model.ppm + " ppm");
  Heltec.display->drawString(0, 50, "Auto-Flush: " + (String)model.nextFlushTime + " s");
  Heltec.display->display();
}

void UI::drawDisinfection()
{
  bool showTimer = model.getEvent() != Event::DisinfectionFinished;

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "Disinfection");
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 20, "Event: " + model.getEventText());
  if (showTimer)
  {
    Heltec.display->drawString(0, 30, "Timer: " + (String)model.getElapsedTime() + " s");
    Heltec.display->drawString(0, 40, "Iteration: " + (String)model.iteration);
  }
  Heltec.display->display();
}
