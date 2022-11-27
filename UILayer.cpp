#include "stdafx.h"
#include "UILayer.h"

using namespace std;

UILayer::UILayer(UINT nFrames, UINT nTextBlocks, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);
    m_nRenderTargets = nFrames;
    m_ppd3d11WrappedRenderTargets = new ID3D11Resource*[nFrames];
    m_ppd2dRenderTargets = new ID2D1Bitmap1*[nFrames];

    m_nTextBlocks = nTextBlocks;
    m_pTextBlocks = new TextBlock[nTextBlocks];
    m_ppoint1 = { FRAME_BUFFER_WIDTH - 70.f ,10.f };
    for (int i = 0; i < OBJNUM; ++i)
    {
        bShowObj[i] = false;
    }

    InitializeDevice(pd3dDevice, pd3dCommandQueue, ppd3dRenderTargets);
}

void UILayer::InitializeDevice(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* pd3d11Device = NULL;
    ID3D12CommandQueue* ppd3dCommandQueues[] = { pd3dCommandQueue };
    ::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, (ID3D11Device **)&pd3d11Device, (ID3D11DeviceContext **)&m_pd3d11DeviceContext, nullptr);

    pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void **)&m_pd3d11On12Device);
    pd3d11Device->Release();

#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY pd3dSeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_MESSAGE_ID pd3dDenyIds[] = { D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE };

        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
    }
    pd3dInfoQueue->Release();
#endif

    IDXGIDevice* pdxgiDevice = NULL;
    m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdxgiDevice);

    ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void **)&m_pd2dFactory);
    HRESULT hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, (ID2D1Device2 **)&m_pd2dDevice);
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2 **)&m_pd2dDeviceContext);

    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pd2dWriteFactory);
    pdxgiDevice->Release();

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_ppd3d11WrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_ppd3d11WrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }
}

ID2D1SolidColorBrush* UILayer::CreateBrush(D2D1::ColorF d2dColor)
{
    ID2D1SolidColorBrush* pd2dDefaultTextBrush = NULL;
    m_pd2dDeviceContext->CreateSolidColorBrush(d2dColor, &pd2dDefaultTextBrush);

    return(pd2dDefaultTextBrush);
}

IDWriteTextFormat* UILayer::CreateTextFormat(WCHAR* pszFontName, float fFontSize)
{
    IDWriteTextFormat* pdwDefaultTextFormat = NULL;
    m_pd2dWriteFactory->CreateTextFormat(L"궁서체", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &pdwDefaultTextFormat);

    pdwDefaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    pdwDefaultTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    //m_pd2dWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fSmallFontSize, L"en-us", &m_pdwDefaultTextFormat);

    return(pdwDefaultTextFormat);
}

void UILayer::UpdateTextOutputs(UINT nIndex, WCHAR* pstrUIText, D2D1_RECT_F* pd2dLayoutRect, IDWriteTextFormat* pdwFormat, ID2D1SolidColorBrush* pd2dTextBrush)
{
    //wcscpy_s(m_pTextBlocks[nIndex].m_pstrText, 256, pstrUIText);
    if (pstrUIText) wcscpy_s(m_pTextBlocks[nIndex].m_pstrText, 256, pstrUIText);
    if (pd2dLayoutRect) m_pTextBlocks[nIndex].m_d2dLayoutRect = *pd2dLayoutRect;
    if (pdwFormat) m_pTextBlocks[nIndex].m_pdwFormat = pdwFormat;
    if (pd2dTextBrush) m_pTextBlocks[nIndex].m_pd2dTextBrush = pd2dTextBrush;
}

