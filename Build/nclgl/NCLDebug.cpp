#include "NCLDebug.h"
#include "Window.h"
#include "Mesh.h"
#include <SOIL.h>
#include <algorithm>
#include <sstream>

Vector3	NCLDebug::g_CameraPosition;
Matrix4	NCLDebug::g_ProjMtx;
Matrix4	NCLDebug::g_ViewMtx;
Matrix4	NCLDebug::g_ProjViewMtx;

int		NCLDebug::g_NumStatusEntries			= 0;
float	NCLDebug::g_MaxStatusEntryWidth			= 0.0f;

bool  NCLDebug::g_StatusVisible		= true;
bool  NCLDebug::g_LogVisible		= false;
FILE* NCLDebug::g_vOutLogFile		= NULL;

std::deque<LogEntry> NCLDebug::g_vLogEntries; 
int NCLDebug::g_vLogOffsetIdx = -1;

std::vector<Vector4> NCLDebug::g_vChars;
uint NCLDebug::g_vCharsLogStart = 0;

DebugDrawList NCLDebug::g_DrawList[2];
DebugDrawList NCLDebug::g_DrawListNDT[2];

Shader*	NCLDebug::g_pShaderPoints		= NULL;
Shader*	NCLDebug::g_pShaderLines		= NULL;
Shader*	NCLDebug::g_pShaderHairLines	= NULL;
Shader*	NCLDebug::g_pShaderText			= NULL;

GLuint	 NCLDebug::g_glArr				= NULL;
GLuint	 NCLDebug::g_glBuf				= NULL;
GLuint	 NCLDebug::g_glBufCapacity		= NULL;
Vector4* NCLDebug::g_glBufPtr			= NULL;
uint	 NCLDebug::g_glBufOffsets[9];

GLuint NCLDebug::g_glLogFontTex			= NULL;
GLuint NCLDebug::g_glDefaultFontTex		= NULL;


int IsOpaque(const Vector4& col)
{
	return col.w > 0.9999f;
}

//Draw Point (circle)
void NCLDebug::GenDrawPoint(bool ndt, const Vector3& pos, float point_radius, const Vector4& color)
{
	int idx = IsOpaque(color);
	auto list = ndt ? &g_DrawListNDT[idx]: &g_DrawList[idx];
	list->_vPoints.push_back(Vector4(pos.x, pos.y, pos.z, point_radius));
	list->_vPoints.push_back(color);
}

