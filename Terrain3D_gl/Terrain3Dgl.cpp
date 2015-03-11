/******************************************************************************

Terrain3Dgl.cpp

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/

#include <string>
#include <stdio.h>
#include <assert.h>

#include <windows.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glut.h>

#include "terrainLib/terrainlib.h"

#include "global.h"
#include "camera.h"
#include "configuration.h"
//#include "overviewmap.h"
#include "recorder.h"

#include "settings.h"

using namespace tum3D;

const char *const			ptcBENCHMARK_OUTPUT_FILE = "RasterizedUtah2560x1600.txt";

DWORDLONG g_dwlSystemMemoryLimit;

// Komponenten
Camera*						g_pCamera = 0;
Recorder*					g_pRecorder = 0;

CommandLine g_commandLine;
SystemConfiguration* g_pSystemConfiguration = 0;

static TerrainConfiguration g_terrainConfiguration;
static DataLoader*          g_dataLoader = 0;
static Renderer::Device*    g_device = 0;
static Terrain*             g_terrain = 0;

static Settings g_Settings;

bool						g_bDemoMode = true;
bool						g_bRestorePosition = false;
int							g_iIdleTime = 90;
double						g_fIdleTime = 0.0;
bool						g_bDemo = false;
bool						g_bActivity = true;

uint						g_uiBenchmarkState = 0;
bool						g_bBenchmarkBusTransfer = false;
bool						g_bBenchmarkStreaming = false;

bool						g_bOnFrameRenderCalledSinceLastOnFrameMove = true;

const float g_fClearColor[4] = //{ 0.227f, 0.431f, 0.647f, 1.0f }; // red, green, blue, alpha
//{ 58.0f / 255.0f, 110.0f / 255.0f, 165.0f / 255.0f, 1.0f };
{ 57.0f / 255.0f, 113.0f / 255.0f, 165.0f / 255.0f, 1.0f }; // S3TC-geeignet (im R5G6B5-Format darstellbar)

uint windowWidth, windowHeight;

double counterFrequency;
long long baseTime;
long long lastTime;

void onMove( double fTime, float fElapsedTime );
void onRender( double fTime, float fElapsedTime );
void rasterizerStateSolid();
void rasterizerStateWireframe();

void initTimer() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency( &frequency);
	counterFrequency = double( frequency.QuadPart );

	LARGE_INTEGER time;
	QueryPerformanceCounter( &time );
	lastTime = baseTime = time.QuadPart;
}

void getTime(double &time, double &elapsedTime) {
	LARGE_INTEGER ctime;
	QueryPerformanceCounter( &ctime );
	long long currentTime = ctime.QuadPart;

	time = (currentTime - baseTime) / counterFrequency;
	elapsedTime = (currentTime - lastTime) / counterFrequency;
	lastTime = currentTime;
}

void checkGLError() {
	GLenum error = glGetError();
	if( error != GL_NO_ERROR ) {
		printf( "%s\n", gluErrorString(error) );
	}
}

void onGLUTRender() {
	double time, elapsedTime;
	getTime( time, elapsedTime );

	onMove( time, elapsedTime );

	onRender( time, elapsedTime );

	glutPostRedisplay();
}

void onRender( double fTime, float fElapsedTime  ) {
	if (fElapsedTime < 5.0f && g_pRecorder->GetState() == REC_STATE_IDLE) { g_fIdleTime += fElapsedTime; }

	g_bOnFrameRenderCalledSinceLastOnFrameMove = true;

	if (g_bBenchmarkBusTransfer)
	{
		return;
	}

	//
	// Clear the back buffer
	//
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glDisable( GL_RASTERIZER_DISCARD );
	glViewport( 0, 0, windowWidth, windowHeight );

	glClearColor( g_fClearColor[0], g_fClearColor[1], g_fClearColor[2], g_fClearColor[3] );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

/*
	if (g_bRenderToBuffer)
	{
		pd3dDevice->ClearRenderTargetView(g_pRenderBuffer->GetRenderTargetView(), g_fClearColor);
	}*/

