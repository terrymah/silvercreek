#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <memory>
#include <mutex>
#include "opencv2/opencv.hpp"
#include "events.h"

// Base class for a video source. The entire purpose of a video source
// is to provide "frames", which is basically a bitmap, and an event
// which fires when a new frame is available.
// For efficencies sake, we don't have virtual functions here, but 
// rather subclasses replace m_currentFrame (a data member in the base
// class) and fire the event.
class VideoSource
{
public:
    virtual ~VideoSource() {}

    std::shared_ptr<cv::Mat> CurrentFrame() { std::lock_guard<std::mutex> lock(m_frameLock); return m_currentFrame; }

    tjm::events::Event1<VideoSource, std::shared_ptr<cv::Mat>> NewFrame;
protected:

    std::mutex m_frameLock; // protects m_currentFrame
    std::shared_ptr<cv::Mat> m_currentFrame;
    void Fire() { NewFrame.Fire(CurrentFrame()); }
};

#endif