void NCLDebug::DrawPoint(const Vector3& pos, float point_radius, const Vector3& color)
{
	GenDrawPoint(false, pos, point_radius, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawPoint(const Vector3& pos, float point_radius, const Vector4& color)
{
	GenDrawPoint(false, pos, point_radius, color);
}
void NCLDebug::DrawPointNDT(const Vector3& pos, float point_radius, const Vector3& color)
{
	GenDrawPoint(true, pos, point_radius, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawPointNDT(const Vector3& pos, float point_radius, const Vector4& color)
{
	GenDrawPoint(true, pos, point_radius, color);
}



//Draw Line with a given thickness 
void NCLDebug::GenDrawThickLine(bool ndt, const Vector3& start, const Vector3& end, float line_width, const Vector4& color)
{
	int idx = IsOpaque(color);
	auto list = ndt ? &g_DrawListNDT[idx] : &g_DrawList[idx];

	//For Depth Sorting
	Vector3 midPoint = (start + end) * 0.5f;
	float camDist = Vector3::Dot(midPoint - g_CameraPosition, midPoint - g_CameraPosition);

	//Add to Data Structures
	list->_vThickLines.push_back(Vector4(start.x, start.y, start.z, line_width));
	list->_vThickLines.push_back(color);

	list->_vThickLines.push_back(Vector4(end.x, end.y, end.z, camDist));
	list->_vThickLines.push_back(color);

	GenDrawPoint(ndt, start, line_width * 0.5f, color);
	GenDrawPoint(ndt, end, line_width * 0.5f, color);
}
void NCLDebug::DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector3& color)
{
	GenDrawThickLine(false, start, end, line_width, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector4& color)
{
	GenDrawThickLine(false, start, end, line_width, color);
}
void NCLDebug::DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector3& color)
{
	GenDrawThickLine(true, start, end, line_width, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector4& color)
{
	GenDrawThickLine(true, start, end, line_width, color);
}


//Draw line with thickness of 1 screen pixel regardless of distance from camera
void NCLDebug::GenDrawHairLine(bool ndt, const Vector3& start, const Vector3& end, const Vector4& color)
{
	int idx = IsOpaque(color);
	auto list = ndt ? &g_DrawListNDT[idx] : &g_DrawList[idx];

	list->_vHairLines.push_back(Vector4(start.x, start.y, start.z, 1.0f));
	list->_vHairLines.push_back(color);

	list->_vHairLines.push_back(Vector4(end.x, end.y, end.z, 1.0f));
	list->_vHairLines.push_back(color);
}
void NCLDebug::DrawHairLine(const Vector3& start, const Vector3& end, const Vector3& color)
{
	GenDrawHairLine(false, start, end, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawHairLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	GenDrawHairLine(false, start, end, color);
}
void NCLDebug::DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector3& color)
{
	GenDrawHairLine(true, start, end, Vector4(color.x, color.y, color.z, 1.0f));
}
void NCLDebug::DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector4& color)
{
	GenDrawHairLine(true, start, end, color);
}


//Draw Matrix (x,y,z axis at pos)
void NCLDebug::DrawMatrix(const Matrix4& mtx)
{
	Vector3 position = mtx.GetPositionVector();
	GenDrawHairLine(false, position, position + Vector3(mtx.values[0], mtx.values[1], mtx.values[2]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + Vector3(mtx.values[4], mtx.values[5], mtx.values[6]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + Vector3(mtx.values[8], mtx.values[9], mtx.values[10]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrix(const Matrix3& mtx, const Vector3& position)
{
	GenDrawHairLine(false, position, position + mtx.GetCol(0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + mtx.GetCol(1), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + mtx.GetCol(2), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrixNDT(const Matrix4& mtx)
{
	Vector3 position = mtx.GetPositionVector();
	GenDrawHairLine(true, position, position + Vector3(mtx.values[0], mtx.values[1], mtx.values[2]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + Vector3(mtx.values[4], mtx.values[5], mtx.values[6]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + Vector3(mtx.values[8], mtx.values[9], mtx.values[10]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrixNDT(const Matrix3& mtx, const Vector3& position)
{
	GenDrawHairLine(true, position, position + mtx.GetCol(0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + mtx.GetCol(1), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + mtx.GetCol(2), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}



//Draw Triangle 
void NCLDebug::GenDrawTriangle(bool ndt, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& color)
{
	int idx = IsOpaque(color);
	auto list = ndt ? &g_DrawListNDT[idx] : &g_DrawList[idx];

	//For Depth Sorting
	Vector3 midPoint = (v0 + v1 + v2) * (1.0f / 3.0f);
	float camDist = Vector3::Dot(midPoint - g_CameraPosition, midPoint - g_CameraPosition);

	//Add to data structures
	list->_vTris.push_back(Vector4(v0.x, v0.y, v0.z, camDist));
	list->_vTris.push_back(color);

	list->_vTris.push_back(Vector4(v1.x, v1.y, v1.z, 1.0f));
	list->_vTris.push_back(color);

	list->_vTris.push_back(Vector4(v2.x, v2.y, v2.z, 1.0f));
	list->_vTris.push_back(color);
}

void NCLDebug::DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& color)
{
	GenDrawTriangle(false, v0, v1, v2, color);
}

void NCLDebug::DrawTriangleNDT(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& color)
{
	GenDrawTriangle(true, v0, v1, v2, color);
}

//Draw Polygon (Renders as a triangle fan, so verts must be arranged in order)
void NCLDebug::DrawPolygon(int n_verts, const Vector3* verts, const Vector4& color)
{
	for (int i = 2; i < n_verts; ++i)
	{
		GenDrawTriangle(false, verts[0], verts[i - 1], verts[i], color);
	}
}

void NCLDebug::DrawPolygonNDT(int n_verts, const Vector3* verts, const Vector4& color)
{
	for (int i = 2; i < n_verts; ++i)
	{
		GenDrawTriangle(true, verts[0], verts[i - 1], verts[i], color);
	}
}

void NCLDebug::DrawSphere(const Vector3& pos, const float radius, const Vector4& color)
{
	//Draw Filled Circle
	NCLDebug::DrawPointNDT(pos, radius, Vector4(1.0f, 1.0f, 1.0f, 0.2f));

	//Draw Perimeter Axes
	Vector3 lastX = pos + Vector3(0.0f, 1.0f, 0.0f) * radius;
	Vector3 lastY = pos + Vector3(1.0f, 0.0f, 0.0f) * radius;
	Vector3 lastZ = pos + Vector3(1.0f, 0.0f, 0.0f) * radius;
	const int nSubdivisions = 20;
	for (int itr = 1; itr <= nSubdivisions; ++itr)
	{
		float angle = itr / float(nSubdivisions) * PI * 2.f;
		float alpha = cosf(angle) * radius;
		float beta = sinf(angle) * radius;

		Vector3 newX = pos + Vector3(0.0f, alpha, beta);
		Vector3 newY = pos + Vector3(alpha, 0.0f, beta);
		Vector3 newZ = pos + Vector3(alpha, beta, 0.0f);

		NCLDebug::DrawThickLineNDT(lastX, newX, 0.02f, color);
		NCLDebug::DrawThickLineNDT(lastY, newY, 0.02f, color);
		NCLDebug::DrawThickLineNDT(lastZ, newZ, 0.02f, color);

		lastX = newX;
		lastY = newY;
		lastZ = newZ;
	}
}

void NCLDebug::DrawBox(const Vector3 & pos, const Vector3 halfDims, const Vector4 & color)
{
	Vector3 leftUpForwardPos = Vector3(pos.x + halfDims.x, pos.y + halfDims.y, pos.z + halfDims.z);
	NCLDebug::DrawThickLineNDT(leftUpForwardPos, leftUpForwardPos - Vector3(halfDims.x * 2.0f, 0.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(leftUpForwardPos, leftUpForwardPos - Vector3(0.0f, halfDims.y * 2.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(leftUpForwardPos, leftUpForwardPos - Vector3(0.0f, 0.0f, halfDims.z * 2.0f), 0.02f, color);

	Vector3 rightUpBackwardPos = Vector3(pos.x - halfDims.x, pos.y + halfDims.y, pos.z - halfDims.z);
	NCLDebug::DrawThickLineNDT(rightUpBackwardPos, rightUpBackwardPos + Vector3(halfDims.x * 2.0f, 0.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(rightUpBackwardPos, rightUpBackwardPos - Vector3(0.0f, halfDims.y * 2.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(rightUpBackwardPos, rightUpBackwardPos + Vector3(0.0f, 0.0f, halfDims.z * 2.0f), 0.02f, color);

	Vector3 leftDownForwardPos = Vector3(pos.x - halfDims.x, pos.y - halfDims.y, pos.z + halfDims.z);
	NCLDebug::DrawThickLineNDT(leftDownForwardPos, leftDownForwardPos + Vector3(halfDims.x * 2.0f, 0.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(leftDownForwardPos, leftDownForwardPos + Vector3(0.0f, halfDims.y * 2.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(leftDownForwardPos, leftDownForwardPos - Vector3(0.0f, 0.0f, halfDims.z * 2.0f), 0.02f, color);

	Vector3 rightDownBackwardPos = Vector3(pos.x + halfDims.x, pos.y - halfDims.y, pos.z - halfDims.z);
	NCLDebug::DrawThickLineNDT(rightDownBackwardPos, rightDownBackwardPos - Vector3(halfDims.x * 2.0f, 0.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(rightDownBackwardPos, rightDownBackwardPos + Vector3(0.0f, halfDims.y * 2.0f, 0.0f), 0.02f, color);
	NCLDebug::DrawThickLineNDT(rightDownBackwardPos, rightDownBackwardPos + Vector3(0.0f, 0.0f, halfDims.z * 2.0f), 0.02f, color);
}




void NCLDebug::DrawTextCs(const Vector4& cs_pos, const float font_size, const std::string& text, const TextAlignment alignment, const Vector4 color)
{
	const Vector2 ss = Window::GetWindow().GetScreenSize();
	Vector3 cs_size = Vector3(font_size / ss.x, font_size / ss.y, 0.0f);
	cs_size = cs_size * cs_pos.w;

	//Work out the starting position of text based off desired alignment
	float x_offset = 0.0f;
	int text_len = (int)text.length();

	switch (alignment)
	{
	case TEXTALIGN_RIGHT:
		x_offset = -text_len * cs_size.x;
		break;

	case TEXTALIGN_CENTRE:
		x_offset = -text_len * cs_size.x * 0.5f;
		break;
	}



	//Add each characters to the draw list individually
	for (int i = 0; i < text_len; ++i)
	{
		Vector4 char_pos = Vector4(cs_pos.x + x_offset, cs_pos.y, cs_pos.z, cs_pos.w);
		Vector4 char_data = Vector4(cs_size.x, cs_size.y, (float)(text[i]), 0.0f);

		g_vChars.push_back(char_pos);
		g_vChars.push_back(char_data);
		g_vChars.push_back(color);
		g_vChars.push_back(color);	//We dont really need this, but we need the padding to match the same vertex format as all the other debug drawables

		x_offset += cs_size.x;
	}
}


//Draw Text WorldSpace
void NCLDebug::DrawTextWs(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 color, const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	std::string formatted_text = std::string(buf, (size_t)length);

	Vector4 cs_pos = g_ProjViewMtx * Vector4(pos.x, pos.y, pos.z, 1.0f);
	cs_pos = Vector4(cs_pos.x / cs_pos.w, cs_pos.y / cs_pos.w, -1.0f, 1.0f);
	DrawTextCs(cs_pos, font_size, formatted_text, alignment, color);
}

void NCLDebug::DrawTextWsNDT(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 color, const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	std::string formatted_text = std::string(buf, (size_t)length);

	Vector4 cs_pos = g_ProjViewMtx * Vector4(pos.x, pos.y, pos.z, 1.0f);
	cs_pos.z = -1.0f * cs_pos.w;
	DrawTextCs(cs_pos, font_size, formatted_text, alignment, color);
}


//Status Entry
void NCLDebug::AddStatusEntry(const Vector4& color, const std::string text, ...)
{
	if (g_StatusVisible)
	{
		const Vector2 ss = Window::GetWindow().GetScreenSize();
		float cs_size_x = STATUS_TEXT_SIZE / ss.x * 2.0f;
		float cs_size_y = STATUS_TEXT_SIZE / ss.y * 2.0f;

		va_list args;
		va_start(args, text);

		char buf[1024];
		int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
		va_end(args);

		int length = (needed < 0) ? 1024 : needed;

		std::string formatted_text = std::string(buf, (size_t)length);

		DrawTextCs(Vector4(-1.0f + cs_size_x * 0.5f, 1.0f - (g_NumStatusEntries * cs_size_y) - cs_size_y, -1.0f, 1.0f), STATUS_TEXT_SIZE, formatted_text, TEXTALIGN_LEFT, color);
		g_NumStatusEntries++;
		g_MaxStatusEntryWidth = max(g_MaxStatusEntryWidth, cs_size_x * 0.6f * length);
	}
}


//Log
void NCLDebug::AddLogEntry(const Vector3& color, const std::string& text)
{
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);

	time_t secondsSinceEpoch = time(NULL);
	tm brokenTime;
	string prepend;
	if (!localtime_s(&brokenTime, &secondsSinceEpoch))
	{
		char buf[80];
		strftime(buf, sizeof(buf), "[%T] ", &brokenTime);
		prepend = string(buf);
	}

	LogEntry le;
	le.text = prepend + text;
	le.color = Vector4(color.x, color.y, color.z, 1.0f);

	g_vLogEntries.push_back(le);	
	if (g_vLogOffsetIdx == int(g_vLogEntries.size()) - 2)
	{
		g_vLogOffsetIdx++;
	}

	std::cout << le.text << std::endl;

#ifdef LOG_OUTPUT_FILE_ENABLED
	if (!g_vOutLogFile)
	{
		if (fopen_s(&g_vOutLogFile, LOG_OUTPUT_FILE, "w"))
			g_vOutLogFile = NULL;
	}
	if (g_vOutLogFile)
	{
		fputs(le.text.c_str(), g_vOutLogFile);
		fputs("\r\n", g_vOutLogFile);
		fflush(g_vOutLogFile);
	}
#endif
}

void NCLDebug::Log(const Vector3& color, const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;
	AddLogEntry(color, std::string(buf, (size_t)length));
}

void NCLDebug::Log(const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;
	AddLogEntry(Vector3(0.4f, 1.0f, 0.6f), std::string(buf, (size_t)length));
}

void NCLDebug::LogE(const char* filename, int linenumber, const std::string text, ...)
{
	//Error Format:
	//<text>
	//		-> <line number> : <file name> 

	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	Log(Vector3(1.0f, 0.25f, 0.25f), "[ERROR] %s:%d", filename, linenumber);
	AddLogEntry(Vector3(1.0f, 0.5f, 0.5f), "\t \x01 \"" + std::string(buf, (size_t)length) + "\"");

	std::cout << std::endl;
}




void NCLDebug::_ClearDebugLists()
{
	g_vChars.clear();

	auto clear_list = [](DebugDrawList& list)
	{
		list._vPoints.clear();
		list._vThickLines.clear();
		list._vHairLines.clear();
		list._vTris.clear();
	};
	clear_list(g_DrawList[0]);
	clear_list(g_DrawList[1]);
	clear_list(g_DrawListNDT[0]);
	clear_list(g_DrawListNDT[1]);

	g_NumStatusEntries = 0;
	g_MaxStatusEntryWidth = 0.0f;
}

void NCLDebug::_ClearLog()
{
	g_vLogEntries.clear();
	g_vLogOffsetIdx = -1;
}

struct PointVertex
{
	Vector4 pos;
	Vector4 col;
};

struct LineVertex
{
	PointVertex p0;
	PointVertex p1;
};

struct TriVertex
{
	PointVertex p0;
	PointVertex p1;
	PointVertex p2;
};

void NCLDebug::_BuildRenderLists()
{
	//Sort the lists for transparent rendering
	_SortRenderLists();

	//Draw background to text areas
	// - This is done last as to avoid additional triangle-sorting
	_BuildTextBackgrounds();

	//Send all the data to the GPU, ready for rendering
	_BuildRenderVBO();
}

void NCLDebug::_SortRenderLists()
{
	auto sort_lists = [](DebugDrawList& list)
	{
		//Sort Points
		if (!list._vPoints.empty())
		{
			PointVertex* points = reinterpret_cast<PointVertex*>(&list._vPoints[0].x);
			std::sort(points, points + list._vPoints.size() / 2, [&](const PointVertex& a, const PointVertex& b)
			{
				float a2 = Vector3::Dot(a.pos.ToVector3() - g_CameraPosition, a.pos.ToVector3() - g_CameraPosition);
				float b2 = Vector3::Dot(b.pos.ToVector3() - g_CameraPosition, b.pos.ToVector3() - g_CameraPosition);
				return (a2 > b2);
			});
		}

		//Sort Lines
		if (!list._vThickLines.empty())
		{
			LineVertex* lines = reinterpret_cast<LineVertex*>(&list._vThickLines[0].x);
			std::sort(lines, lines + list._vThickLines.size() / 4, [](const LineVertex& a, const LineVertex& b)
			{
				return (a.p1.pos.w > b.p1.pos.w);
			});
		}

		//Sort Triangles
		if (!list._vTris.empty())
		{
			TriVertex* tris = reinterpret_cast<TriVertex*>(&list._vTris[0].x);
			std::sort(tris, tris + list._vTris.size() / 6, [](const TriVertex& a, const TriVertex& b)
			{
				return (a.p0.pos.w > b.p0.pos.w);
			});
		}
	};
	//Only sort transparent objects!!
	sort_lists(g_DrawList[0]);
	sort_lists(g_DrawListNDT[0]);
}

void NCLDebug::_BuildTextBackgrounds()
{
	const Vector2 ss = Window::GetWindow().GetScreenSize();
	float cs_size_x = LOG_TEXT_SIZE / ss.x * 2.0f;
	float cs_size_y = LOG_TEXT_SIZE / ss.y * 2.0f;

	Matrix4 invProjView = Matrix4::Inverse(g_ProjViewMtx);
	float rounded_offset_x = 10.f / ss.x * 2.0f;
	float rounded_offset_y = 10.f / ss.y * 2.0f;

	Vector2 cs_mouse = Vector2(-10.f, -10.f);
	if (Window::GetWindow().GetMouseScreenPos(&cs_mouse))
	{
		cs_mouse = Vector2(cs_mouse.x / ss.x * 2.0f - 1.0f, (1.f - cs_mouse.y / ss.y) * 2.0f - 1.0f);
	}

	Vector3 centre;
	Vector3 last;

	auto NextTri = [&](const Vector3& point, const Vector4& col)
	{
		DrawTriangle(centre, last, point, col);
		DrawHairLine(last, point);
		last = point;
	};

	//  A - B
	//  |   |
	//  C - D
	auto DrawBox_log = [&](Vector3 A, Vector3 B, Vector3 C, Vector3 D, Vector4 col)
	{
		centre = invProjView * C;
		last = invProjView * D;

		NextTri(invProjView * Vector3(B.x, B.y - rounded_offset_y, 0.0f), col);
		for (int i = 0; i < 5; ++i)
		{
			Vector3 round_offset = Vector3(
				cosf((float)DegToRad(i * 22.5f)) * rounded_offset_x,
				sinf((float)DegToRad(i * 22.5f)) * rounded_offset_y,
				0.0f);

			NextTri(invProjView * Vector3(B.x + round_offset.x - rounded_offset_x, B.y + round_offset.y - rounded_offset_y, 0.0f), col);
		}
		NextTri(invProjView * A, col);
	};
	

	auto DrawBox_status = [&](Vector3 A, Vector3 B, Vector3 C, Vector3 D, Vector4 col)
	{
		centre = invProjView * C;
		last = invProjView * D;

		NextTri(invProjView * Vector3(B.x - rounded_offset_x, B.y, 0.0f), col);
		for (int i = 0; i < 5; ++i)
		{
			Vector3 round_offset = Vector3(
				sinf((float)DegToRad(i * 22.5f)) * rounded_offset_x,
				cosf((float)DegToRad(i * 22.5f)) * rounded_offset_y,
				0.0f);

			NextTri(invProjView * Vector3(B.x + round_offset.x - rounded_offset_x, B.y - round_offset.y + rounded_offset_y, 0.0f), col);
		}
		NextTri(invProjView * A, col);
	};

	const float minimise_size = 0.05f;
	Vector4 log_background_col(0.1f, 0.1f, 0.1f, 0.5f);
	const Vector4 log_background_highlight(0.3f, 0.2f, 0.1f, 0.5f);


	if (g_StatusVisible)
	{
		//Draw Status Background
		if (g_NumStatusEntries > 0)
		{
			float cs_size_x = STATUS_TEXT_SIZE / ss.x * 2.0f;
			float cs_size_y = STATUS_TEXT_SIZE / ss.y * 2.0f;

			float btm_y = 1 - g_NumStatusEntries * cs_size_y - cs_size_y;
			float max_x = -1 + cs_size_x + g_MaxStatusEntryWidth;

			DrawBox_status(
				Vector3(max_x, 1, 0.0f),
				Vector3(max_x, btm_y, 0.0f),
				Vector3(-1, 1, 0),
				Vector3(-1, btm_y, 0),
				log_background_col
			);


			//Is MouseOver?
			Vector3 cs_size = Vector3(LOG_TEXT_SIZE / ss.x, LOG_TEXT_SIZE / ss.y, 0.0f) * 2.f;

			if (cs_mouse.x >= max_x - cs_size.x * 1.2 && cs_mouse.x <= max_x &&
				cs_mouse.y >= 1.f - cs_size.y * 1.2)
			{
				if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
					g_StatusVisible = false;

				DrawTextCs(Vector4(max_x, 1.f - cs_size.y, -1.f, 1.f), LOG_TEXT_SIZE, "X", TEXTALIGN_RIGHT, log_background_highlight);
			}
			else
			{
				DrawTextCs(Vector4(max_x, 1.f - cs_size.y, -1.f, 1.f), LOG_TEXT_SIZE, "X", TEXTALIGN_RIGHT);
			}

		}
	}
	else
	{
		Vector3 cs_size = Vector3(STATUS_TEXT_SIZE / ss.x * 5.f, STATUS_TEXT_SIZE / ss.y, 0.0f) * 2.f;

		Vector4 col = log_background_col;

		//Is MouseOver?
		if (cs_mouse.x >= -1.f && cs_mouse.x <= -1 + cs_size.x * 1.2 &&
			cs_mouse.y >= 1.f - cs_size.y * 1.2 && cs_mouse.y <= 1)
		{
			col = log_background_highlight;

			if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
				g_StatusVisible = true;
		}

		DrawBox_status(
			Vector3(-1.f + cs_size.x * 1.2f, 1, 0.0f),
			Vector3(-1.f + cs_size.x * 1.2f, 1.f - cs_size.y * 1.2f, 0.0f),
			Vector3(-1, 1, 0),
			Vector3(-1, 1.f - cs_size.y * 1.2f, 0),
			col
		);

		DrawTextCs(Vector4(-1.f + cs_size.x * 0.25f, 1.f - cs_size.y * 0.6f, -1.f, 1.f), STATUS_TEXT_SIZE, "Status");
	}

	g_vCharsLogStart = (uint)g_vChars.size();

	if (g_LogVisible)
	{
		//Draw log text
		size_t log_len = g_vLogEntries.size();

		float max_x = 0.0f;
		int i_min = max(g_vLogOffsetIdx - MAX_LOG_SIZE + 1, 0);

		float start_x = (g_vLogEntries.size() > MAX_LOG_SIZE) ? -0.99f : -1.0f;
		for (int i = g_vLogOffsetIdx; i >= i_min; --i)
		{
			int base_i = i - g_vLogOffsetIdx;
			LogEntry& e = g_vLogEntries[i];

			max_x = max(max_x, e.text.length() * cs_size_x * 0.6f);

			float alpha = ((g_vLogEntries.size() - base_i) / (float(MAX_LOG_SIZE)));
			alpha = 1.0f - (alpha * alpha);

			DrawTextCs(Vector4(start_x + cs_size_x * 0.5f, -1.0f + ((-base_i - 0.25f) * cs_size_y) + cs_size_y, 0.0f, 1.0f), LOG_TEXT_SIZE, e.text, TEXTALIGN_LEFT, e.color);
		}

		
		//Draw Log Background
		if (g_vLogEntries.size() > 0)
		{
			float top_y = -1 + min(g_vLogEntries.size(), MAX_LOG_SIZE) * cs_size_y + cs_size_y;
			max_x = 1.f;// max_x - 1 + cs_size_x;


			DrawBox_log(
				Vector3(-1, top_y, 0),
				Vector3(max_x, top_y, 0),
				Vector3(-1, -1, 0),
				Vector3(max_x, -1, 0),
				log_background_col
			);


			//Is MouseOver?
			Vector3 cs_size = Vector3(LOG_TEXT_SIZE / ss.x, LOG_TEXT_SIZE / ss.y, 0.0f) * 2.f;

			if (cs_mouse.x >= 1.f - cs_size.x * 1.2 &&
				cs_mouse.y >= top_y - cs_size.y * 2.f && cs_mouse.y <= top_y)
			{
				if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
					g_LogVisible = false;

				DrawTextCs(Vector4(max_x, top_y - cs_size.y, -1.f, 1.f), LOG_TEXT_SIZE, "X", TEXTALIGN_RIGHT, log_background_highlight);
			}
			else
			{
				DrawTextCs(Vector4(max_x, top_y - cs_size.y, -1.f, 1.f), LOG_TEXT_SIZE, "X", TEXTALIGN_RIGHT);
			}

			if (g_vLogEntries.size() > MAX_LOG_SIZE)
			{
				//Scroll bar
				DrawThickLine(
					invProjView* Vector3(-0.992f, top_y - 0.01f, -0.995f),
					invProjView* Vector3(-0.992f, -0.99f, -0.995f),
					0.0002f,
					Vector4(0.2f, 0.2f, 0.2f, 1.0f));

				float total_size = top_y + 1.f - 0.02f;
				float size = max(total_size * float(MAX_LOG_SIZE) / float(g_vLogEntries.size()), 0.03f);
				float offset = (g_vLogOffsetIdx - MAX_LOG_SIZE + 1) / float(g_vLogEntries.size() - MAX_LOG_SIZE) * (total_size - size);

				DrawThickLine(
					invProjView* Vector3(-0.992f, top_y - 0.01f - offset, -1.00f),
					invProjView* Vector3(-0.992f, top_y - 0.01f - offset - size, -1.00f),
					0.0002f);

				g_vLogOffsetIdx = min(max(g_vLogOffsetIdx - Window::GetMouse()->GetWheelMovement(), MAX_LOG_SIZE - 1), (int)g_vLogEntries.size() - 1);

				if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_HOME)) g_vLogOffsetIdx = -1;
				if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_END)) g_vLogOffsetIdx = (int)g_vLogEntries.size() - 1;
				if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_PRIOR)) g_vLogOffsetIdx = max(g_vLogOffsetIdx - MAX_LOG_SIZE, -1);
				if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NEXT)) g_vLogOffsetIdx = min(g_vLogOffsetIdx + MAX_LOG_SIZE, (int)g_vLogEntries.size() - 1);
			}
		}
	}
	else
	{
		Vector3 cs_size = Vector3(LOG_TEXT_SIZE / ss.x * 3.f, LOG_TEXT_SIZE / ss.y, 0.0f) * 2.f;

		Vector4 col = log_background_col;

		//Is MouseOver?
		if (cs_mouse.x >= -1.f && cs_mouse.x <= -1 + cs_size.x * 1.2 &&
			cs_mouse.y >= -1.f && cs_mouse.y <= -1 + cs_size.y * 1.2)
		{
			col = log_background_highlight;

			if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
				g_LogVisible = true;
		}

		DrawBox_log(
			Vector3(-1, -1 + cs_size.y * 1.2f, 0),
			Vector3(-1 + cs_size.x * 1.2f, -1 + cs_size.y * 1.2f, 0),
			Vector3(-1, -1, 0),
			Vector3(-1 + cs_size.x * 1.2f, -1, 0),
			col
		);

		DrawTextCs(Vector4(-1.f + cs_size.x * 0.25f, -1.f + cs_size.y * 0.5f, -1.f, 1.f), LOG_TEXT_SIZE, "Log");

	}

	
}

size_t GetDrawSize(const DebugDrawList& list)
{
	return list._vHairLines.size()
		+ list._vPoints.size()
		+ list._vThickLines.size()
		+ list._vTris.size();
}
void NCLDebug::_BuildRenderVBO()
{
	//Buffer all data into the single buffer object
	uint max_size = 0;
	uint offsets[17];

	//Compute offsets
	for (int i = 0; i < 2; ++i)
	{
		DebugDrawList& listO = (i == 0) ? g_DrawList[0] : g_DrawListNDT[0];
		DebugDrawList& listT = (i == 0) ? g_DrawList[1] : g_DrawListNDT[1];
		int i8 = i * 8;

		//Points
		offsets[i8 + 0] = max_size;		max_size += (uint)listO._vPoints.size();
		offsets[i8 + 1] = max_size;		max_size += (uint)listT._vPoints.size();

		//Thicklines
		offsets[i8 + 2] = max_size;		max_size += (uint)listO._vThickLines.size();
		offsets[i8 + 3] = max_size;		max_size += (uint)listT._vThickLines.size();

		//Hairlines
		offsets[i8 + 4] = max_size;		max_size += (uint)listO._vHairLines.size();
		offsets[i8 + 5] = max_size;		max_size += (uint)listT._vHairLines.size();

		//Triangles
		offsets[i8 + 6] = max_size;		max_size += (uint)listO._vTris.size();
		offsets[i8 + 7] = max_size;		max_size += (uint)listT._vTris.size();
	}

	//Text
	offsets[16] = max_size;
	max_size += (uint)g_vChars.size();

	//Cram offset data down into draw start, draw end 
	for (int i = 0; i < 8; ++i)
		g_glBufOffsets[i] = offsets[i*2];
	
	g_glBufOffsets[8] = offsets[16];


	glBindVertexArray(g_glArr);
	if (g_glBufCapacity < max_size)
	{
		if (g_glBuf) glDeleteBuffers(1, &g_glBuf);
		glGenBuffers(1, &g_glBuf);
		glBindBuffer(GL_ARRAY_BUFFER, g_glBuf);

		g_glBufCapacity = (uint)max_size;
		const size_t stride = 2 * sizeof(Vector4);


		const GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		//glBufferStorage(GL_ARRAY_BUFFER, max_size * sizeof(Vector4), NULL, flags);
		//g_glBufPtr = reinterpret_cast<Vector4*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, max_size * sizeof(Vector4), flags));
		glBufferData(GL_ARRAY_BUFFER, max_size * sizeof(Vector4), 0, GL_STREAM_DRAW);
		glVertexAttribPointer(VERTEX_BUFFER, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0));
		glEnableVertexAttribArray(VERTEX_BUFFER);
		glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vector4)));
		glEnableVertexAttribArray(COLOUR_BUFFER);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, g_glBuf);
	}

	//Upload data to buffer
	auto buffer_data = [](std::vector<Vector4>& arr, uint offset) -> void
	{
		if (!arr.empty())
			glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vector4), arr.size() * sizeof(Vector4), &arr[0]);
	};

	auto buffer_drawlist = [&](DebugDrawList* list, uint* offsets)
	{
		DebugDrawList& listO = list[0];
		DebugDrawList& listT = list[1];

		//Points
		buffer_data(listO._vPoints, offsets[0]);
		buffer_data(listT._vPoints, offsets[1]);

		//Thicklines
		buffer_data(listO._vThickLines, offsets[2]);
		buffer_data(listT._vThickLines, offsets[3]);

		//Hairlines
		buffer_data(listO._vHairLines, offsets[4]);
		buffer_data(listT._vHairLines, offsets[5]);

		//Triangles
		buffer_data(listO._vTris, offsets[6]);
		buffer_data(listT._vTris, offsets[7]);
	};

	buffer_drawlist(g_DrawList, &offsets[0]);
	buffer_drawlist(g_DrawListNDT, &offsets[8]);


	if (!g_vChars.empty())
		glBufferSubData(GL_ARRAY_BUFFER, offsets[16] * sizeof(Vector4), g_vChars.size() * sizeof(Vector4), &g_vChars[0]);
		//memcpy(g_glBufPtr + g_glBufOffsets[8], &g_vChars[0], g_vChars.size() * sizeof(Vector4));
}


void NCLDebug::_RenderDrawlist(uint* offsets)
{
	float aspectRatio = Window::GetWindow().GetScreenSize().y / Window::GetWindow().GetScreenSize().x;

	uint n_points = (offsets[1] - offsets[0]) >> 1;
	uint n_tlines = (offsets[2] - offsets[1]) >> 1;
	uint n_hlines = (offsets[3] - offsets[2]) >> 1;
	uint n_tris   = (offsets[4] - offsets[3]) >> 1;

	if (g_pShaderPoints && n_points > 0)
	{
		glUseProgram(g_pShaderPoints->GetProgram());
		glUniformMatrix4fv(glGetUniformLocation(g_pShaderPoints->GetProgram(), "uProjMtx"), 1, GL_FALSE, &g_ProjMtx.values[0]);
		glUniformMatrix4fv(glGetUniformLocation(g_pShaderPoints->GetProgram(), "uViewMtx"), 1, GL_FALSE, &g_ViewMtx.values[0]);

		glDrawArrays(GL_POINTS, offsets[0] >> 1, n_points);
	}

	if (g_pShaderLines && n_tlines > 0)
	{
		glUseProgram(g_pShaderLines->GetProgram());
		glUniformMatrix4fv(glGetUniformLocation(g_pShaderLines->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, &g_ProjViewMtx.values[0]);
		glUniform1f(glGetUniformLocation(g_pShaderLines->GetProgram(), "uAspect"), aspectRatio);

		glDrawArrays(GL_LINES, offsets[1] >> 1, n_tlines);
	}

	if (g_pShaderHairLines && (n_hlines + n_tris) > 0)
	{
		glUseProgram(g_pShaderHairLines->GetProgram());
		glUniformMatrix4fv(glGetUniformLocation(g_pShaderHairLines->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, &g_ProjViewMtx.values[0]);

		if (n_hlines) glDrawArrays(GL_LINES, offsets[2] >> 1, n_hlines);
		if (n_tris) glDrawArrays(GL_TRIANGLES, offsets[3] >> 1, n_tris);
	}
}
void NCLDebug::_RenderDebugDepthTested()
{
	if (!g_glArr)
	{
		return;
	}

	glBindVertexArray(g_glArr);
	_RenderDrawlist(&g_glBufOffsets[0]);
	glBindVertexArray(0);
}

void NCLDebug::_RenderDebugNonDepthTested()
{
	if (!g_glArr)
	{
		return;
	}

	glBindVertexArray(g_glArr);
	glDisable(GL_DEPTH_TEST);
	_RenderDrawlist(&g_glBufOffsets[4]);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

//This Function has been abstracted from previous set of draw calls as to avoid supersampling the font bitmap.. which is bad enough as it is.
void NCLDebug::_RenderDebugClipSpace()
{
	//All text data already updated in main DebugDrawLists
	// - we just need to rebind and draw it

	if (g_pShaderText && g_vChars.size() > 0)
	{
		glBindVertexArray(g_glArr);
		glUseProgram(g_pShaderText->GetProgram());
		glUniform1i(glGetUniformLocation(g_pShaderText->GetProgram(), "uFontTex"), 5);

		glActiveTexture(GL_TEXTURE5);
		
		glBindTexture(GL_TEXTURE_2D, g_glDefaultFontTex);
		glDrawArrays(GL_LINES, g_glBufOffsets[8] >> 1, g_vCharsLogStart >> 1);

		glBindTexture(GL_TEXTURE_2D, g_glLogFontTex);
		glDrawArrays(GL_LINES, (g_glBufOffsets[8] + g_vCharsLogStart) >> 1, ((uint)g_vChars.size()-g_vCharsLogStart) >> 1);
		
		glBindVertexArray(0);
	}
}

void NCLDebug::_LoadShaders()
{
	g_pShaderPoints = new Shader(
		SHADERDIR"DebugShaders/PointVertex.glsl",
		SHADERDIR"DebugShaders/PointFragment.glsl",
		SHADERDIR"DebugShaders/PointGeometry.glsl");
	if (!g_pShaderPoints->LinkProgram())
	{
		NCLERROR("NCLDebug Point shader could not be loaded");
		return;
	}
	
	g_pShaderLines = new Shader(
		SHADERDIR"DebugShaders/Vertex.glsl",
		SHADERDIR"DebugShaders/Fragment.glsl",
		SHADERDIR"DebugShaders/LineGeometry.glsl");
	if (!g_pShaderLines->LinkProgram())
	{
		NCLERROR("NCLDebug ThickLine shader could not be loaded");
		return;
	}

	g_pShaderHairLines = new Shader(
		SHADERDIR"DebugShaders/VertexColOnly.glsl",
		SHADERDIR"DebugShaders/Fragment.glsl");
	if (!g_pShaderHairLines->LinkProgram())
	{
		NCLERROR("NCLDebug HairLine shader could not be loaded");
		return;
	}

	g_pShaderText = new Shader(
		SHADERDIR"DebugShaders/TextVertex.glsl",
		SHADERDIR"DebugShaders/TextFragment.glsl",
		SHADERDIR"DebugShaders/TextGeometry.glsl");
	if (!g_pShaderText->LinkProgram())
	{
		NCLERROR("NCLDebug Text shader could not be loaded");
		return;
	}

	//Create Buffers
	glGenVertexArrays(1, &g_glArr);
	glGenBuffers(1, &g_glBuf);
	//g_glBufLock = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	//Load Font Texture
	g_glLogFontTex = _GenerateFontBitmap(LOG_TEXT_FONT, LOG_TEXT_SIZE, false, false, false);
	g_glDefaultFontTex = _GenerateFontBitmap(STATUS_TEXT_FONT, STATUS_TEXT_SIZE, false, false, false);
}


void NCLDebug::_ReleaseShaders()
{
	SAFE_DELETE(g_pShaderPoints);
	SAFE_DELETE(g_pShaderLines);
	SAFE_DELETE(g_pShaderHairLines);
	SAFE_DELETE(g_pShaderText);

	if (g_glLogFontTex)
	{
		glDeleteTextures(1, &g_glLogFontTex);
		g_glLogFontTex = NULL;
	}

	if (g_glDefaultFontTex)
	{
		glDeleteTextures(1, &g_glDefaultFontTex);
		g_glDefaultFontTex = NULL;
	}

	if (g_glArr)
	{
		glDeleteVertexArrays(1, &g_glArr);
		glDeleteBuffers(1, &g_glBuf);
		g_glArr = NULL;
		g_glBufCapacity = 0;
		g_glBufPtr = NULL;
	}

#ifdef LOG_OUTPUT_FILE_ENABLED
	if (g_vOutLogFile)
	{
		fclose(g_vOutLogFile);
		g_vOutLogFile = NULL;
	}
#endif
}


GLuint NCLDebug::_GenerateFontBitmap(const char* font_name, int font_size, bool bold, bool italic, bool underline)
{
	const int num_tiles = 16;

	int tex_size = font_size * num_tiles;
	int step = tex_size / num_tiles;
	HDC hdc = GetDC(NULL);
	HDC mdc = CreateCompatibleDC(hdc);
	HBITMAP bm = CreateCompatibleBitmap(hdc, tex_size, tex_size);
	HFONT hf = CreateFont(
		font_size, 0,                      // width, height
		0, 0, bold ? FW_BOLD : FW_NORMAL,  // escapement, orientation, weight
		italic,                            // italic
		underline, FALSE, DEFAULT_CHARSET, // underline, strikeout, charset
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH, font_name);
	RECT r = { 0, 0, tex_size, tex_size };
	

	SelectObject(mdc, bm);
	FillRect(mdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
	SelectObject(mdc, hf);
	SetBkMode(mdc, TRANSPARENT);
	SetTextColor(mdc, 0xFFFFFF);

	int i = 0;
	SIZE sz;
	for (int y = 0; y < num_tiles; y++) {
		for (int x = 0; x < num_tiles; x++, i++) {
			// 1. Glyph screen positions
			int scx = x * step;
			int scy = y * step;
			char glyph[2] = { (char)i, 0 };

			GetTextExtentPoint(mdc, glyph, 1, &sz);
			int offset_x = (font_size - sz.cx) / 2;
			TextOut(mdc, scx + offset_x, scy, glyph, 1);
		}
	}

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biWidth = tex_size;
	bmi.biHeight = -tex_size;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = tex_size * tex_size * 3;

	unsigned char* pixels = new unsigned char[bmi.biSizeImage * 2];
	GetDIBits(hdc, bm, 0, tex_size, pixels, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	DeleteObject(hf);
	DeleteObject(bm);
	DeleteObject(mdc);

	unsigned char* pixels_flipped = &pixels[bmi.biSizeImage];
	const unsigned int byte_width = tex_size * 3;
	for (int y = 0; y < tex_size; y++)
	{
		for (int x = 0; x < tex_size * 3; ++x)
		{
			pixels_flipped[y * byte_width + x] = pixels[(tex_size - y - 1) * byte_width + x];
		}
	}

	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_size, tex_size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_flipped);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	delete[] pixels;

	return texid;
}