/*
	if (g_bRenderToBuffer)
	{
		pd3dDevice->ClearDepthStencilView(g_pDepthStencil->GetDepthStencilView(), D3D10_CLEAR_DEPTH, 1.0, 0);
	}*/


	/*
	if (g_bRenderToBuffer)
		{
			g_rDevice->SetOffscreenRenderTarget( g_pRenderBuffer, g_pDepthStencil );
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
*/
	//
	// Update variables that change once per frame
	//
	static float fLastHeight = 0.0f;
	Vec3f vPosition;
	g_pCamera->GetPosition(vPosition);
	if (fLastHeight <= 10000.0f && vPosition.z() > 10000.0f || fLastHeight > 10000.0f && vPosition.z() <= 10000.0f)
	{
/*
		if (g_bRenderToBuffer)
		{
			OnResize(g_uiRenderBufferWidth, g_uiRenderBufferHeight);
		}
		else*/

/*
		{
			OnResize(g_uiWidth, g_uiHeight);
		}*/

	}
	fLastHeight = vPosition.z();

	if (g_Settings.GetSatModify())
	{
		g_terrain->SetSaturation(g_Settings.GetSaturation());
	}

	if (g_Settings.GetWireframe()) {
		rasterizerStateWireframe();
	}

	if (g_Settings.GetPageBoundingBoxes())
	{
		g_dataLoader->Render(g_terrain);
	}

	g_terrain->Render(g_Settings.GetBoundingBoxes(), g_Settings.GetSatModify());

	// frustum rendering
/*
	if (g_Settings.GetStaticMesh())
	{
		// Kanten des View Frustums rendern
		D3DXMATRIX mView;
		g_pCamera->GetViewMatrix(&mView);
		g_effect.m_pmWorldViewVariable->SetMatrix((float*)mView);
		// TODO: wrap terrain3deffect too? [8/12/2009 Andreas Kirsch]
		//#g_rDevice->SetVertexBufferAndInputLayout( g_pFrustumVertexBuffer, sizeof( D3DXVECTOR4 ), g_effect.m_pInputLayoutFrustum );
		uint stride = sizeof( D3DXVECTOR4 ), offset = 0;
		pd3dDevice->IASetVertexBuffers( 0, 1, &g_pFrustumVertexBuffer->GetBuffer(), &stride, &offset );
		pd3dDevice->IASetInputLayout( g_effect.m_pInputLayoutFrustum );

		g_rDevice->SetIndexBuffer( g_pFrustumIndexBuffer, 0 );
		g_effect.m_pTechniqueFrustum->GetPassByIndex(0)->Apply(0);
		g_rDevice->DrawElements( Renderer::LINELIST, 24 );
	}*/


/*
	if (g_Settings.GetWireframe()) {
		pd3dDevice->RSSetState(g_pRasterizerStateSolid);
	}*/


	/*
	if (g_bRenderToBuffer)
		{
			pd3dDevice->OMSetRenderTargets(1, &pRTV, pDSV);
			pd3dDevice->RSSetViewports(1, &g_viewport);
	
			g_effect.m_pSourceBitBltVariable->SetResource( g_pRenderBuffer->GetShaderResourceView() );
	
			g_rDevice->SetNoVertexBuffer();
	
			g_effect.m_pTechniqueBitBlt->GetPassByIndex(0)->Apply(0);
			g_rDevice->DrawArrays( Renderer::TRIANGLELIST, 3 );
		}*/
	

/*
	if (g_Settings.m_bIsRemoteControl && g_pOverviewMap != 0 && g_bRenderOverviewMap)
	{
		g_pOverviewMap->Render();
	}*/


	/*
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
			g_effect.m_pSourceBitBltVariable->SetResource(g_pTumLogoTexture->GetShaderResourceView());
	
			g_rDevice->SetNoVertexBuffer();
	
			g_effect.m_pTechniqueBitBlt->GetPassByIndex(1)->Apply(0);
			g_rDevice->DrawArrays( Renderer::TRIANGLELIST, 3 );
	
		}	

	/*
	if (g_bRenderHelp)
		{
			RenderHelp();
		}
		else if (g_Settings.GetRenderStatistics())
		{
			RenderStatistics();
		}*/
	

