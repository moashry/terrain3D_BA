/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/


#include <time.h>
#include "tchar.h"

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

//#include "debug.h"
//#include "camera.h"
#include "global.h"
#include "recorder.h"
#include "configuration.h"
#include "overviewmap.h"

#include <stdio.h>

#include "tumlogo.h"

#include "terrain3deffect.h"

#include "terrainLib\terrainLib.h"

#include "settings.h"

#ifndef DEMO
#define PERFHUD // Programm mit NVIDIA PerfHUD analysieren
#else
#undef PERFHUD
#endif

//#define WRITE_IMAGE_SEQUENCE // Aktivieren, um Serie von Standbildern zu erzeugen

#ifdef WRITE_IMAGE_SEQUENCE
const char*const			ptcWRITE_IMAGE_SEQUENCE_DIR = "E:\\TerrainVideo\\Utah\\1";
const unsigned int			uiTARGET_FRAME_RATE = 30;
const unsigned int			uiSTART_FRAME_NUM = 0;
#endif

const char*const			ptcBENCHMARK_OUTPUT_FILE = "C:\\Temp\\RasterizedUtah2560x1600.txt";

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager;// manager for shared resources of dialogs
CD3DSettingsDlg             g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D UI
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
CDXUTDialog					g_ClusterUI;

ID3DX10Font*                g_pFont = NULL;         // Font for drawing text
ID3DX10Sprite*              g_pSprite = NULL;       // Sprite for batching text drawing
CDXUTTextHelper*            g_pTxtHelper = NULL;

Terrain3DEffect				g_effect;

ID3D10Buffer*				g_pFrustumVertexBuffer = NULL;
ID3D10Buffer*				g_pFrustumIndexBuffer = NULL;

ID3D10Texture2D*			g_pTumLogoTexture = NULL;
ID3D10ShaderResourceView*	g_pTumLogoSRV = NULL;

ID3D10RasterizerState*		g_pRasterizerStateSolid = NULL;
ID3D10RasterizerState*		g_pRasterizerStateWireframe = NULL;

D3D10_VIEWPORT				g_viewport;

// Flags
bool						g_bRenderUI = false;
bool						g_bRenderHelp = false;
bool						g_bCaptureScreenshot = false;
bool						g_bToggleSettingsDlg = false;
bool						g_bRenderToBuffer = false;
unsigned int				g_uiBenchmarkState = 0;
bool						g_bBenchmarkBusTransfer = false;
bool						g_bBenchmarkStreaming = false;
bool						g_bRenderOverviewMap = true;
float						g_fOverviewMapSize = 35.0f;

// Komponenten
Camera*					g_pCamera = NULL;

Recorder*					g_pRecorder = NULL;

CommandLine					g_commandLine;
SystemConfiguration*		g_pSystemConfiguration = NULL;
ITerrainInstance*			g_pTerrainInstance = NULL;
DataLoader*					g_pDataLoader = NULL;
Terrain*					g_pTerrain = NULL;
ResourcePool*				g_pResourcePool = NULL;
// TODO: move the overview map to terrain lib, too [Andreas Kirsch]
OverviewMap*				g_pOverviewMap = NULL;

unsigned int				g_uiWidth;
unsigned int				g_uiHeight;

ID3D10Texture2D*			g_pRenderBuffer = NULL;
ID3D10RenderTargetView*		g_pRenderBufferRTV = NULL;
ID3D10ShaderResourceView*	g_pRenderBufferSRV = NULL;

float						g_fRenderBufferScale = 2.0f;

unsigned int				g_uiRenderBufferWidth;
unsigned int				g_uiRenderBufferHeight;
ID3D10Texture2D*			g_pDepthStencil = NULL;
ID3D10DepthStencilView*		g_pDepthStencilView = NULL;

Settings					g_Settings;


const float g_fClearColor[4] = //{ 0.227f, 0.431f, 0.647f, 1.0f }; // red, green, blue, alpha
							//{ 58.0f / 255.0f, 110.0f / 255.0f, 165.0f / 255.0f, 1.0f };
							{ 57.0f / 255.0f, 113.0f / 255.0f, 165.0f / 255.0f, 1.0f }; // S3TC-geeignet (im R5G6B5-Format darstellbar)

unsigned int				g_uiImageSequenceNumber;

bool						g_bOnFrameRenderCalledSinceLastOnFrameMove = true;

bool						g_bDeviceSettingsChanged = false;
CCDeviceSettings			g_ccDeviceSettings;


bool						g_bDemoMode = true;
bool						g_bRestorePosition = false;
int							g_iIdleTime = 90;
double						g_fIdleTime = 0.0;
bool						g_bDemo = false;
bool						g_bActivity = true;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum
{
	IDC_TOGGLEFULLSCREEN = 1,
	IDC_CHANGEDEVICE,
	IDC_TOGGLEWIREFRAME,
	IDC_TOGGLEPAGEBOUNDINGBOXES,
	IDC_TOGGLEBOUNDINGBOXES,
	IDC_TOGGLESTATICMESH,
	IDC_TOGGLEOCCLUSIONCULLING,
	IDC_TOGGLERENDERTOBUFFER,
	IDC_TOGGLETEXTUREPOSTPROCESSING,
	IDC_TOGGLERENDERSTATISTICS,
	IDC_TOGGLERENDERLOGO,
	IDC_TOGGLERENDEROVERVIEWMAP,
	IDC_OVERVIEW_MAP_SCALE_TEXT,
	IDC_OVERVIEW_MAP_SCALE,
	IDC_SATMODIFY,
	IDC_SATURATION_TEXT,
	IDC_SATURATION,

	IDC_FOCALLENGTH_SCALE_TEXT,
	IDC_FOCALLENGTH_SCALE,
	IDC_EYEDIST_SCALE_TEXT,
	IDC_EYEDIST_SCALE,
	IDC_MAXLEVELLOADING_TEXT,
	IDC_MAXLEVELLOADING,
	IDC_MAXLEVELRENDERING_TEXT,
	IDC_MAXLEVELRENDERING,

	IDC_DEMO_MODE,
	IDC_RESTORE_POSITION,
	IDC_IDLE_TIME_TEXT,
	IDC_IDLE_TIME,
};

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType, DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain *pSwapChain, const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext );
void    CALLBACK OnD3D10DestroyDevice( void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void	CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, INT nMouseWheelDelta, INT xPos, INT yPos, void* pUserContext);
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

void    RenderStatistics();
void	RenderHelp();
void    InitApp();
void    EndApp();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{

	// Prüfen ob das Programm bereits gestartet wurde, wenn ja, Warnung ausgeben
	HWND hOther = FindWindowA("Direct3DWindowClass", NULL);
	if (hOther)
	{
		if (IDNO == MessageBoxA(NULL, "Another Direct3D Application is already running!\nDo you want to continue?", "Warning", MB_YESNO | MB_ICONWARNING))
		{
			return 0;
		}
	}

	HRESULT hr = g_commandLine.ParseCommandLine();
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Error Parsing Command Line\n" __LOCATION__, "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	// Systemkonfiguration einlesen
	g_pSystemConfiguration = new SystemConfiguration();
	if (g_commandLine.IsConfigFileSpecified())
	{
		g_pSystemConfiguration->ReadFile(g_commandLine.GetConfigFileName());
	}
	else
	{
		g_pSystemConfiguration->ReadFile("Terrain3D.cfg");
	}

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(5777);
#endif

	// DXUT will create and use the best device (either D3D9 or D3D10) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set DXUT callbacks
	DXUTSetCallbackD3D10DeviceAcceptable( IsD3D10DeviceAcceptable );
	DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
	DXUTSetCallbackD3D10SwapChainResized( OnD3D10ResizedSwapChain );
	DXUTSetCallbackD3D10SwapChainReleasing( OnD3D10ReleasingSwapChain );
	DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
	DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );

	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
	DXUTSetCallbackMouse(MouseProc);
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen

	InitApp();

	DXUTCreateWindow( L"Terrain3D" );

    DXUTCreateDevice( true, g_pSystemConfiguration->GetViewportWidth(), g_pSystemConfiguration->GetViewportHeight() );
#ifndef _DEBUG
	ShowWindow(DXUTGetHWND(), SW_SHOWMAXIMIZED);
