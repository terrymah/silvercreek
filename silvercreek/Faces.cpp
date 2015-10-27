#include "Faces.h"

FaceDetector::FaceDetector() :
    m_source(nullptr),
    m_videoHandle(0),
    m_active(false)
{
}

FaceDetector::~FaceDetector()
{
    if (m_active)
        m_task.wait();
}

void FaceDetector::SetVideoSource(VideoSource * source)
{
    if (m_source) {
        m_source->CurrentFrame -= m_videoHandle;
        m_source = nullptr;
        m_videoHandle = 0;
    }

    if (source) {
        m_source = source;
        m_videoHandle = m_source->NewFrame += [&](std::shared_ptr<cv::Mat> frame) {
            OnNewFrame(frame);
        };
    }
}

void FaceDetector::OnNewFrame(std::shared_ptr<cv::Mat> frame)
{
    if (m_active && !m_task.is_done())
        return;

    m_active = true;
    m_task = concurrency::create_task([&] {
    });
}

size_t FaceDetector::GetCurrentFaceCount()
{
    std::lock_guard<std::mutex> lock(m_faceLock);
    return m_faces.size();
}

const Face * FaceDetector::GetFace(int i)
{
    std::lock_guard<std::mutex> lock(m_faceLock);
    return m_faces[i].get();
}
