#pragma once

#include "Module.h"
#include "SDL3/SDL.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <climits>
#include <vector>

class Render : public Module
{
public:

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	Render();

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();
	void BeginScene(const glm::mat4& view, const glm::mat4& projection);
	void SetObject(const glm::mat4& model, GLuint texture, bool vertexColor = false);
    void DrawLines(const std::vector<glm::vec3>& points, const glm::vec3& color);
    GLuint GetCheckerTexture() const { return checkerTexture; }

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

public:

	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;

private:
	GLuint shaderProgram = 0;
    GLuint checkerTexture = 0;
    GLuint lineVAO = 0, lineVBO = 0;
	GLint modelLocation = -1;
	GLint viewLocation = -1;
	GLint projectionLocation = -1;
	GLint texturedLocation = -1;
	GLint vertexColorLocation = -1;
	bool vsync = false;
};
