#include <heltec.h>
#include <DataModel.h>

class UI
{
    public:
        void drawScale(DataModel model);
};

void UI::drawScale(DataModel model) 
{
  bool showTimer = model.getEvent() == Event::FilterOn 
    ||  model.getEvent() == Event::FlushPhase1
    ||  model.getEvent() == Event::FlushPhase2
    ||  model.getEvent() == Event::FlushPhase3
    ||  model.getEvent() == Event::GlassEmpty;


  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, "Scale: " + (String)model.weight + " g");
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 20, "Event: " + model.getEventText());
  if (showTimer)
  {
    Heltec.display->drawString(0, 30, "Timer: " + (String)model.timer + " s");
  }
  Heltec.display->display();
}