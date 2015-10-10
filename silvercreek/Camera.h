#ifndef CAMERA_H
#define CAMERA_H

#include "windows.h"
#include "VideoSource.h"
#include <mutex>

class Camera : public VideoSource
{
public:
    Camera();
    ~Camera();

    void Initialize();
    void Shutdown();

    // Threadproc to actually capture images
    static DWORD WINAPI CvThreadProc(LPVOID param);
    DWORD CvThreadProcImpl();

    void Return(cv::Mat*);
private:

    std::mutex m_poolLock; // protects m_pool
    cv::Mat* GetMat();
    std::vector<cv::Mat*> m_pool;

    DWORD m_cvTid;
    volatile bool m_fShutdown;
    bool m_shutdownComplete;
};

#endif
