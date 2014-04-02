#include <string>
#include <iostream>
#include <tuple>
#include <sstream>

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/utils.h"
#include "CommonDeclarations.h"

class Scene {
public:
	Scene()
		: isLeftButtonPressed(false),
		cameraPosition(0, 0, 5), cameraRight(1, 0, 0), cameraUp(0, 1, 0), cameraLook(0, 0, -1),
		projection(glm::perspective(45.0f, 1.0f, 1.0f, 100.0f)),
		floorY(-1)
	{
		UpdateCamera();
	}

	void Init()
	{
		// Blue background
		//glClearColor(100 / 255.f, 149 / 255.f, 237 / 255.f, 1.0f);
		glClearColor(0, 0, 0, 1);
		// Load shaders
		zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
		effect.Attach(vertex.CompileFile("fx/vertex.glsl"))
			.Attach(fragment.CompileFile("fx/fragment.glsl"))
			.Link();

		cubeTexture.LoadFromFile("textures/baku.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
		floorTexture.LoadFromFile("textures/floor.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT);

		worldUniform = effect.Uniform("mWorld");
		viewUniform = effect.Uniform("mView");
		projectionUniform = effect.Uniform("mProjection");
		lightPositionUniform = effect.Uniform("vLightPosition");
		lightColorUniform = effect.Uniform("vLightColor");
		ambientUniform = effect.Uniform("fAmbient");
		textureUniform = effect.Uniform("textureUnit0");
		
		//glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);
		glEnableVertexAttribArray(zf::VertexAttr::POSITION);
		glEnableVertexAttribArray(zf::VertexAttr::NORMAL);
		glEnableVertexAttribArray(zf::VertexAttr::TEXCOORD0);
	}

	void Render()
	{
		// Clear the window with current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		effect.Apply();
		
		glUniformMatrix4fv(viewUniform, 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(projectionUniform, 1, false, glm::value_ptr(projection));
		auto v = glm::vec3(2, 2, -2);
		glUniform3fv(lightPositionUniform, 1, glm::value_ptr(cameraPosition));
		glUniform4f(lightColorUniform, 1, 1, 1, 1);
		glUniform1i(textureUniform, 0);

		// draw objects mirror
		auto world = glm::translate(glm::scale(
			glm::translate(glm::mat4(), glm::vec3(0, floorY, 0)),
			glm::vec3(1, -1, 1)), glm::vec3(0, -floorY, 0));
		glUniformMatrix4fv(worldUniform, 1, false, glm::value_ptr(world));
		glUniform1f(ambientUniform, 0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture.ID());
		RenderObjects();

		// draw floor
		float size = 100;
		world = glm::mat4(
			size, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, size, 0,
			0, floorY, 0, 1);
		glUniformMatrix4fv(worldUniform, 1, false, glm::value_ptr(world));
		glUniform1f(ambientUniform, 1);
		glBindTexture(GL_TEXTURE_2D, floorTexture.ID());
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
		glBlendColor(0, 0, 0, .7f);
		RenderFloor();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		world = glm::mat4();
		glUniformMatrix4fv(worldUniform, 1, false, glm::value_ptr(world));
		glUniform1f(ambientUniform, 0.1f);
		glBindTexture(GL_TEXTURE_2D, cubeTexture.ID());
		RenderObjects();

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

	void RenderFloor()
	{
		glm::vec3 points[] = {
			glm::vec3(-1, 0, -1),  // 0----1   o--> x
			glm::vec3( 1, 0, -1),  // |    |   |
			glm::vec3( 1, 0,  1),  // |    |   v
			glm::vec3(-1, 0,  1),  // 3----2   z
		};
		auto up = glm::vec3(0, 1, 0);
		glm::vec3 normals[] = { up, up, up, up };
		float repeatCount = 30;
		float texCoords[] = {
			0, repeatCount,
			repeatCount, repeatCount,
			repeatCount, 0,
			0, 0,
		};
		GLubyte indices[] = {
			0, 3, 1,
			3, 2, 1,
		};

		glVertexAttribPointer(zf::VertexAttr::POSITION, 3, GL_FLOAT, false, 0, points);
		glVertexAttribPointer(zf::VertexAttr::NORMAL, 3, GL_FLOAT, false, 0, normals);
		glVertexAttribPointer(zf::VertexAttr::TEXCOORD0, 2, GL_FLOAT, false, 0, texCoords);
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_BYTE, indices);
	}

	void RenderObjects()
	{
		glBindTexture(GL_TEXTURE_2D, cubeTexture.ID());

		glm::vec3 p[] = {
			glm::vec3(-.5f,  .5f,  .5f),  //   1----2
			glm::vec3(-.5f,  .5f, -.5f),  //  /:   /|
			glm::vec3( .5f,  .5f, -.5f),  // 0----3 |
			glm::vec3( .5f,  .5f,  .5f),  // | 5..|.6
			glm::vec3(-.5f, -.5f,  .5f),  // |.   |/
			glm::vec3(-.5f, -.5f, -.5f),  // 4----7
			glm::vec3( .5f, -.5f, -.5f),
			glm::vec3( .5f, -.5f,  .5f),
		};
		glm::vec3 points[] = {
			p[0], p[2], p[1],  //    0 5
			p[0], p[3], p[2],  //    |/
			p[4], p[6], p[7],  // 2--o--3
			p[4], p[5], p[6],  //   /|
			p[4], p[1], p[5],  //  4 1
			p[4], p[0], p[1], 
			p[7], p[2], p[3], 
			p[7], p[6], p[2], 
			p[4], p[3], p[0], 
			p[4], p[7], p[3], 
			p[5], p[2], p[6], 
			p[5], p[1], p[2],
		};
		glm::vec3 n[] = {
			glm::vec3( 0,  1,  0),
			glm::vec3( 0, -1,  0),
			glm::vec3(-1,  0,  0),
			glm::vec3( 1,  0,  0),
			glm::vec3( 0,  0,  1),
			glm::vec3( 0,  0, -1),
		};
		glm::vec3 normals[] = {
			n[0], n[0], n[0], n[0], n[0], n[0],
			n[1], n[1], n[1], n[1], n[1], n[1],
			n[2], n[2], n[2], n[2], n[2], n[2],
			n[3], n[3], n[3], n[3], n[3], n[3],
			n[4], n[4], n[4], n[4], n[4], n[4],
			n[5], n[5], n[5], n[5], n[5], n[5],
		};
		float texCoords[] = {
			0, .75f, .5f, 1, 0, 1,
			0, .75f, .5f, .75f, .5f, 1,
			.5f, 1, 1, .75f, 1, 1,
			.5f, 1, .5f, .75f, 1, .75f,
			.5f, .5f, 0, .75f, 0, .5f,
			.5f, .5f, .5f, .75f, 0, .75f,
			.5f, .5f, 1, .75f, .5f, .75f,
			.5f, .5f, 1, .5f, 1, .75f,
			0, .25f, .5f, .5f, 0, .5f,
			0, .25f, .5f, .25f, .5f, .5f,
			1, .25f, .5f, .5f, .5f, .25f,
			1, .25f, 1, .5f, .5f, .5f,
		};

		glVertexAttribPointer(zf::VertexAttr::POSITION, 3, GL_FLOAT, false, 0, points);
		glVertexAttribPointer(zf::VertexAttr::NORMAL, 3, GL_FLOAT, false, 0, normals);
		glVertexAttribPointer(zf::VertexAttr::TEXCOORD0, 2, GL_FLOAT, false, 0, texCoords);
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
	}

	void UpdateCamera()
	{
		auto position = cameraPosition;
		position.y = std::max(0.0f, position.y);
		view = glm::lookAt(position, position + cameraLook, cameraUp);
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
		float value = (direction > 0 ? 1 : -1);
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
	zf::Texture2D cubeTexture, floorTexture;
	GLint worldUniform, viewUniform, projectionUniform;
	GLint lightPositionUniform, lightColorUniform, ambientUniform;
	GLint textureUniform;

	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPosition;
	glm::vec3 cameraRight, cameraUp, cameraLook;

	float floorY;

	bool isLeftButtonPressed;
	bool isRightButtonPressed;
	glm::vec2 mousePosition;
};

Scene scene;

void onError(std::string message)
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
		onError(ex.what());
		return 3;
	} catch (std::runtime_error &err) {
		onError(err.what());
		return 4;
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
	glutMouseWheelFunc([](int button, int direction, int x, int y){
		scene.OnMouseWheel(button, direction);
	});

	glutMainLoop();
	return 0;
}
