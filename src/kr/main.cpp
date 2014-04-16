#include <string>
#include <iostream>
#include <tuple>
#include <sstream>

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/utils.h"
#include "../common/CommonDeclarations.h"

class Scene {
public:
	Scene()
		: isLeftButtonPressed(false),
		cameraPosition(0, 0, 5), cameraRight(1, 0, 0), cameraUp(0, 1, 0), cameraLook(0, 0, -1)
	{
		UpdateCamera();
	}

	void Init()
	{
		// Black background
		glClearColor(0, 0, 0, 1);
		
		glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);
	}

	void Render()
	{
		// Clear the window with current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//TODO: draw something

		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);
		glRasterPos2f(-0.85f, 0.85f);

		std::ostringstream os;
		os << "camera position: (" << zf::ToString(cameraPosition) << ")"
		   << "\n" "camera look: (" << zf::ToString(cameraLook) << ")";
		std::string info = os.str();
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>(info.c_str()));

		glutSwapBuffers();
	}

	void UpdateCamera()
	{
		auto position = cameraPosition;
		position.y = std::max(0.0f, position.y);
		view = glm::lookAt(position, position + cameraLook, cameraUp);
	}

	void UpdateProjection(int viewportWidth, int viewportHeight)
	{
		float aspectRatio = 1.0f * viewportWidth / viewportHeight;
		projection = glm::perspective(45.0f, aspectRatio, 1.0f, 500.0f);
	}

	void OnMouseAction(int button, int state, int x, int y)
	{
		mousePosition = glm::vec2(x, y);
		if (button == GLUT_LEFT_BUTTON) {
			isLeftButtonPressed = state == GLUT_DOWN;
		} else if (button == GLUT_RIGHT_BUTTON) {
			isRightButtonPressed = state == GLUT_DOWN;
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
				glutPostRedisplay();
			}
		}
	}

	void OnMouseWheel(int button, int direction)
	{
		float value = (direction > 0 ? 1.0f : -1.0f);
		cameraPosition += cameraLook * value;
		UpdateCamera();
		glutPostRedisplay();
	}

	void OnKeyPress(unsigned char key, int x, int y)
	{
		glutPostRedisplay();
	}

	void OnSpecialKeyPress(int key, int x, int y)
	{
		glutPostRedisplay();
	}

private:
	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPosition;
	glm::vec3 cameraRight, cameraUp, cameraLook;

	bool isLeftButtonPressed;
	bool isRightButtonPressed;
	glm::vec2 mousePosition;
};

Scene scene;

void onError(const std::string &message)
{
	std::cerr << message << std::endl;
	std::cout << "Press <Enter> to exit...";
	std::string s;
	std::getline(std::cin, s);
}

bool isRequirementsFullfiled() {
	bool isSuccess = false;
	if (!GLEW_VERSION_3_3)
		onError("Error: OpenGL 3.3 required to run this application.");
	else if (!GLEW_ARB_multitexture)
		onError("Error: missing required ARB_multitexture extension.");
	else if (!GLEW_EXT_texture_env_combine)
		onError("Error: missing required EXT_texture_env_combine extension.");
	else
		isSuccess = true;
	return isSuccess;
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
	glutInitWindowSize(800, 800);
	
	glutCreateWindow("Lab 2 -- OpenGL");
	glutReshapeFunc([](int w, int h){
		glViewport(0, 0, w, h);
	});

	GLenum glewInitResult = glewInit();
	if (glewInitResult != GLEW_OK) {
		onError(std::string("GLEW error: ") + reinterpret_cast<const char *>(glewGetErrorString(glewInitResult)));
		return 1;
	}
	
	if (!isRequirementsFullfiled())
		return 2;

	try {
		scene.Init();
	} catch (zf::ShaderLoadException &ex) {
		std::string message = ex.ShaderPath().length() > 0
			? (ex.ShaderPath() + ": ") : "<in-memory shader>: ";
		onError(message + ex.what());
		return 3;
	} catch (std::runtime_error &err) {
		onError(err.what());
		return 4;
	}

	glutReshapeFunc([](int width, int height){
		glViewport(0, 0, width, height);
		scene.UpdateProjection(width, height);
	});
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
	glutMouseWheelFunc([](int button, int direction, int x, int y){
		scene.OnMouseWheel(button, direction);
	});

	int menu = glutCreateMenu([](int selectedItem){
		//if (selectedItem == 1)
		//	scene.ToggleIsBumpEnabled();
	});
	//glutAddMenuEntry("Toggle Bump Mapping", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();
	return 0;
}
