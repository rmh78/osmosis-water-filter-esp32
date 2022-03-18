#include <heltec.h>
#include <DataModel.h>

class UI
{
    public:
        void drawScale(DataModel model);
};

void UI::drawScale(DataModel model) 
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, (String)model.weight + " g");
  Heltec.display->drawString(0, 16, (String)model.ppm + " ppm");
  Heltec.display->drawString(0, 32, model.event + " " + model.timer);
  Heltec.display->display();
}