#ifdef WRITE_IMAGE_SEQUENCE
	if (g_uiImageSequenceNumber >= uiSTART_FRAME_NUM && g_pRecorder->GetState() == REC_STATE_PLAYING_BACK)
	{
		pd3dDevice->IASetInputLayout(0);
		g_effect.m_pTechniqueBitBlt->GetPassByIndex(3)->Apply(0);
		glDrawArrays( GL_TRIANGLES, 0, 3 );
		checkGLError();
		// Capture Screenshot
		char filename[MAX_STRING_SIZE];
		_stprintf_s(filename, MAX_STRING_SIZE, "%s\\image%05u.png", ptcWRITE_IMAGE_SEQUENCE_DIR, g_uiImageSequenceNumber);
		const D3DX10_IMAGE_FILE_FORMAT format = D3DX10_IFF_PNG;

		ID3D10Texture2D* pRT;
		if (g_bRenderToBuffer)
		{
			pRT = g_pRenderBuffer->GetTexture();
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
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
				exit(-1);
			}
		}
		else
		{
			texDesc.Usage = D3D10_USAGE_DEFAULT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			ID3D10Texture2D* pTexture;
			HRESULT hr = pd3dDevice->CreateTexture2D(&texDesc, 0, &pTexture);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
				exit(-1);
			}
			pd3dDevice->ResolveSubresource(pTexture, 0, pRT, 0, texDesc.Format); // Copy a multisampled resource into a non-multisampled resource
			hr = D3DX10SaveTextureToFile(pTexture, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
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

/*
	if (g_bCaptureScreenshot)
	{
		g_rDevice->SetNoVertexBuffer();
		g_effect.m_pTechniqueBitBlt->GetPassByIndex(3)->Apply(0);
		g_rDevice->DrawArrays( Renderer::TRIANGLELIST, 3 );

		// Capture Screenshot
		const char*const filename = "Screenshot.png";
		const D3DX10_IMAGE_FILE_FORMAT format = D3DX10_IFF_PNG;

		ID3D10Texture2D* pRT;
		if (g_bRenderToBuffer)
		{
			pRT = g_pRenderBuffer->GetTexture();
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
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
				exit(-1);
			}
		}
		else
		{
			texDesc.Usage = D3D10_USAGE_DEFAULT;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			ID3D10Texture2D* pTexture;
			// TODO: cant wrap this directly because it uses another texture's description -> move all into device? [7/6/2009 Andreas Kirsch]
			HRESULT hr = pd3dDevice->CreateTexture2D(&texDesc, 0, &pTexture);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
				exit(-1);
			}
			pd3dDevice->ResolveSubresource(pTexture, 0, pRT, 0, texDesc.Format); // Copy a multisampled resource into a non-multisampled resource
			hr = D3DX10SaveTextureToFile(pTexture, format , filename);
			if (FAILED(hr))
			{
				DXTrace(__FILE__, static_cast<DWORD>(__LINE__), hr, 0, TRUE);
				exit(-1);
			}
			pTexture->Release();
		}
		if (!g_bRenderToBuffer)
		{
			pRT->Release();
		}

		g_bCaptureScreenshot = false;
	}*/

	//if (!g_Settings.m_bLocalOnly) { g_Settings.SyncFrame(); }

	checkGLError();
	glutSwapBuffers();
}