#endif

	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();

	EndApp();

	delete g_pSystemConfiguration;

	return DXUTGetExitCode();

}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_D3DSettingsDlg.Init( &g_DialogResourceManager );
	g_HUD.Init( &g_DialogResourceManager );
	g_SampleUI.Init( &g_DialogResourceManager );
	g_ClusterUI.Init( &g_DialogResourceManager );

	if (g_Settings.m_bIsRemoteControl)
	{
		g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
		g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle Full Screen", 35, iY, 125, 22 );
		g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change Device (F2)", 35, iY += 24, 125, 22 );

		g_HUD.SetSize( 170, 170 );
		g_HUD.SetVisible(g_bRenderUI);

		g_SampleUI.SetCaptionText(L"Main Controls");
		g_SampleUI.EnableCaption(true);
		g_SampleUI.SetBackgroundColors(D3DXCOLOR(1.0f, 0.0f, 0.0f, 0.5f));
		g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;

		g_SampleUI.AddCheckBox(IDC_TOGGLEWIREFRAME, L"Wireframe (W)", 10, iY += 24, 180, 22, g_Settings.GetWireframe());
#ifndef DEMO
		g_SampleUI.AddCheckBox(IDC_TOGGLEPAGEBOUNDINGBOXES, L"Page Bounding Boxes (P)", 10, iY += 24, 180, 22, g_Settings.GetPageBoundingBoxes());
#endif
		g_SampleUI.AddCheckBox(IDC_TOGGLEBOUNDINGBOXES, L"Tile Bounding Boxes (B)", 10, iY += 24, 180, 22, g_Settings.GetBoundingBoxes());
#ifndef DEMO
		g_SampleUI.AddCheckBox(IDC_TOGGLESTATICMESH, L"Static Mesh (S)", 10, iY += 24, 180, 22, g_Settings.GetStaticMesh());
		g_SampleUI.AddCheckBox(IDC_TOGGLEOCCLUSIONCULLING, L"Occlusion Culling", 10, iY += 24, 180, 22, g_Settings.GetOcclusionCulling());
		g_SampleUI.AddCheckBox(IDC_TOGGLERENDERTOBUFFER, L"Render To Buffer", 10, iY += 24, 180, 22, g_bRenderToBuffer);
		g_SampleUI.AddCheckBox(IDC_TOGGLETEXTUREPOSTPROCESSING, L"Texture Post-Processing", 10, iY += 24, 180, 22, g_Settings.GetTexturePostProcessing());
#endif
		g_SampleUI.AddCheckBox(IDC_TOGGLERENDERSTATISTICS, L"Render Statistics (Q)", 10, iY += 24, 180, 22, g_Settings.GetRenderStatistics());
#ifndef DEMO
		g_SampleUI.AddCheckBox(IDC_TOGGLERENDERLOGO, L"Render Logo", 10, iY += 24, 180, 22, g_Settings.GetRenderLogo());
#endif
		g_SampleUI.AddCheckBox(IDC_TOGGLERENDEROVERVIEWMAP, L"Render Overview Map (O)", 10, iY += 24, 180, 22, g_bRenderOverviewMap);
		g_SampleUI.GetCheckBox(IDC_TOGGLERENDEROVERVIEWMAP)->SetEnabled(false);
		wchar_t str[MAX_STRING_SIZE];
		swprintf_s(str, MAX_STRING_SIZE, L"Overview Map Size: %.0f", g_fOverviewMapSize);		
		g_SampleUI.AddStatic(IDC_OVERVIEW_MAP_SCALE_TEXT, str, 10, iY += 24, 180, 22);
		g_SampleUI.GetStatic(IDC_OVERVIEW_MAP_SCALE_TEXT)->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
		g_SampleUI.GetStatic(IDC_OVERVIEW_MAP_SCALE_TEXT)->SetEnabled(false);
		g_SampleUI.AddSlider(IDC_OVERVIEW_MAP_SCALE, 20, iY += 24, 160, 22);
		g_SampleUI.GetSlider(IDC_OVERVIEW_MAP_SCALE)->SetRange(5 * 100, 100 * 100);
		g_SampleUI.GetSlider(IDC_OVERVIEW_MAP_SCALE)->SetValue(static_cast<int>(g_fOverviewMapSize * 100.0f));
		g_SampleUI.GetSlider(IDC_OVERVIEW_MAP_SCALE)->SetEnabled(false);
		
		g_SampleUI.AddCheckBox(IDC_SATMODIFY, L"Modify Saturation", 10, iY += 24, 180, 22, g_Settings.GetSatModify());
		swprintf_s(str, MAX_STRING_SIZE, L"Saturation: %.2f", g_Settings.GetSaturation());
		g_SampleUI.AddStatic(IDC_SATURATION_TEXT, str, 10, iY += 24, 180, 22);
		g_SampleUI.GetStatic(IDC_SATURATION_TEXT)->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
		g_SampleUI.GetStatic(IDC_SATURATION_TEXT)->SetEnabled(g_Settings.GetSatModify());
		g_SampleUI.AddSlider(IDC_SATURATION, 20, iY += 24, 160, 22);
		g_SampleUI.GetSlider(IDC_SATURATION)->SetRange(static_cast<int>(0.0f * 100), static_cast<int>(2.0f * 100));
		g_SampleUI.GetSlider(IDC_SATURATION)->SetValue(static_cast<int>(g_Settings.GetSaturation() * 100.0f));
		g_SampleUI.GetSlider(IDC_SATURATION)->SetEnabled(g_Settings.GetSatModify());
#ifndef DEMO
		g_SampleUI.AddStatic( IDC_MAXLEVELLOADING_TEXT, L"Max Level Loading", 10, iY += 24, 140, 22 );
		g_SampleUI.GetStatic( IDC_MAXLEVELLOADING_TEXT )->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
		g_SampleUI.AddSlider( IDC_MAXLEVELLOADING, 20, iY += 24, 120, 22 );

		g_SampleUI.AddStatic( IDC_MAXLEVELRENDERING_TEXT, L"Max Level Rendering", 10, iY += 24, 140, 22 );
		g_SampleUI.GetStatic( IDC_MAXLEVELRENDERING_TEXT )->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
		g_SampleUI.AddSlider( IDC_MAXLEVELRENDERING, 20, iY += 24, 120, 22 );
#endif
		g_SampleUI.AddCheckBox(IDC_DEMO_MODE, L"Demo Mode", 10, iY += 24, 180, 22, g_bDemoMode);
		g_SampleUI.GetCheckBox(IDC_DEMO_MODE)->SetEnabled(false);
		g_SampleUI.AddCheckBox(IDC_RESTORE_POSITION, L"Restore Position", 10, iY += 24, 180, 22, g_bRestorePosition);
		g_SampleUI.GetCheckBox(IDC_RESTORE_POSITION)->SetEnabled(false);
		swprintf_s(str, MAX_STRING_SIZE, L"Idle Time: %is", g_iIdleTime);
		g_SampleUI.AddStatic(IDC_IDLE_TIME_TEXT, str, 10, iY += 24, 180, 22);
		g_SampleUI.GetStatic(IDC_IDLE_TIME_TEXT)->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
		g_SampleUI.GetStatic(IDC_IDLE_TIME_TEXT)->SetEnabled(false);
		g_SampleUI.AddSlider(IDC_IDLE_TIME, 20, iY += 24, 160, 22);
		g_SampleUI.GetSlider(IDC_IDLE_TIME)->SetRange(5, 600);
		g_SampleUI.GetSlider(IDC_IDLE_TIME)->SetValue(g_iIdleTime);
		g_SampleUI.GetSlider(IDC_IDLE_TIME)->SetEnabled(false);
#ifdef DEMO
		g_SampleUI.SetSize( 200, 400 );
#else
		g_SampleUI.SetSize( 200, 645 );
#endif
		g_SampleUI.SetVisible(g_bRenderUI);

#ifndef _DEBUG
		if (!g_Settings.m_bLocalOnly)
#endif
		{
			g_ClusterUI.SetCaptionText(L"Cluster Controls");
			g_ClusterUI.EnableCaption(true);
			g_ClusterUI.SetBackgroundColors(D3DXCOLOR(0,0,1,0.5));
			g_ClusterUI.SetCallback( OnGUIEvent ); iY = 10;

			wchar_t str[MAX_STRING_SIZE];
			swprintf_s(str, MAX_STRING_SIZE, L"Focal Length: %.2f", g_Settings.GetFocalLength());
			g_ClusterUI.AddStatic( IDC_FOCALLENGTH_SCALE_TEXT, str, 10, iY, 140, 22 );
			g_ClusterUI.GetStatic( IDC_FOCALLENGTH_SCALE_TEXT )->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
			g_ClusterUI.AddSlider( IDC_FOCALLENGTH_SCALE, 20, iY += 24, 120, 22 );
			g_ClusterUI.GetSlider( IDC_FOCALLENGTH_SCALE )->SetRange( 100 * 100, 5000 * 100);
			g_ClusterUI.GetSlider( IDC_FOCALLENGTH_SCALE )->SetValue( static_cast<int>(g_Settings.GetFocalLength() * 100.0f) );

			swprintf_s(str, MAX_STRING_SIZE, L"Eye Distance: %.2f", g_Settings.GetEyeDist());
			g_ClusterUI.AddStatic( IDC_EYEDIST_SCALE_TEXT, str, 10, iY += 24, 140, 22 );
			g_ClusterUI.GetStatic( IDC_EYEDIST_SCALE_TEXT )->GetElement(0)->dwTextFormat = DT_LEFT|DT_VCENTER|DT_WORDBREAK;
			g_ClusterUI.AddSlider( IDC_EYEDIST_SCALE, 20, iY += 24, 120, 22 );
			g_ClusterUI.GetSlider( IDC_EYEDIST_SCALE )->SetRange( 0, 100 * 100 );
			g_ClusterUI.GetSlider( IDC_EYEDIST_SCALE )->SetValue(  static_cast<int>(g_Settings.GetEyeDist() * 100.0f) );

			g_ClusterUI.SetSize( 160, 135 );
			g_ClusterUI.SetVisible(g_bRenderUI);
		}
	}
	else
	{
#ifndef _DEBUG
		ShowCursor(FALSE);
#endif
	}

	g_pTerrainInstance = CreateTerrainInstance( 
			g_commandLine.IsTerrainFileSpecified() ? g_commandLine.GetTerrainFileName() : g_pSystemConfiguration->GetTerrainFileName(),
			g_pSystemConfiguration->GetSystemMemoryUsageLimit(),
			g_pSystemConfiguration->GetInitNumVBs(),
			g_pSystemConfiguration->GetInitNumTextures(),
			g_pSystemConfiguration->GetScreenSpaceError(),
			g_Settings.GetTexturePostProcessing()
		);
	g_pTerrain = &g_pTerrainInstance->GetTerrain();
	g_pDataLoader = &g_pTerrainInstance->GetDataLoader();
	g_pResourcePool = &g_pTerrainInstance->GetResourcePool();

	// Komponenten anlegen
	//g_Settings.SetFrustumCulling(g_pTerrain->GetFrustumCulling());

    // only useful if multiple views are being used, but still needs to be called [10/9/2009 Andreas Kirsch]
	g_pDataLoader->GetNewViewIndex();

	if (g_Settings.m_bIsRemoteControl
//#ifndef _DEBUG
//		&& !g_Settings.m_bLocalOnly
//#endif
		)
	{
#ifndef DEMO
		wchar_t str_[MAX_STRING_SIZE];
		g_SampleUI.GetSlider( IDC_MAXLEVELLOADING )->SetRange( 0, g_pTerrainInstance->GetNumLevels() - 1 );
		g_SampleUI.GetSlider( IDC_MAXLEVELLOADING )->SetValue( g_pTerrainInstance->GetNumLevels() - 1 );
		g_pDataLoader->SetMaxLevel( static_cast<unsigned int>(g_SampleUI.GetSlider( IDC_MAXLEVELLOADING )->GetValue()) );
		swprintf_s(str_, MAX_STRING_SIZE, L"Max Level Loading: %u", static_cast<unsigned int>(g_SampleUI.GetSlider( IDC_MAXLEVELLOADING )->GetValue()));
		g_SampleUI.GetStatic(IDC_MAXLEVELLOADING_TEXT)->SetText(str_);

		g_SampleUI.GetSlider( IDC_MAXLEVELRENDERING )->SetRange( 0, g_pTerrainInstance->GetNumLevels() - 1 );
		g_SampleUI.GetSlider( IDC_MAXLEVELRENDERING )->SetValue( g_pTerrainInstance->GetNumLevels() - 1 );
		g_pDataLoader->SetMaxLevel( static_cast<unsigned int>(g_SampleUI.GetSlider( IDC_MAXLEVELRENDERING )->GetValue()) );
		swprintf_s(str_, MAX_STRING_SIZE, L"Max Level Rendering: %u", static_cast<unsigned int>(g_SampleUI.GetSlider( IDC_MAXLEVELRENDERING )->GetValue()));
		g_SampleUI.GetStatic(IDC_MAXLEVELRENDERING_TEXT)->SetText(str_);
#endif
	}

	g_pCamera = Camera::CreateFromTerrainServerInstance( *g_pTerrainInstance, 0);
	g_Settings.SetCamera(g_pCamera);

	g_pRecorder = new Recorder(g_pCamera);

	// TODO:... [Andreas Kirsch]
	if (g_pTerrainInstance->GetOverviewMapFileName() != NULL)
	{
		float fX0, fY0, fSizeX, fSizeY;
		g_pTerrainInstance->GetOverviewMapExtents(&fX0, &fY0, &fSizeX, &fSizeY);
		g_pOverviewMap = new OverviewMap(g_pTerrainInstance->GetOverviewMapFileName(), fX0, fY0, fSizeX, fSizeY, g_pCamera);
		g_SampleUI.GetCheckBox(IDC_TOGGLERENDEROVERVIEWMAP)->SetEnabled(true); // Achtung: UI ist nur verfügbar, wenn m_bIsRemoteControl == true
		g_SampleUI.GetStatic(IDC_OVERVIEW_MAP_SCALE_TEXT)->SetEnabled(true);
		g_SampleUI.GetSlider(IDC_OVERVIEW_MAP_SCALE)->SetEnabled(true);
	}
	if (g_pTerrainInstance->GetDemoFlightFileName() != NULL)
	{
		g_SampleUI.GetCheckBox(IDC_DEMO_MODE)->SetEnabled(true);
		g_SampleUI.GetCheckBox(IDC_RESTORE_POSITION)->SetEnabled(g_bDemoMode);
		g_SampleUI.GetStatic(IDC_IDLE_TIME_TEXT)->SetEnabled(g_bDemoMode);
		g_SampleUI.GetSlider(IDC_IDLE_TIME)->SetEnabled(g_bDemoMode);
	}

	// TODO: adapt g_pTerrainConfiguration.GetStartPosition.. [9/28/2009 Andreas Kirsch]
}


