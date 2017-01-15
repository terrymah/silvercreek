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
    tjm::dash::SolidObject s;
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
    m_frame(&m_camera),
    s(D2D1::ColorF(D2D1::ColorF::OrangeRed))
{
}

void MyApp::InitializeApplication(tjm::dash::DashApplication * app)
{
    // Connect the refresh event to redrawing the application
    m_frame.Refresh += [app]() { app->Refresh(); };
    m_camera.Initialize();
    app->SetRoot(&m_frame);

    m_fd.SetVideoSource(&m_camera);

    m_fd.NewFace += [=](Face* f) {
        auto position = D2D1::Point2F((FLOAT)f->m_lastPosition.x, (FLOAT)f->m_lastPosition.y);
        auto size = D2D1::SizeF((FLOAT)f->m_lastPosition.width, (FLOAT)f->m_lastPosition.height);
        app->OnMainThread([=]() {
            s.SetPosition(position);
            s.SetSize(size);
            s.SetVisible(true);
            m_frame.AddChild(&s);
        });
    };

    m_fd.UpdatedFace += [=](Face* f) {
        auto position = D2D1::Point2F((FLOAT)f->m_lastPosition.x, (FLOAT)f->m_lastPosition.y);
        auto size = D2D1::SizeF((FLOAT)f->m_lastPosition.width, (FLOAT)f->m_lastPosition.height);
        bool visible = f->visible;
        app->OnMainThread([=]() {
            s.SetPosition(position);
            s.SetSize(size);
            s.SetVisible(visible);
        });
    };
}

void MyApp::DestroyApplication(tjm::dash::DashApplication * /*app*/)
{
    m_camera.Shutdown();
}