void GLUTCALLBACK onGLUTReshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;

	glViewport(0,0, width, height);

	g_pCamera->SetParameters(width, height, g_pSystemConfiguration->GetFovy());

	Vec3f vPosition;
	g_pCamera->GetPosition(vPosition);
	float fZNear = g_pSystemConfiguration->GetZNear();
	if (vPosition.z() > 10000.0f && fZNear < 100.0f)
	{
		fZNear = 100.0f; // Bei zNear = 10.0f stürzt in großen Höhen der Treiber ab (Version 158.43, 17.05.2007)
	}

	float fAspectRatio = static_cast<float>(width) / static_cast<float>(height);
	if (g_Settings.m_bIsRemoteControl) {
		float radians = DEG2RAD(g_pSystemConfiguration->GetFovy()) / 2;
		float wd2     = fZNear * float(tan(radians));

		float left   = - fAspectRatio * wd2;
		float right  =   fAspectRatio * wd2;
		float top    =   wd2;
		float bottom = - wd2;

		perspectiveOffCenterProjMatGL( left, right, bottom, top, fZNear, g_pSystemConfiguration->GetZFar(), g_Settings.m_mProj );
	}
	else {
		assert( false );
/*
		g_Settings.ComputeFrustum(DEG2RAD(g_pSystemConfiguration->GetFovy()), fAspectRatio, fZNear, g_pSystemConfiguration->GetZFar(), g_Settings.GetFocalLength());
		D3DXMATRIX temp;
		g_pCamera->GetViewMatrix(&temp);
		g_Settings.ModifyAndStoreUserViewMatrix(temp);
*/
	}

	g_terrain->SetParameters(g_Settings.m_mProj, height, g_pSystemConfiguration->GetFovy(), fAspectRatio, fZNear, g_pSystemConfiguration->GetScreenSpaceError());
	g_dataLoader->ComputeLoadingRadiuses(height, g_pSystemConfiguration->GetFovy(), fAspectRatio, g_pSystemConfiguration->GetZFar(), g_pSystemConfiguration->GetScreenSpaceError(), g_pSystemConfiguration->GetPrefetchingFactor());
}

Vec3f g_vPositionBackup;
float g_fHeadingBackup, g_fPitchBackup, g_fRollBackup;


void startDemo()
{
	assert(!g_bDemo);

	g_bDemo = true;

	g_pCamera->GetPosition(g_vPositionBackup);
	g_pCamera->GetAngles(&g_fHeadingBackup, &g_fPitchBackup, &g_fRollBackup);

	g_pRecorder->SetFileName(g_terrainConfiguration.GetDemoFlightFileName().c_str());
	g_pRecorder->StartPlayingBack(true, false);
}

void onActivity()
{
	if (g_bDemo)
	{
		g_pRecorder->StopPlayingBack();

		if (g_bRestorePosition)
		{
			g_pCamera->SetPosition(g_vPositionBackup);
			g_pCamera->SetAngles(g_fHeadingBackup, g_fPitchBackup, g_fRollBackup);
		}

		g_bDemo = false;
	}
	g_fIdleTime = 0.0;
}