void EndApp()
{
	// TODO: remove this if it is not used really
	g_Settings.SetCamera(NULL); 
	//g_Settings.SetFrustumCulling(NULL);

	if (g_pOverviewMap != NULL)
	{
		delete g_pOverviewMap;
	}

	delete g_pRecorder;
	delete g_pCamera;

	delete g_pTerrainInstance;
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType, DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC *pBufferSurfaceDesc, void* pUserContext )
{
	SetForegroundWindow(DXUTGetHWND());
	SetFocus(DXUTGetHWND());

	V_RETURN( g_DialogResourceManager.OnD3D10CreateDevice( pd3dDevice ) );
	V_RETURN( g_D3DSettingsDlg.OnD3D10CreateDevice( pd3dDevice ) );
	V_RETURN( D3DX10CreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
								OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
								L"Arial", &g_pFont ) );
	V_RETURN( D3DX10CreateSprite( pd3dDevice, 512, &g_pSprite ) );
	g_pTxtHelper = new CDXUTTextHelper( NULL, NULL, g_pFont, g_pSprite, 15 );

	g_effect.Create(pd3dDevice);

	D3D10_RASTERIZER_DESC RSDesc;
	ZeroMemory(&RSDesc, sizeof(RSDesc));
	RSDesc.FillMode = D3D10_FILL_SOLID;
	RSDesc.CullMode = D3D10_CULL_BACK;
	RSDesc.FrontCounterClockwise = false;
	RSDesc.DepthClipEnable = true; // Clipping
	RSDesc.MultisampleEnable = true;
	pd3dDevice->CreateRasterizerState(&RSDesc, &g_pRasterizerStateSolid);

	ZeroMemory(&RSDesc, sizeof(RSDesc));
	RSDesc.FillMode = D3D10_FILL_WIREFRAME;
	RSDesc.CullMode = D3D10_CULL_BACK;
	RSDesc.FrontCounterClockwise = false;
	RSDesc.DepthClipEnable = true; // Clipping
	RSDesc.MultisampleEnable = true;
	pd3dDevice->CreateRasterizerState(&RSDesc, &g_pRasterizerStateWireframe);

	pd3dDevice->RSSetState(g_pRasterizerStateSolid);

	// View Frustum Vertex Buffer anlegen
	D3D10_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D10_BUFFER_DESC));
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = 8 * sizeof(D3DXVECTOR4);
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	V_RETURN( pd3dDevice->CreateBuffer(&bd, NULL, &g_pFrustumVertexBuffer) );

	// View Frustum Index Buffer anlegen
	// Bounding Boxes Index Buffer anlegen
	const unsigned int indicesBB[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D10_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(indicesBB);
	bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = indicesBB;
	V_RETURN( pd3dDevice->CreateBuffer(&bd, &initData, &g_pFrustumIndexBuffer) );

	// Logos erzeugen
	D3D10_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D10_TEXTURE2D_DESC));
	textureDesc.Width = 256;
	textureDesc.Height = 128;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ZeroMemory(&initData, sizeof(D3D10_SUBRESOURCE_DATA));
	initData.pSysMem = ucTUMLogo;
	initData.SysMemPitch = 256 * 4;
	V_RETURN( pd3dDevice->CreateTexture2D(&textureDesc, &initData, &g_pTumLogoTexture) );
	V_RETURN( pd3dDevice->CreateShaderResourceView(g_pTumLogoTexture, NULL, &g_pTumLogoSRV) );

    V_RETURN( g_pTerrainInstance->Create( pd3dDevice ) );

	if (g_pOverviewMap != NULL)
	{
		V_RETURN( g_pOverviewMap->OnCreateDevice(pd3dDevice, g_effect.m_pEffect) );
	}

	return S_OK;
}


void CreateRenderBuffer()
{
	ID3D10Device* pd3dDevice = DXUTGetD3D10Device();

	g_uiRenderBufferWidth = static_cast<unsigned int>(g_fRenderBufferScale * static_cast<float>(g_uiWidth));
	g_uiRenderBufferHeight = static_cast<unsigned int>(g_fRenderBufferScale * static_cast<float>(g_uiHeight));

	// TODO Breite und Höhe auf 8192 begrenzen

	D3D10_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D10_TEXTURE2D_DESC));
	textureDesc.Width = g_uiRenderBufferWidth;
	textureDesc.Height = g_uiRenderBufferHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pRenderBuffer);
	if (FAILED(hr))
	{
		DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
		exit(-1);
	}

	hr = pd3dDevice->CreateRenderTargetView(g_pRenderBuffer, NULL, &g_pRenderBufferRTV);
	if (FAILED(hr))
	{
		DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
		exit(-1);
	}

	hr = pd3dDevice->CreateShaderResourceView(g_pRenderBuffer, NULL, &g_pRenderBufferSRV);
	if (FAILED(hr))
	{
		DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
		exit(-1);
	}

	ZeroMemory(&textureDesc, sizeof(D3D10_TEXTURE2D_DESC));
	textureDesc.Width = g_uiRenderBufferWidth;
	textureDesc.Height = g_uiRenderBufferHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D10_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pDepthStencil);
	if (FAILED(hr))
	{
		DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
		exit(-1);
	}

	hr = pd3dDevice->CreateDepthStencilView(g_pDepthStencil, NULL, &g_pDepthStencilView);
	if (FAILED(hr))
	{
		DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
		exit(-1);
	}

}


void ReleaseRenderBuffer()
{
	SAFE_RELEASE(g_pDepthStencilView);
	SAFE_RELEASE(g_pDepthStencil);
	SAFE_RELEASE(g_pRenderBufferSRV);
	SAFE_RELEASE(g_pRenderBufferRTV);
	SAFE_RELEASE(g_pRenderBuffer);
}


