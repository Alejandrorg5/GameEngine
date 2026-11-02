#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{
}

// Called before render is available
namespace {
GLuint CompileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string message(length > 0 ? length : 1, '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(message.size()), nullptr, &message[0]);
        LOG("Shader compilation failed: %s", message.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
}

bool Render::Awake() {
    const char* vertexSource = R"glsl(#version 330 core
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 vertexColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useVertexColor;
out vec2 texCoord;
out vec3 color;
void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    texCoord = uv;
    color = useVertexColor ? vertexColor : vec3(1.0);
}
)glsl";
    const char* fragmentSource = R"glsl(#version 330 core
in vec2 texCoord;
in vec3 color;
uniform sampler2D diffuseTexture;
uniform bool textured;
out vec4 fragmentColor;
void main() {
    fragmentColor = textured ? texture(diffuseTexture, texCoord) * vec4(color, 1.0)
                             : vec4(color, 1.0);
}
)glsl";
    const GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertex) return false;
    const GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragment) {
        glDeleteShader(vertex);
        return false;
    }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint success = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
        std::string message(length > 0 ? length : 1, '\0');
        glGetProgramInfoLog(shaderProgram, static_cast<GLsizei>(message.size()), nullptr, &message[0]);
        LOG("Shader link failed: %s", message.c_str());
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        return false;
    }
    modelLocation = glGetUniformLocation(shaderProgram, "model");
    viewLocation = glGetUniformLocation(shaderProgram, "view");
    projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    texturedLocation = glGetUniformLocation(shaderProgram, "textured");
    vertexColorLocation = glGetUniformLocation(shaderProgram, "useVertexColor");
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 0);
    glUseProgram(0);
    unsigned char pixels[64 * 64 * 4];
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            const unsigned char value = ((x / 8 + y / 8) % 2) ? 255 : 0;
            const int offset = (y * 64 + x) * 4;
            pixels[offset] = pixels[offset + 1] = pixels[offset + 2] = value;
            pixels[offset + 3] = 255;
        }
    }
    glGenTextures(1, &checkerTexture);
    glBindTexture(GL_TEXTURE_2D, checkerTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    LOG("OpenGL default shader compiled and linked successfully");
    return true;
}

void Render::BeginScene(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

void Render::SetObject(const glm::mat4& model, GLuint texture, bool vertexColor) {
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(texturedLocation, texture != 0);
    glUniform1i(vertexColorLocation, vertexColor);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Render::DrawLines(const std::vector<glm::vec3>& points, const glm::vec3& color) {
    if (points.empty()) return;
    SetObject(glm::mat4(1.0f), 0, true);
    glVertexAttrib3f(3, color.x, color.y, color.z);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	//// back background
	//if (!SDL_GetRenderViewport(renderer, &viewport))
	//{
	//	LOG("SDL_GetRenderViewport failed: %s", SDL_GetError());
	//}
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	//SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	//SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	//SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	glUseProgram(0);
	if (shaderProgram) glDeleteProgram(shaderProgram);
	shaderProgram = 0;
    if (checkerTexture) glDeleteTextures(1, &checkerTexture);
    if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
    if (lineVBO) glDeleteBuffers(1, &lineVBO);
    checkerTexture = lineVAO = lineVBO = 0;
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_SetRenderViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_SetRenderViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	// SDL3 uses float rects for rendering
	SDL_FRect rect;
	rect.x = (float)((int)(camera.x * speed) + x * scale);
	rect.y = (float)((int)(camera.y * speed) + y * scale);

	if (section != NULL)
	{
		rect.w = (float)(section->w * scale);
		rect.h = (float)(section->h * scale);
	}
	else
	{
		float tw = 0.0f, th = 0.0f;
		if (!SDL_GetTextureSize(texture, &tw, &th))
		{
			LOG("SDL_GetTextureSize failed: %s", SDL_GetError());
			return false;
		}
		rect.w = tw * scale;
		rect.h = th * scale;
	}

	const SDL_FRect* src = NULL;
	SDL_FRect srcRect;
	if (section != NULL)
	{
		srcRect.x = (float)section->x;
		srcRect.y = (float)section->y;
		srcRect.w = (float)section->w;
		srcRect.h = (float)section->h;
		src = &srcRect;
	}

	SDL_FPoint* p = NULL;
	SDL_FPoint pivot;
	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = (float)pivotX;
		pivot.y = (float)pivotY;
		p = &pivot;
	}

	// SDL3: returns bool; map to int-style check
	int rc = SDL_RenderTextureRotated(renderer, texture, src, &rect, angle, p, SDL_FLIP_NONE) ? 0 : -1;
	if (rc != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderTextureRotated error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FRect rec;
	if (use_camera)
	{
		rec.x = (float)((int)(camera.x + rect.x * scale));
		rec.y = (float)((int)(camera.y + rect.y * scale));
		rec.w = (float)(rect.w * scale);
		rec.h = (float)(rect.h * scale);
	}
	else
	{
		rec.x = (float)(rect.x * scale);
		rec.y = (float)(rect.y * scale);
		rec.w = (float)(rect.w * scale);
		rec.h = (float)(rect.h * scale);
	}

	int result = (filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderRect(renderer, &rec)) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect/SDL_RenderRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	float X1, Y1, X2, Y2;

	if (use_camera)
	{
		X1 = (float)(camera.x + x1 * scale);
		Y1 = (float)(camera.y + y1 * scale);
		X2 = (float)(camera.x + x2 * scale);
		Y2 = (float)(camera.y + y2 * scale);
	}
	else
	{
		X1 = (float)(x1 * scale);
		Y1 = (float)(y1 * scale);
		X2 = (float)(x2 * scale);
		Y2 = (float)(y2 * scale);
	}

	int result = SDL_RenderLine(renderer, X1, Y1, X2, Y2) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderLine error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_FPoint points[360];

	float factor = (float)M_PI / 180.0f;

	float cx = (float)((use_camera ? camera.x : 0) + x * scale);
	float cy = (float)((use_camera ? camera.y : 0) + y * scale);

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = cx + (float)(radius * cos(i * factor));
		points[i].y = cy + (float)(radius * sin(i * factor));
	}

	result = SDL_RenderPoints(renderer, points, 360) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderPoints error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}