void onMove( double fTime, float fElapsedTime )
{
	//printf("%f\n", g_fIdleTime);
	static double s_dStatLastTime = fTime;
	static RECORDER_STATE eRecLastState = REC_STATE_IDLE;
	static char str1[MAX_STRING_SIZE];
	static const char * str2 = "";

	bool bUpdateTitle = false;

	//g_rDevice->StringMarker( "Terrain Updating" );

	static ullong		s_ui64StatLastNumTrianglesRendered = 0;
	static ullong		s_ui64StatLastDiskNumBytesTransferred = 0;
	static ullong		s_ui64StatLastNumTilesTransferred = 0;
	static ullong		s_ui64StatLastNumBytesTransferred = 0;
	static ullong		s_ui64StatLastNumBytesTransferredDecompressed = 0;

	double dt = fTime - s_dStatLastTime;
	if (dt >= 1.0f)
	{
#ifndef DEMO
		sprintf_s(str1, 256, "Terrain3D @ %.2f fps (%.2f MTri/s, Disk %.2f MB/s, Bus %.2f Tiles/s %.2f MB/s [%.2f MB/s])     Press F1 for help",
			1.0 / fElapsedTime,
			(static_cast<double>(g_terrain->GetStatNumTrianglesRendered() - s_ui64StatLastNumTrianglesRendered) / (1000.0 * 1000.0)) / dt,
			static_cast<double>(g_dataLoader->GetStatNumBytesTransferred() - s_ui64StatLastDiskNumBytesTransferred) / (1024.0 * 1024.0) / dt,
			static_cast<double>(g_terrain->GetStatNumTilesTransferred() - s_ui64StatLastNumTilesTransferred) /  dt,
			static_cast<double>(g_terrain->GetStatNumBytesTransferred() - s_ui64StatLastNumBytesTransferred) / (1024.0 * 1024.0) / dt,
			static_cast<double>(g_terrain->GetStatNumBytesTransferredDecompressed() - s_ui64StatLastNumBytesTransferredDecompressed) / (1024.0 * 1024.0) / dt
			);
#else
		sprintf_s(str1, 256, "Terrain3D @ %.2f fps (%.2f MTri/s, Disk %.2f MB/s)     Press F1 for help",
			DXUTGetFPS(),
			(static_cast<double>(g_terrain->GetStatNumTrianglesRendered() - s_ui64StatLastNumTrianglesRendered) / (1000.0 * 1000.0)) / dt,
			static_cast<double>(g_dataLoader->GetStatNumBytesTransferred() - s_ui64StatLastDiskNumBytesTransferred) / (1024.0 * 1024.0) / dt
			);
#endif

		s_ui64StatLastNumTrianglesRendered = g_terrain->GetStatNumTrianglesRendered();
		s_ui64StatLastDiskNumBytesTransferred = g_dataLoader->GetStatNumBytesTransferred();
		s_ui64StatLastNumTilesTransferred = g_terrain->GetStatNumTilesTransferred();
		s_ui64StatLastNumBytesTransferred = g_terrain->GetStatNumBytesTransferred();
		s_ui64StatLastNumBytesTransferredDecompressed = g_terrain->GetStatNumBytesTransferredDecompressed();

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
				str2 = "";
				break;
			}
		case REC_STATE_RECORDING:
			{
				str2 = "R E C O R D I N G";
				break;
			}
		case REC_STATE_PLAYING_BACK_PAUSED:
			{
				str2 = "P L A Y I N G   B A C K   P A U S E D   (Press Space to continue)";
				break;
			}
		case REC_STATE_PLAYING_BACK:
			{
				str2 = "P L A Y I N G   B A C K";
				break;
			}
		}
		eRecLastState = eRecState;
		bUpdateTitle = true;
	}

	if (/*DXUTIsWindowed() &&*/ bUpdateTitle)
	{
		char str[MAX_STRING_SIZE];
		sprintf_s(str, MAX_STRING_SIZE, "%s          %s", str1, str2);
		glutSetWindowTitle(str);
	}

	//***** Benchmarking *****

	struct BenchmarkEntryFPS
	{
		double			m_dTime;
		float			m_fFPS;
		float			m_fNumTrianglesRenderedPerFrame;
		float			m_fNumTilesRenderedPerFrame;
		float			m_fCameraSpeed;
		uint			m_uiGeoSize;
		uint			m_uiTexSize;
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

	static ullong s_ui64BenchmarkNumFrames;
	static ullong s_ui64BenchmarkLastNumFrames;
	static ullong s_ui64BenchmarkLastNumTrianglesRendered;
	static ullong s_ui64BenchmarkLastNumTilesRendered;
	static ullong s_ui64BenchmarkLastDiskNumBytesTransferred;
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
		s_ui64BenchmarkLastNumTrianglesRendered = g_terrain->GetStatNumTrianglesRendered();
		s_ui64BenchmarkLastNumTilesRendered = g_terrain->GetStatNumTilesRendered();
		s_ui64BenchmarkLastDiskNumBytesTransferred = g_dataLoader->GetStatNumBytesTransferred();
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
				for (uint ui = 0; ui < s_benchmarkDataFPS.size(); ui++)
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

				for (uint ui = 0; ui < s_benchmarkDataDisk.size(); ui++)
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
				entry.m_fNumTrianglesRenderedPerFrame = static_cast<float>(static_cast<double>(g_terrain->GetStatNumTrianglesRendered() - s_ui64BenchmarkLastNumTrianglesRendered) / static_cast<double>(s_ui64BenchmarkNumFrames - s_ui64BenchmarkLastNumFrames));
				entry.m_fNumTilesRenderedPerFrame = static_cast<float>(static_cast<double>(g_terrain->GetStatNumTilesRendered() - s_ui64BenchmarkLastNumTilesRendered) / static_cast<double>(s_ui64BenchmarkNumFrames - s_ui64BenchmarkLastNumFrames));
				entry.m_fCameraSpeed = static_cast<float>((g_pCamera->GetStatCoveredDistance() - s_dBenchmarkLastDistanceCovered) / dt);
				entry.m_uiGeoSize = g_terrain->GetStatGeoSize();
				entry.m_uiTexSize = g_terrain->GetStatTexSize();

				s_benchmarkDataFPS.push_back(entry);

				s_ui64BenchmarkLastNumFrames = s_ui64BenchmarkNumFrames;
				s_ui64BenchmarkLastNumTrianglesRendered = g_terrain->GetStatNumTrianglesRendered();
				s_ui64BenchmarkLastNumTilesRendered = g_terrain->GetStatNumTilesRendered();
				s_dBenchmarkLastDistanceCovered = g_pCamera->GetStatCoveredDistance();

				s_dBenchmarkLastTimeFPS = fTime;
			}

			dt = fTime - s_dBenchmarkLastTimeDisk;
			if (dt >= 1.0f)
			{
				BenchmarkEntryDisk entry;
				entry.m_dTime = fTime - s_dBenchmarkStartTime;
				entry.m_fDiskTransferRate = static_cast<float>(static_cast<double>(g_dataLoader->GetStatNumBytesTransferred() - s_ui64BenchmarkLastDiskNumBytesTransferred) / dt);

				s_benchmarkDataDisk.push_back(entry);

				s_ui64BenchmarkLastDiskNumBytesTransferred = g_dataLoader->GetStatNumBytesTransferred();	

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
			Vec3f vPosition;
			float fHeading, fPitch, fRoll;
			g_pCamera->GetPosition(vPosition);
			g_pCamera->GetAngles(&fHeading, &fPitch, &fRoll);

			g_pCamera->Update(fElapsedTime);

			Vec3f vNewPosition;
			float fNewHeading, fNewPitch, fNewRoll;
			g_pCamera->GetPosition(vNewPosition);
			g_pCamera->GetAngles(&fNewHeading, &fNewPitch, &fNewRoll);
			if (vNewPosition != vPosition || fNewHeading != fHeading || fNewPitch != fPitch || fNewRoll != fRoll)
			{
				onActivity();
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

		if (g_bDemoMode && !g_terrainConfiguration.GetDemoFlightFileName().empty() && !g_bDemo && g_fIdleTime > static_cast<float>(g_iIdleTime) && eRecState == REC_STATE_IDLE)
		{
			startDemo();
		}
#endif
		if (g_bBenchmarkBusTransfer || g_bBenchmarkStreaming)
		{
			g_terrain->PageOutAllAndFlushCache();
		}

		g_Settings.OnCameraUpdate();
	}

	if (!g_Settings.GetStaticMesh())
	{
		ViewInfo viewInfo;
		viewInfo.viewIndex = 0;

		g_pCamera->GetPosition(viewInfo.position);
		g_pCamera->GetCameraViewOrthogonalFrame(viewInfo.viewDirection, 0, 0);

#ifdef WRITE_IMAGE_SEQUENCE
		g_dataLoader->Update(vPosition, vViewDirection, (g_uiImageSequenceNumber >= uiSTART_FRAME_NUM && g_pRecorder->GetState() == REC_STATE_PLAYING_BACK));
#else
		ViewInfoVector viewInfoVector;
		viewInfoVector.push_back(viewInfo);
		g_dataLoader->Update(viewInfoVector);
#endif
		g_terrain->Update(g_Settings.m_mView, viewInfo.position, viewInfo.viewDirection, g_Settings.GetOcclusionCulling());

	}
	else
	{
		g_terrain->UpdateViewOnly(g_Settings.m_mView);
	}

	/*#
	if (g_Settings.m_bDeviceSettingsChanged)
		{
			g_Settings.m_bDeviceSettingsChanged = false;
			g_bDeviceSettingsChanged = true;
			g_ccDeviceSettings = g_Settings.GetDeviceSettings();
			PostMessage(DXUTGetHWND(), WM_NULL, 0, 0); // Sicherstellen, dass MsgProc aufgerufen wird
		}*/
	

	//#if (g_Settings.m_bCloseApp) { SendMessage( DXUTGetHWND(), WM_CLOSE, 0, 0 ); }
	/*
	if (g_Settings.m_bGoWindow) { GoWindow(); g_Settings.m_bGoWindow = false; }
	if (g_Settings.m_bGoFullscreen) { EndWindow(); g_Settings.m_bGoFullscreen = false; }
	*/

	g_bOnFrameRenderCalledSinceLastOnFrameMove = false;
}

