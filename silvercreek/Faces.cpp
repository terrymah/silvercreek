#include "Faces.h"

FaceDetector::FaceDetector() :
    m_source(nullptr),
    m_videoHandle(0),
    m_active(false)
{
    if (!m_cs.load("haarcascade_frontalface_alt.xml"))
        throw std::runtime_error("Couldn't load haarcascade_frontalface_alt.xml");
}

FaceDetector::~FaceDetector()
{
    if (m_active)
        m_task.wait();
}

void FaceDetector::SetVideoSource(VideoSource * source)
{
    if (m_source) {
        m_source->NewFrame -= m_videoHandle;
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

int Face::Score(cv::Rect& rect)
{
    int widthDelta = abs(rect.width - m_lastPosition.width);
    int heightDelta = abs(rect.height - m_lastPosition.height);
    int xDelta = abs(rect.x - m_lastPosition.x);
    int yDelta = abs(rect.y - m_lastPosition.y);
    int total = widthDelta + heightDelta + xDelta + yDelta;

    if (widthDelta > MAX_SIZE_CHANGE)
        return 0;

    if (heightDelta > MAX_SIZE_CHANGE)
        return 0;

    if (xDelta > MAX_DIST_CHANGE)
        return 0;

    if (yDelta > MAX_DIST_CHANGE)
        return 0;

    // Higher is better
    return TOTAL_CHANGE - total;
}


struct ScoreResult
{
    int score;
    Face* face;
    cv::Rect* rect;
};

void FaceDetector::OnNewFrame(std::shared_ptr<cv::Mat> frame)
{
    if (m_active && !m_task.is_done())
        return;

    m_active = true;
    m_task = concurrency::create_task([=] {
        cv::Mat frame_gray;
        cv::cvtColor(*frame, frame_gray, CV_BGR2GRAY);
        cv::equalizeHist(frame_gray, frame_gray);

        // This currently isn't working because I need a VS2015 version of opencv!
        std::vector<cv::Rect> faceRects;
        faceRects.reserve(10);
        m_cs.detectMultiScale(frame_gray, faceRects, 1.1, 2, CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
        
        // Time to match
        std::lock_guard<std::mutex> lock(m_faceLock);
        for (auto& face : m_faces) {
            face->matched = false;
        }

        // Score each rect against each face
        std::vector<ScoreResult> scores;
        for (auto& rect : faceRects) {
            for (auto& face : m_faces) {
                int score = face->Score(rect);
                if (score) {
                    scores.push_back({ score, face.get(), &rect });
                }
            }
        }

        std::sort(begin(scores), end(scores), [](auto& left, auto& right) {
            return left.score > right.score;
        });
        
        // For each candidate rect/face, match them up in decending score
        for (auto& score : scores) {
            if (!score.face->matched && (score.))
        }
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
