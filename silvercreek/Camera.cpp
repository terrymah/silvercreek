#include "Camera.h"
#include "windows.h"

const int MAX_POOL_SIZE = 20;

Camera::Camera() :
    m_cvTid(0),
    m_fShutdown(false),
    m_shutdownComplete(true)
{
}

void Camera::Shutdown()
{
    if (!m_shutdownComplete) {
        m_fShutdown = true;
    
        HANDLE cv = OpenThread(SYNCHRONIZE, false, m_cvTid);
        WaitForSingleObject(cv, INFINITE);
        CloseHandle(cv);

        m_fShutdown = false;
        m_shutdownComplete = true;
    }
}

Camera::~Camera(void)
{
    Shutdown();

    for (auto& mat : m_pool)
    {
        delete mat;
    }
}

void Camera::Initialize()
{
    ::CreateThread(nullptr, 0, &Camera::CvThreadProc, this, 0, &m_cvTid);
    m_shutdownComplete = false;
}

DWORD WINAPI Camera::CvThreadProc(LPVOID param)
{
    Camera* me = (Camera*)param;
    return me->CvThreadProcImpl();
}

DWORD Camera::CvThreadProcImpl()
{
    cv::VideoCapture cap(0); // open the default camera

    if (!cap.isOpened())  // check if we succeeded
        return (DWORD)-1;

    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    //cap.set(CV_CAP_PROP_FORMAT, CV_8UC4);

    while (!m_fShutdown)
    {
        cv::Mat* frame = GetMat();
        cap >> *frame; // get a new frame from camera

        std::shared_ptr<cv::Mat> pframe = std::shared_ptr<cv::Mat>(frame, [this](cv::Mat* mat) { Return(mat); });
        {
            std::lock_guard<std::mutex> lock(m_frameLock);
            m_currentFrame = pframe;
        }
        Fire();
    }

    return S_OK;
}

void Camera::Return(cv::Mat * mat)
{
    { 
        std::lock_guard<std::mutex> lock(m_poolLock);
        if (m_pool.size() < MAX_POOL_SIZE) {
            m_pool.push_back(mat);
            mat = nullptr;
        }
    }

    delete mat;
}

cv::Mat * Camera::GetMat()
{
    cv::Mat * retVal = nullptr;

    { 
        std::lock_guard<std::mutex> lock(m_poolLock);
        if (m_pool.size()) {
            retVal = m_pool.back();
            m_pool.pop_back();
        }
    }

    if (!retVal) {
        retVal = new cv::Mat();
    }

    return retVal;
}