void initialize() {
	g_terrainConfiguration.ReadFile(g_commandLine.IsTerrainFileSpecified() ? g_commandLine.GetTerrainFileName() : g_pSystemConfiguration->GetTerrainFileName());

	g_dataLoader = CreateDataLoader(g_terrainConfiguration, g_pSystemConfiguration->GetSystemMemoryUsageLimit(), g_pSystemConfiguration->GetScreenSpaceError());

	g_device = new Renderer::Device();
	g_terrain = new Terrain(g_dataLoader, g_pSystemConfiguration->GetInitNumVBs(), g_pSystemConfiguration->GetInitNumTextures(),
		g_terrainConfiguration.GetLevelZeroTileExtent(), g_terrainConfiguration.GetNumLevels(),
		g_terrainConfiguration.GetMinVerticalTolerance(), g_terrainConfiguration.GetTextureCompression());

	// only useful if multiple views are being used, but still needs to be called [10/9/2009 Andreas Kirsch]
	g_dataLoader->GetNewViewIndex();

	const StartLocation& startLocation = g_terrainConfiguration.GetStartLocation(0);
	g_pCamera = new Camera(g_dataLoader, g_terrainConfiguration.GetMaxSpeed(), g_terrainConfiguration.GetMinHeight(), startLocation.vStartPosition, startLocation.fStartHeading, startLocation.fStartPitch);
	g_Settings.SetCamera(g_pCamera);

	g_pRecorder = new Recorder(g_pCamera);

	//if (g_Settings.m_bIsRemoteControl)
	//{
	//	if (g_pTerrainConfiguration->GetOverviewMapFileName() != 0)
	//	{
	//		float fX0, fY0, fSizeX, fSizeY;
	//		g_pTerrainConfiguration->GetOverviewMapExtents(&fX0, &fY0, &fSizeX, &fSizeY);
	//		g_pOverviewMap = new OverviewMap(g_pTerrainConfiguration->GetOverviewMapFileName(), fX0, fY0, fSizeX, fSizeY, g_pCamera);
	//	}
	//}
}