void OnResize(unsigned int uiLocalWidth, unsigned int uiLocalHeight)
{
	D3DXVECTOR3 vPosition;
	g_pCamera->GetPosition((float*) vPosition);
	float fZNear = g_pSystemConfiguration->GetZNear();
	if (vPosition.z > 10000.0f && fZNear < 100.0f)
	{
		fZNear = 100.0f; // Bei zNear = 10.0f stürzt in großen Höhen der Treiber ab (Version 158.43, 17.05.2007)
	}

	unsigned int uiWidth = uiLocalWidth;
	unsigned int uiHeight = uiLocalHeight;
	if (!g_Settings.m_bIsRemoteControl && g_Settings.m_fEyeDist == 0.0f) // Nur im Tiled-Mode
	{
		//uiWidth *= g_Settings.m_viTiles[0];
		//uiHeight *= g_Settings.m_viTiles[1];
		// Auskommentiert da Arbeitsspeicherbedarf zu groß wird
	}
	float fAspectRatio = static_cast<float>(uiWidth) / static_cast<float>(uiHeight);

	if (g_Settings.m_bIsRemoteControl)
	{
		D3DXMATRIX mProjection;
		D3DXMatrixPerspectiveFovLH( &mProjection, DEG2RAD(g_pSystemConfiguration->GetFovy()), static_cast<float>(uiLocalWidth) / static_cast<float>(uiLocalHeight), fZNear, g_pSystemConfiguration->GetZFar() ); // Bei zNear = 10.0f stürzt in großen Höhen der Treiber ab (Version 158.43, 17.05.2007)

		g_Settings.m_mProj = (float*) mProjection;
	}
	/*
	else
	{
		g_Settings.ComputeFrustum(DEG2RAD(g_pSystemConfiguration->GetFovy()), fAspectRatio, fZNear, g_pSystemConfiguration->GetZFar(), g_Settings.GetFocalLength());
		D3DXMATRIX temp;
		g_pCamera->GetViewMatrix(&temp);
		g_Settings.ModifyAndStoreUserViewMatrix(temp);
	}
	*/

	g_pTerrainInstance->GetTerrain().SetParameters(g_Settings.m_mProj, uiHeight, g_pSystemConfiguration->GetFovy(), fAspectRatio, fZNear, g_pSystemConfiguration->GetScreenSpaceError());
	g_pTerrainInstance->GetDataLoader().ComputeLoadingRadiuses(uiHeight, g_pSystemConfiguration->GetFovy(), fAspectRatio, g_pSystemConfiguration->GetZFar(), g_pSystemConfiguration->GetScreenSpaceError(), g_pSystemConfiguration->GetPrefetchingFactor());
	if (g_pOverviewMap != NULL)
	{
		g_pOverviewMap->OnResizeSwapChain(uiLocalWidth, uiLocalHeight);
		unsigned int uiOverviewMapHeight = static_cast<unsigned int>(static_cast<float>(uiLocalHeight) * g_fOverviewMapSize / 100.0f);
		g_pOverviewMap->SetLocation(0, uiLocalHeight - uiOverviewMapHeight);
		g_pOverviewMap->SetHeight(uiOverviewMapHeight);
	}

	g_effect.m_pmProjectionVariable->SetMatrix((float*)&g_Settings.m_mProj);
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain *pSwapChain, const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext )
{
	V_RETURN( g_DialogResourceManager.OnD3D10ResizedSwapChain( pd3dDevice, pBufferSurfaceDesc ) );
	V_RETURN( g_D3DSettingsDlg.OnD3D10ResizedSwapChain( pd3dDevice, pBufferSurfaceDesc ) );

	UINT numViewports = 1;
	pd3dDevice->RSGetViewports(&numViewports, &g_viewport);

	g_uiWidth = pBufferSurfaceDesc->Width;
	g_uiHeight = pBufferSurfaceDesc->Height;

	g_pCamera->SetParameters(g_uiWidth, g_uiHeight, g_pSystemConfiguration->GetFovy());

	if (g_bRenderToBuffer)
	{
		ReleaseRenderBuffer();
		CreateRenderBuffer();
		OnResize(g_uiRenderBufferWidth, g_uiRenderBufferHeight);
	}
	else
	{
		OnResize(g_uiWidth, g_uiHeight);
	}


	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D10 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	if (fElapsedTime < 5.0f && g_pRecorder->GetState() == REC_STATE_IDLE) { g_fIdleTime += fElapsedTime; }

	g_bOnFrameRenderCalledSinceLastOnFrameMove = true;

	if (g_bBenchmarkBusTransfer)
	{
		return;
	}

	//
	// Clear the back buffer
	//
	ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
	pd3dDevice->ClearRenderTargetView( pRTV, g_fClearColor );
	if (g_bRenderToBuffer)
	{
		pd3dDevice->ClearRenderTargetView(g_pRenderBufferRTV, g_fClearColor);
	}

	//
	// Clear the depth stencil
	//
	ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
	pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );
	if (g_bRenderToBuffer)
	{
		pd3dDevice->ClearDepthStencilView(g_pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0, 0);
	}

	// If the settings dialog is being shown, then
	// render it instead of rendering the app's scene
	if( g_D3DSettingsDlg.IsActive() )
	{
		pd3dDevice->OMSetRenderTargets(1, &pRTV, pDSV);
		pd3dDevice->RSSetViewports(1, &g_viewport);
		g_D3DSettingsDlg.OnRender( fElapsedTime );
		return;
	}

	if (g_bRenderToBuffer)
	{
		pd3dDevice->OMSetRenderTargets(1, &g_pRenderBufferRTV, g_pDepthStencilView);
		D3D10_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = g_uiRenderBufferWidth;
		viewport.Height = g_uiRenderBufferHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		pd3dDevice->RSSetViewports(1, &viewport);
	}
	else
	{
		pd3dDevice->OMSetRenderTargets(1, &pRTV, pDSV);
		pd3dDevice->RSSetViewports(1, &g_viewport);
	}

	//
	// Update variables that change once per frame
	//
	static float fLastHeight = 0.0f;
	D3DXVECTOR3 vPosition;
	g_pCamera->GetPosition((float*) vPosition);
	if (fLastHeight <= 10000.0f && vPosition.z > 10000.0f || fLastHeight > 10000.0f && vPosition.z <= 10000.0f)
	{
		if (g_bRenderToBuffer)
		{
			OnResize(g_uiRenderBufferWidth, g_uiRenderBufferHeight);
		}
		else
		{
			OnResize(g_uiWidth, g_uiHeight);
		}
	}
	fLastHeight = vPosition.z;

	if (g_Settings.GetSatModify())
	{
		g_pTerrainInstance->GetTerrain().SetSaturation(g_Settings.GetSaturation());
	}

	if (g_Settings.GetWireframe()) {
		pd3dDevice->RSSetState(g_pRasterizerStateWireframe);
	}

	if (g_Settings.GetPageBoundingBoxes())
	{
		g_pTerrainInstance->GetDataLoader().Render();
	}

	g_pTerrainInstance->GetTerrain().Render(g_Settings.GetBoundingBoxes(), g_Settings.GetSatModify());

	if (g_Settings.GetStaticMesh())
	{
		// Kanten des View Frustums rendern
		D3DXMATRIX mView;
		g_pCamera->GetViewMatrix((float*) mView);
		g_effect.m_pmWorldViewVariable->SetMatrix((float*)mView);
		UINT stride = sizeof(D3DXVECTOR4);
		UINT offset = 0;
		pd3dDevice->IASetVertexBuffers(0, 1, &g_pFrustumVertexBuffer, &stride, &offset);
		pd3dDevice->IASetIndexBuffer(g_pFrustumIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		pd3dDevice->IASetInputLayout(g_effect.m_pInputLayoutFrustum);
		g_effect.m_pTechniqueFrustum->GetPassByIndex(0)->Apply(0);
		pd3dDevice->DrawIndexed(24, 0, 0);
	}

	if (g_Settings.GetWireframe()) {
		pd3dDevice->RSSetState(g_pRasterizerStateSolid);
	}

	if (g_bRenderToBuffer)
	{
		pd3dDevice->OMSetRenderTargets(1, &pRTV, pDSV);
		pd3dDevice->RSSetViewports(1, &g_viewport);

		g_effect.m_pSourceBitBltVariable->SetResource(g_pRenderBufferSRV);

		pd3dDevice->IASetInputLayout(NULL);
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_effect.m_pTechniqueBitBlt->GetPassByIndex(0)->Apply(0);
		pd3dDevice->Draw(3, 0);
	}

	if (g_Settings.m_bIsRemoteControl && g_pOverviewMap != NULL && g_bRenderOverviewMap)
	{
		g_pOverviewMap->Render();
	}

	// TUM-Logo rendern
	// Nummerierung der Knoten bei Tiled View:
	// 2 3
	// 0 1
	if (g_Settings.GetRenderLogo() &&
		(g_Settings.m_bIsRemoteControl || g_Settings.GetEyeDist() == 0.0f && g_Settings.m_iTileID == g_Settings.m_viTiles[0] - 1)
		)
	{
		D3D10_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));
		viewport.Width = 256;
		viewport.Height = 128;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = g_uiWidth - 256 - 20;
		viewport.TopLeftY = g_uiHeight - 128;
		pd3dDevice->RSSetViewports(1, &viewport);
		g_effect.m_pSourceBitBltVariable->SetResource(g_pTumLogoSRV);

		pd3dDevice->IASetInputLayout(NULL);
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_effect.m_pTechniqueBitBlt->GetPassByIndex(1)->Apply(0);
		pd3dDevice->Draw(3, 0);
	}

	pd3dDevice->RSSetViewports(1, &g_viewport);
	//
	// Render the UI
	//
	if (g_Settings.m_bIsRemoteControl)
	{
		g_HUD.OnRender( fElapsedTime );
		g_SampleUI.OnRender( fElapsedTime );

#ifndef _DEBUG
		if (!g_Settings.m_bLocalOnly)
#endif
		{
			g_ClusterUI.OnRender( fElapsedTime );
		}
	}

	if (g_bRenderHelp)
	{
		RenderHelp();
	}
	else if (g_Settings.GetRenderStatistics())
	{
		RenderStatistics();
	}

#ifdef WRITE_IMAGE_SEQUENCE
	if (g_uiImageSequenceNumber >= uiSTART_FRAME_NUM && g_pRecorder->GetState() == REC_STATE_PLAYING_BACK)
	{
		pd3dDevice->IASetInputLayout(NULL);
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_effect.m_pTechniqueBitBlt->GetPassByIndex(3)->Apply(0);
		pd3dDevice->Draw(3, 0);

		// Capture Screenshot
		char filename[MAX_STRING_SIZE];
		_stprintf_s(filename, MAX_STRING_SIZE, "%s\\image%05u.png", ptcWRITE_IMAGE_SEQUENCE_DIR, g_uiImageSequenceNumber);
		const D3DX10_IMAGE_FILE_FORMAT format = D3DX10_IFF_PNG;

		ID3D10Texture2D* pRT;
		if (g_bRenderToBuffer)
		{
			pRT = g_pRenderBuffer;
		}
		else
		{
			DXUTGetD3D10RenderTargetView()->GetResource(reinterpret_cast<ID3D10Resource**>(&pRT));
		}
		D3D10_TEXTURE2D_DESC texDesc;
		pRT->GetDesc(&texDesc);
		if (texDesc.SampleDesc.Count == 1)
		{
			HRESULT hr = D3DX10SaveTextureToFile(pRT, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
		}
		else
		{
			texDesc.Usage = D3D10_USAGE_DEFAULT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			ID3D10Texture2D* pTexture;
			HRESULT hr = pd3dDevice->CreateTexture2D(&texDesc, NULL, &pTexture);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
			pd3dDevice->ResolveSubresource(pTexture, 0, pRT, 0, texDesc.Format); // Copy a multisampled resource into a non-multisampled resource
			hr = D3DX10SaveTextureToFile(pTexture, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
			pd3dDevice->Flush();
			pTexture->Release();
		}
		if (!g_bRenderToBuffer)
		{
			pRT->Release();
		}

	}
	if (g_pRecorder->GetState() == REC_STATE_PLAYING_BACK)
	{
		g_uiImageSequenceNumber++;
	}
#endif

	if (g_bCaptureScreenshot)
	{
		pd3dDevice->IASetInputLayout(NULL);
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_effect.m_pTechniqueBitBlt->GetPassByIndex(3)->Apply(0);
		pd3dDevice->Draw(3, 0);

		// Capture Screenshot
		const char*const filename = "Screenshot.png";
		const D3DX10_IMAGE_FILE_FORMAT format = D3DX10_IFF_PNG;

		ID3D10Texture2D* pRT;
		if (g_bRenderToBuffer)
		{
			pRT = g_pRenderBuffer;
		}
		else
		{
			DXUTGetD3D10RenderTargetView()->GetResource(reinterpret_cast<ID3D10Resource**>(&pRT));
		}
		D3D10_TEXTURE2D_DESC texDesc;
		pRT->GetDesc(&texDesc);
		if (texDesc.SampleDesc.Count == 1)
		{
			HRESULT hr = D3DX10SaveTextureToFileA(pRT, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
		}
		else
		{
			texDesc.Usage = D3D10_USAGE_DEFAULT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			ID3D10Texture2D* pTexture;
			HRESULT hr = pd3dDevice->CreateTexture2D(&texDesc, NULL, &pTexture);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
			pd3dDevice->ResolveSubresource(pTexture, 0, pRT, 0, texDesc.Format); // Copy a multisampled resource into a non-multisampled resource
			hr = D3DX10SaveTextureToFileA(pTexture, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, NULL, TRUE);
				exit(-1);
			}
			pTexture->Release();
		}
		if (!g_bRenderToBuffer)
		{
			pRT->Release();
		}

		g_bCaptureScreenshot = false;
	}
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderStatistics()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos( 2, 0 );
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine( DXUTGetFrameStats(true) );
	g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

	WCHAR str[1024];
	D3DXVECTOR3 vPosition;
	float fHeading, fPitch, fRoll;
	g_pCamera->GetPosition((float*) vPosition);
	g_pCamera->GetAngles(&fHeading, &fPitch, &fRoll);
	swprintf_s(str, 1024, L"x=%.3f y=%.3f z=%.3f heading=%.2f pitch=%.2f roll=%.2f", vPosition.x, vPosition.y, vPosition.z, fHeading, fPitch, fRoll);
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"DATALOADER: Request Queue = %u Pages = %u Tiles = %u", g_pDataLoader->GetStatRequestQueueSize(), g_pDataLoader->GetStatNumPages(), g_pDataLoader->GetStatNumTiles());
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"System Memory Usage = %.1f MB", static_cast<float>(g_pDataLoader->GetSystemMemoryUsage()) / 1048576.0f);
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"TERRAIN: Visible Tiles = %u Paged In Tiles = %u Triangles = %.0f k", g_pTerrain->GetStatNumVisibleTiles(), g_pTerrain->GetStatNumPagedInTiles(), static_cast<float>(g_pTerrain->GetStatNumTriangles()) / 1000.0f);
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"Geo Size = %.2f MB Tex Size = %.2f MB", static_cast<float>(g_pTerrain->GetStatGeoSize()) / 1048576.0f, static_cast<float>(g_pTerrain->GetStatTexSize()) / 1048576.0f);
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"RESOURCEPOOL: VBs=%u VB Blocks=%u VB Block Mem=%.2f MB Textures=%u Tex Mem=%.2f MB",
		g_pResourcePool->GetStatNumVBsInUse(), g_pResourcePool->GetStatNumVBBlocksInUse(),
		static_cast<float>(g_pResourcePool->GetStatVBBlockMemoryUsage()) / 1048576.0f,
		g_pResourcePool->GetStatNumTexturesInUse(), static_cast<float>(g_pResourcePool->GetStatTextureMemoryUsage()) / 1048576.0f);
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"CAMERA: Speed = %.1f m/s", g_pCamera->GetKeyboardSpeed());
	g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"RECORDER: Time = %.2f s", g_pRecorder->GetTime());
	g_pTxtHelper->DrawTextLine(str);

	// TODO
	//swprintf_s(str, 1024, L"OCCLUSION CULLING: Num Horizon Nodes = %u Max Depth = %u", g_pOcclusionCulling->GetStatNumHorizonNodes(), g_pOcclusionCulling->GetStatMaxDepth());
	//g_pTxtHelper->DrawTextLine(str);

	swprintf_s(str, 1024, L"CLUSTER: Local Only = %s Remote Control = %s ID = %u", 
		g_Settings.m_bLocalOnly ? L"true" : L"false",
		g_Settings.m_bIsRemoteControl ? L"true" : L"false",
		g_Settings.m_iTileID);
	g_pTxtHelper->DrawTextLine(str);

	g_pTxtHelper->End();
}


