#include <string>
#include <iostream>
#include <tuple>
#include <sstream>
#include <algorithm>

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/utils.h"

static std::tuple<GLenum, std::string> blendModes[] = {
	std::make_tuple(GL_ZERO,			    "0"),
	std::make_tuple(GL_ONE,				    "1"),
	std::make_tuple(GL_SRC_ALPHA,			"SrcAlpha"),
	std::make_tuple(GL_ONE_MINUS_SRC_ALPHA, "1 - SrcAlpha"),
	std::make_tuple(GL_DST_ALPHA,			"DstAlpha"),
	std::make_tuple(GL_ONE_MINUS_DST_ALPHA, "1 - DstAlpha"),
};

static std::tuple<GLenum, std::string> alphaModes[] = {
	std::make_tuple(GL_ALWAYS,   "always"),
    std::make_tuple(GL_LESS,     "<"),
    std::make_tuple(GL_EQUAL,    "=="),
    std::make_tuple(GL_LEQUAL,   "<="),
    std::make_tuple(GL_GREATER,  ">"),
    std::make_tuple(GL_NOTEQUAL, "!="),
    std::make_tuple(GL_GEQUAL,   ">="),
	std::make_tuple(GL_NEVER,    "never"),
};

namespace VertexAttr {
	enum VertexAttr {
		POSITION = 0,
		COLOR = 1,
	};
}

class Scene {
public:
	Scene()
		: isLeftButtonPressed(false),
		cameraPosition(0, 0, 2), cameraRight(1, 0, 0), cameraUp(0, 1, 0), cameraLook(0, 0, -1),
		projection(glm::perspective(45.0f, 1.0f, 1.0f, 10.0f)),
		srcMode(2), dstMode(3), alphaMode(0), alphaRef(0.5),
		isScissorEnabled(false), scissorX(200), scissorY(200), scissorWidth(300), scissorHeight(300)
	{
		UpdateCamera();
	}

	void Init()
	{
		// Blue background
		glClearColor(100 / 255.f, 149 / 255.f, 237 / 255.f, 1.0f);
		// Load shaders
		zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
		effect.Attach(vertex.CompileFile("fx/vertex.glsl"))
			.Attach(fragment.CompileFile("fx/fragment.glsl"))
			.Link();

		viewUniform = effect.Uniform("mView");
		projectionUniform = effect.Uniform("mProjection");
		
		//glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);
		glEnableVertexAttribArray(VertexAttr::POSITION);
		glEnableVertexAttribArray(VertexAttr::COLOR);
	}

	void Render()
	{
		// Clear the window with current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(std::get<0>(alphaModes[alphaMode]), alphaRef);
		glBlendFunc(std::get<0>(blendModes[srcMode]), std::get<0>(blendModes[dstMode]));

		if (isScissorEnabled) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(
				static_cast<GLint>(scissorX), static_cast<GLint>(scissorY),
				static_cast<GLsizei>(scissorWidth), static_cast<GLsizei>(scissorHeight));
		}

		effect.Apply();

		glUniformMatrix4fv(viewUniform, 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(projectionUniform, 1, false, glm::value_ptr(projection));

		float points[] = {
			-.5f,  .5f,  .5f,
			-.5f,  .5f, -.5f,
			 .5f,  .5f, -.5f,
			 .5f,  .5f,  .5f,
			-.5f, -.5f,  .5f,
			-.5f, -.5f, -.5f,
			 .5f, -.5f, -.5f,
			 .5f, -.5f,  .5f,
		};
		GLuint indices[] = {
			0, 2, 1,
			0, 3, 2,
			4, 6, 7, 
			4, 5, 6, 
			4, 1, 5, 
			4, 0, 1, 
			7, 2, 3, 
			7, 6, 2, 
			4, 3, 0, 
			4, 7, 3, 
			5, 2, 6, 
			5, 1, 2, 
		};
		float colors[] = {
			1, 1, 1, 0.4f,
			1, 0, 0, 0.5f,
			1, 0, 1, 0.5f,
			0, 0, 1, 0.5f,
			0, 1, 0, 0.5f,
			1, 1, 0, 0.5f,
			0, 0, 0, 0.6f,
			0, 1, 1, 0.5f,
		};
		
		glVertexAttribPointer(VertexAttr::POSITION, 3, GL_FLOAT, false, 0, points);
		glVertexAttribPointer(VertexAttr::COLOR, 4, GL_FLOAT, false, 0, colors);
		glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, indices);

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		glUseProgram(0);
		glRasterPos2f(-0.85f, 0.85f);