void rasterizerStateSolid() {
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW );
}

void rasterizerStateWireframe() {
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW );
}

bool initDevice() {
	g_device->Create();
	g_terrain->Create(g_device);

	glDisable( GL_DITHER );
	glClampColor( GL_CLAMP_VERTEX_COLOR, GL_FALSE );
	glClampColor( GL_CLAMP_FRAGMENT_COLOR, GL_FALSE );
	glClampColor( GL_CLAMP_READ_COLOR, GL_FALSE );

	rasterizerStateSolid();

	checkGLError();

	return true;
}

void cleanup() {
	g_Settings.SetCamera(0); // Notwendig, da der ClusterCom-Nachrichten-Empfangs-Thread im Hintergrund weiterläuft!
	//g_Settings.SetFrustumCulling(0);

	delete g_pRecorder;
	delete g_pCamera;

	g_terrain->SafeRelease();
	g_device->SafeRelease();
	delete g_terrain;
	delete g_device;
	
	delete g_dataLoader;

	delete g_pSystemConfiguration;
}

void MouseCallback( int button, int state, int x, int y ) {
	MOUSE_BUTTON cam_button;

	switch( button ) {
	case GLUT_LEFT_BUTTON:
		cam_button = MOUSE_BUTTON_LEFT;
		break;
	case GLUT_MIDDLE_BUTTON:
		cam_button = MOUSE_BUTTON_MIDDLE;
		break;
	case GLUT_RIGHT_BUTTON:
		cam_button = MOUSE_BUTTON_RIGHT;
		break;
	}

	g_pCamera->MouseButtonEvent( cam_button, state == GLUT_DOWN, x, y );
}

