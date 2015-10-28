#include "silvercreek.h"
#include "dgui.h"
#include "Camera.h"
#include "Faces.h"
#include "LiveFrame.h"

class MyApp : public tjm::dash::ApplicationCore
{
public:
    MyApp();

    // Inherited via ApplicationCore
    virtual void InitializeApplication(tjm::dash::DashApplication * app) override;
    virtual void DestroyApplication(tjm::dash::DashApplication * app) override;

private:
    Camera m_camera;
    LiveFrame m_frame;
    FaceDetector m_fd;
};

#pragma warning(disable : 4297)
int APIENTRY wWinMain(_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR,
	_In_ int)
{
    try {
        tjm::dash::DashApplication app;
        MyApp appcore;

        app.Run(&appcore);
        //app.Run();
    }
    catch (std::runtime_error& err) {
        if (::MessageBoxA(nullptr, err.what(), nullptr, MB_OKCANCEL) == IDCANCEL) {
            throw;
        }
    }

}

MyApp::MyApp() :
    m_frame(&m_camera)
{
}

void MyApp::InitializeApplication(tjm::dash::DashApplication * app)
{
    // Connect the refresh event to redrawing the application
    m_frame.Refresh += [app]() { app->Refresh(); };
    m_camera.Initialize();
    app->SetRoot(&m_frame);

    m_fd.SetVideoSource(&m_camera);
}

void MyApp::DestroyApplication(tjm::dash::DashApplication * /*app*/)
{
    m_camera.Shutdown();
}
