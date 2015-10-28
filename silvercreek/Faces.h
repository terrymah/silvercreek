#ifndef FACES_H
#define FACES_H

#include "VideoSource.h"
#include "events.h"
#include <ppltasks.h>

const int MAX_SIZE_CHANGE = 10;
const int MAX_DIST_CHANGE = 25;
const int TOTAL_CHANGE = MAX_DIST_CHANGE * 2 + MAX_SIZE_CHANGE * 2;
class Face
{
public:
    bool visible; // at last detect, was this face even visible?
    bool matched; // during this detect, has it been matched with a face?

    int Score(cv::Rect& rect);

    cv::Rect2i m_lastPosition; // position at last match
};

class FaceDetector
{
public:
    FaceDetector();
    ~FaceDetector();

    void SetVideoSource(VideoSource* source);

    // Executes on a background thread
    void OnNewFrame(std::shared_ptr<cv::Mat> frame);
    
    size_t GetCurrentFaceCount();
    const Face* GetFace(int i);

    // Events fire from a background thread
    tjm::events::Event1<FaceDetector, Face*> NewFace;
    tjm::events::Event1<FaceDetector, Face*> Clear;

private:
    VideoSource* m_source;
    int m_videoHandle;

    bool m_active;
    concurrency::task<void> m_task;
    std::mutex m_faceLock; // protects m_faces
    std::vector<std::unique_ptr<Face>> m_faces;
   
    cv::CascadeClassifier m_cs;
};

#endif