		std::ostringstream os;
		os << std::string("blend: (s, w, d, e) [") << std::get<1>(blendModes[srcMode]) << ", " << std::get<1>(blendModes[dstMode]) << "]"
		   << "\n" "alpha: (a, q, f, r) " << std::get<1>(alphaModes[alphaMode]) << " " << std::to_string(alphaRef)
		   << "\n" "scissor: (z) " << (isScissorEnabled ? "enabled" : "disabled")
		   << "\n" "camera position: (" << zf::ToString(cameraPosition) << ")"
		   << "\n" "camera look: (" << zf::ToString(cameraLook) << ")";
		std::string info = os.str();
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>(info.c_str()));

		glutSwapBuffers();
	}

	void UpdateCamera()
	{
		view = glm::lookAt(cameraPosition, cameraPosition + cameraLook, cameraUp);
	}

	void UpdateScissor()
	{
		scissorX = mousePosition.x - scissorWidth * 0.5f;
		scissorY = mousePosition.y - scissorHeight * 0.5f;
	}

	void OnMouseAction(int button, int state, int x, int y)
	{
		mousePosition = glm::vec2(x, y);
		if (button == GLUT_LEFT_BUTTON) {
			isLeftButtonPressed = state == GLUT_DOWN;
		} else if (button == GLUT_RIGHT_BUTTON) {
			isRightButtonPressed = state == GLUT_DOWN;
			UpdateScissor();
			glutPostRedisplay();
		}
	}

	void OnMouseMove(int isActive, int x, int y)
	{
		if (isActive) {
			if (isLeftButtonPressed) {
				auto newMousePosition = glm::vec2(x, y);
				auto diff = newMousePosition - mousePosition;
				mousePosition = newMousePosition;

				glm::mat4 rotation = glm::rotate(glm::mat4(), diff.y * .005f, cameraRight);
				cameraUp = (rotation * glm::vec4(cameraUp, 1)).xyz;
				cameraLook = (rotation * glm::vec4(cameraLook, 1)).xyz;
				cameraPosition = (rotation * glm::vec4(cameraPosition, 1)).xyz;

				rotation = glm::rotate(glm::mat4(), -diff.x * .005f, glm::vec3(0, 1, 0));
				cameraRight = (rotation * glm::vec4(cameraRight, 1)).xyz;
				cameraUp = (rotation * glm::vec4(cameraUp, 1)).xyz;
				cameraLook = (rotation * glm::vec4(cameraLook, 1)).xyz;
				cameraPosition = (rotation * glm::vec4(cameraPosition, 1)).xyz;

				UpdateCamera();
				glutPostRedisplay();
			} else if (isRightButtonPressed) {
				mousePosition = glm::vec2(x, y);
				UpdateScissor();
				glutPostRedisplay();
			}
		}
	}

	void OnKeyPress(unsigned char key, int x, int y)
	{
		const int BlendModesCount = 6;
		const int AlphaModesCount = 8;
		if (key == 's') {
			srcMode = (srcMode + 1) % BlendModesCount;
		} else if (key == 'w') {
			srcMode--;
			if (srcMode < 0) { srcMode = BlendModesCount - 1; }
		} else if (key == 'd') {
			dstMode = (dstMode + 1) % BlendModesCount;
		} else if (key == 'e') {
			dstMode--;
			if (dstMode < 0) { dstMode = BlendModesCount - 1; }
		} else if (key == 'a') {
			alphaMode = (alphaMode + 1) % AlphaModesCount;
		} else if (key == 'q') {
			alphaMode--;
			if (alphaMode < 0) { alphaMode = AlphaModesCount - 1; }
		} else if (key == 'f') {
			alphaRef = std::min(alphaRef + .1f, 1.0f);
		} else if (key == 'r') {
			alphaRef = std::max(alphaRef - .1f, 0.0f);
		} else if (key == 'z') {
			isScissorEnabled = !isScissorEnabled;
		}
		glutPostRedisplay();
	}

	void OnSpecialKeyPress(int key, int x, int y)
	{
		//const float MovementVelocity = 0.05f;
		//if (key == GLUT_KEY_LEFT) {
		//	cameraPosition -= cameraRight * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_RIGHT) {
		//	cameraPosition += cameraRight * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_UP) {
		//	cameraPosition += cameraLook * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_DOWN) {
		//	cameraPosition -= cameraLook * MovementVelocity;
		//	UpdateCamera();
		//}

		glutPostRedisplay();
	}

private:
	zf::Effect effect;
	GLint viewUniform;
	GLint projectionUniform;

	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPosition;
	glm::vec3 cameraRight, cameraUp, cameraLook;

	bool isLeftButtonPressed;
	bool isRightButtonPressed;
	glm::vec2 mousePosition;

	int srcMode, dstMode, alphaMode;
	float alphaRef;
	bool isScissorEnabled;
	float scissorX, scissorY, scissorWidth, scissorHeight;
};

Scene scene;

void onError(std::string message)
{
	std::cerr << message << std::endl;
	std::cout << "Press <Enter> to exit...";
	std::string s;
	std::getline(std::cin, s);
}

int main(int argc, char *argv[])
{
	glutInitWarningFunc([](const char *fmt, va_list ap){
		onError("GLUT warning: " + zf::vformat(fmt, ap));
	});
	glutInitErrorFunc([](const char *fmt, va_list ap){
		onError("GLUT error: " + zf::vformat(fmt, ap));
	});

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	
	glutCreateWindow("Lab 2 -- OpenGL");
	glutReshapeFunc([](int w, int h){
		glViewport(0, 0, w, h);
	});

	GLenum glewInitResult = glewInit();
	if (glewInitResult != GLEW_OK) {
		onError(std::string("GLEW error: ") + reinterpret_cast<const char *>(glewGetErrorString(glewInitResult)));
		return 1;
	}
	if (!GLEW_VERSION_3_3) {
		onError("Error: OpenGL 3.3 required to run this application.");
		return 2;
	}

	try {
		scene.Init();
	} catch (zf::ShaderLoadException &ex) {
		onError(ex.what());
		return 3;
	}

	glutDisplayFunc([](){
		scene.Render();
	});
	glutKeyboardFunc([](unsigned char key, int x, int y){
		scene.OnKeyPress(key, x, y);
	});
	glutSpecialFunc([](int key, int x, int y){
		scene.OnSpecialKeyPress(key, x, y);
	});
	glutMouseFunc([](int button, int state, int x, int y){
		scene.OnMouseAction(button, state, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});
	glutMotionFunc([](int x, int y){
		scene.OnMouseMove(true, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});
	glutPassiveMotionFunc([](int x, int y){
		scene.OnMouseMove(false, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});

	glutMainLoop();
	return 0;
}