CAM_KEY GlutSpecialKeyToCamKey( int key ) {
	switch( key ) {
	case GLUT_KEY_UP:
		return CAM_KEY_UP;
	case GLUT_KEY_DOWN:
		return CAM_KEY_DOWN;
	case GLUT_KEY_LEFT:
		return CAM_KEY_LEFT;
	case GLUT_KEY_RIGHT:
		return CAM_KEY_RIGHT;
	case GLUT_KEY_PAGE_UP:
		return CAM_KEY_PAGEUP;
	case GLUT_KEY_PAGE_DOWN:
		return CAM_KEY_PAGEDOWN;
	}
	return CAM_KEY_UNKNOWN;
}

void KeyboardUpCallback(uchar key, int x, int y) {
	key = toupper( key );
	g_pRecorder->KeyboardEvent( key );
}


void SpecialCallback( int key, int x, int y ){
	CAM_KEY cam_key = GlutSpecialKeyToCamKey( key );
	g_pCamera->KeyboardEvent( cam_key, true );

	g_pCamera->KeyboardEvent( CAM_KEY_SHIFT, (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0 );
}

void SpecialUpCallback( int key, int x, int y ) {
	CAM_KEY cam_key = GlutSpecialKeyToCamKey( key );
	g_pCamera->KeyboardEvent( cam_key, false );

	g_pCamera->KeyboardEvent( CAM_KEY_SHIFT, (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0 );
}

void MouseMotionCallback( int x, int y ) {
	g_pCamera->MouseMoveEvent( x, y );
}

int main(int argc, char * argv[]) {
	// enable run-time memory check for debug builds
	#if defined( _WIN32 ) && ( defined( DEBUG ) || defined( _DEBUG ) )
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	bool result = g_commandLine.ParseCommandLine(argc, argv);
	if (!result)
	{
		MessageBoxA(0, "Error Parsing Command Line\n", "Error", MB_OK | MB_ICONERROR);
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

	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	glutCreateWindow("GLEW Test");
	glutDisplayFunc( onGLUTRender );
	glutReshapeFunc( onGLUTReshape );

	glutKeyboardUpFunc( KeyboardUpCallback );
	glutSpecialFunc( SpecialCallback );
	glutSpecialUpFunc( SpecialUpCallback );
	glutMouseFunc( MouseCallback );
	glutMotionFunc( MouseMotionCallback );
	glutPassiveMotionFunc( MouseMotionCallback );

#ifndef GLEW_MX
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong.
		printf("Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	initTimer();

	initialize();

	initDevice();

	//g_Settings.SetBoundingBoxes( true );

	atexit( cleanup );
	glutMainLoop();
	return 0;
}