void RenderHelp()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos( 2, 0 );
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"Controls (F1 to hide):");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"PROGRAM");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->DrawTextLine(L"  U :  Toggle UI");
	g_pTxtHelper->DrawTextLine(L"  F2 :  Device Settings");
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->DrawTextLine(L"  W :  Toggle Wireframe");
#ifndef DEMO
	g_pTxtHelper->DrawTextLine(L"  P :  Toggle Page Bounding Boxes");
#endif
	g_pTxtHelper->DrawTextLine(L"  B :  Toggle Tile Bounding Boxes");
#ifndef DEMO
	g_pTxtHelper->DrawTextLine(L"  S :  Toggle Static Mesh");
#endif
	g_pTxtHelper->DrawTextLine(L"  Q :  Toggle Statistics");
	g_pTxtHelper->DrawTextLine(L"  O :  Toggle Overview Map");
	g_pTxtHelper->DrawTextLine(L"");
#ifndef DEMO
	g_pTxtHelper->DrawTextLine(L"  D :  Capture Screenshot");
#endif
	g_pTxtHelper->DrawTextLine(L"  R :  Record Flight");
	g_pTxtHelper->DrawTextLine(L"  T :  Play Back Flight (Pause with SPACE)");
#ifndef DEMO
	g_pTxtHelper->DrawTextLine(L"  K :  Benchmark");
	g_pTxtHelper->DrawTextLine(L"  Y :  Benchmark Bus Transfer");
	g_pTxtHelper->DrawTextLine(L"  Z :  Upload All Data in Every Frame");
#endif
	g_pTxtHelper->DrawTextLine(L"  J :  Detect Joystick");
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"CAMERA");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"  Mouse");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"    Left Mouse Button :  View Direction");
	g_pTxtHelper->DrawTextLine(L"    Right Mouse Button :  Picking");
	g_pTxtHelper->DrawTextLine(L"    Mouse Wheel :  Flight Speed (Keyboard only)");
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"  Keyboard");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"    Arrow Keys :  Move Forward / Backward / Left / Right");
	g_pTxtHelper->DrawTextLine(L"    Page Up / Down :  Move Up / Down");
	g_pTxtHelper->DrawTextLine(L"    Arrow Keys (Numeric) :  Move Forward / Backward /");
	g_pTxtHelper->DrawTextLine(L"                          Left / Right (Keep Height)");
	g_pTxtHelper->DrawTextLine(L"    Page Up / Down (Numeric) :  Move Up / Down");
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->DrawTextLine(L"    N :  Set Heading to North");
	g_pTxtHelper->DrawTextLine(L"    2 :  View From Above, Set Heading to North");
	g_pTxtHelper->DrawTextLine(L"    X :  Reset Camera Position");
	g_pTxtHelper->DrawTextLine(L"");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"  Joystick");
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	g_pTxtHelper->DrawTextLine(L"    X-Axis :  Heading");
	g_pTxtHelper->DrawTextLine(L"    Y-Axis :  Pitch");
	g_pTxtHelper->DrawTextLine(L"    Slider :  Flight Speed (Joystick only)");
	g_pTxtHelper->DrawTextLine(L"    Button 1 :  Accelerate, Move Forward");
	g_pTxtHelper->DrawTextLine(L"    Button 2 :  Slow Down");
	g_pTxtHelper->DrawTextLine(L"    Button 3 :  Move Up");
	g_pTxtHelper->DrawTextLine(L"    Button 4 :  Move Down");
	g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext )
{
	g_DialogResourceManager.OnD3D10ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
	g_pTerrainInstance->SafeRelease();

	SAFE_RELEASE(g_pTumLogoSRV);
	SAFE_RELEASE(g_pTumLogoTexture);

	SAFE_RELEASE(g_pFrustumIndexBuffer);
	SAFE_RELEASE(g_pFrustumVertexBuffer);

	if (g_bRenderToBuffer)
	{
		ReleaseRenderBuffer();
	}

	if (g_pOverviewMap != NULL)
	{
		g_pOverviewMap->OnDestroyDevice();
	}

	SAFE_RELEASE(g_pRasterizerStateSolid);
	SAFE_RELEASE(g_pRasterizerStateWireframe);

	g_effect.SafeRelease();

	g_DialogResourceManager.OnD3D10DestroyDevice();
	g_D3DSettingsDlg.OnD3D10DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	SAFE_RELEASE( g_pFont );
	SAFE_RELEASE( g_pSprite );
	SAFE_DELETE( g_pTxtHelper );

}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	static bool bFirstTime = true;
	// Default-Einstellung von VSync auf off
	if (bFirstTime)
	{
#ifdef PERFHUD
		// Achtung: In DXUT.cpp muss die Zeile
		//      if( pNewDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_HARDWARE )
		// in DXUTCreate3DEnvironment10 auskommentiert werden!

		IDXGIFactory* pDXGIFactory = DXUTGetDXGIFactory();
		UINT nAdapter = 0;
		IDXGIAdapter* pAdapter = NULL;
		bool bIsPerfHUD = false;

		while (pDXGIFactory->EnumAdapters(nAdapter, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			if (pAdapter)
			{
				DXGI_ADAPTER_DESC adaptDesc;
				if (SUCCEEDED(pAdapter->GetDesc(&adaptDesc)))
				{
					bIsPerfHUD = wcscmp(adaptDesc.Description, L"NVIDIA PerfHUD") == 0;
					if (bIsPerfHUD)
					{
						break;
					}
				}
			}
			nAdapter++;
		}

		if (bIsPerfHUD)
		{
			pDeviceSettings->d3d10.AdapterOrdinal = nAdapter;
			pDeviceSettings->d3d10.DriverType = D3D10_DRIVER_TYPE_REFERENCE;
		}
#endif

		pDeviceSettings->d3d10.SyncInterval = 0;
		//pDeviceSettings->d3d10.sd.SampleDesc.Count = 8;
		//pDeviceSettings->d3d10.sd.SampleDesc.Quality = 16;

		bFirstTime = false;
	}

	if (g_Settings.m_bIsRemoteControl)
	{
		CCDeviceSettings ccDeviceSettings;
		ccDeviceSettings.uiSyncInterval = pDeviceSettings->d3d10.SyncInterval;
		ccDeviceSettings.bWindowed = pDeviceSettings->d3d10.sd.Windowed;
		ccDeviceSettings.uiSampleCount = pDeviceSettings->d3d10.sd.SampleDesc.Count;
		ccDeviceSettings.uiSampleQuality = pDeviceSettings->d3d10.sd.SampleDesc.Quality;
		// TODO: are these device settings used anywhere? if not, remove [Andreas Kirsch]
		g_Settings.SetDeviceSettings(ccDeviceSettings);
	}

	return true;
}

/*
void GoWindow() {    
	if (!DXUTGetDeviceSettings().d3d10.sd.Windowed) DXUTToggleFullScreen();
}

void EndWindow() {
	try {
		if (DXUTGetDeviceSettings().d3d10.sd.Windowed) DXUTToggleFullScreen();
	} catch (...) {
		// if running on a cluster this call sometimes fails, avoid crash by having a try/catch ready
	}
}

void ToggleClusterFullScreen() {
	NetworkStruct message;
	message.type = (DXUTGetDeviceSettings().d3d10.sd.Windowed) ?  GOFULLSCREEN : GOWINDOW;
	g_Settings.SendMessage(message,false);
}
*/

D3DXVECTOR3 g_vPositionBackup;
float g_fHeadingBackup, g_fPitchBackup, g_fRollBackup;


void StartDemo()
{
	assert(!g_bDemo);

	g_bDemo = true;

	g_pCamera->GetPosition((float*) g_vPositionBackup);
	g_pCamera->GetAngles(&g_fHeadingBackup, &g_fPitchBackup, &g_fRollBackup);

    g_pRecorder->SetFileName(g_pTerrainInstance->GetDemoFlightFileName());
	g_pRecorder->StartPlayingBack(true, false);
}