void UILayer::Render(UINT nFrame)
{
    ID3D11Resource* ppResources[] = { m_ppd3d11WrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[nFrame]);
    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    //Make circle
    float fBigRadius = 60.f;
    D2D1_POINT_2F d2dcenter;
    d2dcenter.x = FRAME_BUFFER_WIDTH - 70.f;
    d2dcenter.y = 70.f;

    D2D1_ELLIPSE d2dElipse0 = D2D1::Ellipse(d2dcenter, 30.0f, 30.0f);
    D2D1_ELLIPSE d2dElipse1 = D2D1::Ellipse(d2dcenter, 15.0f, 15.0f);
    D2D1_ELLIPSE d2dElipse2 = D2D1::Ellipse(d2dcenter, 30.0f, 30.0f);
    D2D1_ELLIPSE d2dElipse3 = D2D1::Ellipse(d2dcenter, 45.0f, 45.0f);
    D2D1_ELLIPSE d2dElipse4 = D2D1::Ellipse(d2dcenter, 60.0f, 60.0f);
    D2D1_ELLIPSE d2dElipsePlayer = D2D1::Ellipse(d2dcenter, 2.0f, 2.0f);
    //Make Time Rect
    D2D1_RECT_F d2dRect = D2D1::RectF(30.0f, 20.0f, 70.f, 40.f);
    D2D1_ROUNDED_RECT d2dRect1 = D2D1::RoundedRect(d2dRect, 0.1f, 0.1f);
    //Make Line
    D2D1_POINT_2F d2dpoint0 = d2dcenter;

    //Make Hp Rect
    if (playerHp <= 0)
    {
        m_fpHpControl = 300.f;
    }
    D2D1_RECT_F d2dRectHp1 = D2D1::RectF(FRAME_BUFFER_WIDTH/2-150.f, 20.0f, FRAME_BUFFER_WIDTH/2 + 150.f - m_fpHpControl, 30.f);
    D2D1_RECT_F d2dRectHp2 = D2D1::RectF(FRAME_BUFFER_WIDTH / 2 + 150.f - m_fpHpControl, 20.0f, FRAME_BUFFER_WIDTH / 2 + 150.f, 30.f);

    ID2D1SolidColorBrush* pd2dBrushElipse0 = CreateBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f));
    ID2D1SolidColorBrush* pd2dBrush = CreateBrush(D2D1::ColorF(D2D1::ColorF::LawnGreen, 1.0f));
    ID2D1SolidColorBrush* pd2dBrushRect = CreateBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.4f));
    ID2D1SolidColorBrush* pd2dBrushHpRect = CreateBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));
    ID2D1SolidColorBrush* pd2dBrushHp2Rect = CreateBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.5f));
    ID2D1SolidColorBrush* pd2dBrushElipseObj = CreateBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));


    //rader caculate
    if (m_ppoint1.x >= d2dcenter.x + fBigRadius)
        m_fPlusRidius *= -1;
    else if (m_ppoint1.x <= d2dcenter.x - fBigRadius)
        m_fPlusRidius *= -1;

    m_ppoint1.x += m_fPlusRidius;
    if (m_ppoint1.x < d2dcenter.x)
        m_fPlusRidius *= 1;
    if(m_ppoint1.x - d2dcenter.x != 0.f)
        m_ppoint1.y = -m_fPlusRidius * sqrtf(((fBigRadius * fBigRadius) - ((m_ppoint1.x - d2dcenter.x) * (m_ppoint1.x - d2dcenter.x)))) + d2dcenter.y;
    if (m_fPlusRidius == 1 && (m_ppoint1.y < d2dpoint0.y && m_ppoint1.x > d2dpoint0.x) && bUpdate)
    {
        UpdateRader();
        bUpdate = false;
    }
    else if(m_fPlusRidius == -1 && (m_ppoint1.y > d2dpoint0.y && m_ppoint1.x < d2dpoint0.x) && !bUpdate)
        bUpdate = true;


    m_pd2dDeviceContext->BeginDraw();

    m_pd2dDeviceContext->DrawRectangle(d2dRect, pd2dBrushRect, 50.0f, NULL);
   // m_pd2dDeviceContext->DrawRoundedRectangle(d2dRect1, pd2dBrushRect, 50.0f, NULL); //??
    for (UINT i = 0; i < m_nTextBlocks; i++)
    {
        m_pd2dDeviceContext->DrawText(m_pTextBlocks[i].m_pstrText, (UINT)wcslen(m_pTextBlocks[i].m_pstrText), m_pTextBlocks[i].m_pdwFormat, m_pTextBlocks[i].m_d2dLayoutRect, m_pTextBlocks[i].m_pd2dTextBrush);
    }
    m_pd2dDeviceContext->DrawEllipse(d2dElipse0, pd2dBrushElipse0, 60.0f, NULL);
    m_pd2dDeviceContext->DrawEllipse(d2dElipse1, pd2dBrush, 1.0f, NULL);
    m_pd2dDeviceContext->DrawEllipse(d2dElipse2, pd2dBrush, 1.0f, NULL);
    m_pd2dDeviceContext->DrawEllipse(d2dElipse3, pd2dBrush, 1.0f, NULL);
    m_pd2dDeviceContext->DrawEllipse(d2dElipse4, pd2dBrush, 1.0f, NULL);
    m_pd2dDeviceContext->DrawEllipse(d2dElipsePlayer, pd2dBrush, 4.0f, NULL);
    m_pd2dDeviceContext->DrawLine(d2dpoint0, m_ppoint1, pd2dBrush, 1.0f, NULL);
    //m_pd2dDeviceContext->DrawRectangle(d2dRectHp1, pd2dBrushHpRect, 10.0f, NULL); //??
    m_pd2dDeviceContext->DrawRectangle(d2dRectHp2, pd2dBrushHp2Rect, 10.0f, NULL);
    for (int i = 0; i < OBJNUM; ++i)
    {
        if (bShowObj[i])
        {
            m_pd2dDeviceContext->DrawEllipse(ObjEllipse[i], pd2dBrushElipseObj, 2.0f, NULL);
        }
    }

    m_pd2dDeviceContext->EndDraw();

    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();

    pd2dBrushElipse0->Release();
    pd2dBrush->Release();
    pd2dBrushRect->Release();
    pd2dBrushHpRect->Release();
    pd2dBrushHp2Rect->Release();
    pd2dBrushElipseObj->Release();
}

