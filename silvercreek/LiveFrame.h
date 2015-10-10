#ifndef LIVEFRAME_H
#define LIVEFRAME_H

#include "DGui.h"
#include "events.h"
#include "VideoSource.h"

class LiveFrame : public tjm::dash::Object
{
public:
    LiveFrame(VideoSource* videoSource);
    ~LiveFrame();

    void OnRenderBackground(ID2D1RenderTarget*, const D2D1_RECT_F& box, DOUBLE effectiveOpacity);

    // Events
    tjm::events::Event0<LiveFrame> Refresh;

private:
    VideoSource* m_source;	
    int m_key;

    ID2D1Bitmap* m_pBitmap;
};

#endif