void OnActivity()
{
	if (g_bDemo)
	{
		g_pRecorder->StopPlayingBack();

		if (g_bRestorePosition)
		{
			g_pCamera->SetPosition((const float*) g_vPositionBackup);
			g_pCamera->SetAngles(g_fHeadingBackup, g_fPitchBackup, g_fRollBackup);
		}
		
		g_bDemo = false;
	}
	g_fIdleTime = 0.0;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	//outdbg(L"%f\n", g_fIdleTime);
	static double s_dStatLastTime = fTime;
	static RECORDER_STATE eRecLastState = REC_STATE_IDLE;
	static char str1[MAX_STRING_SIZE];
	static char str2[MAX_STRING_SIZE];
	
	bool bUpdateTitle = false;

	static unsigned __int64		s_ui64StatLastNumTrianglesRendered = 0;
	static unsigned __int64		s_ui64StatLastDiskNumBytesTransferred = 0;
	static unsigned __int64		s_ui64StatLastNumTilesTransferred = 0;
	static unsigned __int64		s_ui64StatLastNumBytesTransferred = 0;
	static unsigned __int64		s_ui64StatLastNumBytesTransferredDecompressed = 0;

	double dt = fTime - s_dStatLastTime;
	if (dt >= 1.0f)
	{
#ifndef DEMO
		sprintf_s(str1, 256, "Terrain3D @ %.2f fps (%.2f MTri/s, Disk %.2f MB/s, Bus %.2f Tiles/s %.2f MB/s [%.2f MB/s])     Press F1 for help",
			DXUTGetFPS(),
			(static_cast<double>(g_pTerrain->GetStatNumTrianglesRendered() - s_ui64StatLastNumTrianglesRendered) / (1000.0 * 1000.0)) / dt,
			static_cast<double>(g_pDataLoader->GetStatNumBytesTransferred() - s_ui64StatLastDiskNumBytesTransferred) / (1024.0 * 1024.0) / dt,
			static_cast<double>(g_pTerrain->GetStatNumTilesTransferred() - s_ui64StatLastNumTilesTransferred) /  dt,
			static_cast<double>(g_pTerrain->GetStatNumBytesTransferred() - s_ui64StatLastNumBytesTransferred) / (1024.0 * 1024.0) / dt,
			static_cast<double>(g_pTerrain->GetStatNumBytesTransferredDecompressed() - s_ui64StatLastNumBytesTransferredDecompressed) / (1024.0 * 1024.0) / dt
			);
#else
		sprintf_s(str1, 256, "Terrain3D @ %.2f fps (%.2f MTri/s, Disk %.2f MB/s)     Press F1 for help",
			DXUTGetFPS(),
			(static_cast<double>(g_pTerrain->GetStatNumTrianglesRendered() - s_ui64StatLastNumTrianglesRendered) / (1000.0 * 1000.0)) / dt,
			static_cast<double>(g_pDataLoader->GetStatNumBytesTransferred() - s_ui64StatLastDiskNumBytesTransferred) / (1024.0 * 1024.0) / dt
			);
#endif

		s_ui64StatLastNumTrianglesRendered = g_pTerrain->GetStatNumTrianglesRendered();
		s_ui64StatLastDiskNumBytesTransferred = g_pDataLoader->GetStatNumBytesTransferred();
		s_ui64StatLastNumTilesTransferred = g_pTerrain->GetStatNumTilesTransferred();
		s_ui64StatLastNumBytesTransferred = g_pTerrain->GetStatNumBytesTransferred();
		s_ui64StatLastNumBytesTransferredDecompressed = g_pTerrain->GetStatNumBytesTransferredDecompressed();

		s_dStatLastTime = fTime;
		bUpdateTitle = true;
	}


	RECORDER_STATE eRecState = g_pRecorder->GetState();
	if (eRecState != eRecLastState)
	{
		switch (eRecState)
		{
			case REC_STATE_IDLE:
			{
				strcpy_s(str2, 256, "");
				break;
			}
			case REC_STATE_RECORDING:
			{
				strcpy_s(str2, 256, "R E C O R D I N G");
				break;
			}
			case REC_STATE_PLAYING_BACK_PAUSED:
			{
				strcpy_s(str2, 256, "P L A Y I N G   B A C K   P A U S E D   (Press Space to continue)");
				break;
			}
			case REC_STATE_PLAYING_BACK:
			{
				strcpy_s(str2, 256, "P L A Y I N G   B A C K");
				break;
			}
		}
		eRecLastState = eRecState;
		bUpdateTitle = true;
	}
	
	if (DXUTIsWindowed() && bUpdateTitle)
	{
		char str[MAX_STRING_SIZE];
		sprintf_s(str, MAX_STRING_SIZE, "%s          %s", str1, str2);
		SetWindowTextA(DXUTGetHWND(), str);
	}

	//***** Benchmarking *****

	struct BenchmarkEntryFPS
	{
		double			m_dTime;
		float			m_fFPS;
		float			m_fNumTrianglesRenderedPerFrame;
		float			m_fNumTilesRenderedPerFrame;
		float			m_fCameraSpeed;
		unsigned int	m_uiGeoSize;
		unsigned int	m_uiTexSize;
	};

	struct BenchmarkEntryDisk
	{
		double			m_dTime;
		float			m_fDiskTransferRate;
	};

	static std::vector<BenchmarkEntryFPS>		s_benchmarkDataFPS;
	static std::vector<BenchmarkEntryDisk>		s_benchmarkDataDisk;

	static double s_dBenchmarkStartTime;
	static double s_dBenchmarkLastTimeFPS;
	static double s_dBenchmarkLastTimeDisk;

	static unsigned __int64 s_ui64BenchmarkNumFrames;
	static unsigned __int64 s_ui64BenchmarkLastNumFrames;
	static unsigned __int64 s_ui64BenchmarkLastNumTrianglesRendered;
	static unsigned __int64 s_ui64BenchmarkLastNumTilesRendered;
	static unsigned __int64 s_ui64BenchmarkLastDiskNumBytesTransferred;
	static double s_dBenchmarkLastDistanceCovered;

	if (g_uiBenchmarkState == 2) // Initialisiere Benchmarking
	{
		if (eRecState == REC_STATE_IDLE)
		{
			g_uiBenchmarkState = 0;
		}

		s_dBenchmarkStartTime = fTime;
		s_dBenchmarkLastTimeFPS = fTime;
		s_dBenchmarkLastTimeDisk = fTime;

		s_benchmarkDataFPS.clear();
		s_benchmarkDataDisk.clear();

		s_ui64BenchmarkNumFrames = 0;
		s_ui64BenchmarkLastNumFrames = 0;
		s_ui64BenchmarkLastNumTrianglesRendered = g_pTerrain->GetStatNumTrianglesRendered();
		s_ui64BenchmarkLastNumTilesRendered = g_pTerrain->GetStatNumTilesRendered();
		s_ui64BenchmarkLastDiskNumBytesTransferred = g_pDataLoader->GetStatNumBytesTransferred();
		s_dBenchmarkLastDistanceCovered = g_pCamera->GetStatCoveredDistance();

		g_uiBenchmarkState = 3;
	}
	else if (g_uiBenchmarkState == 3) // Benchmarking
	{
		s_ui64BenchmarkNumFrames++;

		if (eRecState == REC_STATE_IDLE)
		{
			// Benchmark-Daten schreiben
			g_uiBenchmarkState = 0;
			FILE* hFile;
			errno_t err = fopen_s(&hFile, ptcBENCHMARK_OUTPUT_FILE, "wt");
			if (err == 0)
			{
				for (unsigned int ui = 0; ui < s_benchmarkDataFPS.size(); ui++)
				{
					fprintf_s(hFile, "%f %f %f %f %f %u %u\n",
						s_benchmarkDataFPS[ui].m_dTime,
						s_benchmarkDataFPS[ui].m_fFPS,
						s_benchmarkDataFPS[ui].m_fNumTrianglesRenderedPerFrame,
						s_benchmarkDataFPS[ui].m_fNumTilesRenderedPerFrame,
						s_benchmarkDataFPS[ui].m_fCameraSpeed,
						s_benchmarkDataFPS[ui].m_uiGeoSize,
						s_benchmarkDataFPS[ui].m_uiTexSize
						);
				}

				fprintf_s(hFile, "\n\n");

				for (unsigned int ui = 0; ui < s_benchmarkDataDisk.size(); ui++)
				{
					fprintf_s(hFile, "%f %f\n",
						s_benchmarkDataDisk[ui].m_dTime,
						s_benchmarkDataDisk[ui].m_fDiskTransferRate
						);
				}

				fclose(hFile);

			}

			s_benchmarkDataFPS.clear();
			s_benchmarkDataDisk.clear();
		}
		else
		{
			double dt = fTime - s_dBenchmarkLastTimeFPS;
			if (dt >= 0.20f)
			{
				BenchmarkEntryFPS entry;
				entry.m_dTime = fTime - s_dBenchmarkStartTime;
				entry.m_fFPS = static_cast<float>(static_cast<double>(s_ui64BenchmarkNumFrames - s_ui64BenchmarkLastNumFrames) / dt);
				entry.m_fNumTrianglesRenderedPerFrame = static_cast<float>(static_cast<double>(g_pTerrain->GetStatNumTrianglesRendered() - s_ui64BenchmarkLastNumTrianglesRendered) / static_cast<double>(s_ui64BenchmarkNumFrames - s_ui64BenchmarkLastNumFrames));
				entry.m_fNumTilesRenderedPerFrame = static_cast<float>(static_cast<double>(g_pTerrain->GetStatNumTilesRendered() - s_ui64BenchmarkLastNumTilesRendered) / static_cast<double>(s_ui64BenchmarkNumFrames - s_ui64BenchmarkLastNumFrames));
				entry.m_fCameraSpeed = static_cast<float>((g_pCamera->GetStatCoveredDistance() - s_dBenchmarkLastDistanceCovered) / dt);
				entry.m_uiGeoSize = g_pTerrain->GetStatGeoSize();
				entry.m_uiTexSize = g_pTerrain->GetStatTexSize();

				s_benchmarkDataFPS.push_back(entry);

				s_ui64BenchmarkLastNumFrames = s_ui64BenchmarkNumFrames;
				s_ui64BenchmarkLastNumTrianglesRendered = g_pTerrain->GetStatNumTrianglesRendered();
				s_ui64BenchmarkLastNumTilesRendered = g_pTerrain->GetStatNumTilesRendered();
				s_dBenchmarkLastDistanceCovered = g_pCamera->GetStatCoveredDistance();

				s_dBenchmarkLastTimeFPS = fTime;
			}

			dt = fTime - s_dBenchmarkLastTimeDisk;
			if (dt >= 1.0f)
			{
				BenchmarkEntryDisk entry;
				entry.m_dTime = fTime - s_dBenchmarkStartTime;
				entry.m_fDiskTransferRate = static_cast<float>(static_cast<double>(g_pDataLoader->GetStatNumBytesTransferred() - s_ui64BenchmarkLastDiskNumBytesTransferred) / dt);

				s_benchmarkDataDisk.push_back(entry);
				
				s_ui64BenchmarkLastDiskNumBytesTransferred = g_pDataLoader->GetStatNumBytesTransferred();	

				s_dBenchmarkLastTimeDisk = fTime;
			}

		}
	}

	//*****

	if (g_Settings.m_bIsRemoteControl)
	{
		RECORDER_STATE eRecState = g_pRecorder->GetState();

		if (eRecState == REC_STATE_IDLE)
		{
			D3DXVECTOR3 vPosition;
			float fHeading, fPitch, fRoll;
			g_pCamera->GetPosition((float*) vPosition);
			g_pCamera->GetAngles(&fHeading, &fPitch, &fRoll);

			g_pCamera->Update(fElapsedTime);

			D3DXVECTOR3 vNewPosition;
			float fNewHeading, fNewPitch, fNewRoll;
			g_pCamera->GetPosition((float*) vNewPosition);
			g_pCamera->GetAngles(&fNewHeading, &fNewPitch, &fNewRoll);
			if (vNewPosition != vPosition || fNewHeading != fHeading || fNewPitch != fPitch || fNewRoll != fRoll)
			{
				OnActivity();
			}

		}
		else if (eRecState == REC_STATE_RECORDING)
		{
			g_pCamera->Update(fElapsedTime);
		}
#ifdef WRITE_IMAGE_SEQUENCE
		if (eRecState == REC_STATE_RECORDING)
		{
			g_pRecorder->Update(fElapsedTime);
		}
		else if (eRecState == REC_STATE_PLAYING_BACK)
		{
			if (g_bOnFrameRenderCalledSinceLastOnFrameMove) { g_pRecorder->Update(1.0/static_cast<double>(uiTARGET_FRAME_RATE)); }
		}
#else
		if (eRecState == REC_STATE_RECORDING || eRecState == REC_STATE_PLAYING_BACK)
		{
			g_pRecorder->Update(fElapsedTime);
		}

		if (g_bDemoMode && g_pTerrainInstance->GetDemoFlightFileName() != NULL && !g_bDemo && g_fIdleTime > static_cast<float>(g_iIdleTime) && eRecState == REC_STATE_IDLE)
		{
			StartDemo();
		}
#endif
		if (g_bBenchmarkBusTransfer || g_bBenchmarkStreaming)
		{
			g_pTerrain->PageOutAllAndFlushCache();
		}

		g_Settings.OnCameraUpdate();

		
		if (g_bRenderUI)
		{
			g_HUD.SetLocation( g_uiWidth - (g_HUD.GetWidth() + 10), 10 );

			if (g_SampleUI.GetMinimized())
			{
				g_SampleUI.SetLocation( g_uiWidth - (g_SampleUI.GetWidth() + 10), g_uiHeight - 30 );
			}
			else
			{
				g_SampleUI.SetLocation( g_uiWidth - (g_SampleUI.GetWidth() + 10), g_uiHeight - (g_SampleUI.GetHeight() + 10) );
			}

#ifndef _DEBUG
			if (!g_Settings.m_bLocalOnly)
#endif
			{
				if (g_ClusterUI.GetMinimized())
				{
					g_ClusterUI.SetLocation( (g_uiWidth - g_ClusterUI.GetWidth()) / 2, g_uiHeight - 30 );
				}
				else
				{
					g_ClusterUI.SetLocation( (g_uiWidth - g_ClusterUI.GetWidth()) / 2, g_uiHeight - (g_ClusterUI.GetHeight() + 10) );
				}
			}
		}
		else
		{
			g_HUD.SetLocation( 20000, 20000 );
			g_SampleUI.SetLocation( 20000, 20000 );
#ifndef _DEBUG
			if (!g_Settings.m_bLocalOnly)
#endif
			{
				g_ClusterUI.SetLocation( 20000, 20000 );
			}
		}

	}

	if (!g_Settings.GetStaticMesh())
	{
		DataLoader::ViewInfo viewInfo;
		viewInfo.viewIndex = 0;

		g_pCamera->GetPosition(viewInfo.position);
		g_pCamera->GetCameraViewOrthogonalFrame(viewInfo.viewDirection, NULL, NULL);

#ifdef WRITE_IMAGE_SEQUENCE
		g_pDataLoader->Update(vPosition, vViewDirection, (g_uiImageSequenceNumber >= uiSTART_FRAME_NUM && g_pRecorder->GetState() == REC_STATE_PLAYING_BACK));
#else
		DataLoader::ViewInfoVector viewInfoVector;
		viewInfoVector.push_back(viewInfo);
		g_pDataLoader->Update(viewInfoVector);
#endif
		g_pTerrain->Update(g_Settings.m_mView, viewInfo.position, viewInfo.viewDirection, g_Settings.GetOcclusionCulling());

	}
	else
	{
		g_pTerrain->UpdateViewOnly(g_Settings.m_mView);
	}

	if (g_Settings.m_bDeviceSettingsChanged)
	{
		g_Settings.m_bDeviceSettingsChanged = false;
		g_bDeviceSettingsChanged = true;
		g_ccDeviceSettings = g_Settings.GetDeviceSettings();
		PostMessage(DXUTGetHWND(), WM_NULL, 0, 0); // Sicherstellen, dass MsgProc aufgerufen wird
	}

	if (g_Settings.m_bCloseApp) { SendMessage( DXUTGetHWND(), WM_CLOSE, 0, 0 ); }
/*
	if (g_Settings.m_bGoWindow) { GoWindow(); g_Settings.m_bGoWindow = false; }
	if (g_Settings.m_bGoFullscreen) { EndWindow(); g_Settings.m_bGoFullscreen = false; }
*/

	g_bOnFrameRenderCalledSinceLastOnFrameMove = false;
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	if (!g_Settings.m_bIsRemoteControl)
	{
		if (g_bDeviceSettingsChanged)
		{
			g_bDeviceSettingsChanged = false;
			DXUTDeviceSettings dxutDeviceSettings = DXUTGetDeviceSettings();
			if (g_ccDeviceSettings.uiSyncInterval != dxutDeviceSettings.d3d10.SyncInterval
				|| g_ccDeviceSettings.uiSampleCount != dxutDeviceSettings.d3d10.sd.SampleDesc.Count
				|| g_ccDeviceSettings.uiSampleQuality != dxutDeviceSettings.d3d10.sd.SampleDesc.Quality)
			{
				dxutDeviceSettings.d3d10.SyncInterval = g_ccDeviceSettings.uiSyncInterval;
				dxutDeviceSettings.d3d10.sd.SampleDesc.Count = g_ccDeviceSettings.uiSampleCount;
				dxutDeviceSettings.d3d10.sd.SampleDesc.Quality = g_ccDeviceSettings.uiSampleQuality;

				DXUTCreateDeviceFromSettings(&dxutDeviceSettings); // Aufruf innerhalb von DXUTRender3DEnvironment10 (d.h. innerhalb von OnFrameMove oder OnFrameRender) schlägt fehl
			}
			if (g_ccDeviceSettings.bWindowed != dxutDeviceSettings.d3d10.sd.Windowed)
			{
				DXUTToggleFullScreen();
			}

		}

	}

	if (!g_Settings.m_bIsRemoteControl) { return 0; }

	switch (uMsg)
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		//case WM_MOUSEMOVE: // Es kommen solche Messages an, obwohl die Maus nicht bewegt wird (trotzdem hat das einmal funktioniert)
		case WM_MOUSEWHEEL:
			OnActivity();
			break;
	}

	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Pass messages to settings dialog if its active
	if( g_D3DSettingsDlg.IsActive() )
	{
		g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
		return 0;
	}

	if (g_bToggleSettingsDlg)
	{
		g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() );
		g_bToggleSettingsDlg = false;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