void UILayer::ReleaseResources()
{
    for (UINT i = 0; i < m_nTextBlocks; i++)
    {
        m_pTextBlocks[i].m_pdwFormat->Release();
        m_pTextBlocks[i].m_pd2dTextBrush->Release();
    }

    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        ID3D11Resource* ppResources[] = { m_ppd3d11WrappedRenderTargets[i] };
        m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    }

    m_pd2dDeviceContext->SetTarget(nullptr);
    m_pd3d11DeviceContext->Flush();

    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        m_ppd2dRenderTargets[i]->Release();
        m_ppd3d11WrappedRenderTargets[i]->Release();
    }

    m_pd2dDeviceContext->Release();
    m_pd2dWriteFactory->Release();
    m_pd2dDevice->Release();
    m_pd2dFactory->Release();
    m_pd3d11DeviceContext->Release();
    m_pd3d11On12Device->Release();
}

void UILayer::UpdateRader()
{
    D2D1_POINT_2F d2dcenter;
    d2dcenter.x = FRAME_BUFFER_WIDTH - 70.f;
    d2dcenter.y = 70.f;

    
    for (int i = 0; i < OBJNUM; ++i)
    {
        bShowObj[i] = false;
    }

    for (int i = 0; i < OBJNUM; ++i)
    {
        float distX = PlayerPosition.x - ObjPosition[i].x;
        float distZ = PlayerPosition.z - ObjPosition[i].z;
        if (sqrtf((distX) * (distX) + (distZ) * (distZ)) <= 1000.f)
        {
            bShowObj[i] = true;
        }
        if (ObjPosition[i].y < 600.f)
        {
            bShowObj[i] = false;
        }
            

        if (bShowObj[i])
        {
            D2D1_POINT_2F point = d2dcenter;
            point.x -= distX * 60 / 1000;
            point.y += distZ * 60 / 1000; //좌표계 반대
            ObjEllipse[i] = D2D1::Ellipse(point, 1.0f, 1.0f);
        }
    }

}

