#include <memory>
#include "LiveFrame.h"

LiveFrame::LiveFrame(VideoSource * videoSource) :
    m_source(videoSource),
    m_pBitmap(nullptr)
{
    m_key = m_source->NewFrame += [&](std::shared_ptr<cv::Mat> frame) {
        Refresh.Fire();
    };
}

LiveFrame::~LiveFrame()
{
    m_source->NewFrame -= m_key;
}

void LiveFrame::OnRenderBackground(ID2D1RenderTarget* pTarget, const D2D1_RECT_F&, DOUBLE effectiveOpacity)
{
    std::shared_ptr<cv::Mat> mat = m_source->CurrentFrame();

    if (!mat)
        return;

    // Create matching bitmap
    if (!m_pBitmap ||
        m_pBitmap->GetSize().height != mat->size().height ||
        m_pBitmap->GetSize().width != mat->size().width)
    {
        if (m_pBitmap)
            m_pBitmap->Release();

        D2D1_SIZE_U size = { (UINT32)mat->size().width, (UINT32)mat->size().height };
        D2D1_BITMAP_PROPERTIES props;
        props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        props.dpiX = 96;
        props.dpiY = 96;
        pTarget->CreateBitmap(size, props, &m_pBitmap);
    }

    cv::Mat newSrc;
    assert(mat->type() == CV_8UC3);
    cv::cvtColor(*mat, newSrc, CV_RGB2RGBA);
    m_pBitmap->CopyFromMemory(nullptr, newSrc.data, newSrc.step);

    // Now render bitmap
    D2D1_SIZE_F targetSize = GetSize();
    D2D1_SIZE_F srcSize = m_pBitmap->GetSize();
    D2D1_RECT_F targetRect;

    // Match X. Does Y fit?
    float xRatio = targetSize.width / srcSize.width;
    float yRatio = targetSize.height / srcSize.height;

    if (srcSize.height * xRatio < targetSize.height)
    {
        targetRect.left = 0;
        targetRect.right = targetSize.width;
        targetRect.top = (targetSize.height - (srcSize.height * xRatio)) / 2;
        targetRect.bottom = targetRect.top + srcSize.height * xRatio;

        // correct Y ratio
        yRatio = (targetRect.bottom - targetRect.top) / srcSize.height;
    }
    // else, scale to Y
    else
    {
        targetRect.left = (targetSize.width - (srcSize.width * yRatio)) / 2;
        targetRect.right = targetRect.left + srcSize.width * yRatio;
        targetRect.top = 0;
        targetRect.bottom = targetSize.height;

        // correct X ratio
        xRatio = (targetRect.right - targetRect.left) / srcSize.width;
    }

    pTarget->DrawBitmap(m_pBitmap, targetRect, (FLOAT)effectiveOpacity);
}