#ifndef _DEBUG
	if (!g_Settings.m_bLocalOnly)
#endif
	{
		*pbNoFurtherProcessing = g_ClusterUI.MsgProc( hWnd, uMsg, wParam, lParam );
		if( *pbNoFurtherProcessing )
			return 0;
	}

	if (g_pOverviewMap != NULL && g_bRenderOverviewMap)
	{
		*pbNoFurtherProcessing = g_pOverviewMap->MsgProc(hWnd, uMsg, wParam, lParam);
		if (*pbNoFurtherProcessing)
		{
			return 0;
		}
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	if (g_pCamera)
	{
		g_pCamera->HandleMessages(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


BOOL OpenFileNameDialog(char* ptcFileName, DWORD dwMaxLength, const char* ptcFilter, DWORD dwFilterIndex)
{
	bool bToggleFullScreen = !DXUTIsWindowed();
	if (bToggleFullScreen)
	{
		DXUTToggleFullScreen();
	}

	OPENFILENAMEA openFileName;
	ZeroMemory(&openFileName, sizeof(OPENFILENAME));
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = DXUTGetHWND();
	openFileName.lpstrFilter = ptcFilter;
	openFileName.nFilterIndex = dwFilterIndex;
	openFileName.lpstrFile = ptcFileName;
	ptcFileName[0] = 0;
	openFileName.nMaxFile = dwMaxLength;
	openFileName.Flags = OFN_FILEMUSTEXIST;
	BOOL bResult = GetOpenFileNameA(&openFileName);

	if (bToggleFullScreen)
	{
		DXUTToggleFullScreen();
	}

	return bResult;

}


BOOL SaveFileNameDialog(char* ptcFileName, DWORD dwMaxLength, const char* ptcFilter, DWORD dwFilterIndex)
{
	bool bToggleFullScreen = !DXUTIsWindowed();
	if (bToggleFullScreen)
	{
		DXUTToggleFullScreen();
	}

	OPENFILENAMEA openFileName;
	ZeroMemory(&openFileName, sizeof(OPENFILENAME));
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = DXUTGetHWND();
	openFileName.lpstrFilter = ptcFilter;
	openFileName.nFilterIndex = dwFilterIndex;
	openFileName.lpstrFile = ptcFileName;
	ptcFileName[0] = 0;
	openFileName.nMaxFile = dwMaxLength;
	BOOL bResult = GetSaveFileNameA(&openFileName);

	if (bToggleFullScreen)
	{
		DXUTToggleFullScreen();
	}

	return bResult;

}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (!g_Settings.m_bIsRemoteControl) { return; }

	if (bKeyDown)
	{
		if (!g_D3DSettingsDlg.IsActive()) // Auf Tastatureingaben hier nur reagieren, wenn der Settings Dialog nicht angezeigt wird
		{
			switch (nChar)
			{
				case VK_F2:
				{
					g_bToggleSettingsDlg = true; // Hier nur Flag setzen, dass Dialog angezeigt werden soll
					break;
				}
				case 'W':
				{
					g_Settings.SetWireframe(!g_Settings.GetWireframe());
					g_SampleUI.GetCheckBox(IDC_TOGGLEWIREFRAME)->SetChecked(g_Settings.GetWireframe());
					break;
				}
#ifndef DEMO
				case 'P':
				{
					g_Settings.SetPageBoundingBoxes(!g_Settings.GetPageBoundingBoxes());
					g_SampleUI.GetCheckBox(IDC_TOGGLEPAGEBOUNDINGBOXES)->SetChecked(g_Settings.GetPageBoundingBoxes());
					break;
				}
#endif
				case 'B':
				{
					g_Settings.SetBoundingBoxes(!g_Settings.GetBoundingBoxes());
					g_SampleUI.GetCheckBox(IDC_TOGGLEBOUNDINGBOXES)->SetChecked(g_Settings.GetBoundingBoxes());
					break;
				}
#ifndef DEMO				
				case 'S':
				{
					if (!g_Settings.GetStaticMesh())
					{
						D3DXMATRIX m;
						D3DXMATRIX mProjection = (D3DXMATRIX)g_Settings.m_mProj;
						g_pCamera->GetViewMatrix((float *) m);
						D3DXMatrixMultiply(&m, &m, &static_cast<D3DXMATRIX>(g_Settings.m_mProj));
						D3DXMatrixInverse(&m, NULL, &m);
						D3DXVECTOR4 verticesWS[8];
						const D3DXVECTOR4 vertices[8] = {
							D3DXVECTOR4(-1.0f, -1.0f, 0.0f, 1.0f),
							D3DXVECTOR4(-1.0f, 1.0f, 0.0f, 1.0f),
							D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f),
							D3DXVECTOR4(1.0f, -1.0f, 0.0f, 1.0f),
							D3DXVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f),
							D3DXVECTOR4(-1.0f, 1.0f, 1.0f, 1.0f),
							D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f),
							D3DXVECTOR4(1.0f, -1.0f, 1.0f, 1.0f)
						};
						D3DXVec4TransformArray(verticesWS, sizeof(D3DXVECTOR4), vertices, sizeof(D3DXVECTOR4), &m, 8);
						DXUTGetD3D10Device()->UpdateSubresource(g_pFrustumVertexBuffer, 0, NULL, verticesWS, 0, 0);
					}
					g_Settings.SetStaticMesh(!g_Settings.GetStaticMesh());
					g_SampleUI.GetCheckBox(IDC_TOGGLESTATICMESH)->SetChecked(g_Settings.GetStaticMesh());
					break;
				}
#endif
				case 'O':
				{
					g_bRenderOverviewMap = !g_bRenderOverviewMap;
					g_SampleUI.GetCheckBox(IDC_TOGGLERENDEROVERVIEWMAP)->SetChecked(g_bRenderOverviewMap);
					break;
				}
				case 'U':
				{
					g_bRenderUI = !g_bRenderUI;
					g_HUD.SetVisible(g_bRenderUI);
					g_SampleUI.SetVisible(g_bRenderUI);
					g_ClusterUI.SetVisible(g_bRenderUI);
					break;
				}
				case 'R':
				{
					RECORDER_STATE eRecState = g_pRecorder->GetState();
					if (eRecState == REC_STATE_IDLE)
					{
						char atcFileName[MAX_STRING_SIZE];
						//_tcscpy_s(atcFileName, MAX_STRING_SIZE, "benchmark.rec");
						BOOL bResult = SaveFileNameDialog(atcFileName, MAX_STRING_SIZE, "All Files (*.*)\0*.*\0Recorded Flights (*.rec)\0*.rec\0", 2);
						if (bResult)
						{
							g_pRecorder->SetFileName(atcFileName);
							g_pRecorder->StartRecording();
						}
					}
					else if (eRecState == REC_STATE_RECORDING)
					{
						g_pRecorder->StopRecording();
					}
					break;
				}
#ifndef DEMO
				case 'K':
				{
					if (g_uiBenchmarkState == 0)
					{
						g_uiBenchmarkState = 1;
					}
					// Fall through
				}
#endif
				case 'T':
				{
					RECORDER_STATE eRecState = g_pRecorder->GetState();
					if (eRecState == REC_STATE_IDLE)
					{
						char atcFileName[MAX_STRING_SIZE];
						//_tcscpy_s(atcFileName, MAX_STRING_SIZE, "1.rec"); BOOL bResult = TRUE;
						BOOL bResult = OpenFileNameDialog(atcFileName, MAX_STRING_SIZE, "All Files (*.*)\0*.*\0Recorded Flights (*.rec)\0*.rec\0", 2);
						if (bResult)
						{
							g_pRecorder->SetFileName(atcFileName);
							g_pRecorder->StartPlayingBack();
							g_uiImageSequenceNumber = 0;
						}
						else
						{
							g_uiBenchmarkState = 0;
						}
					}
					else if (eRecState == REC_STATE_PLAYING_BACK || eRecState == REC_STATE_PLAYING_BACK_PAUSED)
					{
						g_pRecorder->StopPlayingBack();
					}
					break;
				}
				case VK_SPACE:
				{
					RECORDER_STATE eRecState = g_pRecorder->GetState();
					if (eRecState == REC_STATE_PLAYING_BACK_PAUSED)
					{
						g_pRecorder->ContinuePlayingBack();
						if (g_uiBenchmarkState == 1)
						{
							g_uiBenchmarkState = 2;
						}
					}
					else if (eRecState == REC_STATE_PLAYING_BACK)
					{
						g_pRecorder->PausePlayingBack();
					}
					break;
				}
				case 'N':
				{
					float fHeading, fPitch, fRoll;
					g_pCamera->GetAngles(NULL, &fPitch, &fRoll);
					fHeading = 270.0f;
					g_pCamera->SetAngles(fHeading, fPitch, fRoll);
					break;
				}
				case '2':
				{
					float fHeading, fPitch, fRoll;
					fHeading = 270.0f;
					fPitch = 90.0f;
					fRoll = 0.0f;
					g_pCamera->SetAngles(fHeading, fPitch, fRoll);
					break;
				}
				case VK_F1:
				{
					g_bRenderHelp = !g_bRenderHelp;
					break;
				}
#ifndef DEMO
				case 'D':
				{
					g_bCaptureScreenshot = true;
					break;
				}
#endif
				case 'X':
				{
					// Start-Position und Start-Winkel setzen
					g_pCamera->Reset();
					break;
				}
				case 'Q':
				{
					g_Settings.SetRenderStatistics(!g_Settings.GetRenderStatistics());
					g_SampleUI.GetCheckBox(IDC_TOGGLERENDERSTATISTICS)->SetChecked(g_Settings.GetRenderStatistics());
					break;
				}
#ifndef DEMO
				case 'Y':
				{
					g_bBenchmarkBusTransfer = !g_bBenchmarkBusTransfer;
					g_bBenchmarkStreaming = false;
					break;
				}
				case 'Z':
				{
					g_bBenchmarkStreaming = !g_bBenchmarkStreaming;
					g_bBenchmarkBusTransfer = false;
					break;
				}
#endif
			} // switch (nChar)
		}

	} // if (bKeyDown)
}


void CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, INT nMouseWheelDelta, INT xPos, INT yPos, void* pUserContext)
{
}



//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	wchar_t str[MAX_STRING_SIZE];

	switch( nControlID )
	{
		case IDC_TOGGLEFULLSCREEN:
			//if (!g_Settings.m_bLocalOnly) { ToggleClusterFullScreen(); }
			DXUTToggleFullScreen();
			break;
		case IDC_CHANGEDEVICE:
			g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() );
			break;
		case IDC_TOGGLEWIREFRAME			: g_Settings.SetWireframe( g_SampleUI.GetCheckBox(IDC_TOGGLEWIREFRAME)->GetChecked() ); break;
		case IDC_TOGGLEPAGEBOUNDINGBOXES	: g_Settings.SetPageBoundingBoxes( g_SampleUI.GetCheckBox(IDC_TOGGLEPAGEBOUNDINGBOXES)->GetChecked() ); break;
		case IDC_TOGGLEBOUNDINGBOXES		: g_Settings.SetBoundingBoxes( g_SampleUI.GetCheckBox(IDC_TOGGLEBOUNDINGBOXES)->GetChecked() ); break;
		case IDC_TOGGLESTATICMESH			: g_Settings.SetStaticMesh( g_SampleUI.GetCheckBox(IDC_TOGGLESTATICMESH)->GetChecked() ); break;
		case IDC_TOGGLEOCCLUSIONCULLING		: g_Settings.SetOcclusionCulling( g_SampleUI.GetCheckBox(IDC_TOGGLEOCCLUSIONCULLING)->GetChecked() ); break;
		case IDC_TOGGLERENDERTOBUFFER:
		{
			g_bRenderToBuffer = g_SampleUI.GetCheckBox(IDC_TOGGLERENDERTOBUFFER)->GetChecked();

			if (g_bRenderToBuffer)
			{
				CreateRenderBuffer();
				OnResize(g_uiRenderBufferWidth, g_uiRenderBufferHeight);
			}
			else
			{
				ReleaseRenderBuffer();
				OnResize(g_uiWidth, g_uiHeight);
			}

			break;
		}
		case IDC_TOGGLETEXTUREPOSTPROCESSING	: g_Settings.SetTexturePostProcessing( g_SampleUI.GetCheckBox(IDC_TOGGLETEXTUREPOSTPROCESSING)->GetChecked() ); g_pTerrain->SetTexturePostProcessing(g_Settings.GetTexturePostProcessing()); break;
		case IDC_TOGGLERENDERSTATISTICS			: g_Settings.SetRenderStatistics( g_SampleUI.GetCheckBox(IDC_TOGGLERENDERSTATISTICS)->GetChecked() ); break;
		case IDC_TOGGLERENDERLOGO				: g_Settings.SetRenderLogo( g_SampleUI.GetCheckBox(IDC_TOGGLERENDERLOGO)->GetChecked() ); break;
		case IDC_TOGGLERENDEROVERVIEWMAP		: g_bRenderOverviewMap = g_SampleUI.GetCheckBox(IDC_TOGGLERENDEROVERVIEWMAP)->GetChecked(); break;
		case IDC_OVERVIEW_MAP_SCALE				:
			{
				if (g_pOverviewMap != NULL)
				{
					g_fOverviewMapSize = static_cast<float>(g_SampleUI.GetSlider(IDC_OVERVIEW_MAP_SCALE)->GetValue()) / 100.0f;
					unsigned int uiOverviewMapHeight = static_cast<unsigned int>(static_cast<float>(g_uiHeight) * g_fOverviewMapSize / 100.0f);
					g_pOverviewMap->SetLocation(0, g_uiHeight - uiOverviewMapHeight);
					g_pOverviewMap->SetHeight(uiOverviewMapHeight);
				}
				swprintf_s(str, MAX_STRING_SIZE, L"Overview Map Size: %.0f", g_fOverviewMapSize);		
				g_SampleUI.GetStatic(IDC_OVERVIEW_MAP_SCALE_TEXT)->SetText(str);
				break;
			}
		case IDC_FOCALLENGTH_SCALE				:
			g_Settings.SetFocalLength( static_cast<float>(g_ClusterUI.GetSlider(IDC_FOCALLENGTH_SCALE)->GetValue()) / 100.0f );
			swprintf_s(str, MAX_STRING_SIZE, L"Focal Length: %.2f", g_Settings.GetFocalLength());
			g_ClusterUI.GetStatic(IDC_FOCALLENGTH_SCALE_TEXT)->SetText(str);
			break;
		case IDC_EYEDIST_SCALE					:
			g_Settings.SetEyeDist( static_cast<float>(g_ClusterUI.GetSlider(IDC_EYEDIST_SCALE)->GetValue()) / 100.0f );
			swprintf_s(str, MAX_STRING_SIZE, L"Eye Distance: %.2f", g_Settings.GetEyeDist());
			g_ClusterUI.GetStatic(IDC_EYEDIST_SCALE_TEXT)->SetText(str);
			break;
		case IDC_MAXLEVELLOADING				:
			g_pDataLoader->SetMaxLevel(static_cast<unsigned int>(g_SampleUI.GetSlider(IDC_MAXLEVELLOADING)->GetValue()));
			swprintf_s(str, MAX_STRING_SIZE, L"Max Level Loading: %u", static_cast<unsigned int>(g_SampleUI.GetSlider(IDC_MAXLEVELLOADING)->GetValue()));
			g_SampleUI.GetStatic(IDC_MAXLEVELLOADING_TEXT)->SetText(str);
			break;
		case IDC_MAXLEVELRENDERING				:
			g_pTerrain->SetMaxLevel(static_cast<unsigned int>(g_SampleUI.GetSlider(IDC_MAXLEVELRENDERING)->GetValue()));
			swprintf_s(str, MAX_STRING_SIZE, L"Max Level Rendering: %u", static_cast<unsigned int>(g_SampleUI.GetSlider(IDC_MAXLEVELRENDERING)->GetValue()));
			g_SampleUI.GetStatic(IDC_MAXLEVELRENDERING_TEXT)->SetText(str);
			break;
		case IDC_SATMODIFY						:
			g_Settings.SetSatModify( g_SampleUI.GetCheckBox(IDC_SATMODIFY)->GetChecked() );
			g_SampleUI.GetStatic(IDC_SATURATION_TEXT)->SetEnabled(g_Settings.GetSatModify());
			g_SampleUI.GetSlider(IDC_SATURATION)->SetEnabled(g_Settings.GetSatModify());
			break;
		case IDC_SATURATION						:
			g_Settings.SetSaturation( static_cast<float>(g_SampleUI.GetSlider(IDC_SATURATION)->GetValue()) / 100.0f );
			swprintf_s(str, MAX_STRING_SIZE, L"Saturation: %.2f", g_Settings.GetSaturation());
			g_SampleUI.GetStatic(IDC_SATURATION_TEXT)->SetText(str);
			break;
		case IDC_DEMO_MODE						:
			g_bDemoMode = g_SampleUI.GetCheckBox(IDC_DEMO_MODE)->GetChecked();
			g_SampleUI.GetCheckBox(IDC_RESTORE_POSITION)->SetEnabled(g_bDemoMode);
			g_SampleUI.GetStatic(IDC_IDLE_TIME_TEXT)->SetEnabled(g_bDemoMode);
			g_SampleUI.GetSlider(IDC_IDLE_TIME)->SetEnabled(g_bDemoMode);
			break;
		case IDC_RESTORE_POSITION				:
			g_bRestorePosition = g_SampleUI.GetCheckBox(IDC_RESTORE_POSITION)->GetChecked();
			break;
		case IDC_IDLE_TIME						:
			g_iIdleTime = g_SampleUI.GetSlider(IDC_IDLE_TIME)->GetValue();
			swprintf_s(str, MAX_STRING_SIZE, L"Idle Time: %is", g_iIdleTime);
			g_SampleUI.GetStatic(IDC_IDLE_TIME_TEXT)->SetText(str);
			break;
	}    
}
